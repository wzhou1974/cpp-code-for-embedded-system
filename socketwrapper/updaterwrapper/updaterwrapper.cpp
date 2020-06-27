#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>
#include <boost/assert.hpp>
#include <boost/smart_ptr/make_unique.hpp>
#include "json/nlohmann/json.hpp"
#include "updaterwrapper/inc/updaterwrapper.h"
#include "ppk_assert.h"
#include "common/inc/tinyformat.h"
#include "common/inc/utils.h"
#include "interface/fota.h"
#include "common/inc/https_download.h"
#include "rpc_server.h"
#include "fmt/format.h"
#include "prettyprint.hpp"

#define QNX_PACKAGES_PATH   "/shared_partition" // for IDCM's self-installer

using namespace rest_rpc::rpc_service;
namespace boostfs = boost::filesystem;

namespace socketwrapper {

std::shared_ptr<spdlog::logger> UpdaterWrapper::GetLogger(const std::string& log_file)
{
#ifdef PC_SIMULATION
    auto console = spdlog::stdout_color_mt("console");
    return spdlog::get("console");
#else
    return spdlog::rotating_logger_mt("self-updater", log_file, 10 * 1024 * 1024, 5);
#endif
}

UpdaterWrapper::UpdaterWrapper(NodeId node,
                               const std::shared_ptr<ISelfUpdater>& updater)
 : node_(node), updater_(updater), rpc_server_(nullptr),
   saved_path_(updater->GetPackageStorageLocation())
{
    std::string error_info{saved_path_.string()};

    if (node == NodeId::eIDCM) {
        if (saved_path_ != QNX_PACKAGES_PATH) {
            error_info += ": hope ";
            error_info += QNX_PACKAGES_PATH;
            throw std::invalid_argument(error_info);
        }
    } else {
        if (!saved_path_.is_absolute()) {
            error_info += ": isn't absolute path!";
            throw std::invalid_argument(error_info);
        }
            
        if (!boostfs::exists(saved_path_)) {
            error_info += ": doesn't exist!";
            throw std::invalid_argument(error_info);
        }

        if (!boostfs::is_directory(saved_path_)) {
            error_info += ": is not a directory";
            throw std::invalid_argument(error_info);
        }

        // check the directory is readable and writable
        boostfs::file_status status = boostfs::status(saved_path_);
        boostfs::perms owner_rw = boostfs::owner_read | boostfs::owner_write;

        if ((status.permissions() & owner_rw) != owner_rw) {
            boostfs::perms group_rw = boostfs::group_read | boostfs::group_write;

            if ((status.permissions() & group_rw) != group_rw) {
                error_info += ": self-updater couldn't read / write in it";
                throw std::invalid_argument(error_info);
            }
        }
    }
}
	
bool UpdaterWrapper::Init()
{
    uint16_t rpc_port{SELFUPDATER_RPC_PORT};
    std::string log_file{SELFUPDATER_LOG};

    bool succ{false};

    logger_ = GetLogger(log_file);
    if (logger_) {
        logger_->set_level(spdlog::level::trace);

        SPDLOG_LOGGER_INFO(logger_, "Packages will be saved in {}", saved_path_.string());
        SPDLOG_LOGGER_INFO(logger_, "selfupdater init [{}]: {} - {}", NodeToECUName(node_), rpc_port, log_file);
        SPDLOG_LOGGER_INFO(logger_, "RPC service init");

        if (!IsPortInUse(rpc_port)) {
            if (succ = InitRpc(rpc_port)) {
                SPDLOG_LOGGER_INFO(logger_, "RPC service init: done");
            } else {
                SPDLOG_LOGGER_CRITICAL(logger_, "RPC service init: failed");
            }
        } else {
            SPDLOG_LOGGER_CRITICAL(logger_, "port {} is in use !", rpc_port);
        }

        logger_->flush();
        spdlog::flush_every(std::chrono::seconds(3));
    }

    return succ;
}
	
void UpdaterWrapper::Deinit(UpdaterLeaveCause cause)
{
    switch (cause) {
    case UpdaterLeaveCause::eABSwitch:
        SPDLOG_LOGGER_INFO(logger_, "AB Swich exit");
        break;
    case UpdaterLeaveCause::eExit:
        SPDLOG_LOGGER_INFO(logger_, "Self-Installer exit");
        break;
    }

    rpc_server_->publish("Exit", cause);
    logger_->flush();
}

void UpdaterWrapper::ReportStatus(const UpdateProgressState_t& state)
{
    ReportState(state);
}

bool UpdaterWrapper::InitRpc(uint16_t rpc_port)
{
    auto rpc_server_job = [this, rpc_port] () {
        rpc_server_ = boost::make_unique<rpc_server>(rpc_port, std::thread::hardware_concurrency());

        rpc_server_->register_handler("PrepareUpgrade", &UpdaterWrapper::PrepareUpgrade, this);
        rpc_server_->register_handler("Upgrade", &UpdaterWrapper::Upgrade, this);
        rpc_server_->register_handler("Finalize", &UpdaterWrapper::Finalize, this);
        rpc_server_->register_handler("Rollback", &UpdaterWrapper::Rollback, this);
        rpc_server_->register_handler("Distribute", &UpdaterWrapper::Distribute, this);
        rpc_server_->register_handler("GetState", &UpdaterWrapper::GetState, this);
        rpc_server_->register_handler("GetVersion", &UpdaterWrapper::GetVersion, this);
        rpc_server_->register_handler("GetAllSoftwareVersion", &UpdaterWrapper::GetAllSoftwareVersion, this);
        rpc_server_->register_handler("GetErrorCause", &UpdaterWrapper::GetErrorCause, this);

        SPDLOG_LOGGER_INFO(logger_, "rpc service is ready");
        rpc_server_->run();

        SPDLOG_LOGGER_CRITICAL(logger_, "rpc service thread exit ?!");
        logger_->flush();
    };

    std::thread rpc_server_thread(rpc_server_job);

    thread_id_ = rpc_server_thread.get_id();
    rpc_server_thread.detach();

    SPDLOG_LOGGER_INFO(logger_, "{} OK", __func__);
    return  true;
}

SELFINSTALLER_CODE UpdaterWrapper::PrepareUpgrade(rpc_conn conn, const Packages& pkgs)
{
    Log("PrepareUpgrade", pkgs);
    return updater_->PrepareUpgrade(pkgs.manifests_);
}

SELFINSTALLER_CODE UpdaterWrapper::Upgrade(rpc_conn conn, const Packages& pkgs)
{
    Log("Upgrade", pkgs);
    return updater_->Upgrade(pkgs.manifests_);
}

SELFINSTALLER_CODE UpdaterWrapper::Finalize(rpc_conn conn, const Packages& pkgs)
{
    Log("Finalize", pkgs);
    return updater_->Finalize(pkgs.manifests_);
}

SELFINSTALLER_CODE UpdaterWrapper::Rollback(rpc_conn conn)
{   
    auto error = updater_->Rollback();
    if (error == SELFINSTALLER_OK) {
        SPDLOG_LOGGER_INFO(logger_, "Rollback successfully");
    } else {        
        SPDLOG_LOGGER_ERROR(logger_, "Rollback: [{}] {}", error, updater_->GetErrorCause(error));
    }
    
    return error;
}

GetAllVersionsResult UpdaterWrapper::GetAllSoftwareVersion(rpc_conn conn)
{    
    GetAllVersionsResult result;
    result.error_ = updater_->GetAllSoftwareVersion(result.versions_);
    Log(result);
    return  result;
}

GetVersionResult UpdaterWrapper::GetVersion(rpc_conn conn, const std::string& software_id)
{
    GetVersionResult result;
    result.error_ = updater_->GetVersion(software_id, result.version_);
    if (result.error_ == SELFINSTALLER_OK) {
        SPDLOG_LOGGER_INFO(logger_, "GetVersion {} {}", software_id, result.version_);
    } else {
        SPDLOG_LOGGER_ERROR(logger_, "GetVersion: [{}] {}", result.error_,
                            updater_->GetErrorCause(result.error_));
    }
    return result;
}

GetStateResult UpdaterWrapper::GetState(rpc_conn conn)
{
    GetStateResult result;
    result.error_ = updater_->GetState(result.state_);
    if (result.error_ == SELFINSTALLER_OK) {
        Log("GetState", result.state_);
    } else {
        SPDLOG_LOGGER_ERROR(logger_, "GetState: [{}] {}", result.error_,
                            updater_->GetErrorCause(result.error_));
    }
    
    return  result;
}

void UpdaterWrapper::ReportState(const UpdateProgressState_t& state)
{
    Log("ReportState", state);
    rpc_server_->publish("ReportStatus", state);
}

std::string UpdaterWrapper::GetErrorCause(rpc_conn conn, SELFINSTALLER_CODE error)
{
    return updater_->GetErrorCause(error);
}

bool UpdaterWrapper::Distribute(rpc_conn conn, const DownloadFile& info)
{   
    SPDLOG_LOGGER_INFO(logger_, "Distribute {}", info.uri_);

    boostfs::path downloaded_file = saved_path_ / boostfs::path{info.uri_}.filename();

    // special case for IDCM box
    if (node_ == NodeId::eIDCM) {
        //
        // The self-installer of IDCM needn't download by network, the package has 
        // already been there.
        // The "/shared" directory in Android == the "/shared_partition" in QNX
        //
        assert(boostfs::exists(downloaded_file));

        std::string log = downloaded_file.string() + " is OK";
        this->StatusCallback(info.uri_, DownloadStatus::eDONE, DownloadError::eOK,
                            100, log);
        return true;
    }

    std::thread download_work_thread([this, info, downloaded_file] () {
        loff_t  from{0};

        if (boostfs::exists(downloaded_file)) {
            size_t current_size = boostfs::file_size(downloaded_file);
            if (current_size == info.size_) {
                if (md5sum(downloaded_file.string()) == info.checksum_) {
                    std::string log = fmt::format("{} [{}] exist and is good", info.uri_, downloaded_file.string());
                    SPDLOG_LOGGER_INFO(logger_, log);

                    this->StatusCallback(info.uri_, DownloadStatus::eDONE, DownloadError::eOK,
                                        100, log);
                    return;
                }
            }

            if (current_size > info.size_) {
                // bad package, remove it

                SPDLOG_LOGGER_ERROR(logger_, "Bad package, {} size: {}, {} size: {}", info.uri_,
                                    info.size_, downloaded_file.string(), current_size);

                boost::system::error_code error;
                auto succ = boostfs::remove(downloaded_file, error);
                assert(succ);

            } else {
                from = current_size;
            }
        }

        SPDLOG_LOGGER_INFO(logger_, "{}: Start download it from {}", info.uri_, from);

        HttpsDownload downloader(info.uri_, info.size_, downloaded_file);

        PROGRESS reporter = [this, info] (size_t percent) {             
            SPDLOG_LOGGER_INFO(logger_, "Downloading {}: {}%", info.uri_, percent);
            this->StatusCallback(info.uri_, DownloadStatus::ePROGRESS, DownloadError::eOK,
                                percent, "");
        };

        bool succ{false};
        loff_t  current{from};

        // if downloading is interrupted, we will try MAX_REPEAT_COUNT times
        // before give up 
        static constexpr size_t MAX_REPEAT_COUNT = 10; 
        size_t repeat_count{0};

        do {
            succ = downloader.DownloadFrom(current, reporter);
            repeat_count++;
            if (!succ) {
                current = boostfs::file_size(downloaded_file);
                SPDLOG_LOGGER_WARN(logger_, "{} interrupted [{}]: {}", info.uri_,
                                repeat_count, current);
            }                
        } while (!succ && repeat_count < MAX_REPEAT_COUNT);

        fmt::memory_buffer buf;

        if (succ) {
            fmt::format_to(buf, "{} download successfully [{}, {}]", info.uri_,
                        boostfs::file_size(downloaded_file), repeat_count);

            this->StatusCallback(info.uri_, DownloadStatus::eDONE, DownloadError::eOK,
                                100, fmt::to_string(buf));
        } else {
            fmt::format_to(buf, "{} download failed [{}, {}]", info.uri_,
                        boostfs::file_size(downloaded_file), repeat_count);

            this->StatusCallback(info.uri_, DownloadStatus::eFAIL, DownloadError::eREPEAT,
                                0, fmt::to_string(buf));
        }
    });
    
    download_work_thread.detach();

    SPDLOG_LOGGER_INFO(logger_, "Start handle {}", info.uri_);
    return true;
}

void UpdaterWrapper::StatusCallback(const std::string& uri,
                                    DownloadStatus status,
                                    DownloadError error,
                                    int percent,
                                    const std::string& log)
{
    BOOST_ASSERT(!uri.empty());
    BOOST_ASSERT(percent >= 0 && percent <= 100);

    if (status == DownloadStatus::ePROGRESS) {
        return;
    }

    DownloadResult  result;
    result.uri_ = uri;
    result.status_ = status;
    result.error_ = error;
    
    fmt::memory_buffer buf;
    fmt::format_to(buf, "Downloading: {} {} {}", uri, static_cast<int>(status),
                static_cast<int>(error));
    SPDLOG_LOGGER_INFO(logger_, fmt::to_string(buf));

    rpc_server_->publish("DistributeResult", result);
}

void UpdaterWrapper::Log(const std::string& operation, const Packages& pkgs)
{
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{} {} manifests: ", operation, pkgs.manifests_.size());
    fmt::format_to(buf, "{} ", "{");

    for (auto& manifest : pkgs.manifests_) {
        fmt::format_to(buf, "{}", "[");
        fmt::format_to(buf, "{}, {}, {}, {}, {}, {}, {}, ", manifest.softwre_id,
                    manifest.filename, manifest.version, manifest.delta,
                    manifest.original_version, static_cast<int>(manifest.type),
                    static_cast<int>(manifest.flashing));

        std::stringstream ss;
        ss << manifest.attrs;
        fmt::format_to(buf, "{}", ss.str());
        fmt::format_to(buf, "{} ", "]");
    }

    fmt::format_to(buf, "{}", "}");

    SPDLOG_LOGGER_INFO(logger_, fmt::to_string(buf));
    logger_->flush();
}

void UpdaterWrapper::Log(const std::string& operation, const UpdateProgressState_t& state)
{
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}: {} {} {} {}", operation, static_cast<int>(state.state_),
                state.package_, state.percent_, state.log_);
    SPDLOG_LOGGER_INFO(logger_, fmt::to_string(buf));
    logger_->flush();
}

void UpdaterWrapper::Log(const GetAllVersionsResult& result)
{
    if (result.error_ == SELFINSTALLER_OK) {
        fmt::memory_buffer buf;
        fmt::format_to(buf, "GetAllVersions: ");
        std::stringstream ss;
        ss << result.versions_;
        fmt::format_to(buf, "{}", ss.str());

        SPDLOG_LOGGER_INFO(logger_, fmt::to_string(buf));
    } else {
        SPDLOG_LOGGER_ERROR(logger_, "GetAllVersions: [{}] {}", result.error_, 
                            updater_->GetErrorCause(result.error_));
    }

    logger_->flush();
}

}   // namespace socketwrapper

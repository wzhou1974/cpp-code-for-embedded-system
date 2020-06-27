#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>
#include "boost/filesystem.hpp"
#include "json/nlohmann/json.hpp"
#include "interface/idcm-log.h"
#include "common/inc/utils.h"
#include "cinatra.hpp"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

using namespace cinatra;
namespace boostfs = boost::filesystem;

#define HTTPS_SERVER_CONF       "/etc/https-server.json"

int main(int argc, char *argv[])
{
    std::string https_conf;

    if (argc > 1) {
        https_conf = argv[1];
    } else {
        https_conf = HTTPS_SERVER_CONF;
    }

    if (!boostfs::exists(https_conf)) {
        LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "%s not exist !", https_conf.c_str());
        return -1;
    } 

    std::string cert_file, key_file;
    std::string https_port{"443"};
    std::string static_files_dir{"/shared/fota"};
    std::string upload_files_dir{"upload"};
    std::string log_file{"/var/fota-https-server.log"};
    
    // read conf file
    std::ifstream conf_file(https_conf);

    try {
        nlohmann::json json;
        conf_file >> json;
        assert(json.contains("cert-file"));
        assert(json.contains("key-file"));

        assert(json["cert-file"].is_string());
        cert_file = json["cert-file"];
        assert(json["key-file"].is_string());
        key_file = json["key-file"];

        if (json.contains("https-port")) {
            assert(json["https-port"].is_number_unsigned());
            https_port = std::to_string(static_cast<unsigned>(json["https-port"]));
        }

        if (json.contains("static-files-dir")) {
            assert(json["static-files-dir"].is_string());
            static_files_dir = json["static-files-dir"];
        }

        if (json.contains("upload-files-dir")) {
            assert(json["upload-files-dir"].is_string());
            upload_files_dir = json["upload-files-dir"];
        }

        if (json.contains("log-file")) {
            assert(json["log-file"].is_string());
            log_file = json["log-file"];
        }
    } catch (nlohmann::json::exception& e) {
        std::string json_content((std::istreambuf_iterator<char>(conf_file)),
                        std::istreambuf_iterator<char>());

        LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "Invalid json format: %s - %s", e.what(),
                json_content.c_str());

        return  -2;
    }

    conf_file.close();

    std::shared_ptr<spdlog::logger> logger = spdlog::rotating_logger_mt("fota-https-server", log_file, 10 * 1024 * 1024, 5);
    assert(logger);
    logger->set_level(spdlog::level::trace);

    if (!boostfs::exists(cert_file)) {
        LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "%s not exist", cert_file.c_str());
        SPDLOG_LOGGER_CRITICAL(logger, "{} not exist", cert_file);
        return  -3;
    }

    if (!boostfs::exists(key_file)) {
        LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "%s not exist", key_file.c_str());
        SPDLOG_LOGGER_CRITICAL(logger, "{} not exist", key_file);
        return  -4;
    }

    if (socketwrapper::IsPortInUse(static_cast<unsigned short>(std::stoi(https_port)))) {
        LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "port %s is in use !", https_port.c_str());
        SPDLOG_LOGGER_CRITICAL(logger, "port {} is in use", https_port);
        return  -5;
    }
 
    http_ssl_server server(std::thread::hardware_concurrency());
    server.set_ssl_conf({ cert_file, key_file });
    int r = server.listen("0.0.0.0", https_port);
    
    if (r < 0) {
        LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "fail to listen port: %s", https_port.c_str());
        SPDLOG_LOGGER_CRITICAL(logger, "fail to listen port: {}", https_port);
        return	-5;
    } else {
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "listen port: %s successfully", https_port.c_str());
        SPDLOG_LOGGER_INFO(logger, "listen port: {} successfully", https_port);
    }

    server.set_static_dir(static_files_dir);

    std::string upload_directory = static_files_dir + std::string("/") + upload_files_dir;
    server.set_upload_dir(upload_directory);

    // for http upload(multipart)
    server.set_http_handler<GET, POST>("/upload_multipart", [&logger] (request& req, response& res) {
        assert(req.get_content_type() == content_type::multipart);

        auto& files = req.get_upload_files();
        assert(files.size() == 1);

        auto original_filename = req.get_header_value("filename");
        
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "upload %s [%zu]", std::string{original_filename}.c_str(),
                files[0].get_file_size());

        SPDLOG_LOGGER_INFO(logger, "upload {} [{}]", std::string{original_filename}, files[0].get_file_size());

        boostfs::path current_file{files[0].get_file_path()};
        boostfs::path original_file{std::string{original_filename}};

        boostfs::path upload_file_path = current_file.parent_path() / original_file;

        boostfs::rename(current_file, upload_file_path);

        std::string content = std::string("upload ") + upload_file_path.string() + std::string(" finished");
        res.set_status_and_content(status_type::ok, content.c_str());

        logger->flush();
    });

    // for getting static file size
    server.set_http_handler<HEAD, GET>("/file_size", [&static_files_dir, &logger] (request& req, response& res) {
        auto filename = req.get_header_value("filename");
        if (filename.empty()) {
            res.set_status_and_content(status_type::ok, "-1");
            LOG_PRINT(IDCM_LOG_LEVEL_WARN, "There is no \"filename\" header");
            SPDLOG_LOGGER_WARN(logger, "There is no \"filename\" header");
            logger->flush();
        } else {
            boost::system::error_code code;
            std::string full_path = static_files_dir + std::string{"/"} + std::string{filename};
            boostfs::path file_full_path{full_path};
            if (boostfs::exists(file_full_path)) {

                auto file_size = boostfs::file_size(file_full_path, code);
                if (code) {
                    LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "Get %s size error !", full_path.c_str());
                    SPDLOG_LOGGER_ERROR(logger, "Get {} size error !", full_path);
                    res.set_status_and_content(status_type::ok, "-1");spdlog::flush_every(std::chrono::seconds(3));spdlog::flush_every(std::chrono::seconds(3));
                } else {
                    LOG_PRINT(IDCM_LOG_LEVEL_INFO, "%s size: %zu", full_path.c_str(), file_size); 
                    SPDLOG_LOGGER_INFO(logger, "{} size: {}", full_path, file_size);
                    res.set_status_and_content(status_type::ok, std::to_string(file_size));
                }
            } else {
                LOG_PRINT(IDCM_LOG_LEVEL_WARN, "%s: no this file on https server !", full_path.c_str());
                SPDLOG_LOGGER_WARN(logger, "{}: no this file on https server !", full_path);
                res.set_status_and_content(status_type::ok, "-1");
            }
            logger->flush();
        }
    });

    LOG_PRINT(IDCM_LOG_LEVEL_INFO, "https server is working ...");
    SPDLOG_LOGGER_INFO(logger, "https server is working ...");

    logger->flush();
    spdlog::flush_every(std::chrono::seconds(2));
    server.run();

	return	0;
}
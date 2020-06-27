#include <string>
#include <thread>
#include <functional>
#include <chrono>
#include "json/nlohmann/json.hpp"
#include "idcm/dlc/inc/downloadclient.h"
#include "idcm/dlc/inc/error.h"
#include "rest_rpc/include/rpc_server.h"
#include "interface/idcm-log.h"
#include "inc/L1Manifest.h"
#include "common/inc/utils.h"
#include "interface/error.h"
#include "interface/fota.h"
#include "interface/hh.h"
#include <rpc_client.hpp>
#include "codec.h"

using namespace socketwrapper;
using namespace rest_rpc;
using namespace rpc_service;

// DLC server
static rpc_server  *server{nullptr};

// orchestrator client
static rpc_client client_orchestrator;

// The L1 manifest is handling currently
static std::string L1_Manifest;

static void HandlePackages(const std::vector<Package>& pkgs)
{   
    LOG_PRINT(IDCM_LOG_LEVEL_INFO, "Start to download packages");
    std::string start_download = GetCurrentTime();

    DlError             dl_error;
    DownloadPackages    info;
    bool success = DownloadAllPackages(pkgs, info, dl_error);
    std::string finsih_download = GetCurrentTime();

    if (success) {
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "All packages are download successfully !");
        dl_error.code_ = ERROR_OK;

        success = client_orchestrator.call<bool>("PackageReady", info);
        assert(success);        
    } else {
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "fail to handle %s [%u]", dl_error.url_.c_str(), dl_error.code_);

        // send error status report to DMC
        DLCStatusReport reporter(L1_Manifest, dl_error);
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "Send error report: %s", reporter.dump().c_str());
        server->publish("cloud", reporter.dump());
    }
}

static bool HandleRemoteRequest(rpc_conn conn, const std::string& json)
{
    DlError error;

    try {
        auto request = nlohmann::json::parse(json);

        // handle L1 Manifest file
        if (request.contains("manifest")) {
            LOG_PRINT(IDCM_LOG_LEVEL_INFO, "handle L1 Manifest request");

            L1Manifest  L1{request};
            if (L1.IsWellFormated()) {
                auto packages = L1.GetDownloadPackages();
                std::thread worker(HandlePackages, packages);
                worker.detach();

                L1_Manifest = json;
            } else {
                LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "Invalid L1 Manifest format: %s", json.c_str());

                error.code_ = DLC_ERROR_L1_WELLFORMATED;
                DLCStatusReport reporter(json, error);
                server->publish("cloud", reporter.dump());
            }
        }

        // other request


    } catch (nlohmann::json::exception& e) {
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "Invalid json format: %s", e.what());
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "========================================");
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s", json.c_str());
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "========================================");

        error.code_ = DLC_ERROR_L1_INVALID;
        
        // send error status report to DMC
        DLCStatusReport reporter(json, error);

        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "Send error report: %s", reporter.dump().c_str());
        server->publish("cloud", reporter.dump());
    }
    
    return  true;
}

static bool GetUpgradeInfo(const std::string& L1,
                        std::string& service_pack_id,
                        std::vector<ReleaseNoteInternal>& release_notes)
{
    return  true;
}

static UpgradeInfo CheckNewPackage(rpc_conn conn)
{
    UpgradeInfo info;

    std::string L1;
    L1 = client_orchestrator.call<std::string>("CheckNewPackage");
    if (L1.empty()) {
        info.status = false;
    } else {
        assert(L1 == L1_Manifest);

        info.status = true;
        GetUpgradeInfo(L1_Manifest, info.service_pack_id, info.release_notes);
    }

    return  info;
}

static std::string GetServicePackId(const std::string& L1)
{
    return  std::string();
}

static bool StartUpgrade (rpc_conn conn, const std::string& service_pack_id)
{
    assert(GetServicePackId(L1_Manifest) == service_pack_id);
    return client_orchestrator.call<bool>("StartUpgrade", L1_Manifest);
}

static bool InitRPCClient()
{
    static constexpr size_t MAX_REPEAT_TRY = 10;

    size_t count{MAX_REPEAT_TRY};
    bool connected{false};

    // connect to dlc proxy in CGW box, take 10 seconds to wait orchestraor [in CGW] to be ready
    while (!(connected = client_orchestrator.connect(NodeToIp(NodeId::eCGW), DLC_PROXY_PORT)) && count >= 0) {
        // wait orchestrator to startup
        std::this_thread::sleep_for(std::chrono::seconds(1));
        count--;
    }

    if (connected) {
        client_orchestrator.enable_auto_reconnect();
        client_orchestrator.enable_auto_heartbeat();

        client_orchestrator.subscribe("DownloadFinished", [] (std::string_view notification) {
            std::string msg {notification.data(), notification.size()};

            LOG_PRINT(IDCM_LOG_LEVEL_INFO, "orchestrator send DownloadFinished notification");
            // to-do    
        });

        client_orchestrator.subscribe("UpdateResult", [] (std::string_view notification) {
            std::string msg {notification.data(), notification.size()};

            // forward "UpgradeResult" message to DMC
            if (server != nullptr)
                server->publish("UpdateResult", msg);
        });

        client_orchestrator.subscribe("UpdateStatus", [] (std::string_view notification) {
            std::string msg {notification.data(), notification.size()};

            // forward "UpgradeStatus" message to DMC
            if (server != nullptr)
                server->publish("UpdateStatus", msg);
        });

        return  true;
    } else {
        // no orchestrator, FOTA could not work
        return  false;
    }
}

int main(int argc, char *argv[])
{
    LOG_PRINT(IDCM_LOG_LEVEL_INFO, "start DLC service !");

    if (IsPortInUse(DLC_SERVER_PORT)) {
        LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "port %u is in use !", DLC_SERVER_PORT);
        return  -1; 
    }

    // connect to orchestrator
    if (InitRPCClient()) {
        server = new (std::nothrow) rpc_server(DLC_SERVER_PORT, std::thread::hardware_concurrency());
        assert(server != nullptr);
        auto cleaner = finally([&] {delete server;});

        // for cloud interface
        server->register_handler("toDLC", HandleRemoteRequest);

        // for hmi interface
        server->register_handler("CheckNewPackage", CheckNewPackage);
        server->register_handler("StartUpgrade", StartUpgrade);    

        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "DLC is in service !");
        server->run();

        LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "DLC exit !!!");
    } else {
        LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "connect to orchestrator failed !");
        return  -2;
    }

    return  0;
}


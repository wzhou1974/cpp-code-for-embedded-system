#include <string>
#include <vector>
#include <iostream>
#include <utility>
#include <thread>
#include <cassert>
#include <boost/make_unique.hpp>
#include <cassert>
#include "common/inc/utils.h"
#include "json/nlohmann/json.hpp"
#include "rest_rpc/include/rpc_server.h"
#include "interface/idcm-log.h"
#include "interface/fota.h"
#include "interface/rpc.h"
#include "interface/msg-passer-public.h"

using namespace socketwrapper;
using namespace rest_rpc;
using namespace rpc_service;

// DLC server
static std::string L1_Manifest;

static bool GetUpgradeInfo(const std::string& L1,
                        std::string& service_pack_id,
                        std::vector<ReleaseNoteInternal>& release_notes)
{
    assert(service_pack_id.empty());
    assert(release_notes.empty());

    service_pack_id = "fake-package";
    ReleaseNoteInternal note = {
        .locale = "en",
        .description = "fake release"
    };
    release_notes.push_back(note);

    return  true;
}

int main(int argc, char *argv[])
{
    LOG_PRINT(IDCM_LOG_LEVEL_INFO, "start fake DLC service !");

    rpc_server fake_server(DLC_SERVER_PORT, std::thread::hardware_concurrency());

    // for cloud interface
    fake_server.register_handler("toDLC", [] (rpc_conn conn, const std::string& json) -> bool {
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "%s - %s", __func__, json.c_str());
        L1_Manifest = json;
        return  true;        
    });

    // for hmi interface
    fake_server.register_handler("CheckNewPackage", [&fake_server] (rpc_conn conn) -> UpgradeInfo {
        static constexpr size_t REPEAT_INTERVAL = 10;

        static size_t count{1};
        UpgradeInfo info;

        if (count % REPEAT_INTERVAL != 0) {
            LOG_PRINT(IDCM_LOG_LEVEL_INFO, "%s - %s", __func__, "No new package to upgrade");
            info.status = false;
        } else {
            info.status = true;
            LOG_PRINT(IDCM_LOG_LEVEL_INFO, "%s - %s", __func__, "There is a new package to upgrade");
            GetUpgradeInfo(L1_Manifest, info.service_pack_id, info.release_notes);

            std::thread upgrader([&fake_server] () {

                UpgradeStatusInternal status = {
                    .dev_id = "aaa",
                    .soft_id = "bbb",
                    .esti_time = GetCurrentTime(),
                    .start_time = GetCurrentTime(),
                    .time_stamp = GetCurrentTime(),
                    .door_module = "yes",
                    .status = "in progress",
                    .progress_percent = 30
                };

                for (int i = 0; i < 5; i++) {                
                    status.esti_time = GetCurrentTime();
                    status.start_time = GetCurrentTime();
                    status.time_stamp = GetCurrentTime();
                    status.progress_percent = i * 20;

                    LOG_PRINT(IDCM_LOG_LEVEL_INFO, "%s", "before publish upgrade status");
                    fake_server.publish("UpdateStatus", status);
                    LOG_PRINT(IDCM_LOG_LEVEL_INFO, "%s", "after publish upgrade status");
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }

                LOG_PRINT(IDCM_LOG_LEVEL_INFO, "exit %s", __func__);
            });

            upgrader.detach();            
        }

        ++count;
        return  info;
    });

    fake_server.register_handler("StartUpgrade", [] (rpc_conn conn, const std::string& service_pack_id) -> bool {
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "%s - %s", __func__, service_pack_id.c_str());
        return  true;
    });    

    LOG_PRINT(IDCM_LOG_LEVEL_INFO, "fake DLC is in service [%u] !", DLC_SERVER_PORT);
    fake_server.run();

    LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "fake-dlc server exit !");
    return  0;
}
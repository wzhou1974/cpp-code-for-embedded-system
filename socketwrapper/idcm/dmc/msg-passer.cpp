#include <functional>
#include <thread>
#include <chrono>
#include <assert.h>
#include "idcm/dmc/inc/msg-passer.h"
#include "interface/fota.h"
#include "interface/idcm-log.h"
#include "rpc_client.hpp"

namespace socketwrapper {

bool MessagePasser::Init(const std::shared_ptr<IVehicleNotification>& notifier)
{
    assert(notifier);
    notifier_ = notifier;
    return  InitRPCClient();
}

bool MessagePasser::ToVehicle(const std::string& message)
{
    return client_.call<bool>("toDLC", message);
}

bool MessagePasser::CheckNewPackage(std::string& service_pack_id,
                                    std::vector<ReleaseNote>& release_notes)
{
    assert(service_pack_id.empty());
    assert(release_notes.empty());

    UpgradeInfo info;

    info = client_.call<UpgradeInfo>("CheckNewPackage");
    if (info.status) {
        assert(!info.service_pack_id.empty());
        assert(info.release_notes.size() > 0);

        service_pack_id = info.service_pack_id;
        
        for (auto& internal : info.release_notes) {
            ReleaseNote note = {
                .locale = internal.locale,
                .description = internal.description
            };

            release_notes.push_back(note);
        }
        
        return  true;
    } else {
        return  false;
    }
}

bool MessagePasser::StartUpgrade(const std::string& service_pack_id)
{
    return client_.call<bool>("StartUpgrade", service_pack_id);
}

bool MessagePasser::InitRPCClient()
{
    static constexpr size_t MAX_REPEAT_TRY = 10;

    int count{MAX_REPEAT_TRY};
    bool connected{false};

    while (!(connected = client_.connect("127.0.0.1", DLC_SERVER_PORT)) && count >= 0) {
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "fail to connect DLC server: %u", DLC_SERVER_PORT);
        // wait DLC server startup
        std::this_thread::sleep_for(std::chrono::seconds(1));
        count--;
    }

	client_.enable_auto_reconnect();
	client_.enable_auto_heartbeat();

    if (connected) {
        // subscribe the notification from DLC
        client_.subscribe("UpdateResult", [this] (string_view notification) {
            std::string msg {notification.data(), notification.size()};
            notifier_->ToCloud(msg);
        });

        client_.subscribe("UpdateStatus", [this] (string_view notification) {
            msgpack_codec codec;
            UpgradeStatusInternal internal_status = codec.unpack<UpgradeStatusInternal>(notification.data(), notification.size());

            UpgradeStatus status;
            status.dev_id = internal_status.dev_id;
            status.soft_id = internal_status.soft_id;
            status.esti_time = internal_status.esti_time;
            status.start_time = internal_status.start_time;
            status.time_stamp = internal_status.time_stamp;
            status.door_module = internal_status.door_module;
            status.status = internal_status.status;
            status.progress_percent = internal_status.progress_percent;
            
            notifier_->ToHMI(status);
        });

		LOG_PRINT(IDCM_LOG_LEVEL_INFO, "init successfully");
        return  true;
    } else {
        LOG_PRINT(IDCM_LOG_LEVEL_FATAL, "fail to connect to DLC");
        return  false;
    }
}

}	// end socketwrapper namespace

IMessagePasser* GetMessagePasser()
{
    static socketwrapper::MessagePasser passer;
    return &passer;
}


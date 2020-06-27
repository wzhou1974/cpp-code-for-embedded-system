#pragma once

#include <string>
#include "codec.h"
#include "interface/msg-passer-public.h"
#include "rpc_client.hpp"
#include "interface/rpc.h"

namespace socketwrapper {

class MessagePasser : public IMessagePasser
{
public:
    bool Init(const std::shared_ptr<IVehicleNotification>& notifier) override;
    bool ToVehicle(const std::string& message) override;

    bool CheckNewPackage(std::string& service_pack_id,
                        std::vector<ReleaseNote>& release_notes) override;

    bool StartUpgrade(const std::string& service_pack_id) override;

private:
	bool InitRPCClient();

private:
	rest_rpc::rpc_client 					client_;
    std::shared_ptr<IVehicleNotification>   notifier_;
};


}	// end socketwrapper namespace
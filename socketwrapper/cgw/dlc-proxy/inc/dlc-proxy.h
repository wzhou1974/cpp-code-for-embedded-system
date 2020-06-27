#pragma once

#include <memory>
#include "rpc_server.h"
#include "interface/dlc-proxy-public.h"

namespace socketwrapper {

class DLCProxy : public IDLCProxy
{
public:
    bool Init(const std::shared_ptr<IOrchestrator>& orchestrator) override;
    
    void DownloadFinished() override;
    void UpdateResult(const std::string& result) override;
    void UpdateStatus(const UpgradeStatusInternal& status) override;
    bool DistributeFile(const std::string& url, std::string& local_file) override;

private:
    bool HandlePackageReady(rest_rpc::rpc_service::rpc_conn conn, const std::string& L1,
                            const DownloadPackages& downloads);
    UpgradeInfo HandleCheckNewPackage(rest_rpc::rpc_service::rpc_conn conn);
    bool HandleStartUpgrade(rest_rpc::rpc_service::rpc_conn conn, const std::string& L1);

private:
    std::shared_ptr<IOrchestrator>                      orchestrator_;
    std::unique_ptr<rest_rpc::rpc_service::rpc_server>  rpc_server_;
};

}	// end socketwrapper namespace


#pragma once

#include <memory>
#include <array>
#include "interface/error.h"
#include "interface/fota.h"
#include "interface/hh.h"
#include "interface/rpc.h"
#include "rpc_client.hpp"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "interface/selfupdater-proxy-public.h"

namespace socketwrapper {

class UpdaterProxy
{
public:
    bool Init(const std::string& ip, const std::shared_ptr<IUpdaterNotification>& notifier,
            std::vector<std::pair<std::string, std::string>>& versions);

    FotaError PrepareUpgrade(const Packages& pkgs);
    FotaError Upgrade(const Packages& pkgs);
    FotaError Finalize(const Packages& pkgs);

    FotaError Rollback();
    bool Distribute(const DownloadFile& info);
    FotaError GetState(UpdateProgressState& state);
    FotaError GetVersion(const std::string& software_id, std::string& version);
    FotaError GetAllSoftwareVersion(std::vector<std::pair<std::string, std::string>>& versions);

private:
    FotaError DoOperation(const std::string& operation, const Packages& pkgs);

private:
    std::shared_ptr<IUpdaterNotification>   notifier_;
    std::unique_ptr<rest_rpc::rpc_client>   rpc_client_;
};

class UpdaterProxyMgr : public IUpdaterProxyMgr
{
public:
    bool Init(NodeId id, const std::shared_ptr<IUpdaterNotification>& notifier,
            std::vector<std::pair<std::string, std::string>>& versions) override;

    FotaError PrepareUpgrade(NodeId id, const std::vector<manifest>& pkgs) override;
    FotaError Upgrade(NodeId id, const std::vector<manifest>& pkgs) override;
    FotaError Finalize(NodeId id, const std::vector<manifest>& pkgs) override;
    FotaError Rollback(NodeId id) override;
    bool Distribute(NodeId id, const DownloadFile& info) override;
    FotaError GetState(NodeId id, UpdateProgressState& state) override;
    FotaError GetVersion(NodeId id, const std::string& software_id, std::string& version) override;
    FotaError GetAllSoftwareVersion(NodeId id, std::vector<std::pair<std::string, std::string>>& versions) override;

private:
    std::array<std::unique_ptr<UpdaterProxy>, NODE_NUM> proxies_;
    std::shared_ptr<spdlog::logger>                     logger_;
};

}	// end socketwrapper namespace
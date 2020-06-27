#include <chrono>
#include <thread>
#include <utility>
#include <boost/make_unique.hpp>
#include <boost/assert.hpp>
#include "rpc_client.hpp"
#include "interface/rpc.h"
#include "interface/hh.h"
#include "cgw/self-updater-proxy/inc/selfupdater-proxy.h"
#include "fmt/format.h"
#include "prettyprint.hpp"
#include "interface/fota.h"

using namespace rest_rpc;
using namespace rest_rpc::rpc_service;

namespace socketwrapper {

bool UpdaterProxy::Init(const std::string& ip, const std::shared_ptr<IUpdaterNotification>& notifier,
    std::vector<std::pair<std::string, std::string>>& versions)
{
    notifier_ = notifier;
    rpc_client_ = std::make_unique<rpc_client>(ip, SELFUPDATER_RPC_PORT);

    int try_count{10};

    while (try_count > 0) {
        if (rpc_client_->connect()) {
            break;                
        }

        // wait 1 second and try again
        std::this_thread::sleep_for(std::chrono::seconds(1));
        try_count--;
    }

    if (!rpc_client_->has_connected()) {
        BOOST_ASSERT(try_count < 0);
        return  false;
    }

    rpc_client_->enable_auto_reconnect();
    rpc_client_->enable_auto_heartbeat();

    // register status notificatione
    rpc_client_->subscribe("ReportStatus", [this, ip] (string_view rpc_serialize_data) {
        msgpack_codec codec;
        UpdateProgressState_t status;

        status = codec.unpack<UpdateProgressState_t>(rpc_serialize_data.data(),
            rpc_serialize_data.size());

        this->notifier_->UpdateStatus(IpToNode(ip), status);
    });

    rpc_client_->subscribe("DistributeResult", [this, ip] (string_view rpc_serialize_data) {
        msgpack_codec codec;
        DownloadResult result;
        result = codec.unpack<DownloadResult>(rpc_serialize_data.data(), rpc_serialize_data.size());

        this->notifier_->DistributeStatus(IpToNode(ip), result);
    });

    // register "Exit" notification
    rpc_client_->subscribe("Exit", [this, ip] (string_view rpc_serialize_data) {
        msgpack_codec codec;
        UpdaterLeaveCause cause = codec.unpack<UpdaterLeaveCause>(rpc_serialize_data.data(),
            rpc_serialize_data.size());

        this->notifier_->Exit(IpToNode(ip), cause);
    });

    auto error = GetAllSoftwareVersion(versions);

    return error.error_code_ == ERROR_OK;
}

FotaError UpdaterProxy::PrepareUpgrade(const Packages& pkgs)
{
    return DoOperation("PrepareUpgrade", pkgs);
}

FotaError UpdaterProxy::Upgrade(const Packages& pkgs)
{
    return DoOperation("Upgrade", pkgs);
}

FotaError UpdaterProxy::Finalize(const Packages& pkgs)
{
    return DoOperation("Finalize", pkgs);
}

FotaError UpdaterProxy::DoOperation(const std::string& operation, const Packages& pkgs)
{
    FotaError   error;

    try {
        SELFINSTALLER_CODE code;
        code = rpc_client_->call<SELFINSTALLER_CODE>(operation, pkgs);

        if (code == SELFINSTALLER_OK) {
            error.error_code_ = ERROR_OK;
        } else {        
            error.error_code_ = code;
            error.catagory_ = CATAGORY_SELFINSTALLER;
            error.error_casue_ = rpc_client_->call<std::string>("GetErrorCause", code);
        }
    } catch (const std::exception& e) {
        error.catagory_ = CATAGORY_RPC;
        error.error_code_ = ERROR_RPC_OPERATION;
        error.error_casue_ = e.what();
    }

    return  error;
}

FotaError UpdaterProxy::Rollback()
{
    FotaError   error;

    try {
        SELFINSTALLER_CODE code;
        code = rpc_client_->call<SELFINSTALLER_CODE>("Rollback");

        if (code == SELFINSTALLER_OK) {
            error.error_code_ = ERROR_OK;
        } else {        
            error.error_code_ = code;
            error.catagory_ = CATAGORY_SELFINSTALLER;
            error.error_casue_ = rpc_client_->call<std::string>("GetErrorCause", code);
        }
    } catch (const std::exception& e) {
        error.catagory_ = CATAGORY_RPC;
        error.error_code_ = ERROR_RPC_ROLLBACK;
        error.error_casue_ = e.what();
    }

    return  error;
}

bool UpdaterProxy::Distribute(const DownloadFile& info)
{
    try {
        bool succ = rpc_client_->call<bool>("Distribute", info);
        return succ;
    } catch (const std::exception& e) {
        return  false;
    }
}

FotaError UpdaterProxy::GetState(UpdateProgressState& state)
{
    FotaError   error;

    try {
        GetStateResult result;
        result = rpc_client_->call<GetStateResult>("GetState");
        if (result.error_ == SELFINSTALLER_OK) {
            error.error_code_ = ERROR_OK;

            state = result.state_;
        } else {
            error.catagory_ = CATAGORY_SELFINSTALLER;
            error.error_code_ = result.error_;

            error.error_casue_ = rpc_client_->call<std::string>("GetErrorCause", result.error_);
        }
    } catch (const std::exception& e) {
        error.catagory_ = CATAGORY_RPC;
        error.error_code_ = ERROR_RPC_GETSTATE;
        error.error_casue_ = e.what();
    }

    return  error;
}

FotaError UpdaterProxy::GetVersion(const std::string& software_id, std::string& version)
{
    FotaError   error;

    try {
        GetVersionResult result = rpc_client_->call<GetVersionResult>("GetVersion", software_id);
        if (result.error_ == SELFINSTALLER_OK) {
            error.error_code_ = ERROR_OK;
            version = result.version_;
        } else {
            error.catagory_ = CATAGORY_SELFINSTALLER;
            error.error_code_ = result.error_;
            error.error_casue_ = rpc_client_->call<std::string>("GetErrorCause", result.error_);
        }
    } catch (const std::exception& e) {
        error.catagory_ = CATAGORY_RPC;
        error.error_code_ = ERROR_RPC_GETVERSION;
        error.error_casue_ = e.what();
    }

    return  error;
}

FotaError UpdaterProxy::GetAllSoftwareVersion(std::vector<std::pair<std::string, std::string>>& versions)
{
    FotaError   error;

    try {
        GetAllVersionsResult    result;
        result = rpc_client_->call<GetAllVersionsResult>("GetAllSoftwareVersion");
        if (result.error_ == SELFINSTALLER_OK) {
            error.error_code_ = ERROR_OK;
            versions = std::move(result.versions_);
        } else {
            error.catagory_ = CATAGORY_SELFINSTALLER;
            error.error_code_ = result.error_;
            error.error_casue_ = rpc_client_->call<std::string>("GetErrorCause", result.error_);
        }
    } catch (const std::exception& e) {
        error.catagory_ = CATAGORY_RPC;
        error.error_code_ = ERROR_RPC_GETALLVERSIONS;
        error.error_casue_ = e.what();
    }

    return  error;
}

bool UpdaterProxyMgr::Init(NodeId id, const std::shared_ptr<IUpdaterNotification>& notifier,
    std::vector<std::pair<std::string, std::string>>& versions)
{
    BOOST_ASSERT(NodeToInt(id) < NODE_NUM);

    auto proxy = boost::make_unique<UpdaterProxy>();
    if (proxy->Init(NodeToIp(id), notifier, versions)) {        
        proxies_[NodeToInt(id)] = std::move(proxy);
        return  true;
    } else {
        return  false;
    }
}                        

FotaError UpdaterProxyMgr::PrepareUpgrade(NodeId id, const std::vector<manifest>& pkgs)
{
    BOOST_ASSERT(NodeToInt(id) < NODE_NUM);
    BOOST_ASSERT(proxies_[NodeToInt(id)]);

    Packages p;
    p.manifests_ = pkgs;
    return proxies_[NodeToInt(id)]->PrepareUpgrade(p);
}

FotaError UpdaterProxyMgr::Upgrade(NodeId id, const std::vector<manifest>& pkgs)
{
    BOOST_ASSERT(NodeToInt(id) < NODE_NUM);
    BOOST_ASSERT(proxies_[NodeToInt(id)]);

    Packages p;
    p.manifests_ = pkgs;
    return proxies_[NodeToInt(id)]->Upgrade(p);
}

FotaError UpdaterProxyMgr::Finalize(NodeId id, const std::vector<manifest>& pkgs)
{
    BOOST_ASSERT(NodeToInt(id) < NODE_NUM);
    BOOST_ASSERT(proxies_[NodeToInt(id)]);

    Packages p;
    p.manifests_ = pkgs;
    return proxies_[NodeToInt(id)]->Finalize(p);
}

FotaError UpdaterProxyMgr::Rollback(NodeId id)
{
    BOOST_ASSERT(NodeToInt(id) < NODE_NUM);
    BOOST_ASSERT(proxies_[NodeToInt(id)]);
    return proxies_[NodeToInt(id)]->Rollback();
}

bool UpdaterProxyMgr::Distribute(NodeId id, const DownloadFile& info)
{
    BOOST_ASSERT(NodeToInt(id) < NODE_NUM);
    BOOST_ASSERT(proxies_[NodeToInt(id)]);
    return proxies_[NodeToInt(id)]->Distribute(info);
}

FotaError UpdaterProxyMgr::GetState(NodeId id, UpdateProgressState& state)
{
    BOOST_ASSERT(NodeToInt(id) < NODE_NUM);
    BOOST_ASSERT(proxies_[NodeToInt(id)]);

    FotaError error;
    error = proxies_[NodeToInt(id)]->GetState(state);
    return error;
}

FotaError UpdaterProxyMgr::GetVersion(NodeId id, const std::string& software_id, std::string& version)
{
    BOOST_ASSERT(NodeToInt(id) < NODE_NUM);
    BOOST_ASSERT(proxies_[NodeToInt(id)]);
    return proxies_[NodeToInt(id)]->GetVersion(software_id, version);
}

FotaError UpdaterProxyMgr::GetAllSoftwareVersion(NodeId id, std::vector<std::pair<std::string, std::string>>& versions)
{
    BOOST_ASSERT(NodeToInt(id) < NODE_NUM);
    BOOST_ASSERT(proxies_[NodeToInt(id)]);
    return proxies_[NodeToInt(id)]->GetAllSoftwareVersion(versions);
}

}   // namespace socketwrapper

IUpdaterProxyMgr* GetUpdaterProxyMgr()
{
    static socketwrapper::UpdaterProxyMgr mgr;
    return &mgr;
}
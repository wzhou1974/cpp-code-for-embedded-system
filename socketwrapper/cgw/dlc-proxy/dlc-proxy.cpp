#include <thread>
#include <boost/assert.hpp>
#include <cassert>
#include <boost/smart_ptr/make_unique.hpp>
#include <boost/filesystem.hpp>
#include "common/inc/utils.h"
#include "cgw/dlc-proxy/inc/dlc-proxy.h"
#include "interface/fota.h"
#include "rpc_server.h"
#include "common/inc/https_download.h"

using namespace rest_rpc;
using namespace rest_rpc::rpc_service;
namespace fs = boost::filesystem;

namespace socketwrapper {

static rpc_server  *server{nullptr};

bool DLCProxy::Init(const std::shared_ptr<IOrchestrator>& orchestrator)
{
    orchestrator_ = orchestrator;

    if (!IsPortInUse(DLC_PROXY_PORT)) {
        auto proxy_job = [this] () {
            rpc_server_ = boost::make_unique<rpc_server>(DLC_PROXY_PORT, std::thread::hardware_concurrency());
            rpc_server_->register_handler("PackageReady", &DLCProxy::HandlePackageReady, this);
            rpc_server_->register_handler("CheckNewPackage", &DLCProxy::HandleCheckNewPackage, this);
            rpc_server_->register_handler("StartUpgrade", &DLCProxy::HandleStartUpgrade, this);

            rpc_server_->run();
        };

        std::thread dlc_proxy_server_thread(proxy_job);
        dlc_proxy_server_thread.detach();

        return  true;
    } else {
        // DLC_PROXY_PORT is in use
        return  false;
    }
}

void DLCProxy::DownloadFinished()
{
    if (rpc_server_)
        rpc_server_->publish("DownloadFinished", std::string());
}

void DLCProxy::UpdateResult(const std::string& result)
{
    if (rpc_server_)
        rpc_server_->publish("UpdateResult", result);
}

void DLCProxy::UpdateStatus(const UpgradeStatusInternal& status)
{
    if (rpc_server_)
        rpc_server_->publish("UpdateStatus", status);
}

bool DLCProxy::HandlePackageReady(rpc_conn conn, const std::string& L1,
                                const DownloadPackages& downloads)
{
    return  orchestrator_->PackageReady(L1, downloads.packages_);
}

UpgradeInfo DLCProxy::HandleCheckNewPackage(rpc_conn conn)
{
    std::string L1 = orchestrator_->CheckNewPackage();

    UpgradeInfo info;

    if (L1.empty()) {
        info.status = false;
    } else {
        info.status = true;
        // to-do
        // retrieve service_pcak_id and release notes
    }
    return  info;
}

bool DLCProxy::HandleStartUpgrade(rpc_conn conn, const std::string& L1)
{
    return  orchestrator_->StartUpgrade(L1);
}

bool DLCProxy::DistributeFile(const std::string& url, std::string& local_file)
{
    size_t size;
    if (!GetFileSize(url, size)) {
        return  false;
    }

    fs::path local_path{local_file};
    if (fs::exists(local_path)) {
        fs::remove(local_path);
    }
    
    auto download_job = [url, size, local_path, this] () {
        HttpsDownload downloader(url, size, local_path);

        static constexpr size_t MAX_REPEAT_COUNT = 10;
        size_t repeat_count{0};

        PROGRESS reporter = [] (size_t percent) { 
            
        };

        loff_t current{0};

        while (repeat_count < MAX_REPEAT_COUNT) {
            auto succ = downloader.DownloadFrom(current, reporter);
            if (succ)
                break;

            current = fs::file_size(local_path);
            assert(current < size);
        }

        bool result;
        if (fs::file_size(local_path) == size)
            result = true;
        else 
            result = false;

        orchestrator_->DistributeResult(url, result);
    };

    std::thread downloader_thread(download_job);
    downloader_thread.detach();
    return  true;
}

}   // namespace socketwrapper

IDLCProxy* GetDLCProxy()
{
    static socketwrapper::DLCProxy proxy;
    return  &proxy;
}
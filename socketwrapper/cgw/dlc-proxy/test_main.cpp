#include <memory>
#include <iostream>
#include <utility>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <boost/assert.hpp>
#include "interface/dlc-proxy-public.h"
#include "interface/error.h"
#include "interface/fota.h"
#include "interface/hh.h"
#include "interface/rpc.h"
#include "prettyprint.hpp"
#include "fmt/format.h"
#include "boost/pointer_cast.hpp"

using namespace socketwrapper;

class Orchestrator : public IOrchestrator
{
public:
    bool PackageReady(const std::string& L1, const std::vector<DownloadInfo>& info) override;
    std::string CheckNewPackage() override;
    bool StartUpgrade(const std::string& L1) override;
    void DistributeResult(const std::string& url, bool& result) override;
};

bool Orchestrator::PackageReady(const std::string& L1, const std::vector<DownloadInfo>& info)
{
    std::cout << __func__ << std::endl;

    for (auto& download : info) {
        std::cout << download.url_cdn_ << std::endl;
        std::cout << download.url_dlc_ << std::endl;
        std::cout << download.size_ << std::endl;
    }

    return  0;
}

std::string Orchestrator::CheckNewPackage()
{
    std::cout << __func__ << std::endl;
    return  std::string();
}

bool Orchestrator::StartUpgrade(const std::string& L1)
{
    std::cout << __func__ << std::endl;
    return  true;
}

void Orchestrator::DistributeResult(const std::string& url, bool& result)
{
    std::cout << __func__ << std::endl;
    std::cout << fmt::format("Download {} : {}", url, result);
}

int main(int argc, char *argv[])
{
    auto orch = std::make_shared<Orchestrator>();

    auto dlc = GetDLCProxy();
    if (dlc->Init(boost::dynamic_pointer_cast<IOrchestrator>(orch))) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        dlc->DownloadFinished();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        


    }
    return  0;
}
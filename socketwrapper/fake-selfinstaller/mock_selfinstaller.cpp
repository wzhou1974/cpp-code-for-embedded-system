#include <memory>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <utility>

#include <boost/assert.hpp>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

#if __cplusplus < 201703L
#include "boost/filesystem.hpp"
#else
#include <filesystem>
#endif

#include "json/nlohmann/json.hpp"
#include "interface/selfinstaller_error.h"
#include "updaterwrapper/inc/selfupdater.h"
#include "updaterwrapper/inc/updaterwrapper.h"
#include "ppk_assert.h"

using namespace boost::assign;
using namespace socketwrapper;

namespace fs = boost::filesystem;

struct TestConfig
{
    NodeId      node;
    std::vector<std::pair<std::string, std::string>> versions;

    uint32_t prepare;
    uint32_t upgrade;
    uint32_t finalize;
    uint32_t rollback;

    uint32_t report_state_interval;
};

TestConfig config;

UpdateProgressState_t   g_state;

enum class MOCK_ACTION
{
    ePrepare,
    eUpgrade,
    eFinialize,
    eRollback
};

std::unique_ptr<UpdaterWrapper>     wrapper;

class MockSelfUpdater : public ISelfUpdater
{
public:
    MockSelfUpdater() {}   

    SELFINSTALLER_CODE GetVersion(const std::string& software_id, std::string& version) override;   
    SELFINSTALLER_CODE GetAllSoftwareVersion(std::vector<std::pair<std::string, std::string>>& versions) override;
    std::string GetPackageStorageLocation() override;
    SELFINSTALLER_CODE PrepareUpgrade(const std::vector<manifest>& partitions) override;
    SELFINSTALLER_CODE Upgrade(const std::vector<manifest>& partitions) override;
    SELFINSTALLER_CODE Finalize(const std::vector<manifest>& partitions) override;
    SELFINSTALLER_CODE Rollback() override;
    SELFINSTALLER_CODE GetState(UpdateProgressState_t& state) override;
    std::string GetErrorCause(SELFINSTALLER_CODE error) override;
};

void mock_action(uint32_t seconds, MOCK_ACTION action, const std::vector<manifest>& partitions)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));

    for (auto& partition : partitions) {
        g_state.package_ = partition.softwre_id;
        g_state.log_ = "OK";

        switch (action) {
        case MOCK_ACTION::ePrepare:
            g_state.state_ = UpdateState::ePREPARATION_DONE;
            break;

        case MOCK_ACTION::eUpgrade:
            g_state.state_ = UpdateState::eACTIVATION_DONE;
            break;

        case MOCK_ACTION::eFinialize:
            g_state.state_ = UpdateState::eFINAILIZE_DONE;
            break;
        }

        wrapper->ReportStatus(g_state);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

SELFINSTALLER_CODE MockSelfUpdater::GetVersion(const std::string& software_id, std::string& version)
{
    auto version_ptr = std::find_if(config.versions.cbegin(),
                                    config.versions.cend(), 
                                    [&] (const std::pair<std::string, std::string>& n)
                                    {
                                        return software_id == n.first;
                                    });
    if (version_ptr != config.versions.cend()) {
        version = version_ptr->second;

        return  SELFINSTALLER_OK;
    } else {
        return  -1;
    }
}

SELFINSTALLER_CODE MockSelfUpdater::GetAllSoftwareVersion(std::vector<std::pair<std::string, std::string>>& versions)
{
    versions = config.versions;

    return  SELFINSTALLER_OK;  
}

std::string MockSelfUpdater::GetPackageStorageLocation()
{
    fs::path temp_dir{fs::temp_directory_path()};
    return  temp_dir.string();
}

SELFINSTALLER_CODE MockSelfUpdater::PrepareUpgrade(const std::vector<manifest>& partitions)
{
    std::cout << "Prepare Upgrade: ";

    for (auto& partition : partitions) {
        std::cout << partition.softwre_id << " ";        
    }
    std::cout << std::endl;
   
    std::thread prepare_thread(mock_action, config.prepare, MOCK_ACTION::ePrepare, partitions);
    prepare_thread.detach();

    return SELFINSTALLER_OK;
}


SELFINSTALLER_CODE MockSelfUpdater::Upgrade(const std::vector<manifest>& partitions)
{
    std::cout << "Upgrade: ";

    for (auto& partition : partitions) {
        std::cout << partition.softwre_id << " ";
    }
    std::cout << std::endl;

    std::thread upgrade_thread(mock_action, config.upgrade, MOCK_ACTION::eUpgrade, partitions);
    upgrade_thread.detach();

    return SELFINSTALLER_OK;
}

SELFINSTALLER_CODE MockSelfUpdater::Finalize(const std::vector<manifest>& partitions)
{
    std::cout << "Finalize: ";

    for (auto& partition : partitions) {
        std::cout << partition.softwre_id << " ";
    }
    std::cout << std::endl;

    std::thread finalize_thread(mock_action, config.finalize, MOCK_ACTION::eFinialize, partitions);
    finalize_thread.detach();

    return SELFINSTALLER_OK;
}

SELFINSTALLER_CODE MockSelfUpdater::Rollback()
{
    std::cout << "Rollback" << std::endl;

    return SELFINSTALLER_OK;
}

SELFINSTALLER_CODE MockSelfUpdater::GetState(UpdateProgressState_t& state)
{
    state = g_state;

    return  SELFINSTALLER_OK;
}

std::string MockSelfUpdater::GetErrorCause(SELFINSTALLER_CODE error)
{
    switch (error) {
    case SELFINSTALLER_OK:
        return  "ok";
    default:
        return  "error";
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " unittest_updaterwrapper.json" << std::endl;
        return  1;
    }

    std::ifstream test_conf_file(argv[1]);
    nlohmann::json json;
    
    try {
        test_conf_file >> json;

        BOOST_ASSERT(json.contains("versions"));

        std::string node_name = json["node"];
        boost::algorithm::to_upper(node_name);
        config.node = ECUNameToNode(node_name);

        config.prepare = json["prepare"];
        config.upgrade = json["upgrade"];
        config.finalize = json["finalize"];
        config.rollback = json["rollback"];

        config.report_state_interval = json["report-state"];

        auto versions = json["versions"];
        BOOST_ASSERT(versions.is_array());
        
        for (size_t i = 0; i < versions.size(); ++i) {
            auto version = versions[i];
            for (auto& el : version.items())
                config.versions.push_back(std::make_pair(el.key(), el.value()));
        }

    } catch (nlohmann::json::exception& e) {
        std::cerr << argv[1] << ": invalid format (" << e.what() << ")" << std::endl;
        return  -1;
    }

    auto updater = std::make_shared<MockSelfUpdater>();
    wrapper = boost::make_unique<UpdaterWrapper>(config.node, updater);

    g_state.state_ = UpdateState::eIDLE;
    g_state.percent_ = 0;

    auto res = wrapper->Init();
    BOOST_ASSERT(res);

    std::cout << "MockSelfInstaller is running:" << std::endl;
    std::cout << "Press any key to exit" << std::endl;

    std::cin.get();

    wrapper->Deinit(UpdaterLeaveCause::eExit);

    return  0;
}



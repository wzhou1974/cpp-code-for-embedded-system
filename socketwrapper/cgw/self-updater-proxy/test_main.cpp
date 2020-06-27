#include <memory>
#include <iostream>
#include <utility>
#include <string>
#include <vector>
#include <boost/assert.hpp>
#include "interface/selfupdater-proxy-public.h"
#include "interface/error.h"
#include "interface/fota.h"
#include "interface/hh.h"
#include "interface/rpc.h"
#include "prettyprint.hpp"

using namespace socketwrapper;

void DumpState(const UpdateProgressState& status)
{
    switch (status.state_) {
        case UpdateState::eIDLE:
            std::cout << "\t" << "IDLE" << std::endl;
            break;
        case UpdateState::eDOWNLOADING:
            std::cout << "\t" << "DOWNLOADING" << std::endl;
            break;
        case UpdateState::eDOWNLOAD_DONE:
            std::cout << "\t" << "DOWNLOAD OK" << std::endl;
            break;
        case UpdateState::ePREPARATION:
            std::cout << "\t" << "PREPARATION" << std::endl;
            break;
        case UpdateState::ePREPARATION_DONE:
            std::cout << "\t" << "PREPARATION DONE" << std::endl;
            break;
        case UpdateState::eACTIVATION:
            std::cout << "\t" << "ACTIVATION" << std::endl;
            break;
        case UpdateState::eACTIVATION_DONE:
            std::cout << "\t" << "ACTIVATION DONE" << std::endl;
            break;
        case UpdateState::eFINAILIZE:
            std::cout << "\t" << "FINAILIZE" << std::endl;
            break;            
        case UpdateState::eFINAILIZE_DONE:
            std::cout << "\t" << "FINAILIZE DONE" << std::endl;
            break;            
        case UpdateState::eROLLBACK:
            std::cout << "\t" << "ROLLBACK" << std::endl;
            break;
        case UpdateState::eROLLBACK_DONE:
            std::cout << "\t" << "ROLLBACK DONE" << std::endl;
            break;
        case UpdateState::eERROR:
            std::cout << "\t" << "ERROR" << std::endl;
            break;            
    }

    std::cout << status.package_ << std::endl;
    std::cout << status.percent_ << std::endl;
    std::cout << status.log_ << std::endl;
}

class MessageDump : public IUpdaterNotification
{
public:
    void UpdateStatus(NodeId id, const UpdateProgressState& status) override;
    void DistributeStatus(NodeId id, const DownloadResult& result) override;
    void Exit(NodeId id, UpdaterLeaveCause cause) override;
};

void MessageDump::UpdateStatus(NodeId id, const UpdateProgressState& status)
{
    std::cout << "Status from: " << NodeToECUName(id) << std::endl;

    DumpState(status);
}

void MessageDump::DistributeStatus(NodeId id, const DownloadResult& result)
{
    std::cout << "Download result from: " << NodeToECUName(id) << std::endl;
    std::cout << result.uri_ << ", ";
    if (result.status_ == DownloadStatus::eDONE)
        std::cout << "done";
    else if (result.status_ == DownloadStatus::eFAIL) {
        std::cout << "failed, " << static_cast<int>(result.error_);
    }

    std::cout << std::endl;
}

void MessageDump::Exit(NodeId id, UpdaterLeaveCause cause)
{
    if (cause == UpdaterLeaveCause::eABSwitch) {
        std::cout << "AB Switch" << std::endl;
    } else if (cause == UpdaterLeaveCause::eExit) {
        std::cout << "Selfinstall exit" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << argv[1] << " domain-controler-name" << std::endl;
        return  -1;
    }

    NodeId node = ECUNameToNode(argv[1]);

    IUpdaterProxyMgr* selfinstaller = GetUpdaterProxyMgr();

    auto msg = std::make_shared<MessageDump>();    

    std::vector<std::pair<std::string, std::string>> versions;

    if (selfinstaller->Init(node, msg, versions)) {
        std::cout << "Init " << argv[1] << " successfully !" << std::endl;
        std::cout << "Versions: " << versions << std::endl;
    } else {
        std::cerr << "Fail to init " << argv[1] << std::endl;
    }

    std::vector<std::pair<std::string, std::string>> versions2;
    auto error = selfinstaller->GetAllSoftwareVersion(node, versions2);
    if (error.error_code_ == ERROR_OK) {
        std::cout << "Get all versions: " << versions2 << std::endl;
    } else {
        std::cout << "Get all versions error !" << std::endl;
    }

    std::string package_1_version;
    error = selfinstaller->GetVersion(node, "package-1", package_1_version);
    if (error.error_code_ == ERROR_OK) {
        std::cout << "Get package-1 version: " << package_1_version << std::endl;
    } else {
        std::cout << "Get package-1 version error !" << std::endl;
    }

    std::string package_2_version;
    error = selfinstaller->GetVersion(node, "package-2", package_2_version);
    if (error.error_code_ == ERROR_OK) {
        std::cout << "Get package-2 version: " << package_2_version << std::endl;
    } else {
        std::cout << "Get package-2 version error !" << std::endl;
    }

    std::vector<manifest> pkgs(2);
    pkgs[0].softwre_id = "aaa";
    pkgs[1].softwre_id = "bbb";

    error = selfinstaller->PrepareUpgrade(node, pkgs);
    if (error.error_code_ == ERROR_OK) {
        std::cout << "Prepareupgrade successfully !" << std::endl;
    } else {
        std::cout << "Prepareupgrade error: [" << error.catagory_ << "-" << error.error_code_ << "-" << error.error_casue_ << std::endl;
    }

    error = selfinstaller->Upgrade(node, pkgs);
    if (error.error_code_ == ERROR_OK) {
        std::cout << "Upgrade successfully !" << std::endl;
    } else {
        std::cout << "Upgrade error: [" << error.catagory_ << "-" << error.error_code_ << "-" << error.error_casue_ << std::endl;
    }

    error = selfinstaller->Finalize(node, pkgs);
    if (error.error_code_ == ERROR_OK) {
        std::cout << "Finalize successfully !" << std::endl;
    } else {
        std::cout << "Finalize error: [" << error.catagory_ << "-" << error.error_code_ << "-" << error.error_casue_ << std::endl;
    }

    error = selfinstaller->Rollback(node);
    if (error.error_code_ == ERROR_OK) {
        std::cout << "Rollback successfully !" << std::endl;
    } else {
        std::cout << "Rollback error: [" << error.catagory_ << "-" << error.error_code_ << "-" << error.error_casue_ << std::endl;
    }

    UpdateProgressState state;
    error = selfinstaller->GetState(node, state);
    if (error.error_code_ == ERROR_OK) {
        DumpState(state);
    } else {
        std::cout << "Rollback error: [" << error.catagory_ << "-" << error.error_code_ << "-" << error.error_casue_ << std::endl;
    }

    std::cin.get();

    return  0;  
}

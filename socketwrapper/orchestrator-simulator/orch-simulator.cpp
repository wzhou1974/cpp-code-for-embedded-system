#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <thread>
#include <chrono>
#include <memory>
#include <cstdlib>      // for std::system
#include <cassert>
#include "boost/filesystem.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/algorithm/string/case_conv.hpp"
#include "json/nlohmann/json.hpp"
#include "interface/selfupdater-proxy-public.h"
#include "interface/hh.h"
#include "common/inc/utils.h"
#include "uri-library/uri.hh"
#include "fmt/format.h"

namespace fs = boost::filesystem;

static IUpdaterProxyMgr* self_installer_mgr{nullptr};

static std::string GetFileNameFromUri(const std::string& url)
{
    uri https_uri{url};
    std::string path_on_server = "/" + https_uri.get_path();
    return fs::path{path_on_server}.filename().string();
}

class OrchestratorReceiver : public IUpdaterNotification
{
public:
    void UpdateStatus(NodeId id, const UpdateProgressState& status) override;
    void DistributeStatus(NodeId id, const DownloadResult& result) override;
    void Exit(NodeId id, UpdaterLeaveCause cause) override;
};

static void DumpStatus(const UpdateProgressState& status)
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

void OrchestratorReceiver::UpdateStatus(NodeId id, const UpdateProgressState& status)
{
    std::string box = NodeToECUName(id);

    std::cout << "Status from " << box << std::endl;

    DumpStatus(status);
}

void OrchestratorReceiver::DistributeStatus(NodeId id, const DownloadResult& result)
{
    std::string box = NodeToECUName(id);

    std::string filename = GetFileNameFromUri(result.uri_);
    if (result.status_ == DownloadStatus::eDONE) {
        assert(result.error_ == DownloadError::eOK);
        std::cout << filename << " download successfully" << std::endl;
    } else {
        assert(result.status_ == DownloadStatus::eFAIL);

        std::cerr << filename << " download failed: ";
        switch (result.error_) {
            case DownloadError::eMD5:
                std::cerr << "Wrong checksum";
                break;
            case DownloadError::eSIZE:
                std::cerr << "Wrong file size";
                break;
            case DownloadError::eSPACE:
                std::cerr << "No space";
                break;
            case DownloadError::eFILEIO:
                std::cerr << "File IO";
                break;
            case DownloadError::eNOFILE:
                std::cerr << "Not exist";
                break;
            case DownloadError::eRPC:
                std::cerr << "RPC error";
                break;
            case DownloadError::eREPEAT:
                std::cerr << "download repeatly";
                break;
            default:
                std::cerr << "Unknow error";
                break;
        }

        std::cerr << std::endl;
    }
}

void OrchestratorReceiver::Exit(NodeId id, UpdaterLeaveCause cause)
{
    std::string box = NodeToECUName(id);

    if (cause == UpdaterLeaveCause::eABSwitch) {
        std::cout << box << " want to AB Switch !" << std::endl;
    } else {
        std::cout << box << " want to exit !" << std::endl;
    }
}

/*
    
    {   
        "https-server-ip": "127.0.0.1",
        "node": "TBOX",
        "manifest": [{
            "software_id": "SV-1",
            "filename": "pkg-1",
            "version": "ver.1",
            "delta": false,
            "original_version": "",
            "type": 1,
            "flashing": 0,
            "attrs": [{
                "attr0": "a0"
            }, {
                "attr1": "a1"
            }, {
                "attr2": "a2"
            }]
        }, {
            "software_id": "SV-2",
            "filename": "pkg-2",
            "version": "ver.2",
            "delta": false,
            "original_version": "",
            "type": 1,
            "flashing": 0,
            "attrs": [{
                "attr0": "a0"
            }, {
                "attr1": "a1"
            }, {
                "attr2": "a2"
            }]
        }],
        "operation": [	
            {
                "task": "PrepareUpgrade"
            },
            {
                "task": "Upgrade"
            },
            {
                "task": "GetState"
            },
            {
                "task": "Finalize"
            }
        ]
    }

 */

static void ShowMenu()
{
    std::cout << "--------Menu--------" << std::endl;
    std::cout << "0: Run script" << std::endl;
    std::cout << "1: Rollback" << std::endl;
    std::cout << "2: Download Package" << std::endl;
    std::cout << "3: Show state" << std::endl;
    std::cout << "4: Show version" << std::endl;
    std::cout << "5: Show all versions" << std::endl;
    std::cout << "6: Exit" << std::endl;
    std::cout << "--------------------" << std::endl;
}

static void DumpError(FotaError error)
{
    if (error.error_code_ != ERROR_OK) {
        switch (error.catagory_) {
            case CATAGORY_SELFINSTALLER:
                std::cerr << "selfinstaller error" << std::endl;
                break;
            case CATAGORY_SELFUPDATER:
                std::cerr << "selfupdater error" << std::endl;
                break;
            case CATAGORY_ORCHESTRATOR:
                std::cerr << "orchestrator error" << std::endl;
                break;
            case CATAGORY_DLC:
                std::cerr << "DLC error" << std::endl;
                break;
            case CATAGORY_RPC:
                std::cerr << "RPC error" << std::endl;
                break;
            default:
                std::cerr << "Unknow error type !" << std::endl;
                break;
        }

        std::cerr << error.error_casue_ << std::endl;
    }
}

static void RunScript(NodeId node, const std::vector<std::string>& operations, const std::vector<manifest>& params)
{
    FotaError   error;

    for (auto& op : operations) {
        std::cout << "Press any key to call " << op << std::endl;
        std::cin.get();

        if (boost::algorithm::to_upper_copy(op) == "PREPAREUPGRADE") {
            error = self_installer_mgr->PrepareUpgrade(node, params);
        } 

        if (boost::algorithm::to_upper_copy(op) == "UPGRADE") {
            error = self_installer_mgr->Upgrade(node, params);
        }

        if (boost::algorithm::to_upper_copy(op) == "FINALIZE") {
            error = self_installer_mgr->Finalize(node, params);
        }

        if (error.error_code_ == ERROR_OK) {
            std::cout << op << " is OK" << std::endl;
        } else {
            std::cerr << op << " failed" << std::endl;
            DumpError(error);

            break;
        }
    }
}

static void DistributeFiles(NodeId node, const std::string& https_ip, const nlohmann::json& https_conf)
{
    std::cout << "Your choice: Download Package" << std::endl;
    std::cout << "Number of files: ";
    int n;
    std::cin >> n;

    std::string https_root = https_conf["static-files-dir"];

    unsigned port{0};
    if (https_conf.contains("https-port")) {
        assert(https_conf["https-port"].is_number_unsigned());
        port = https_conf["https-port"];
    }

    std::string https_uri{"https://" + https_ip};
    if (port != 0)
        https_uri += ":" + std::to_string(port);
        
    https_uri += "/";

    std::vector<DownloadFile>   infos;

    int i{0};
    while (i < n) {
        std::cout << "Uri: ";
        std::string file;
        std::cin >> file;

        fs::path file_path{https_root + "/" + file};
        if (!fs::exists(file_path)) {
            std::cerr << file << " not exist !" << std::endl;
        } else {
            DownloadFile info;    

            info.uri_ = https_uri + file;
            info.size_ = fs::file_size(file_path);
            info.checksum_ = md5sum(file_path.string());

            std::cout << fmt::format("File size: {}, Checksum: {}", info.size_, info.checksum_) << std::endl;
            infos.push_back(info);
            i++;
        }
    }

    if (!infos.empty()) {
        for (auto& info : infos) {

            std::string filename = GetFileNameFromUri(info.uri_);

            std::cout << fmt::format("Start download {}", filename) << std::endl;
            if (self_installer_mgr->Distribute(node, info)) {
                std::cout << fmt::format("Distribute {} OK", filename) << std::endl;
            } else {
                std::cerr << fmt::format("Fail to distribute {}", filename) << std::endl;
            }
        }        
    }
}

void wait_cgdb_connect()
{
    auto temp_path = fs::temp_directory_path();
    auto wait_cgdb = temp_path / "wait-cgdb";

    while (fs::exists(wait_cgdb)) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

int main(int argc, char **argv)
{
    wait_cgdb_connect();

    if (argc != 3) {
        std::cerr << argv[0] << " script.json https-server.conf" << std::endl;
        return  -1;
    }

    fs::path script_json{argv[1]};

    if (!fs::exists(script_json)) {
        std::cerr << argv[1] << " not exist !" << std::endl;
        return  -2;
    }

    fs::path https_conf{argv[2]};

    if (!fs::exists(https_conf)) {
        std::cerr << argv[2] << " not exist !" << std::endl;
        return  -3;
    }

    std::ifstream script_file(argv[1]);
    nlohmann::json json;

    try {
        script_file >> json;
    } catch (nlohmann::json::exception& e) {
        std::cerr << argv[1] << ": invalid format (" << e.what() << ")" << std::endl;
        return  -4;
    }

    if (!json.contains("https-server-ip")) {
        std::cerr << argv[1] << ": not set https-server-ip !" << std::endl;
        return  -5;
    }
    assert(json["https-server-ip"].is_string());
    std::string https_ip = json["https-server-ip"];

    if (!(json.contains("node")     &&
        json.contains("manifest")   &&
        json.contains("operation"))) {
        std::cerr << argv[1] << " is not well-formated !" << std::endl;
        return  -7;
    }

    if (!json["node"].is_string()) {
        std::cerr << "node = ?" << std::endl;
        return  -8; 
    }

    if (!json["manifest"].is_array()) {
        std::cerr << "manifest is not array !" << std::endl;
        return  -9;
    }

    if (!json["operation"].is_array()) {
        std::cerr << "operation is not array !" << std::endl;
        return  -10;
    }

    std::ifstream https_conf_file(argv[2]);
    nlohmann::json https_json;

    try {
        https_conf_file >> https_json;
    } catch (nlohmann::json::exception& e) {
        std::cerr << argv[2] << ": invalid format (" << e.what() << ")" << std::endl;
        return  -6;
    }

    if (!https_json.contains("static-files-dir")) {
        std::cerr << fmt::format("Forgot to set \"static-files-dir\" in {}", argv[2]) << std::endl;
        return -7;
    }

    NodeId node = ECUNameToNode(json["node"]);    
    
    auto operations = json["operation"];
    std::vector<std::string> rpc_calls;
    
    for (auto& op : operations) {
        if (!op.is_object()         ||
            !op.contains("task")    ||
            !op["task"].is_string()) {
            std::cerr << "Invalid operation format !" << std::endl;
            return  -11;
        }

        std::string operation = op["task"];
        boost::algorithm::trim(operation);
        rpc_calls.push_back(operation);
    }

    auto manifests = json["manifest"];
    std::vector<manifest> pkgs;

    for (auto& mani : manifests) {
        if (!mani.is_object()                       ||
            !mani.contains("software_id")           ||
            !mani.contains("filename")              ||
            !mani.contains("version")               ||
            !mani.contains("delta")                 ||
            !mani.contains("original_version")      ||
            !mani.contains("type")                  ||
            !mani.contains("flashing")              ||
            !mani.contains("attrs")                 ||
            !mani["software_id"].is_string()        ||
            !mani["filename"].is_string()           ||
            !mani["version"].is_string()            ||
            !mani["delta"].is_boolean()             ||
            !mani["original_version"].is_string()   ||
            !mani["type"].is_number_unsigned()      ||
            !mani["flashing"].is_number_unsigned()  ||
            !mani["attrs"].is_array()) {
            std::cerr << "invalid manifest format !" << std::endl;
            return  -12;
        }

        manifest pkg;
        pkg.softwre_id  = mani["software_id"];
        pkg.filename    = mani["filename"];
        pkg.version     = mani["version"];
        pkg.delta       = mani["delta"];
        pkg.original_version = mani["original_version"];
        pkg.type        = mani["type"];
        pkg.flashing    = mani["flashing"];
        
        for (auto& attr : mani["attrs"]) {
            if (!attr.is_object()) {
                std::cerr << "Invalid attrs format !" << std::endl;
                return  -13;
            }

            for (auto& el : attr.items()) {
                pkg.attrs.push_back(std::make_pair(el.key(), el.value()));
            }
        }

        pkgs.push_back(pkg);
    }

    self_installer_mgr = GetUpdaterProxyMgr();
    auto receiver = std::make_shared<OrchestratorReceiver>();

    std::vector<std::pair<std::string, std::string>> versions;
    if (self_installer_mgr->Init(node, receiver, versions)) {
        std::cout << NodeToECUName(node) << " init successfully" << std::endl;
        std::cout << "The current versions: " << std::endl;
        for (auto& version : versions) {
            std::cout << "\t" << version.first << " : " << version.second << std::endl;
        }
    } else {
        std::cerr << NodeToECUName(node) << " is ready ?" << std::endl;
        return  -14;
    }

    while (true) {
        ShowMenu();

        char option;
        std::cin >> option;

        FotaError error;
        switch (option) {
            case '0':
                RunScript(node, rpc_calls, pkgs);
                break;
            case '1':
                error = self_installer_mgr->Rollback(node);
                if (error.error_code_ == ERROR_OK) {
                    std::cout << "Start to rollback !" << std::endl;
                } else {
                    DumpError(error);
                }
                
                break;
            case '2':
                DistributeFiles(node, https_ip, https_json);
                break;
            case '3':
                {
                    UpdateProgressState stat;
                    error = self_installer_mgr->GetState(node, stat);
                    if (error.error_code_ == ERROR_OK)
                        DumpStatus(stat);
                    else
                        DumpError(error);
                }
                break;
            case '4':
                {
                    std::string version;
                    std::cout << "Please input: ";
                    std::string software_id;
                    std::cin >> software_id;
                    error = self_installer_mgr->GetVersion(node, software_id, version);
                    if (error.error_code_ == ERROR_OK) {
                        std::cout << version << std::endl;
                    } else {
                        DumpError(error);
                    }
                    
                }
                break;
            case '5':
                {
                    std::vector<std::pair<std::string, std::string>> versions;
                    error = self_installer_mgr->GetAllSoftwareVersion(node, versions);
                    if (error.error_code_ == ERROR_OK) {
                        std::cout << "The current versions: " << std::endl;
                        for (auto& version : versions) {
                            std::cout << "\t" << version.first << " : " << version.second << std::endl;
                        }
                    } else {
                        DumpError(error);
                    }                    
                }
                break;
            case '6':
                return  0;
            default:
                std::cerr << "Error option: " << option << std::endl;
                break;
        }
    }

    return  0;
}
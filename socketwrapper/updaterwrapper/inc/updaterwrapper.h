#pragma once

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include "boost/filesystem.hpp"
#include "boost/smart_ptr/make_unique.hpp"
#include "interface/fota.h"
#include "updaterwrapper/inc/selfupdater.h"
#include "updaterwrapper/inc/downloadstatus.h"
#include "rpc_server.h"
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#ifdef PC_SIMULATION
#include "spdlog/sinks/stdout_color_sinks.h"
#else
#include "spdlog/sinks/rotating_file_sink.h"
#endif

using namespace rest_rpc::rpc_service;

// !!! The vendor need set the apposite log path 
#define SELFUPDATER_LOG    "/tmp/updaterwrapper.log"

namespace socketwrapper {

class UpdaterWrapper final:
    public IDownloadStatus,
    public std::enable_shared_from_this<UpdaterWrapper>
{
public:
    /**
     * @brief ctor 
     *  
     * @param [in] node which domain controller 
     * @param [in] updater the instance of implementing ISelfUpdater
     * @see NodeId 
     */
    UpdaterWrapper(NodeId node,
                   const std::shared_ptr<ISelfUpdater>& updater);

    /**
     * @brief dtor
     */
	virtual ~UpdaterWrapper() {}
	
    /**
     * @brief initialize updaterwrapper, make the connection from 
     *        the domain controller to CGW
     * @return if initialization successful, return true, otherwise 
     *         return false.
     */
	bool Init();
	
    /**
     * @brief updaterwrapper deinit, send leave message with leave 
     *        cause to CGW
     * @param [in] cause the leave cause 
     * @see UpdaterLeaveCause 
     */
	void Deinit(UpdaterLeaveCause cause);	
	
    /**
     * @brief Self-installer need to notify socketwrapper the 
     *        current status of self-installer
     * 
     * @param [in] state the current state needed to notified by 
     *        self-installer
     * @see UpdateProgressState_t 
     */
    void ReportStatus(const UpdateProgressState_t& state);

private:
    bool InitRpc(uint16_t rpc_port);

    // RPC handlers
    SELFINSTALLER_CODE PrepareUpgrade(rpc_conn conn, const Packages& pkgs);
	SELFINSTALLER_CODE Upgrade(rpc_conn conn, const Packages& pkgs);
    SELFINSTALLER_CODE Finalize(rpc_conn conn, const Packages& pkgs);
	SELFINSTALLER_CODE Rollback(rpc_conn conn);
	bool Distribute(rpc_conn conn, const DownloadFile& info);
    GetStateResult GetState(rpc_conn conn);
    GetVersionResult GetVersion(rpc_conn conn, const std::string& software_id);
    GetAllVersionsResult GetAllSoftwareVersion(rpc_conn conn);
    std::string GetErrorCause(rpc_conn conn, SELFINSTALLER_CODE error);

    std::shared_ptr<UpdaterWrapper> GetPtr() {
        return shared_from_this();
    }

    void ReportState(const UpdateProgressState_t& state);
    void StatusCallback(const std::string& uri, DownloadStatus status,
                        DownloadError error, int percent, const std::string& log);

    void Log(const std::string& operation, const Packages& pkgs);
    void Log(const std::string& operation, const UpdateProgressState_t& state);
    void Log(const GetAllVersionsResult& result);

    std::shared_ptr<spdlog::logger> GetLogger(const std::string& log_file);

private:
    NodeId                          node_;
	std::shared_ptr<ISelfUpdater> 	updater_;
    std::thread::id                 thread_id_;

    std::unique_ptr<rest_rpc::rpc_service::rpc_server>  rpc_server_;

    std::shared_ptr<spdlog::logger> logger_;
    

    boost::filesystem::path         saved_path_;
};

}	// end socketwrapper namespace




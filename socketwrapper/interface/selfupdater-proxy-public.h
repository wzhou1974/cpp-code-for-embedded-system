#pragma once

#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <array>
#include "interface/fota.h"
#include "interface/rpc.h"
#include "interface/error.h"

class IUpdaterNotification
{
public:
    virtual ~IUpdaterNotification() {}
    /**
     * @brief Self-installer could report upgrade status by the interface
     * @param [in] id domain controller ID
     * @param [in] upgrade status
     */
    virtual void UpdateStatus(NodeId id, const UpdateProgressState& status) = 0;

    /**
     * @brief Get the self-installer's the result of distribute operation
     * @param [in] id domain controller ID
     * @param [in] result
     * @see DownloadResult
     */
    virtual void DistributeStatus(NodeId id, const DownloadResult& result) = 0;

    /**
     * @brief Self-installer could report process exist cause, self-installer 
     *      need send the notification before it exit
     * @param [in] id domain controller ID
     * @param [in] self-install exist cause
     */
    virtual void Exit(NodeId id, UpdaterLeaveCause cause) = 0;
};

class IUpdaterProxyMgr
{
public:
    virtual ~IUpdaterProxyMgr() {}
    /**
     * @brief Initialize UpdaterMgr module
     * @param [in] id domain controller ID
     * @param [in] notifier orchestrator could get all kinds of notification from the interface
     * @param [in] versions if initialize successfully, versions will contains all packages version
     *          reported by the domain controller
     * @return return true, if initialize successfully
     * @see NodeId
     */
    virtual bool Init(NodeId id, const std::shared_ptr<IUpdaterNotification>& notifier,
                    std::vector<std::pair<std::string, std::string>>& versions) = 0;

    /**
     * @brief PrepareUpgrade operation
     * @param [in] id domain controller ID
     * @param [in] pkgs the packages wanted to upgrade  
     * @see FotaError
     */
    virtual FotaError PrepareUpgrade(NodeId id, const std::vector<manifest>& pkgs) = 0;

    /**
     * @brief Upgrade operation
     * @param [in] id domain controller ID
     * @param [in] pkgs the packages wanted to upgrade  
     * @see FotaError
     */
    virtual FotaError Upgrade(NodeId id, const std::vector<manifest>& pkgs) = 0;

    /**
     * @brief Finalize operation
     * @param [in] id domain controller ID
     * @param [in] pkgs the packages wanted to upgrade  
     * @see FotaError
     */
    virtual FotaError Finalize(NodeId id, const std::vector<manifest>& pkgs) = 0;

    /**
     * @brief Rollback operation
     * @param [in] id domain controller ID
     * @see FotaError
     */
    virtual FotaError Rollback(NodeId id) = 0;

    /**
     * @brief Distribute packages from https server
     * @param [in] id domain controller ID
     * @param [in] info The wanted to Distribute package identified by info
     * @see DownloadInfo
     */
    virtual bool Distribute(NodeId id, const DownloadFile& info) = 0;

    /**
     * @brief Get the self-installer state
     * @param [in] id domain controller ID
     * @param [out] state The current self-installer's state if FotaError.error_code_ == ERROR_OK
     * @return FotaError.error_code_ is ERROR_OK, if successfully
     * @see FotaError
     */
    virtual FotaError GetState(NodeId id, UpdateProgressState& state) = 0;

    /**
     * @brief Get the version
     * @param [in] id domain controller ID
     * @param [in] software_id which package
     * @param [out] version The version of the package identified by software_id
     * @return FotaError.error_code_ is ERROR_OK, if successfully
     * @see FotaError
     */
    virtual FotaError GetVersion(NodeId id, const std::string& software_id, std::string& version) = 0;

    /**
     * @brief Get the all packages version
     * @param [in] id domain controller ID
     * @param [out] versions The all packages version
     * @return FotaError.error_code_ is ERROR_OK, if successfully
     * @see FotaError
     */
    virtual FotaError GetAllSoftwareVersion(NodeId id, std::vector<std::pair<std::string, std::string>>& versions) = 0;
};

/**
 * @brief Get the Updater proxy instance, it's a singleton instance, don't delete it
 * @return Updater proxy instance pointer
 */
IUpdaterProxyMgr* GetUpdaterProxyMgr();
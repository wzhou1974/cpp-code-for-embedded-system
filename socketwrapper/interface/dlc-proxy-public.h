#pragma once

#include <string>
#include <memory>
#include <vector>
#include "interface/msg-passer-public.h"
#include "interface/rpc.h"

class IOrchestrator
{
public:
    // dtor
    virtual ~IOrchestrator() {}
    /**
     * @brief DLC notify orchestrator all packages in L1 manifests are downloaded successfully
     * @param [in] L1 L1 manifest
     * @param [in] info The downloaded packages location
     * @return return true, if orchestrator think everything is OK
     */
    virtual bool PackageReady(const std::string& L1, const std::vector<DownloadInfo>& info) = 0;

    /**
     * @brief HMI query whether the vehicle could upgrade
     * @return if there is no package to upgrade, return empty string, otherwise return L1 manifest
     */
    virtual std::string CheckNewPackage() = 0;

    /**
     * @brief HMI want to start upgrade operation
     * @param [in] L1 Start to upgrade the packages identified by L1
     * @return return true, if orchestrator think it could start to upgrade
     */
    virtual bool StartUpgrade(const std::string& L1) = 0;

    /**
     * @brief Get the result of IDLCProxy::DistributeFile(). If the file identified
     *        by url has been downloaded successfully, return true, otherwise return false.
     * @param [in] url which remote file is distributed
     * @param [in] result true if distributing successfully, otherwise false
     * @see IDLCProxy::DistributeFile
     */    
    virtual void DistributeResult(const std::string& url, bool& result) = 0;
};

class IDLCProxy
{
public:
    // dtor
    virtual ~IDLCProxy() {}
    /**
     * @brief Initialize DLC proxy module
     * @param [in] IOrchestrator CGW orchestrator module should provide the interface
     * @return return true, if DLC proxy initialize successfully
     */
    virtual bool Init(const std::shared_ptr<IOrchestrator>& orchestrator) = 0;

    /**
     * @brief Orchestrator module notifies DLC, it has downloaded all packages from https server
     */
    virtual void DownloadFinished() = 0;

    /**
     * @brief Orchestrator could report the upgrade result to Cloud by the interface
     * @param [in] result The result is json format and it is defined by Cloud 
     */
    virtual void UpdateResult(const std::string& result) = 0;

    /**
     * @brief Orchestrator could report the upgrade progress status to HMI by the interface
     * @param [in] status The status is json format and its detail fields need to be discussed (?)
     */
    virtual void UpdateStatus(const UpgradeStatusInternal& status) = 0;

    /**
     * @brief Download file from DLC's https server and save it into local_file.
     *        The function is ssynchronous.
     * @param [in] url the package file's url on DLC's https server
     * @param [in] local_file the saved file's path
     * @return return true means it accept to distribute file, otherwise return false.
     *        If the API's client want to know the distributing result, it could get result
     *        from IOrchestrator::DistributeResult().
     * @see IOrchestrator::DistributeResult
     */
    virtual bool DistributeFile(const std::string& url, std::string& local_file) = 0;
};

/**
 * @brief Get the DLC proxy instance, it's a singleton instance, don't delete it
 * @return DLC proxy instance pointer
 */
IDLCProxy* GetDLCProxy();
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "interface/hh.h"
#include "interface/fota.h"
#include "interface/rpc.h"
#include "interface/selfinstaller_error.h"

namespace socketwrapper {

class ISelfUpdater
{
public:
    virtual ~ISelfUpdater() {}

	/**
	 * @brief retrieve the specific softwware_id's version
     * @param [in] software_id get the specific software_id(partition) version
     * @param [out] version the returned version for the specific 
     *        software_id
     * @return  if OK, please return SELFINSTALLER_OK, otherwise 
     *          return the error code ( < 0)
     * @see GetErrorCause 
	 */
	virtual SELFINSTALLER_CODE GetVersion(const std::string& software_id, std::string& version) = 0;
	
    /**
     * @brief retrieve the versions for all software_ids
     * @param [out] versions the returned versions for the all 
     *        software_ids
     * @return  if OK, please return SELFINSTALLER_OK, otherwise 
     *          return the error code ( < 0)
     * @see GetErrorCause 
     */
	virtual SELFINSTALLER_CODE GetAllSoftwareVersion(std::vector<std::pair<std::string, std::string>>& versions) = 0;
	
    /**
     * @brief return the absolute path for saving the packages 
     *        downloaded from FTPS server, self-installer should
     *        ensure socketwrapper could create / modify / append
     *        files in the path
     * @return the absolute path for saving the packages  
     */
	virtual std::string GetPackageStorageLocation() = 0;
		
    /**
     * @brief Prepare for upgrading, the self-installer could do 
     *        anything before starting to upgrade, for example,
     *        BACKUP the current partition or do nothing.
     * @param [in] partitions the partitions wanted to prepare upgrading
     * @return  if OK, please return SELFINSTALLER_OK, otherwise 
     *          return the error code ( < 0), it could be retrieve
     *          error cause string by GetErrorCause() method.
     * @see GetErrorCause
     */
	virtual SELFINSTALLER_CODE PrepareUpgrade(const std::vector<manifest>& partitions) = 0;

    /**
     * @brief start to upgrade process, return SELFINSTALLER_OK if 
     *        self-installer think partitions installing are OK (Accept
     *        the request), otherwise return error code (< 0)
     * @param [in] partitions the partitions wanted to upgrade
     * @return if OK, please return SELFINSTALLER_OK, otherwise 
     *          return the error code ( < 0), it could be retrieve
     *          error cause string by GetErrorCause() method.
     * @see GetErrorCause 
     */
	virtual SELFINSTALLER_CODE Upgrade(const std::vector<manifest>& partitions) = 0;
	
    /**
     * @brief When the all upgrading are done for the vehicle, CGW would send the  
     *        notification by the API to the specific self-installer.
	 *
     * @param [in] partitions the partitions are same as Upgrade() and PrepareUpgrade()
	 *			APIs
     * @return if OK, please return SELFINSTALLER_OK, otherwise 
     *          return the error code ( < 0), it could be retrieve
     *          error cause string by GetErrorCause() method.
     * @see GetErrorCause 
     */
	virtual SELFINSTALLER_CODE Finalize(const std::vector<manifest>& partitions) = 0;

    /**
     * @brief Send rollback notification, return SELFINSTALLER_OK if 
     *        self-installer think package rollback is
     *        SELFINSTALLER_OK, otherwise return error code (< 0),
     *        fail to start backup, the return value is error code.
     *        it could retrieve error cause string by
     *        GetErrorCause() method
     * @param [in] partitions the partitions wanted to rollback
     * @return if OK, please return SELFINSTALLER_OK, otherwise 
     *          return the error code ( < 0), it could be retrieve
     *          error cause string by GetErrorCause() method.
     * @see GetErrorCause 
     */    	
	virtual SELFINSTALLER_CODE Rollback() = 0;
	
    /**
     * @brief retrieve the self-installer state 
     * @param [out] state  the current state of self-installer
     */
    virtual SELFINSTALLER_CODE GetState(UpdateProgressState_t& state) = 0;

    /**
     * @brief retrieve the error casue description for the specific 
     *        error code
     * @param [in] error the error code is interpreted by the vendor
     *        of the domain controller. It must guarantee 0 means
     *        SELFINSTALLER_OK. the error code's meaning could be
     *        retrieved by the method.
     * @return the description for the specific error code 
     */
    virtual std::string GetErrorCause(SELFINSTALLER_CODE error) = 0;
};

}	// end socketwrapper namespace

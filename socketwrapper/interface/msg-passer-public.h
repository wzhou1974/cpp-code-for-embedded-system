#pragma once
#include <memory>
#include <string>
#include <vector>
#include <utility>      // for std::pair

struct UpgradeStatus {
    std::string dev_id;
    std::string soft_id;
    std::string esti_time;
    std::string start_time;
    std::string time_stamp;
    std::string door_module;        // "yes" or "no"    
    std::string status;             // "pending", "in progress", "failed", "success"
    float progress_percent;         // raw percentage data, e.g., 0, 55, or 100
};

struct ReleaseNote
{
    std::string locale;
    std::string description;
};

/**
 * DMC shoudl implement IVehicleNotification interface to accept message and
 * upgrading status from vehicle
 */
class IVehicleNotification
{
public:
    // dtor
    virtual ~IVehicleNotification() {}
    /**
     * @brief DMC accepts the notification from vehicle and send it to Cloud 
     */
    virtual void ToCloud(const std::string& notification) = 0;

    /**
     * @brief Vehicle send upgrading status to HMI
     */
    virtual void ToHMI(const UpgradeStatus& status) = 0;
};

class IMessagePasser
{
public:
    // dtor
    virtual ~IMessagePasser() {}
    /**
     * @brief Initialize message passer module 
     */
    virtual bool Init(const std::shared_ptr<IVehicleNotification>& notifier) = 0;

    /**
     * @brief DMC trandfers the message from Cloud to vehicle
     */
    virtual bool ToVehicle(const std::string& message) = 0;

    /**
     * @brief DMC check whether there is new package identified by service_pack_id
     *        to upgrade.
     * @param [out] service_pack_id identify the new package to upgrade
     * @param [out] release_notes the release notes for the upgrading package.
     *        Because release notes support multi-language, it is an array.
     * @return return true if there is new package to upgrade, otherwise return false
     */
    virtual bool CheckNewPackage(std::string& service_pack_id,
                                std::vector<ReleaseNote>& release_notes) = 0;

    /**
     * @brief start to upgrade.
     * @param [in] service_pack_id which package to upgrade, the upgrading package is 
     *        identified by service_pack_id
     * @return return true if vehicle side think upgrading is OK, otherwise return false.
     */
    virtual bool StartUpgrade(const std::string& service_pack_id) = 0;
};

/**
 * @brief Message passer module's singleton instance, don't delete it
 */

IMessagePasser* GetMessagePasser();


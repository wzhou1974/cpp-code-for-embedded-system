#pragma once

#include "msgpack.hpp"
#include "interface/selfinstaller_error.h"

using namespace socketwrapper;

enum class UpdateState
{
    eIDLE,
    eDOWNLOADING,
    eDOWNLOAD_DONE,
    ePREPARATION,
    ePREPARATION_DONE,
    eACTIVATION,
    eACTIVATION_DONE,
    eFINAILIZE,
    eFINAILIZE_DONE,
    eROLLBACK,
    eROLLBACK_DONE,
    eERROR
};
MSGPACK_ADD_ENUM(UpdateState);

enum class UpdaterLeaveCause
{
	eABSwitch,
	eExit
};
MSGPACK_ADD_ENUM(UpdaterLeaveCause);

struct UpdateProgressState
{
    UpdateState state_;
    std::string	package_;
    int 		percent_;
    std::string log_;

    MSGPACK_DEFINE(state_, package_, percent_, log_);
};

using UpdateProgressState_t = struct UpdateProgressState;

enum class SoftwareType
{
    eImage,
    eFs,
    eFirmware,
    eOther
};
MSGPACK_ADD_ENUM(SoftwareType);

enum class FlashingType
{
    eImageDiff,
    eImageRestore,
    eFs
};
MSGPACK_ADD_ENUM(FlashingType);

struct manifest
{
    std::string     softwre_id;
    std::string     filename;
    std::string     version;
    bool            delta;
    std::string     original_version;
    SoftwareType    type;
    FlashingType    flashing;
    std::vector<std::pair<std::string, std::string>> attrs;

    MSGPACK_DEFINE(softwre_id, filename, version, delta, original_version, type, flashing, attrs);
};

struct GetVersionResult
{
    SELFINSTALLER_CODE  error_;
    std::string         version_;

    MSGPACK_DEFINE(error_, version_);
};

struct GetAllVersionsResult
{
    SELFINSTALLER_CODE  error_;
    std::vector<std::pair<std::string, std::string>>    versions_;

    MSGPACK_DEFINE(error_, versions_);
};

struct GetStateResult
{
    SELFINSTALLER_CODE      error_;
    UpdateProgressState     state_;
    
    MSGPACK_DEFINE(error_, state_);
};

struct DownloadFile
{
    std::string     uri_;       // wanted to download package's url on https server
    size_t          size_;      // package size
    std::string     checksum_;  // package checksum

    MSGPACK_DEFINE(uri_, size_, checksum_);
};

struct Packages
{
    std::vector<manifest> manifests_;

    MSGPACK_DEFINE(manifests_);
};

enum class DownloadStatus
{
    eDONE,
    eSTART,
    ePROGRESS,
    eFAIL
};
MSGPACK_ADD_ENUM(DownloadStatus);

enum class DownloadError
{
    eOK,
    eMD5,
    eSIZE,
    eSPACE,
    eFILEIO,
    eNOFILE,
    eRPC,
    eREPEAT
};
MSGPACK_ADD_ENUM(DownloadError);

struct DownloadResult
{
    std::string     uri_;
    DownloadStatus  status_;
    DownloadError   error_;

    MSGPACK_DEFINE(uri_, status_, error_);
};

struct DownloadInfo
{
    std::string url_cdn_;   // package's url on CDN
    std::string url_dlc_;   // package's url on DLC's https server
    size_t      size_;      // package file size
    MSGPACK_DEFINE(url_cdn_, url_dlc_, size_);
};

struct DownloadPackages
{
    std::vector<DownloadInfo>   packages_;
    MSGPACK_DEFINE(packages_);
};

struct PackageInfo 
{
	std::string door_module;
	std::string dev_id;
	std::string soft_id;
	MSGPACK_DEFINE(door_module, dev_id, soft_id);
};

struct ReleaseNoteInternal
{
    std::string locale;
    std::string description;
    MSGPACK_DEFINE(locale, description);
};

struct UpgradeInfo
{
	bool        				        status;
	std::string					        service_pack_id;
	std::vector<ReleaseNoteInternal>    release_notes;
	MSGPACK_DEFINE(status, service_pack_id, release_notes);
};

struct UpgradeStatusInternal {
    std::string dev_id;
    std::string soft_id;
    std::string esti_time;
    std::string start_time;
    std::string time_stamp;
    std::string door_module;        // "yes" or "no"
    std::string status;             // "pending", "in progress", "failed", "success"
    float progress_percent;         // raw percentage data, e.g., 0, 55, or 100
    MSGPACK_DEFINE(dev_id, soft_id, esti_time, start_time, time_stamp, door_module, status, progress_percent);
};

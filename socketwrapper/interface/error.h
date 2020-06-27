#pragma once

#include <string>

/*
    FotaError error = invoke-api();
    if (error.error_code_ == ERROR_OK) {
        ......
    } else {
        // error handling
    }
*/
#define ERROR_OK                    0

// error catagory
#define CATAGORY_SELFINSTALLER      1   // error for self-installer
#define CATAGORY_SELFUPDATER        2   // error for self-updater
#define CATAGORY_ORCHESTRATOR       3   // error for CGW orchestrator
#define CATAGORY_DLC                4   // error for IDCM dlc
#define CATAGORY_RPC                5   // error for rest_rpc

struct FotaError
{
    int             catagory_;
    int             error_code_;
    std::string     error_casue_;
};

// error code for RPC catagory
#define ERROR_RPC_OPERATION         1
#define ERROR_RPC_ROLLBACK          2
#define ERROR_RPC_GETSTATE          3
#define ERROR_RPC_GETVERSION        4
#define ERROR_RPC_GETALLVERSIONS    5

#include <string>
#include "inc/L1Manifest.h"
#include "inc/error.h"

namespace socketwrapper {

std::string DLCStatusReport::dump()
{
    // fake report

    std::string error_msg = R"(
        {
            "fotaProtocolVersion":"HHFOTA-0.1",
            "vehicleVersion":{
                "orchestrator":"0.1",
                "dlc":"0.1"
            },
            "upgradeResults":{
                "servicePack":"service pack name",
                "campaign":"campaign id",
                "downloadStartTime":"YYYYMMDD HHMMSS",
                "downloadFinishTime":"YYYYMMDD HHMMSS",
                "userConfirmationTime":"YYYYMMDD HHMMSS",
                "startTime":"YYYYMMDD HHMMSS",
                "finishTime":"YYYYMMDD HHMMSS",
                "result":"success/fail-recovered/fail-irrecoverable",
                "deviceReports":[
                    {
                        "ecu":"ecu id",
                        "softwareId":"software id",
                        "startTime":"YYYYMMDD HHMMSS",
                        "finishTime":"YYYYMMDD HHMMSS",
                        "previousVersion":"previous version id",
                        "result":"success/fail-recovered/fail-irrecoverable",
                        "targetVersion":"target version",
                        "currentVersion":"current version",
                        "logs":[
                            {
                                "timestamp":"YYYYMMDD HHMMSS",
                                "progress":50,
                                "errorCode":-1,
                                "trace":"log and traces"
                            }
                        ]
                    }
                ]
            }
        }
    )";

    return  error_msg;          
}

}	//	end namespace socketwrapper
#pragma once

#include "interface/rpc.h"

namespace socketwrapper {

class IDownloadStatus
{
public:
    virtual ~IDownloadStatus() {}
	virtual	void StatusCallback(const std::string& uri,
                                DownloadStatus status,
                                DownloadError error,
                                int percent,
                                const std::string& log) = 0;
};

}	// end socketwrapper namespace
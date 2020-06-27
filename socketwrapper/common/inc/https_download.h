#pragma once

#include "common/inc/download.h"

namespace socketwrapper {

class HttpsDownload : public Download
{
public:
	HttpsDownload(const std::string& url, size_t size, const boostfs::path& local)
	: Download(url, size, local)
	{		
	}

	bool DownloadFrom(loff_t from, PROGRESS reporter) override;
    bool DownloadFromTo(loff_t from, loff_t to, PROGRESS reporter);
};

bool HttpsDownloadToString(const std::string& url, std::string& content, size_t size);

}	// end socketwrapper namespace
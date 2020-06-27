#pragma once
#include "common/inc/download.h"

namespace socketwrapper {

class CurlDownload : public Download
{
public:
	CurlDownload(const std::string& url, size_t size, const boostfs::path& local)
	: Download(url, size, local)
	{		
	}

	bool DownloadFrom(loff_t from, PROGRESS reporter) override;
};

}	// end socketwrapper namespace

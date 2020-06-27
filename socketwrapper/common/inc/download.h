#pragma once

#include "boost/filesystem.hpp"	
namespace boostfs = boost::filesystem;

#include <string>
#include <memory>
#include <stdio.h>
#include <vector>
#include <functional>

namespace socketwrapper {

// Send "Content-Length" HTTPS server for getting the file size
// It's a conventional way.
bool GetFileSize(const std::string& url, size_t& size);

// The API is only for get the file size from https-server built from CINATRA libiary
// Because CINATRA uses the unconventional way to answer the "file size" query
bool GetFileSize2(const std::string& url, size_t& size);

#if __cplusplus >= 201703L
// use cinatra lib to get file size
bool GetFileSize3(const std::string& url, size_t& size);
#endif

using PROGRESS = std::function<void (size_t percent)>;

class Download
{
public:
	Download(const std::string& url, size_t size, const boostfs::path& local)
    : url_(url), size_(size), file_(local), step_(100)
    {
        progress_.reserve(step_);
        for (size_t i = 0; i < step_; i++)
            progress_[i] = false;
    }
    virtual ~Download() {}
	virtual bool DownloadFrom(loff_t from, PROGRESS reporter) = 0;
    
	boostfs::path GetFile() {
        return file_;
    }

protected:
	const std::string 	url_;
	size_t				size_;
	boostfs::path  	    file_;
	
	// for debugging
	size_t				step_;
	std::vector<bool>	progress_;
};

}	// end socketwrapper namespace

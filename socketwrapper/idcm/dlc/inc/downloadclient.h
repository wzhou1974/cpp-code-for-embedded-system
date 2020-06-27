#pragma once

#include "boost/filesystem.hpp"
namespace boostfs = boost::filesystem;

#include <string>
#include <vector>
#include "idcm/dlc/inc/error.h"
#include "idcm/dlc/inc/L1Manifest.h"
#include "cgw/dlc-proxy/inc/dlc-proxy.h"

namespace socketwrapper {

boostfs::path GetLocalFile(const std::string& url);

bool DownloadPackage(const std::string& url,
                    size_t size,
                    size_t current,
                    boostfs::path local);

bool DownloadAllPackages(const std::vector<Package>& pkgs,
                        const DownloadPackages& info,
                        DlError& error);

}	// end socketwrapper namespace

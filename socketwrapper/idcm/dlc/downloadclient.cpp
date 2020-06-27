#include <memory>
#include <assert.h>
#include <thread>
#include "interface/idcm-log.h"
#include "inc/L1Manifest.h"
#include "json/nlohmann/json.hpp"
#include "interface/msg-passer-public.h"
#include "idcm/dlc/inc/downloadclient.h"
#include "uri-library/uri.hh"
#include "inc/error.h"
#include "common/inc/utils.h"
#include "common/inc/https_download.h"

using namespace rest_rpc;
using namespace rpc_service;

#define DLC_MAJOR_VERSION   1
#define DLC_MINOR_VERSION   0

namespace socketwrapper {

std::string GetDLCVersion()
{
    return std::to_string(DLC_MAJOR_VERSION) + "." + std::to_string(DLC_MINOR_VERSION);
}

static boostfs::path GetSavedDir(void)
{
#ifdef ANDROID_FLATORM
    boostfs::path saved_path{"/shared/fota"};
    return  saved_path;
#else
    return  boostfs::temp_directory_path();
#endif
}

boostfs::path GetLocalFile(const std::string& url)
{
	uri package_uri(url);
	std::string path_on_server = "/" + package_uri.get_path();

    boostfs::path fake_path{path_on_server};
    return GetSavedDir() / fake_path.filename();
}

bool DownloadPackage(const std::string& url,
                    size_t size,
                    size_t current,
                    boostfs::path local)
{
    HttpsDownload worker(url, size, local);

    PROGRESS reporter = [] (size_t percent) { 
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "downloading %zu%%", percent);
    };

    worker.DownloadFrom(current, reporter);

    return boostfs::file_size(local) == size;
}

bool DownloadAllPackages(const std::vector<Package>& pkgs,
                        const DownloadPackages& info,
                        DlError& error)
{
    for (auto& pkg : pkgs) {
        size_t size;
        if (!GetFileSize2(pkg.url_, size)) {
            // wrong URL, package doesn't exist
            LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "Wrong URL: %s", pkg.url_.c_str());

            error.code_ = DLC_ERROR_FILE_NOTEXIST;
            error.url_ = pkg.url_;
            return  false;
        }

        auto local = GetLocalFile(pkg.url_);
        size_t current_size{0};
        if (boostfs::exists(local)) {
            // The package that wants to be downloaded does exist.

            current_size = boostfs::file_size(local);

            if (current_size == size) {
                // check md5 checksum
                if (md5sum(local.string()) == pkg.checksum_) {
                    // The local package is good, needn't download again
                    continue;
                } else {
                    // doesn't pass the md5 check, remove the bad local package
                    boostfs::remove(local);
                    current_size = 0;
                    assert(!boostfs::exists(local));
                } 
            } else if (current_size > size) {
                // The existed package has the wrong size

                // invalid size, remove it
                boostfs::remove(local);
                current_size = 0;
                assert(!boostfs::exists(local));
            } else {
                // need contine to download
                // do nothing at here
            }
        }

        #define MAX_REPEAT_DOWNLOAD_COUNT   100
        int repeat_count{0};
        bool success{false};

        do {
            LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "download %s [%d times] from %zu",
                    pkg.url_.c_str(), repeat_count + 1, current_size);

            assert(current_size < size);
            success = DownloadPackage(pkg.url_, size, current_size, local);
            repeat_count++;
            
            current_size = boostfs::file_size(local);
            if (!success) {                
                assert(current_size < size);
            } else {
                assert(current_size == size);
            }
        } while (!success && repeat_count < MAX_REPEAT_DOWNLOAD_COUNT);
        
        if (!success) {
            // fail to download the package
            LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s download fail [%zu]",
                    pkg.url_.c_str(), current_size);

            error.code_ = DLC_ERROR_DOWNLOAD_FAIL;
            error.url_ = pkg.url_;
            error.size_ = current_size;
            return  false;
        }

        assert(size == current_size);

        // check md5 checksum
        auto calc_md5 = md5sum(local.string());
        if (calc_md5 != pkg.checksum_) {
            // wrong md5 checksum
            LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s checksum error [%s : %s]",
                    pkg.url_.c_str(), pkg.checksum_.c_str(), calc_md5.c_str());

            error.code_ = DLC_ERROR_CHECKSUM;
            error.url_ = pkg.url_;
            return  false;
        }

        // to-do
        // verify signature and decryption        
    }

    return  true;
}


}	//	end namespace socketwrapper

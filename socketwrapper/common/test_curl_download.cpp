#include "common/inc/curl_download.h"
#include "common/inc/utils.h"
#include <iostream>
#include <strings.h>            // for strcasecmp
#include "fmt/format.h"
#include "uri-library/uri.hh"
#include "interface/idcm-log.h"
#include "boost/filesystem.hpp"

using namespace socketwrapper;

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cerr << fmt::format("{} external | internal url", argv[0]) << std::endl;
        return  -1;
    }

    size_t size;
    bool succ;

    if (strcasecmp(argv[1], "external") == 0) {
        succ = GetFileSize(argv[2], size);
    } else {
        succ = GetFileSize2(argv[2], size);
    }

    std::string url{argv[2]};

    if (!succ) {
        std::cerr << fmt::format("fail to get {} size !", url) << std::endl;
        return  -2;
    }

    std::cout << fmt::format("{} size: {}", url, size) << std::endl;

    uri remote_file(url);
    assert(remote_file.get_query().empty());
    assert(remote_file.get_fragment().empty());

    std::string path_on_server = "/" + remote_file.get_path();
    std::string filename = boostfs::path{path_on_server}.filename().string();

    auto current_path = boostfs::current_path();
    auto local_file = current_path / boostfs::path{filename};


    CurlDownload downloader(argv[2], size, local_file);

    PROGRESS reporter = [] (size_t percent) { 
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "downloading %zu%%", percent);
    };

    loff_t from;
    if (boost::filesystem::exists(local_file)) {
        from = boost::filesystem::file_size(local_file);
    } else {
        from = 0;
    }
        
    if (from < size) {
        if (downloader.DownloadFrom(from, reporter)) {
            LOG_PRINT(IDCM_LOG_LEVEL_INFO, "Download successfully");
            return  0;
        } else {
            LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "Download failed");
            return  -3;
        }
    }

    return  0;
}

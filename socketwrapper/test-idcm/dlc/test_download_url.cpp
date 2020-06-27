#include "common/inc/https_download.h"
#include "common/inc/utils.h"
#include "uri-library/uri.hh"
#include "boost/filesystem.hpp"
#include <string>
#include <iostream>

namespace fs = boost::filesystem;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << argv[0] << " url" << std::endl;
        return  1;
    }

    std::string url = argv[1];

    using namespace socketwrapper;

    auto temp_path = fs::temp_directory_path();

    uri package_uri(url);
    std::string path_on_server = "/" + package_uri.get_path();
    auto filename = fs::path{path_on_server}.filename();
    auto temp_file = temp_path / filename;

    if (fs::exists(temp_file)) {
        fs::remove(temp_file);
    }

    size_t size;
    if (GetFileSize2(url, size)) {
        HttpsDownload downloader(url, size, temp_file);
        downloader.DownloadFrom(0, [] (size_t percent) {
            std::cout << percent << std::endl;
        });
    } else {
        std::cout << "GetFileSize() failed" << std::endl;
    }

    return  0;
}
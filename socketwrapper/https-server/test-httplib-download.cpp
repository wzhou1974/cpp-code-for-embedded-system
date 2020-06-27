#include <iostream>
#include <vector>
#include <stdio.h>
#include "boost/filesystem.hpp"
#include <utility>
#include "uri-library/uri.hh"
#include "jthread/jthread.hpp"
#include "common/inc/utils.h"
#include "common/inc/https_download.h"
#include <thread>
#include <chrono>


using namespace socketwrapper;
namespace fs = boost::filesystem;

static void download_worker(const std::pair<std::string, fs::path>& info)
{
    size_t size;
//    if (GetFileSize3(info.first, size)) {
    if (GetFileSize2(info.first, size)) {
        std::cout << "size: " << size << std::endl;
    }

    fs::path local_path{info.second};
    if (fs::exists(local_path)) {
        fs::remove(local_path);
    }

    HttpsDownload downloader(info.first, size, local_path);

    static constexpr size_t MAX_REPEAT_COUNT = 10;
    size_t repeat_count{0};

    PROGRESS reporter = [] (size_t percent) { 
        
    };

    loff_t current{0};

    while (repeat_count < MAX_REPEAT_COUNT) {
        auto succ = downloader.DownloadFrom(current, reporter);
        if (succ)
            break;

        repeat_count++;
        current = fs::file_size(local_path);
        assert(current < size);
    }

    assert(fs::file_size(local_path) == size);
    return;
}

int main(int argc, char *argv[])
{
    std::vector<std::pair<std::string, fs::path>> wanted;

    for (int i = 1; i < argc; i++) {
        std::string url{argv[i]};
        uri package_uri(url);
        std::string path_on_server = "/" + package_uri.get_path();
        fs::path wanted_file = fs::current_path() / fs::path{path_on_server}.filename();

        wanted.push_back(std::make_pair(url, wanted_file));
    }

    std::vector<std::jthread>   threads(wanted.size());
    int n{0};
    for (auto& file : wanted) {
        std::cout << "Downloading " << file.first << std::endl;
        threads[n++] = std::move(std::jthread(download_worker, file));
    }

    std::cout << "waiting ..." << std::endl;

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "All download done" << std::endl;
    return 0;
}


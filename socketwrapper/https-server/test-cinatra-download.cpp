#include <iostream>
#include <vector>
#include <stdio.h>
#include <filesystem>
#include <utility>
#include "uri-library/uri.hh"
#include "cinatra.hpp"
#include "jthread/jthread.hpp"
#include "common/inc/utils.h"

using namespace cinatra;
namespace fs = std::filesystem;

static void download_worker(const std::pair<std::string, fs::path>& info)
{
    std::unique_ptr<FILE, decltype(&std::fclose)> file_ptr {
        fopen64(info.second.string().c_str(), "w+"),
        &fclose
    };

    auto client = cinatra::client_factory::instance().new_client();

    socketwrapper::SyncLock lock;
    client->download(info.first, info.second.string(), [&info, &lock] (response_data data) {
        if (data.ec) {
            std::cout << info.first << " error: " << data.ec.message() << std::endl;
            return;
        }

        std::cout << "end download" << std::endl;
        lock.Resume();        
    });

    lock.Pend();

    std::cout << info.first << " download done !" << std::endl;
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

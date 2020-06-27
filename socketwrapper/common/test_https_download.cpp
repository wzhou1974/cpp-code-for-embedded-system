#include "common/inc/https_download.h"
#define CATCH_CONFIG_MAIN
#include "catch2/inc/catch.hpp"
#include "common/inc/utils.h"
#include <iostream>
#include <random>
#include <array>
#include <stdlib.h>
#include <string.h>     // for memset
#include "fmt/format.h"

using namespace socketwrapper;

TEST_CASE("https fownload 1", "download small file")
{
    std::string url{"https://127.0.0.1/1M.img"};

    size_t size;
    bool res = GetFileSize2(url, size);
    REQUIRE(res == true);

    auto temp_path = boostfs::temp_directory_path();
    auto file = temp_path / "1M.img";

    if (boostfs::exists(file)) {
        boostfs::remove(file);
    }

    HttpsDownload small_file(url, size, file);

    PROGRESS reporter = [] (size_t percent) { std::cout << percent << std::endl; };
    res = small_file.DownloadFrom(0, reporter);
    REQUIRE(res == true);
    REQUIRE(file == small_file.GetFile());
    REQUIRE(boostfs::file_size(file) == size);
    REQUIRE(md5sum(file.string()) == "b6d81b360a5672d80c27430f39153e2c");
}

TEST_CASE("https download 2", "download big file")
{
    std::string url{"https://127.0.0.1/50M.img"};

    size_t size;
    bool res = GetFileSize2(url, size);
    REQUIRE(res == true);

    auto temp_path = boostfs::temp_directory_path();
    auto file = temp_path / "50M.img";

    if (boostfs::exists(file)) {
        boostfs::remove(file);
    }

    HttpsDownload big_file(url, size, file);

    PROGRESS reporter = [] (size_t percent) { std::cout << percent << std::endl; };

    SECTION("download from start") {
        res = big_file.DownloadFrom(0, reporter);
        REQUIRE(res == true);
        REQUIRE(file == big_file.GetFile());
        REQUIRE(boostfs::file_size(file) == size);
        REQUIRE(md5sum(file.string()) == "25e317773f308e446cc84c503a6d1f85");
    }

    SECTION("download from random start") {
        std::random_device r;
        std::default_random_engine e1(r());
        std::uniform_int_distribution<int> uniform_dist(1, size);
        int start = uniform_dist(e1);

        std::cout << "Download from: " << start << std::endl;

        res = big_file.DownloadFrom(start, reporter);
        REQUIRE(res == true);

        size_t part_size = size - start;
        REQUIRE(boostfs::file_size(file) == part_size);

        std::string tmp_file = (temp_path / "tmp-XXXXXX").string();
        auto tmp_fd = mkstemp(const_cast<char *>(tmp_file.c_str()));

        {
            using FILE_PTR = std::unique_ptr<FILE, decltype(&std::fclose)>;
            FILE_PTR file(fdopen(tmp_fd, "a"), &fclose);

            std::array<char, 1024>  buffer;        
            for (auto i = 0; i < part_size / buffer.size(); ++i) {
                memset(buffer.data(), 0, buffer.size());
                fwrite(buffer.data(), 1, buffer.size(), file.get());
            }

            if (auto remainder = part_size % buffer.size()) {
                memset(buffer.data(), 0, remainder);
                fwrite(buffer.data(), 1, remainder, file.get());
            }
        }

        REQUIRE(md5sum(file.string()) == md5sum(tmp_file));
    }
}

// The test is not compatible with https-server implemented by cinatra library
// The current cinatra only support download from position to file end
TEST_CASE("https download 3", "test partal download")
{
    /*
    std::string url{"https://127.0.0.1/700M.img"};

    size_t size;
    bool res = GetFileSize2(url, size);
    REQUIRE(res == true);

    auto temp_path = boostfs::temp_directory_path();
    auto file = temp_path / "700M.img";

    if (boostfs::exists(file)) {
        boostfs::remove(file);
    }

    HttpsDownload big_file(url, size, file);
    PROGRESS reporter = [] (size_t percent) { std::cout << percent << std::endl; };

    size_t block_size = size / 10;

    for (int i = 0; i < 10; i++) {
        loff_t from = i * block_size;
        loff_t to = (i + 1) * block_size - 1;

        std::cout << fmt::format("Download from {} to {}", from, to);
        REQUIRE(big_file.DownloadFromTo(from, to, reporter) == true);
        REQUIRE(boostfs::file_size(file) == to);
        std::cout << "file size: " << boostfs::file_size(file) << std::endl;
    }

    if (size % block_size) {
        REQUIRE(big_file.DownloadFromTo(boostfs::file_size(file), size - 1, reporter) == true);
    }

    REQUIRE(boostfs::file_size(file) == size);
    REQUIRE(md5sum(file.string()) == "7fe5ca2a051d6dbb9ef191fbee0af98c");

    */
}

TEST_CASE("https download 4", "test download file to string")
{
    std::string url{"https://127.0.0.1/1M.img"};

    std::string content;

    size_t size;
    bool res = GetFileSize2(url, size);
    REQUIRE(res == true);

    REQUIRE(HttpsDownloadToString(url, content, size) == true);

    for (auto c : content) {
        REQUIRE(c == '\0');
    }
}
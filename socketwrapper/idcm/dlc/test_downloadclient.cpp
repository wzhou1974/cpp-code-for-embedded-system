#include <string>
#define CATCH_CONFIG_MAIN
#include "catch2/inc/catch.hpp"
#include "common/inc/utils.h"
#include "idcm/dlc/inc/downloadclient.h"
#include "idcm/dlc/inc/L1Manifest.h"

using namespace socketwrapper;

TEST_CASE("downloadclient test 1", "test DownloadPackage function")
{
    std::string url_1M{"https://127.0.0.1/1M.img"};

    auto local_file_1M = GetLocalFile(url_1M);
    if (boostfs::exists(local_file_1M))
        boostfs::remove(local_file_1M);

    REQUIRE(DownloadPackage(url_1M, 1048576, 0, local_file_1M) == true);
    REQUIRE(boostfs::file_size(local_file_1M) == 1048576);
    REQUIRE(md5sum(local_file_1M.string()) == "b6d81b360a5672d80c27430f39153e2c");

    std::string url_50M{"https://127.0.0.1/50M.img"};

    auto local_file_50M = GetLocalFile(url_50M);
    if (boostfs::exists(local_file_50M))
        boostfs::remove(local_file_50M);

    REQUIRE(DownloadPackage(url_50M, 52428800, 0, local_file_50M) == true);
    REQUIRE(boostfs::file_size(local_file_50M) == 52428800);
    REQUIRE(md5sum(local_file_50M.string()) == "25e317773f308e446cc84c503a6d1f85");
}

TEST_CASE("downloadclient test 2", "test download full files")
{
    std::vector<Package> pkgs(3);
    pkgs[0].url_ = "https://127.0.0.1/1M.img";
    pkgs[0].checksum_ = "b6d81b360a5672d80c27430f39153e2c";

    pkgs[1].url_ = "https://127.0.0.1/50M.img";
    pkgs[1].checksum_ = "25e317773f308e446cc84c503a6d1f85";

    pkgs[2].url_ = "https://127.0.0.1/700M.img";
    pkgs[2].checksum_ = "7fe5ca2a051d6dbb9ef191fbee0af98c";

    DlError             error;
    DownloadPackages    info;
    REQUIRE(DownloadAllPackages(pkgs, info, error) == true);
}
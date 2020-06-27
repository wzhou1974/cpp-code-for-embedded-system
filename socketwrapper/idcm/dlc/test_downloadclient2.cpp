#include <string>
#include <iostream>
#include "common/inc/utils.h"
#include "idcm/dlc/inc/downloadclient.h"
#include "idcm/dlc/inc/L1Manifest.h"
#include "cgw/dlc-proxy/inc/dlc-proxy.h"

using namespace socketwrapper;

int main()
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
    if (DownloadAllPackages(pkgs, info, error)) {
        std::cout << "Download all packages successfully!" << std::endl;
        return  0;
    } else {
        std::cerr << "Fail to download " << error.url_ << ", error code: " << error.code_ << std::endl;
        return  -1;
    }
}
#include <filesystem>
#include <iostream>
#include "cpp-httplib-modified/httplib.h"
#include "common/inc/download.h"
#include "uri-library/uri.hh"
#include "cinatra.hpp"
#include "boost/numeric/conversion/cast.hpp"    // for boost::numeric_cast

using namespace socketwrapper;
using namespace cinatra;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " url 1 | 2 | 3" << std::endl;
        return  -1;
    }

    std::string url{argv[1]};
    std::string method{argv[2]};

    size_t size;
    bool succ;

    if (method == "1") {
        succ = GetFileSize(url, size);
    } else if (method == "2") {
        succ = GetFileSize2(url, size);
    } else {
        succ = GetFileSize3(url, size);
    }

    if (succ)
        std::cout << "file size: " << size << std::endl;
    else
        std::cout << "No file" << std::endl;

    return  0;
}
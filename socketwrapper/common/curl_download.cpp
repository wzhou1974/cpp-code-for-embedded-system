#include <fstream>
#include <cassert>
#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <strings.h>            // for strcasecmp
#include "curlite/curlite.hpp"
#include "interface/idcm-log.h"
#include "common/inc/download.h"
#include "uri-library/uri.hh"
#include "boost/numeric/conversion/cast.hpp"    // for boost::numeric_cast
#include "fmt/format.h"
#include "common/inc/curl_download.h"
#include "picohttpparser/picohttpparser.h"
#include "boost/range.hpp"      // for boost::size
#include "boost/algorithm/string/replace.hpp"

namespace socketwrapper {

bool GetFileSize(const std::string& url, size_t& size)
{
    try {
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "Get %s size", url.c_str());

        curlite::Easy easy;

        easy.set(CURLOPT_URL, url);
        easy.set(CURLOPT_NOBODY, 1);
    #ifndef NDEBUG
        easy.set(CURLOPT_VERBOSE, 1);
    #endif        
        easy.set(CURLOPT_NOPROGRESS, 1);
        easy.set(CURLOPT_HEADER, 1);

        easy.set(CURLOPT_SSL_VERIFYPEER, 0);
        easy.set(CURLOPT_SSL_VERIFYHOST, 0);

        easy.perform();

        long res_code = easy.getInfo<long>(CURLINFO_RESPONSE_CODE);
        if (res_code >= 200 && res_code < 300) {
            
            double d_size;
            d_size = easy.getInfo<double>(CURLINFO_CONTENT_LENGTH_DOWNLOAD);
            size = boost::numeric_cast<size_t>(d_size);
            
            LOG_PRINT(IDCM_LOG_LEVEL_INFO, "%s", fmt::format("{} size: {}", url, size).c_str());

            return  true;
        } else {
            LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s", fmt::format("HEAD {}: {}", url, res_code).c_str());
            return  false;
        }
    }
    catch(boost::bad_numeric_cast& e) {
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s", e.what());
        return  false;
    }
    catch(std::invalid_argument &e) {
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s", e.what());
        return  false;
    } 
    catch(std::out_of_range &e) {
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s", e.what());
        return  false;
    }
    catch(std::exception &e) {
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s", e.what());
        return  false;
    }
}

bool GetFileSize2(const std::string& url, size_t& size)
{
    try {
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "Get %s size: ", url.c_str());

        uri remote_file(url);
        assert(remote_file.get_query().empty());
        assert(remote_file.get_fragment().empty());        

        std::string path_on_server = "/" + remote_file.get_path();
        std::string filename = boostfs::path{path_on_server}.filename().string();

        std::string get_file_size_url = boost::algorithm::replace_last_copy(url,
                        remote_file.get_path(), "file_size");

        curlite::Easy easy;
        
        easy.set(CURLOPT_URL, get_file_size_url);

    #ifndef NDEBUG
        easy.set(CURLOPT_VERBOSE, 1);
    #endif

        easy.set(CURLOPT_NOPROGRESS, 1);

        easy.set(CURLOPT_SSL_VERIFYPEER, 0);
        easy.set(CURLOPT_SSL_VERIFYHOST, 0);

        struct curl_slist *header = NULL;
        header = curl_slist_append(header, fmt::format("filename:{}", filename).c_str());
        easy.set(CURLOPT_HTTPHEADER, header);

        easy.set(CURLOPT_HTTPGET, 1);

        easy.onWrite_([&size] (char *response, size_t res_size) -> bool {
            // the file size should be < 10 ^ 11 (90G)
            assert(res_size < 11);

            std::string res(response, res_size);
            size = std::stol(res);
            return  true;
        });
        easy.perform();
        long res_code = easy.getInfo<long>(CURLINFO_RESPONSE_CODE);
        if (res_code >= 200 && res_code < 300) {
            LOG_PRINT(IDCM_LOG_LEVEL_INFO, "%s", fmt::format("{} size: {}", url, size).c_str());
            return  true;
        } else {
            return  false;
        }            
    } 
    catch(std::invalid_argument& e) {
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s", e.what());
        return  false;
    }
    catch(std::exception &e) {
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s", e.what());
        return  false;
    }
}

bool CurlDownload::DownloadFrom(loff_t from, PROGRESS reporter)
{
    try {
        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "download from %zu: ", from);

        // Created if doesn't exist and update at end
        std::ofstream out(file_.string(), std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
        
        curlite::Easy easy;
        easy.set(CURLOPT_URL, url_);
    #ifndef NDEBUG
        easy.set(CURLOPT_VERBOSE, 1);
    #endif

        if (from != 0) {
            static_assert(sizeof(loff_t) == sizeof(curl_off_t));
            static_assert(sizeof(long) == sizeof(curl_off_t));

            curl_off_t start = static_cast<curl_off_t>(from);

            curl_easy_setopt(easy.get(), CURLOPT_RESUME_FROM_LARGE, start);
        }            

        easy.set(CURLOPT_SSL_VERIFYPEER, 0);
        easy.set(CURLOPT_SSL_VERIFYHOST, 0);

        easy.onWrite_( [&out]( char *data, size_t size ) -> bool
        {
            out.write(data, size);
            return out.good();
        });

        easy.onProgress_([&reporter, this] (curl_off_t dTotal, curl_off_t dCurrent, curl_off_t, curl_off_t) -> bool {
            reporter(dCurrent * 100 / size_);
            return  true;
        });

        easy.perform();

        double d_size;
        d_size = easy.getInfo<double>(CURLINFO_SIZE_DOWNLOAD);
        size_t size = boost::numeric_cast<size_t>(d_size);

        LOG_PRINT(IDCM_LOG_LEVEL_INFO, "Downloaded %zu bytes", size);
        return  true;
    }
    catch(boost::bad_numeric_cast& e) {
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s", e.what());
        return  false;
    }
    catch(std::exception &e) {
        LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s", e.what());
        return  false;
    }
}

}	// end socketwrapper namespace

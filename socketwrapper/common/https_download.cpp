#include <fstream>
#include <cassert>
#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <iostream>
#include "cpp-httplib-modified/httplib.h"
#include "interface/idcm-log.h"
#include "common/inc/https_download.h"
#include "uri-library/uri.hh"
#include "boost/numeric/conversion/cast.hpp"    // for boost::numeric_cast
#include "fmt/format.h"

#if __cplusplus >= 201703L
	#include "cinatra.hpp"		// cinatra lib need c++17 feature
#endif

namespace socketwrapper {

bool GetFileSize(const std::string& url, size_t& size)
{
	uri package_uri(url);
	std::string path_on_server = "/" + package_uri.get_path();
	std::string filename = boostfs::path{path_on_server}.filename().string();
		
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    int port {443};
#else
    int port {80};
#endif

	if (package_uri.get_port() != 0)
		port = package_uri.get_port();

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    httplib::SSLClient client(package_uri.get_host(), port);
#else
    httplib::Client client(package_uri.get_host(), port);
#endif

	auto res = client.Head(path_on_server.c_str());
	if (res && res->status == 200) {
		assert(res->has_header("Content-Length"));
		std::string filesize{res->get_header_value("Content-Length")};
		size = std::stoull(filesize);
		return true;
	} else {
		if (res)
			LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s - %s", url.c_str(),
		          	httplib::detail::status_message(res->status));
		else
			LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "no response!!!");

		return false;
	}
}

bool GetFileSize2(const std::string& url, size_t& size)
{
	uri remote_file(url);
	std::string path_on_server = "/" + remote_file.get_path();
	std::string filename = boostfs::path{path_on_server}.filename().string();

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    int port {443};
#else
    int port {80};
#endif

	if (remote_file.get_port() != 0)
		port = remote_file.get_port();

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    httplib::SSLClient client(remote_file.get_host(), port);
#else
    httplib::Client client(remote_file.get_host(), port);
#endif
	
    httplib::Headers headers = {
        { "filename", filename }
    };
	auto res = client.Get("/file_size", headers);
    if (res) {
        // my https server always return "200"
        assert(res->status == 200);
		assert(!res->body.empty());
		// the file size should be < 10 ^ 11 (90G)
		assert(res->body.size() < 11);

		long file_size{0};
		try {
			file_size = std::stol(res->body);
		} catch (std::invalid_argument& e) {
			assert(false);
			return false;
		}
         
        if (file_size == -1) {
            return  false;
        } else {
            assert(file_size > 0);
            size = boost::numeric_cast<size_t>(file_size);
            return true;
        }
    } else {
        assert(false);
        return  false;
    }
}

#if __cplusplus >= 201703L
bool GetFileSize3(const std::string& url, size_t& size)
{
	using namespace cinatra;

	uri remote_file(url);
	std::string path_on_server = "/" + remote_file.get_path();
	std::string filename = boostfs::path{path_on_server}.filename().string();

    auto client = client_factory::instance().new_client();
    client->add_header("filename", filename);

    std::string get_file_size_url = remote_file.get_scheme() + "://" + remote_file.get_host();
    if (remote_file.get_port() != 0) {
        get_file_size_url += ":";
        get_file_size_url += std::to_string(remote_file.get_port());
    }
    get_file_size_url += "/file_size";

    auto result = client->request(http_method::HEAD, get_file_size_url);
    long res = std::stol(result.resp_body.data());
    if (res == -1)
        return  false;
    else {
        size = static_cast<size_t>(res);
        return  true;
    }
}
#endif

bool HttpsDownload::DownloadFrom(loff_t from, PROGRESS reporter)
{
	return	DownloadFromTo(from, size_ - 1, reporter);
}

bool HttpsDownload::DownloadFromTo(loff_t from, loff_t to, PROGRESS reporter)
{
	assert(from < size_);
	assert(to < size_);
	assert(from < to);

	using FILE_PTR = std::unique_ptr<FILE, decltype(&std::fclose)>;

	FILE_PTR file { fopen(file_.string().c_str(), "a"), &fclose };

	auto download_handler = [&] (const char *data, size_t data_length) -> bool {
		fwrite(data, 1, data_length, file.get());

	#ifdef MY_DEBUG
		std::cout << fmt::format("Current file position: {}", ftello64(file.get())) << std::endl;
	#endif
		return true;
	};
	
	auto report_progress = [this, &file, &reporter] (uint64_t current, uint64_t total) -> bool {

	#ifdef __ANDROID__

	#if defined(__arm64__) || defined(__aarch64__)
		size_t percent = std::min(current / (total / step_), step_ - 1);
	#else
		// I don't know why c++ lib for clang++ doesn't define std::min on armeabi-v7
		auto my_min = [] (uint64_t a, uint64_t b) { return (b < a) ? b : a; };
		size_t percent = my_min(current / (total / step_), step_ - 1);
	#endif

	#else
		size_t percent = std::min(current / (total / step_), step_ - 1);
	#endif		

		if (!progress_[percent]) {
			progress_[percent] = true;
			// sync downloading file
			fsync(fileno(file.get()));

			if (reporter)
				reporter(percent);
		}

		return	true;
	};

	uri	package_uri(url_);
	std::string server_path = "/" + package_uri.get_path();

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
	int port{443};	// HTTPS well-known port
#else
	int port{80};	// HTTP well-known port
#endif	

	if (package_uri.get_port() != 0)
		port = package_uri.get_port();

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
	httplib::SSLClient client(package_uri.get_host(), port);
#else
	httplib::Client client(package_uri.get_host(), port);
#endif

	auto res = client.Get(server_path.c_str(),{ httplib::make_range_header({{from, to}}) },
						  download_handler, report_progress);

	if (res) {
		// 200 means OK, 206 means Partial Content
		if (res->status == 200 || res->status == 206) {
			LOG_PRINT(IDCM_LOG_LEVEL_INFO, "%s [%s] download successfully",
					url_.c_str(), file_.string().c_str());

			return	true;
		} else {
			LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "%s - Get error - [%s]",
						url_.c_str(), httplib::detail::status_message(res->status));

			return	false;
		}
	} else {
		assert(false);
		return	false;
	}
}

// Download the file contents pointed by url into the 'content' string
bool HttpsDownloadToString(const std::string& url, std::string& content, size_t size)
{
	assert(content.empty());

	uri	file_uri(url);

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
	int port{443};	// HTTPS well-known port
#else
	int port{80};	// HTTP well-known port
#endif	

	if (file_uri.get_port() != 0)
		port = file_uri.get_port();

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
	httplib::SSLClient client(file_uri.get_host(), port);
#else
	httplib::Client client(file_uri.get_host(), port);
#endif
	
	std::string server_path = "/" + file_uri.get_path();

	auto res = client.Get(server_path.c_str(),
			[&content] (const char *data, size_t data_length) {
				content.append(data, data_length);
				return	true;
			});	
	assert(size == content.length());

	if (size != content.length()) {
		LOG_PRINT(IDCM_LOG_LEVEL_ERROR, "size doesn't match: [%s] %zu - %zu", 
			url.c_str(), size, content.length());
		return	false;				
	} else {
		return	true;				
	}
}

}	//	end namespace socketwrapper

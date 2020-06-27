#include "common/inc/https_download.h"
#define CATCH_CONFIG_MAIN
#include "catch2/inc/catch.hpp"
#include "common/inc/utils.h"
#include "boost/filesystem.hpp"

TEST_CASE("test filedownload", "https download") {

	using namespace socketwrapper;	

	std::string url_1M{"https://127.0.0.1/1M.img"};
	std::string url_50M{"https://127.0.0.1/50M.img"};
	std::string url_700M{"https://127.0.0.1/700M.img"};

	const size_t file_1M = 1048576;
	const size_t file_50M = 52428800;
	const size_t file_700M = 734003200;

	size_t size;
	REQUIRE(GetFileSize2(url_1M, size) == true);
	REQUIRE(size == file_1M);

	REQUIRE(GetFileSize2(url_50M, size) == true);
	REQUIRE(size == file_50M);

	REQUIRE(GetFileSize2(url_700M, size) == true);
	REQUIRE(size == file_700M);

	auto temp_path = boost::filesystem::temp_directory_path();
	auto local_1M = temp_path / "1M.img";
	auto local_50M = temp_path / "50M.img";
	auto local_700M = temp_path / "700M.img";
	
	if (boost::filesystem::exists(local_1M))
		boost::filesystem::remove(local_1M);
	
	if (boost::filesystem::exists(local_50M))
		boost::filesystem::remove(local_50M);
	
	if (boost::filesystem::exists(local_700M))
		boost::filesystem::remove(local_700M);

	SECTION("download 1M image file") {
		HttpsDownload downloader(url_1M, file_1M, local_1M);
		downloader.DownloadFrom(0, [] (size_t percent) {
			std::cout << percent << std::endl;
		});

		REQUIRE(boost::filesystem::exists(local_1M));
		REQUIRE(boost::filesystem::file_size(local_1M) == file_1M);		
		REQUIRE(md5sum(local_1M.string()) == "b6d81b360a5672d80c27430f39153e2c");
	}

	SECTION("download 50M image file") {
		HttpsDownload downloader(url_50M, file_50M, local_50M);
		downloader.DownloadFrom(0, [] (size_t percent) {
			std::cout << percent << std::endl;
		});
		
		REQUIRE(boost::filesystem::exists(local_50M));
		REQUIRE(boost::filesystem::file_size(local_50M) == file_50M);
		REQUIRE(md5sum(local_50M.string()) == "25e317773f308e446cc84c503a6d1f85");
	}

	SECTION("download 700M image file") {
		HttpsDownload downloader(url_700M, file_700M, local_700M);
		downloader.DownloadFrom(0, [] (size_t percent) {
			std::cout << percent << std::endl;
		});
		
		REQUIRE(boost::filesystem::exists(local_700M));
		REQUIRE(boost::filesystem::file_size(local_700M) == file_700M);
		REQUIRE(md5sum(local_700M.string()) == "7fe5ca2a051d6dbb9ef191fbee0af98c");
	}
}


 



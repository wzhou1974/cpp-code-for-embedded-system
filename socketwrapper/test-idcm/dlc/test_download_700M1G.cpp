#include "common/inc/https_download.h"
#define CATCH_CONFIG_MAIN
#include "catch2/inc/catch.hpp"
#include "common/inc/utils.h"
#include "boost/filesystem.hpp"

TEST_CASE("test filedownload", "https download") {

	using namespace socketwrapper;	

	std::string url_700M{"https://127.0.0.1/700M.img"};
	std::string url_1G{"https://127.0.0.1/1G.img"};

	const size_t file_700M = 734003200;
	const size_t file_1G = 1073741824;

	size_t size;
	REQUIRE(GetFileSize2(url_700M, size) == true);
	REQUIRE(size == file_700M);

	REQUIRE(GetFileSize2(url_1G, size) == true);
	REQUIRE(size == file_1G);
		
	auto temp_path = boost::filesystem::temp_directory_path();
	auto local_700M = temp_path / "700M.img";
	auto local_1G = temp_path / "1G.img";
	
	if (boost::filesystem::exists(local_700M))
		boost::filesystem::remove(local_700M);
	
	if (boost::filesystem::exists(local_1G))
		boost::filesystem::remove(local_1G);
	
	SECTION("download 700M image file") {
		HttpsDownload downloader(url_700M, file_700M, local_700M);
		downloader.DownloadFrom(0, [] (size_t percent) {
			std::cout << percent << std::endl;
		});
		
		REQUIRE(boost::filesystem::exists(local_700M));
		REQUIRE(boost::filesystem::file_size(local_700M) == file_700M);
		REQUIRE(md5sum(local_700M.string()) == "7fe5ca2a051d6dbb9ef191fbee0af98c");
	}

	SECTION("download 1G image file") {
		HttpsDownload downloader(url_1G, file_1G, local_1G);
		downloader.DownloadFrom(0, [] (size_t percent) {
			std::cout << percent << std::endl;
		});
		
		REQUIRE(boost::filesystem::exists(local_1G));
		REQUIRE(boost::filesystem::file_size(local_1G) == file_1G);
		REQUIRE(md5sum(local_1G.string()) == "cd573cfaace07e7949bc0c46028904ff");
	}
}


 



#include <fstream>
#include "idcm/dlc/inc/L1Manifest.h"
#define CATCH_CONFIG_MAIN
#include "catch2/inc/catch.hpp"

TEST_CASE("test L1 Manifest", "check L1 Manifest") {
    using namespace socketwrapper;

	std::ifstream file("L1-Manifest.json");
	std::string json((std::istreambuf_iterator<char>(file)),
		            std::istreambuf_iterator<char>());

    auto obj = nlohmann::json::parse(json);
    REQUIRE(obj.contains("manifest"));

    SECTION("test well-formated") {
        auto L1 = std::make_unique<L1Manifest>(obj);
        REQUIRE(L1->IsWellFormated());
    }

    SECTION("test get packages") {
        auto L1 = std::make_unique<L1Manifest>(obj);
        auto packages = L1->GetDownloadPackages();
        REQUIRE(packages.size() == 2);
        REQUIRE(packages[0].url_ == "http://hhfotatest.bosch-mobility-solutions.cn/fota/cdn/api/v1/download/50M.img");
        REQUIRE(packages[0].checksum_ == "25e317773f308e446cc84c503a6d1f85");

        REQUIRE(packages[1].url_ == "http://hhfotatest.bosch-mobility-solutions.cn/fota/cdn/api/v1/download/1M.img");
        REQUIRE(packages[1].checksum_ == "b6d81b360a5672d80c27430f39153e2c");
    }
}


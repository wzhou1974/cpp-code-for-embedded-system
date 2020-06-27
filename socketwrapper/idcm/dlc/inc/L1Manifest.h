#pragma once

#include <string>
#include <vector>
#include "json/nlohmann/json.hpp"

namespace socketwrapper {

struct Package
{
	std::string ecu_;
	std::string softid_;
	std::string url_;
	std::string	checksum_;
	std::string cert_url_;
};

class L1Manifest
{
public:
	explicit L1Manifest(const nlohmann::json& manifest);
	bool IsWellFormated();
	std::vector<Package> GetDownloadPackages();
	
private:
	std::vector<Package>	packages_;
	const nlohmann::json&	L1_;
};

}	// end socketwrapper namespace

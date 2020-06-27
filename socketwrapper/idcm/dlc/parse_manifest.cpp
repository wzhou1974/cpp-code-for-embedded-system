#include <string>
#include <vector>
#include "idcm/dlc/inc/L1Manifest.h"
#include <assert.h>

namespace socketwrapper {

L1Manifest::L1Manifest(const nlohmann::json& manifest)
 : L1_(manifest)
{
}

bool L1Manifest::IsWellFormated()
{
	if (!L1_.contains("fotaProtocolVersion"))
		return	false;

	if (!L1_["fotaProtocolVersion"].is_string())
		return	false;

	if (!L1_.contains("fotaCertUrl"))
		return	false;

	if (!L1_["fotaCertUrl"].is_string())
		return	false;

	if (!L1_.contains("manifest"))
		return 	false;

	if (!L1_["manifest"].is_object())
		return	false;

	auto manifest = L1_["manifest"];
	if (!manifest.contains("packages"))
		return	false;
	if (!manifest["packages"].is_array())
		return	false;	

	auto packages = manifest["packages"];
	for (auto& package : packages) {
		if (!package.is_object())
			return	false;

		if (!package.contains("ecu"))
			return	false;

		if (!package["ecu"].is_string())
			return	false;

		if (!package["softwareId"].is_string())
			return	false;

		if (!package.contains("resources"))
			return	false;
		if (!package["resources"].is_object())
			return	false;

		auto resource = package["resources"];
		if (!resource.contains("fullLicense"))
			return	false;
		if (!resource["fullLicense"].is_string())
			return	false;
		if (!resource.contains("fullCertificateUrl"))
			return	false;
		if (!resource["fullCertificateUrl"].is_string())
			return	false;
		if (!resource.contains("fullDownloadChecksum"))
			return	false;
		if (!resource["fullDownloadChecksum"].is_string())
			return	false;
		if (!resource.contains("fullDownloadUrl"))
			return	false;
		if (!resource["fullDownloadUrl"].is_string())
			return	false;
	}
	return	true;
}

std::vector<Package> L1Manifest::GetDownloadPackages()
{
	assert(!L1_.is_null());

	std::vector<Package> packages_vec;	
	
	auto manifest = L1_["manifest"];
	auto packages = manifest["packages"];
	
	for (auto& package : packages) {
		std::string ecu = package["ecu"];

		Package pkg;
		pkg.ecu_ = std::move(ecu);
		pkg.softid_ = package["softwareId"];

		auto resource = package["resources"];
		std::string url = resource["fullDownloadUrl"];
		if (!url.empty()) {
			pkg.url_ = std::move(url);
			pkg.cert_url_ = resource["fullCertificateUrl"];
			assert(!pkg.cert_url_.empty());
			pkg.checksum_ = resource["fullDownloadChecksum"];
			assert(!pkg.checksum_.empty());
		} else {
			pkg.url_ = resource["deltaDownloadUrl"];
			assert(!pkg.url_.empty());
			pkg.cert_url_ = resource["deltaCertificateUrl"];
			assert(!pkg.cert_url_.empty());
			pkg.checksum_ = resource["deltaChecksum"];
			assert(!pkg.checksum_.empty());
		}

		packages_vec.push_back(pkg);
	}

	assert(!packages_vec.empty());
	return 	packages_vec;
}

}	//	end namespace socketwrapper

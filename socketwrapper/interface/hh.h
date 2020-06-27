#pragma once

#include <string>
#include "interface/fota.h"

namespace socketwrapper {

// NodeId --> Node name
std::string NodeToECUName(NodeId node);

// Node name --> NodeId 
NodeId ECUNameToNode(std::string node);

// NodeId --> ip address
std::string NodeToIp(NodeId node);

// ip address --> NodeId
NodeId IpToNode(const std::string& ip);

int NodeToInt(NodeId node);

}	// end socketwrapper namespace
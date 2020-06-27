#include <string>
#include <array>
#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include "interface/hh.h"
#include "ppk_assert.h"
#include "interface/fota.h"

namespace socketwrapper {

struct NodeConfig {
    NodeId      node_;
    std::string ecu_name_;        
    std::string ip_;
    int         value_;
};

const NodeConfig Nodes[] = {
    {
        .node_      = NodeId::eTBOX,
        .ecu_name_  = TBOX_ECU_NAME,
        .ip_        = TBOX_IP,
        .value_     = static_cast<int>(NodeId::eTBOX)
    },
    {
        .node_      = NodeId::eCGW,
        .ecu_name_  = CGW_ECU_NAME,
        .ip_        = CGW_IP,
        .value_     = static_cast<int>(NodeId::eCGW)
    },
    {
        .node_      = NodeId::eIDCM,
        .ecu_name_  = IDCM_ECU_NAME,
        .ip_        = IDCM_IP,
        .value_     = static_cast<int>(NodeId::eIDCM)
    },
    {
        .node_      = NodeId::eADCM,
        .ecu_name_  = ADCM_ECU_NAME,
        .ip_        = ADCM_IP,
        .value_     = static_cast<int>(NodeId::eADCM)
    },
    {
        .node_      = NodeId::eVDCM,
        .ecu_name_  = VDCM_ECU_NAME,
        .ip_        = VDCM_IP,
        .value_     = static_cast<int>(NodeId::eVDCM)
    },
    {
        .node_      = NodeId::eBDCM, 
        .ecu_name_  = BDCM_ECU_NAME,
        .ip_        = BDCM_IP,
        .value_     = static_cast<int>(NodeId::eBDCM)
    },
    {
        .node_      = NodeId::eDSM,
        .ecu_name_  = DSM_ECU_NAME,
        .ip_        = DSM_IP,
        .value_     = static_cast<int>(NodeId::eDSM)
    },
    {
        .node_      = NodeId::eARC,
        .ecu_name_  = ARC_ECU_NAME,
        .ip_        = ARC_IP,
        .value_     = static_cast<int>(NodeId::eARC)
    },
    {
        .node_      = NodeId::eVSP,
        .ecu_name_  = VSP_ECU_NAME,
        .ip_        = VSP_IP,
        .value_     = static_cast<int>(NodeId::eVSP)
    },
    {
        .node_      = NodeId::eDLP,
        .ecu_name_  = DLP_ECU_NAME,
        .ip_        = DLP_IP,
        .value_     = static_cast<int>(NodeId::eDLP)
    },
    {
        .node_      = NodeId::eIEM_DVR,
        .ecu_name_  = IEM_DVR_ECU_NAME,
        .ip_        = IEM_DVR_IP,
        .value_     = static_cast<int>(NodeId::eIEM_DVR)
    },
    {
        .node_      = NodeId::eFSE,
        .ecu_name_  = FSE_ECU_NAME,
        .ip_        = FSE_IP,
        .value_     = static_cast<int>(NodeId::eFSE)
    }
};

using boost::range::find_if;

std::string NodeToECUName(NodeId node)
{
    auto it = find_if(Nodes,
                      [&] (NodeConfig entry) {return entry.node_ == node;});

    PPK_ASSERT(it != boost::end(Nodes), "%d", static_cast<int>(node));

    return  it->ecu_name_;
}

NodeId ECUNameToNode(std::string node)
{
    auto it = find_if(Nodes, 
                      [&] (NodeConfig entry) {return entry.ecu_name_ == node;});

    PPK_ASSERT(it != boost::end(Nodes), "%s", node.c_str());

    return  it->node_;
}

std::string NodeToIp(NodeId node)
{
    auto it = find_if(Nodes, 
                      [&] (NodeConfig entry) {return entry.node_ == node;});

    PPK_ASSERT(it != boost::end(Nodes), "%d", static_cast<int>(node));

    return  it->ip_;
}

NodeId IpToNode(const std::string& ip)
{
    auto it = find_if(Nodes, 
                      [&] (NodeConfig entry) {return entry.ip_ == ip;});

    PPK_ASSERT(it != boost::end(Nodes), "%s", ip.c_str());

    return  it->node_;
}

int NodeToInt(NodeId node)
{
    auto it = find_if(Nodes, 
                      [&] (NodeConfig entry) {return entry.node_ == node;});

    PPK_ASSERT(it != boost::end(Nodes), "%d", static_cast<int>(node));

    return  it->value_;
}

} // end namespace socketwrapper


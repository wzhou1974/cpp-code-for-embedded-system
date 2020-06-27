#pragma once

enum class NodeId
{
	eTBOX = 0,
	eCGW,
	eIDCM,
	eADCM,
	eVDCM,
	eBDCM,
	eDSM,
	eARC,
	eVSP,
	eDLP,
	eIEM_DVR,
	eFSE,
	eMax
};

#define TBOX_ECU_NAME	"TBOX"
#define CGW_ECU_NAME	"CGW"
#define IDCM_ECU_NAME	"IDCM"
#define ADCM_ECU_NAME	"ADCM"
#define VDCM_ECU_NAME	"VDCM"
#define BDCM_ECU_NAME	"BDCM"
#define DSM_ECU_NAME	"DSM"
#define ARC_ECU_NAME	"ARC"
#define VSP_ECU_NAME	"VSP"
#define DLP_ECU_NAME	"DLP"
#define IEM_DVR_ECU_NAME	"IEM/DVR"
#define FSE_ECU_NAME	"FSE"

#define NODE_NUM 	(static_cast<int>(NodeId::eMax))

#ifdef PC_SIMULATION
#define TBOX_IP		"127.0.0.1"
#define CGW_IP		"127.0.0.1"
#define IDCM_IP		"127.0.0.1"
#define ADCM_IP		"127.0.0.1"
#define VDCM_IP		"127.0.0.1"
#define BDCM_IP		"127.0.0.1"
#define DSM_IP		"127.0.0.1"
#define ARC_IP		"127.0.0.1"
#define VSP_IP		"127.0.0.1"
#define DLP_IP		"127.0.0.1"
#define IEM_DVR_IP	"127.0.0.1"
#define FSE_IP		"127.0.0.1"
#else
#define TBOX_IP		"192.168.0.1"
#define CGW_IP		"192.168.0.2"
#define IDCM_IP		"192.168.0.4"
#define ADCM_IP		""				// not defined, currently
#define VDCM_IP		"192.168.0.7"
#define BDCM_IP		"192.168.0.9"
#define DSM_IP		"192.168.0.11"
#define ARC_IP		"192.168.0.12"
#define VSP_IP		"192.168.0.13"
#define DLP_IP		"192.168.0.14"
#define IEM_DVR_IP	""				// not defined, currently
#define FSE_IP		"192.168.0.16"
#endif

#define SELFUPDATER_RPC_PORT	3000

#define DLC_PROXY_PORT  		2000
#define DLC_SERVER_PORT			3000

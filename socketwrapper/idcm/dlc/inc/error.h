#pragma once

#include <string>

namespace socketwrapper {

#define DLC_ERROR_CODE  1


#define DLC_ERROR_OK                0
#define DLC_ERROR_L1_INVALID        1
#define DLC_ERROR_L1_WELLFORMATED   2
#define DLC_ERROR_FILE_NOTEXIST     3
#define DLC_ERROR_DOWNLOAD_FAIL     4
#define DLC_ERROR_CHECKSUM          5

struct DlError
{
	std::string		url_;
	size_t			size_;
	uint32_t		code_;
};

class DLCStatusReport
{
public:
    DLCStatusReport(const std::string& L1_Manifest, const DlError& error)
    {
        // to do
    }

    std::string dump();
};


}	// end socketwrapper namespace
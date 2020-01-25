#ifndef __DAP_HANDLE_H__
#define __DAP_HANDLE_H__

#include "usbip_defs.h"

void handle_dap_data_request(usbip_stage2_header *header);
void handle_dap_data_response(usbip_stage2_header *header);
#endif
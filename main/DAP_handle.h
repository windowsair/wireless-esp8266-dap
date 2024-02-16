#ifndef __DAP_HANDLE_H__
#define __DAP_HANDLE_H__

#include "components/USBIP/usbip_defs.h"

enum reset_handle_t
{
    NO_SIGNAL = 0,
    RESET_HANDLE = 1,
    DELETE_HANDLE = 2,
};

void handle_dap_data_request(usbip_stage2_header *header, uint32_t length);
void handle_swo_trace_response(usbip_stage2_header *header);
void handle_dap_unlink();

int fast_reply(uint8_t *buf, uint32_t length, int dap_req_num);

#endif
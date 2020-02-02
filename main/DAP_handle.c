/**
 * @file DAP_handle.c
 * @brief Handle DAP packets and transaction push
 * @version 0.1
 * @date 2020-01-25
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdint.h>
#include "usbip_server.h"
#include "DAP_handle.h"
#include "DAP.h"
#include "esp_libc.h"
////TODO: Merge this
#define DAP_PACKET_SIZE 64

static uint8_t data_out[DAP_PACKET_SIZE];
static int dap_respond = 0;

// SWO Trace
static int swo_trace_respond = 0;
static uint8_t *swo_data_to_send;
static uint32_t num_swo_data;

void handle_dap_data_request(usbip_stage2_header *header)
{
    uint8_t *data_in = (uint8_t *)header;
    data_in = &(data_in[sizeof(usbip_stage2_header)]);
    // Point to the beginning of the URB packet
    dap_respond = DAP_ProcessCommand((uint8_t *)data_in, (uint8_t *)data_out);
    //handle_dap_data_response(header);
    send_stage2_submit(header, 0, 0);
}
void handle_dap_data_response(usbip_stage2_header *header)
{
    if (dap_respond)
    {

        //os_printf("*** Will respond");

        send_stage2_submit_data(header, 0, data_out, DAP_PACKET_SIZE);
        dap_respond = 0;
        //os_printf("*** RESPONDED ***");
    }
    else
    {
        //os_printf("*** Will NOT respond");
        send_stage2_submit(header, 0, 0);
    }
}

void handle_swo_trace_response(usbip_stage2_header *header)
{
    if (swo_trace_respond)
    {
        swo_trace_respond = 0;
        //os_printf("*** Will respond");
        send_stage2_submit_data(header, 0, data_out, DAP_PACKET_SIZE);

        //os_printf("*** RESPONDED ***");
    }
    else
    {
        //os_printf("*** Will NOT respond");
        send_stage2_submit(header, 0, 0);
    }
}

// SWO Data Queue Transfer
//   buf:    pointer to buffer with data
//   num:    number of bytes to transfer
void SWO_QueueTransfer(uint8_t *buf, uint32_t num)
{
    swo_data_to_send = buf;
    num_swo_data = num;
    swo_trace_respond = 1;
}

// SWO Data Abort Transfer
void SWO_AbortTransfer(void)
{
    //USBD_EndpointAbort(0U, USB_ENDPOINT_IN(2U));
    ////TODO: unlink might be useful ...
}
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

////TODO: Merge this
#define DAP_PACKET_SIZE 512  


uint8_t data_out[DAP_PACKET_SIZE];
int respond = 0;


void handle_dap_data_request(usbip_stage2_header *header)
{
    uint8_t * data_in = (uint8_t *)header;
    data_in = &(data_in[sizeof(usbip_stage2_header)]);
    // Point to the beginning of the URB packet
    respond = DAP_ProcessCommand((uint8_t *)data_in, (uint8_t *)data_out);
    send_stage2_submit(header, 0, 0);

}
void handle_dap_data_response(usbip_stage2_header *header)
{
    if (respond) {
            respond = 0;
            //os_printf("*** Will respond");
            send_stage2_submit_data(header, 0, data_out, DAP_PACKET_SIZE);
        
            //os_printf("*** RESPONDED ***");
        } else {
            //os_printf("*** Will NOT respond");
            send_stage2_submit(header, 0, 0);
        }
}
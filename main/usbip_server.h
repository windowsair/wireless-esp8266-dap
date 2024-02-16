#ifndef __USBIP_SERVER_H__
#define __USBIP_SERVER_H__
#include <stdint.h>
#include <stddef.h>

#include "components/USBIP/usbip_defs.h"

enum usbip_server_state_t
{
    WAIT_DEVLIST = 0,
    WAIT_IMPORT,
    WAIT_URB,
};

extern int kSock;

int usbip_worker(uint8_t *base, uint32_t length, enum usbip_server_state_t *state);
void send_stage2_submit_data(usbip_stage2_header *req_header, int32_t status, const void * const data, int32_t data_length);
void send_stage2_submit(usbip_stage2_header *req_header, int32_t status, int32_t data_length);
void send_stage2_submit_data_fast(usbip_stage2_header *req_header, const void *const data, int32_t data_length);
int usbip_network_send(int s, const void *dataptr, size_t size, int flags);

#endif
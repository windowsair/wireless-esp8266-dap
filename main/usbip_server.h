#ifndef __USBIP_SERVER_H__
#define __USBIP_SERVER_H__
#include <stdint.h>
#include <stddef.h>

#include "components/USBIP/usbip_defs.h"

enum state_t
{
    ACCEPTING,
    ATTACHING,
    EMULATING,
    EL_DATA_PHASE
};
extern uint8_t kState;
extern int kSock;

int attach(uint8_t *buffer, uint32_t length);
int emulate(uint8_t *buffer, uint32_t length);
void send_stage2_submit_data(usbip_stage2_header *req_header, int32_t status, const void * const data, int32_t data_length);
void send_stage2_submit(usbip_stage2_header *req_header, int32_t status, int32_t data_length);
void send_stage2_submit_data_fast(usbip_stage2_header *req_header, const void *const data, int32_t data_length);
int usbip_network_send(int s, const void *dataptr, size_t size, int flags);

#endif
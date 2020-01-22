#ifndef __USBIP_SERVER_H__
#define __USBIP_SERVER_H__
#include <stdint.h>
enum state_t
{
    ACCEPTING,
    ATTACHING,
    EMULATING
};
extern uint32_t state;
extern int socket;

int attach(uint8_t *buffer, uint32_t length);
int emulate(uint8_t *buffer, uint32_t length);

#endif
#ifndef __WEBSOCKET_SERVER_H__
#define __WEBSOCKET_SERVER_H__

#include <stdint.h>

int websocket_worker(int fd, uint8_t *base, uint32_t length);

#endif

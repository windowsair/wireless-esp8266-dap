#ifndef __TCP_NETCONN_H__
#define __TCP_NETCONN_H__

#include <stdint.h>

int tcp_netconn_send(const void *buffer, size_t len);
void tcp_netconn_task();

#endif
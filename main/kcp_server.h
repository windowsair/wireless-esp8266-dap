#ifndef __KCP_SERVER_H__
#define __KCP_SERVER_H__

void kcp_server_task();
int kcp_network_send(const char *buffer, int len);

#endif
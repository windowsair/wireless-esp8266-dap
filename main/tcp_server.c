/**
 * @file tcp_server.c
 * @brief Handle main tcp tasks
 * @version 0.1
 * @date 2020-01-22
 *
 * @copyright Copyright (c) 2020
 *
 */
#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "main/wifi_configuration.h"
#include "main/usbip_server.h"
#include "main/websocket_server.h"
#include "main/DAP_handle.h"

#include "components/elaphureLink/elaphureLink_protocol.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

extern TaskHandle_t kDAPTaskHandle;
extern int kRestartDAPHandle;

int kSock = -1;

void tcp_server_task(void *pvParameters)
{
    uint8_t tcp_rx_buffer[1500] = {0};
    char addr_str[128];
    enum usbip_server_state_t usbip_state = WAIT_DEVLIST;
    uint8_t *data;
    int addr_family;
    int ip_protocol;
    int header;
    int ret, sz;

    int on = 1;
    while (1)
    {

#ifdef CONFIG_EXAMPLE_IPV4
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
#else // IPV6
        struct sockaddr_in6 destAddr;
        bzero(&destAddr.sin6_addr.un, sizeof(destAddr.sin6_addr.un));
        destAddr.sin6_family = AF_INET6;
        destAddr.sin6_port = htons(PORT);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
        inet6_ntoa_r(destAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
#endif

        int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (listen_sock < 0)
        {
            os_printf("Unable to create socket: errno %d\r\n", errno);
            break;
        }
        os_printf("Socket created\r\n");

        setsockopt(listen_sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));
        setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

        int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0)
        {
            os_printf("Socket unable to bind: errno %d\r\n", errno);
            break;
        }
        os_printf("Socket binded\r\n");

        err = listen(listen_sock, 1);
        if (err != 0)
        {
            os_printf("Error occured during listen: errno %d\r\n", errno);
            break;
        }
        os_printf("Socket listening\r\n");

#ifdef CONFIG_EXAMPLE_IPV6
        struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
#else
        struct sockaddr_in sourceAddr;
#endif
        uint32_t addrLen = sizeof(sourceAddr);
        while (1)
        {
            kSock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
            if (kSock < 0)
            {
                os_printf("Unable to accept connection: errno %d\r\n", errno);
                break;
            }
            setsockopt(kSock, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on));
            setsockopt(kSock, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
            os_printf("Socket accepted\r\n");

            // Read header
            sz = 4;
            data = &tcp_rx_buffer[0];
            do {
                ret = recv(kSock, data, sz, 0);
                if (ret <= 0)
                    goto cleanup;
                sz -= ret;
                data += ret;
            } while (sz > 0);

            header = *((int *)(tcp_rx_buffer));
            header = ntohl(header);

            if (header == EL_LINK_IDENTIFIER) {
                el_dap_work(tcp_rx_buffer, sizeof(tcp_rx_buffer));
            } else if ((header & 0xFFFF) == 0x8003 ||
                       (header & 0xFFFF) == 0x8005) { // usbip OP_REQ_DEVLIST/OP_REQ_IMPORT
                if ((header & 0xFFFF) == 0x8005)
                    usbip_state = WAIT_DEVLIST;
                else
                    usbip_state = WAIT_IMPORT;
                usbip_worker(tcp_rx_buffer, sizeof(tcp_rx_buffer), &usbip_state);
            } else if (header == 0x47455420) { // string "GET "
#ifdef CONFIG_USE_WEBSOCKET_DAP
                websocket_worker(kSock, tcp_rx_buffer, sizeof(tcp_rx_buffer));
#endif
            } else {
                os_printf("Unknown protocol\n");
            }

cleanup:
            if (kSock != -1)
            {
                os_printf("Shutting down socket and restarting...\r\n");
                //shutdown(kSock, 0);
                close(kSock);

                // Restart DAP Handle
                el_process_buffer_free();

                kRestartDAPHandle = RESET_HANDLE;
                if (kDAPTaskHandle)
                    xTaskNotifyGive(kDAPTaskHandle);

                //shutdown(listen_sock, 0);
                //close(listen_sock);
                //vTaskDelay(5);
            }
        }
    }
    vTaskDelete(NULL);
}
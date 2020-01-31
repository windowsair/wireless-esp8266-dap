/**
 * @file tcp_server.c
 * @brief Handle main tcp tasks
 * @version 0.1
 * @date 2020-01-22
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include "tcp_server.h"

#include <string.h>
#include <stdint.h>
#include <sys/param.h>
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

#include "wifi_configuration.h"
#include "usbip_server.h"

uint8_t kState = ACCEPTING;
int kSock = -1;
void tcp_server_task(void *pvParameters)
{
    uint8_t rx_buffer[2048];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

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
            os_printf("Socket accepted\r\n");

            while (1)
            {
                int len = recv(kSock, rx_buffer, 2047, 0);
                // Error occured during receiving
                if (len < 0)
                {
                    os_printf("recv failed: errno %d\r\n", errno);
                    break;
                }
                // Connection closed
                else if (len == 0)
                {
                    os_printf("Connection closed\r\n");
                    break;
                }
                // Data received
                else
                {
#ifdef CONFIG_EXAMPLE_IPV6
                    // Get the sender's ip address as string
                    if (sourceAddr.sin6_family == PF_INET)
                    {
                        inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                    }
                    else if (sourceAddr.sin6_family == PF_INET6)
                    {
                        inet6_ntoa_r(sourceAddr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                    }
#else
                    inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
#endif

                    switch (kState)
                    {
                    case ACCEPTING:
                        kState = ATTACHING;

                    case ATTACHING:
                        attach(rx_buffer, len);
                        break;

                    case EMULATING:
                        emulate(rx_buffer, len);
                        break;
                    default:
                        os_printf("unkonw kstate!\r\n");
                    }
                }
            }
            // kState = ACCEPTING;
            if (kSock != -1)
            {
                os_printf("Shutting down socket and restarting...\r\n");
                //shutdown(kSock, 0);
                close(kSock);

                //shutdown(listen_sock, 0);
                //close(listen_sock);
                //vTaskDelay(5);
            }
        }
    }
    vTaskDelete(NULL);
}
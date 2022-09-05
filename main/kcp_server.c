/**
 * @file kcp_server.c
 * @author windows
 * @brief usbip KCP port
 * @version 0.1
 * @date 2021-10-08
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "main/kcp_server.h"
#include "main/usbip_server.h"
#include "main/wifi_configuration.h"

#include "components/kcp/ikcp.h"
#include "components/kcp/ikcp_util.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"


#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

extern TaskHandle_t kDAPTaskHandle;
extern int kRestartDAPHandle;
extern int kSock;
extern uint8_t kState;

static struct sockaddr_in client_addr = { 0 };
static char kcp_buffer[MTU_SIZE];
static ikcpcb *kcp1 = NULL;


static void set_non_blocking(int sockfd) {
    int flag = fcntl(sockfd, F_GETFL, 0);
    if (flag < 0) {
        os_printf("fcntl F_GETFL fail\n");
        return;
    }
    if (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) < 0) {
        os_printf("fcntl F_SETFL fail\n");
    }
}

static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
	int ret = -1;
    int time = 10;
    // Unfortunately, esp8266 often fails due to lack of memory
    while (ret < 0) {
        ret = sendto(kSock, buf, len, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        if (ret < 0) {
            // os_printf("fail to send, retry\r\n");
            int errcode = errno;
            if (errno != ENOMEM)
                os_printf("unknown errcode %d\r\n", errcode);
            vTaskDelay(pdMS_TO_TICKS(time));
            time += 10;
        }
    }
	return 0;
}

int kcp_network_send(const char *buffer, int len) {
    ikcp_send(kcp1, buffer, len);
    ikcp_flush(kcp1);
    return 0;
}

void kcp_server_task()
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        kSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (kSock < 0) {
            os_printf("Unable to create socket: errno %d", errno);
            break;
        }
        os_printf("Socket created\r\n");

        set_non_blocking(kSock);


        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);

        socklen_t socklen = sizeof(client_addr);


        int err = bind(kSock, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (err < 0) {
            os_printf("Socket unable to bind: errno %d\r\n", errno);
        }
        os_printf("Socket binded\r\n");

        // KCP init
        if (kcp1 == NULL) {
            kcp1 = ikcp_create(1, (void *)0);
        }
        if (kcp1 == NULL) {
            os_printf("can not create kcp control block\r\n");
            break;
        }
        kcp1->output = udp_output;

        ikcp_wndsize(kcp1, 4096, 4096);

        ikcp_nodelay(kcp1, 2, 2, 2, 1); // set fast mode
        kcp1->interval = 0;
        kcp1->rx_minrto = 1;
        kcp1->fastresend = 1;

        ikcp_setmtu(kcp1, 768);



        int ret = -1;
        // KCP task main loop
        while (1) {
            vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1)); // we wanna sleep absolute time
            ikcp_update(kcp1, iclock());

            // recv data from udp
            while (1) {
                ret = recvfrom(kSock, kcp_buffer, MTU_SIZE, 0, (struct sockaddr *)&client_addr, &socklen);
                if (ret < 0) {
                    break;
                }
                ikcp_input(kcp1, kcp_buffer, ret);
            }

            // recv data from kdp
            while (1) {
                ret = ikcp_recv(kcp1, kcp_buffer, MTU_SIZE);
                if (ret < 0) {
                    break;
                }
                // recv user data, then handle it
                switch (kState)
                {
                case EMULATING:
                    emulate((uint8_t *)kcp_buffer, ret);
                    break;

                case ACCEPTING:
                    kState = ATTACHING;
                case ATTACHING:
                    attach((uint8_t *)kcp_buffer, ret);
                    break;

                default:
                    os_printf("unkonw kstate!\r\n");
                }
            }
        }
        if (kcp1) {
            ikcp_release(kcp1);
        }
        if (kSock != -1) {
            os_printf("Shutting down socket and restarting...\r\n");
            shutdown(kSock, 0);
            close(kSock);

            if (kState == EMULATING)
                kState = ACCEPTING;
            // Restart DAP Handle
            kRestartDAPHandle = 1;
            if (kDAPTaskHandle)
                xTaskNotifyGive(kDAPTaskHandle);
        }
    }
    vTaskDelete(NULL);
}
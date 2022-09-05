/*
Copyright (c) 2015, SuperHouse Automation Pty Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "main/wifi_configuration.h"
#include "main/usbip_server.h"
#include "main/tcp_netconn.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netbuf.h"
#include "lwip/api.h"
#include "lwip/tcp.h"
#include <lwip/netdb.h>

#define PORT 3240
#define EVENTS_QUEUE_SIZE 50

#ifdef CALLBACK_DEBUG
#define debug(s, ...) os_printf("%s: " s "\n", "Cb:", ##__VA_ARGS__)
#else
#define debug(s, ...)
#endif

QueueHandle_t xQueue_events;
typedef struct
{
    struct netconn *nc;
    uint8_t type;
} netconn_events;

extern TaskHandle_t kDAPTaskHandle;
extern int kRestartDAPHandle;
extern uint8_t kState;

struct netconn *kNetconn = NULL;

int tcp_netconn_send(const void *buffer, size_t len)
{
    return netconn_write(kNetconn, buffer, len, NETCONN_COPY);
}

/*
 * This function will be call in Lwip in each event on netconn
 */
static void netCallback(struct netconn *conn, enum netconn_evt evt, uint16_t length)
{
    // Show some callback information (debug)
    debug("sock:%u\tsta:%u\tevt:%u\tlen:%u\ttyp:%u\tfla:%02x\terr:%d",
          (uint32_t)conn, conn->state, evt, length, conn->type, conn->flags, conn->pending_err);

    netconn_events events;

    // If netconn got error, it is close or deleted, dont do treatments on it.
    if (conn->pending_err)
    {
        return;
    }
    // Treatments only on rcv events.
    switch (evt)
    {
    case NETCONN_EVT_RCVPLUS:
        events.nc = conn;
        events.type = evt;
        break;
    default:
        return;
        break;
    }

    // Send the event to the queue
    xQueueSend(xQueue_events, &events, 1000);
}

/*
 *  Initialize a server netconn and listen port
 */
static void set_tcp_server_netconn(struct netconn **nc, uint16_t port, netconn_callback callback)
{
    if (nc == NULL)
    {
        os_printf("%s: netconn missing .\n", __FUNCTION__);
        return;
    }
    *nc = netconn_new_with_callback(NETCONN_TCP, netCallback);
    if (!*nc)
    {
        os_printf("Status monitor: Failed to allocate netconn.\n");
        return;
    }
    netconn_set_nonblocking(*nc, NETCONN_FLAG_NON_BLOCKING);
    // netconn_set_recvtimeout(*nc, 10);
    netconn_bind(*nc, IP_ADDR_ANY, port);
    netconn_listen(*nc);
}

/*
 *  Close and delete a socket properly
 */
static void close_tcp_netconn(struct netconn *nc)
{
    nc->pending_err = ERR_CLSD; // It is hacky way to be sure than callback will don't do treatment on a netconn closed and deleted
    netconn_close(nc);
    netconn_delete(nc);
}

void tcp_netconn_task()
{
    xQueue_events = xQueueCreate(EVENTS_QUEUE_SIZE, sizeof(netconn_events));
    struct netconn *nc = NULL; // To create servers

    set_tcp_server_netconn(&nc, PORT, netCallback);
    os_printf("Server netconn %u ready on port %u.\n", (uint32_t)nc, PORT);

    struct netbuf *netbuf = NULL; // To store incoming Data
    struct netconn *nc_in = NULL; // To accept incoming netconn
    //
    char *buffer;
    uint16_t len_buf;

    while (1)
    {
        netconn_events events;
        xQueueReceive(xQueue_events, &events, portMAX_DELAY); // Wait here an event on netconn

        if (events.nc->state == NETCONN_LISTEN) // If netconn is a server and receive incoming event on it
        {
            os_printf("Client incoming on server %u.\n", (uint32_t)events.nc);
            int err = netconn_accept(events.nc, &nc_in);
            if (err != ERR_OK)
            {
                if (nc_in)
                    netconn_delete(nc_in);
            }
            os_printf("New client is %u.\n", (uint32_t)nc_in);
            ip_addr_t client_addr; // Address port
            uint16_t client_port;  // Client port
            netconn_peer(nc_in, &client_addr, &client_port);
            // tcp_nagle_disable(events.nc->pcb.tcp); // crash! DO NOT USE
        }
        else if (events.nc->state != NETCONN_LISTEN) // If netconn is the client and receive data
        {
            // tcp_nagle_disable(events.nc->pcb.tcp);
            if ((netconn_recv(events.nc, &netbuf)) == ERR_OK) // data incoming ?
            {
                do
                {
                    netbuf_data(netbuf, (void *)&buffer, &len_buf);
                    kNetconn = events.nc;
                    switch (kState)
                    {
                    case ACCEPTING:
                        kState = ATTACHING;

                    case ATTACHING:
                        attach((uint8_t *)buffer, len_buf);
                        kState = EMULATING;
                        break;

                    case EMULATING:
                        emulate((uint8_t *)buffer, len_buf);
                        break;
                    default:
                        os_printf("unkonw kstate!\r\n");
                    }
                } while (netbuf_next(netbuf) >= 0);
                netbuf_delete(netbuf);
            }
            else
            {
                if (events.nc->pending_err == ERR_CLSD)
                {
                    continue; // The same hacky way to treat a closed connection
                }
                os_printf("Shutting down socket and restarting...\r\n");
                close_tcp_netconn(events.nc);
                if (kState == EMULATING)
                    kState = ACCEPTING;
                // Restart DAP Handle
                kRestartDAPHandle = 1;
                if (kDAPTaskHandle)
                    xTaskNotifyGive(kDAPTaskHandle);
            }
        }
    }
}
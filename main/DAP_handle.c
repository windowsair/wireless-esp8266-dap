/**
 * @file DAP_handle.c
 * @brief Handle DAP packets and transaction push
 * @version 0.2
 * @date 2020-02-04
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <stdint.h>
#include <string.h>
#include "usbip_server.h"
#include "DAP_handle.h"
#include "DAP.h"
#include "esp_libc.h"
#include "USBd_config.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "freertos/semphr.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

extern int kSock;
extern TaskHandle_t kDAPTaskHandle;
////TODO: Merge this
#define DAP_PACKET_SIZE 255

static uint8_t data_out[DAP_PACKET_SIZE];
static int dap_respond = 0;

// SWO Trace
static int swo_trace_respond = 0;
static uint8_t *swo_data_to_send;
static uint32_t num_swo_data;

static RingbufHandle_t dap_dataIN_handle = NULL;
static RingbufHandle_t dap_dataOUT_handle = NULL;
static SemaphoreHandle_t data_response_mux = NULL;

static void unpack(void *data, int size);

void handle_dap_data_request(usbip_stage2_header *header, uint32_t length)
{
    uint8_t *data_in = (uint8_t *)header;
    data_in = &(data_in[sizeof(usbip_stage2_header)]);
    // Point to the beginning of the URB packet

#if (USE_WINUSB == 1)
    dap_respond = DAP_ProcessCommand((uint8_t *)data_in, (uint8_t *)data_out);
    //handle_dap_data_response(header);
    send_stage2_submit(header, 0, 0);
#else
    xRingbufferSend(dap_dataIN_handle, data_in, length - sizeof(usbip_stage2_header), portMAX_DELAY);
    //os_printf("LENGTH: %d\r\n", length - sizeof(usbip_stage2_header));
    xTaskNotifyGive(kDAPTaskHandle);
    send_stage2_submit(header, 0, 0);

#endif
}

void handle_dap_data_response(usbip_stage2_header *header)
{
    if (dap_respond)
    {
        send_stage2_submit_data(header, 0, data_out, DAP_PACKET_SIZE);
        dap_respond = 0;
    }
    else
    {
        send_stage2_submit(header, 0, 0);
    }
}

void handle_swo_trace_response(usbip_stage2_header *header)
{
    if (swo_trace_respond)
    {
        swo_trace_respond = 0;
        send_stage2_submit_data(header, 0, data_out, DAP_PACKET_SIZE);
    }
    else
    {
        send_stage2_submit(header, 0, 0);
    }
}

// SWO Data Queue Transfer
//   buf:    pointer to buffer with data
//   num:    number of bytes to transfer
void SWO_QueueTransfer(uint8_t *buf, uint32_t num)
{
    swo_data_to_send = buf;
    num_swo_data = num;
    swo_trace_respond = 1;
}

// SWO Data Abort Transfer
void SWO_AbortTransfer(void)
{
    //USBD_EndpointAbort(0U, USB_ENDPOINT_IN(2U));
    ////TODO: unlink might be useful ...
}

void DAP_Thread(void *argument)
{
    dap_dataIN_handle = xRingbufferCreate(DAP_PACKET_SIZE * 20, RINGBUF_TYPE_BYTEBUF);
    dap_dataOUT_handle = xRingbufferCreate(DAP_PACKET_SIZE * 10, RINGBUF_TYPE_BYTEBUF);
    data_response_mux = xSemaphoreCreateMutex();
    size_t packetSize;
    uint8_t *item;

    if (dap_dataIN_handle == NULL || dap_dataIN_handle == NULL ||
        data_response_mux == NULL)
    {
        os_printf("Can not create DAP ringbuf/mux!\r\n");
        vTaskDelete(NULL);
    }
    for (;;)
    {

        while (1)
        {
            ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
            packetSize = 0;
            item = (uint8_t *)xRingbufferReceiveUpTo(dap_dataIN_handle, &packetSize,
                                                     (1 / portTICK_RATE_MS), DAP_PACKET_SIZE);
            if (packetSize == 0)
            {
                break;
            }

            else if (packetSize < DAP_PACKET_SIZE)
            {
                os_printf("Wrong data in packet size:%d , data in remain: %d\r\n", packetSize, (int)xRingbufferGetMaxItemSize(dap_dataIN_handle));
                vRingbufferReturnItem(dap_dataIN_handle, (void *)item);
                break;
                // This may not happen because there is a semaphore acquisition
            }

            if (item[0] == ID_DAP_QueueCommands)
                item[0] = ID_DAP_ExecuteCommands;
            DAP_ProcessCommand(item, data_out);

            vRingbufferReturnItem(dap_dataIN_handle, (void *)item);
            xRingbufferSend(dap_dataOUT_handle, data_out, DAP_PACKET_SIZE, portMAX_DELAY);
            if (xSemaphoreTake(data_response_mux, portMAX_DELAY) == pdTRUE)
            {
                ++dap_respond;
                xSemaphoreGive(data_response_mux);
            }
        }
    }
}

int fast_reply(uint8_t *buf, uint32_t length)
{
    if (length == 48 && buf[3] == 1 && buf[15] == 1 && buf[19] == 1)
    {
        if (dap_respond > 0)
        {
            uint8_t *item;
            size_t packetSize = 0;
            item = (uint8_t *)xRingbufferReceiveUpTo(dap_dataOUT_handle, &packetSize,
                                                     (10 / portTICK_RATE_MS), DAP_PACKET_SIZE);
            if (packetSize == DAP_PACKET_SIZE)
            {
                unpack((uint32_t *)buf, sizeof(usbip_stage2_header));
                send_stage2_submit_data((usbip_stage2_header *)buf, 0, item, DAP_PACKET_SIZE);
                if (xSemaphoreTake(data_response_mux, portMAX_DELAY) == pdTRUE)
                {
                    --dap_respond;
                    xSemaphoreGive(data_response_mux);
                }
                vRingbufferReturnItem(dap_dataOUT_handle, (void *)item);
                return 1;
            }
            else if (packetSize > 0)
            {
                os_printf("Wrong data out packet size:%d!\r\n", packetSize);
            }
            ////TODO: fast reply
        }
        else
        {
            buf[3] = 0x3; // command
            buf[15] = 0;  // direction
            buf[0x16] = 0;
            buf[0x17] = 0;
            buf[27] = 0;
            buf[39] = 0;
            send(kSock, buf, 48, 0);
            return 1;
        }
    }
    return 0;
}

static void unpack(void *data, int size)
{

    // Ignore the setup field
    int sz = (size / sizeof(uint32_t)) - 2;
    uint32_t *ptr = (uint32_t *)data;

    for (int i = 0; i < sz; i++)
    {
        ptr[i] = ntohl(ptr[i]);
    }
}
/**
 * @file DAP_handle.c
 * @brief Handle DAP packets and transaction push
 * @version 0.3
 * @date 2020-02-04 first version
 *       2020-11-11 support WinUSB mode
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
#include "dap_configuration.h"

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

int kRestartDAPHandle = 0;


#if (USE_WINUSB == 1)
typedef struct
{
    uint32_t length;
    uint8_t buf[DAP_PACKET_SIZE];
} DAPPacetDataType;
#else
typedef struct
{
    uint8_t buf[DAP_PACKET_SIZE];
} DAPPacetDataType;
#endif


#define DAP_HANDLE_SIZE (sizeof(DAPPacetDataType))

static DAPPacetDataType DAPDataProcessed;
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
    send_stage2_submit(header, 0, 0);

    // always send constant size buf -> cuz we don't care about the IN packet size
    // and to unify the style, we set aside the length of the section
    xRingbufferSend(dap_dataIN_handle, data_in - sizeof(uint32_t), DAP_HANDLE_SIZE, portMAX_DELAY); 
    xTaskNotifyGive(kDAPTaskHandle);

#else
    send_stage2_submit(header, 0, 0);

    xRingbufferSend(dap_dataIN_handle, data_in, DAP_HANDLE_SIZE, portMAX_DELAY);
    xTaskNotifyGive(kDAPTaskHandle);

#endif

    // dap_respond = DAP_ProcessCommand((uint8_t *)data_in, (uint8_t *)data_out);
    // //handle_dap_data_response(header);
    // send_stage2_submit(header, 0, 0);
}

void handle_dap_data_response(usbip_stage2_header *header)
{
    return;
    // int resLength = dap_respond & 0xFFFF;
    // if (resLength)
    // {

    //     send_stage2_submit_data(header, 0, (void *)DAPDataProcessed.buf, resLength);
    //     dap_respond = 0;
    // }
    // else
    // {
    //     send_stage2_submit(header, 0, 0);
    // }
}

void handle_swo_trace_response(usbip_stage2_header *header)
{
    // TODO:  
    send_stage2_submit(header, 0, 0);
    return;

    if (swo_trace_respond)
    {
        swo_trace_respond = 0;
        //send_stage2_submit_data(header, 0, (void *)DAPDataProcessed.buf, DAP_PACKET_SIZE);
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
    dap_dataIN_handle = xRingbufferCreate(DAP_HANDLE_SIZE * 20, RINGBUF_TYPE_BYTEBUF);
    dap_dataOUT_handle = xRingbufferCreate(DAP_HANDLE_SIZE * 20, RINGBUF_TYPE_BYTEBUF);
    data_response_mux = xSemaphoreCreateMutex();
    size_t packetSize;
    int resLength;
    DAPPacetDataType *item;

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
            if (kRestartDAPHandle)
            {
                vRingbufferDelete(dap_dataIN_handle);
                vRingbufferDelete(dap_dataOUT_handle);
                dap_dataIN_handle = dap_dataOUT_handle = NULL;

                dap_dataIN_handle = xRingbufferCreate(DAP_HANDLE_SIZE * 20, RINGBUF_TYPE_BYTEBUF);
                dap_dataOUT_handle = xRingbufferCreate(DAP_HANDLE_SIZE * 20, RINGBUF_TYPE_BYTEBUF);
                if (dap_dataIN_handle == NULL || dap_dataIN_handle == NULL)
                {
                    os_printf("Can not create DAP ringbuf/mux!\r\n");
                    vTaskDelete(NULL);
                }
                kRestartDAPHandle = 0;
            }

            ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
            packetSize = 0;
            item = (DAPPacetDataType *)xRingbufferReceiveUpTo(dap_dataIN_handle, &packetSize,
                                                          (1 / portTICK_RATE_MS), DAP_HANDLE_SIZE);
            if (packetSize == 0)
            {
                break;
            }

            else if (packetSize < DAP_HANDLE_SIZE)
            {
                os_printf("Wrong data in packet size:%d , data in remain: %d\r\n", packetSize, (int)xRingbufferGetMaxItemSize(dap_dataIN_handle));
                vRingbufferReturnItem(dap_dataIN_handle, (void *)item);
                break;
                // This may not happen because there is a semaphore acquisition
            }

            if (item->buf[0] == ID_DAP_QueueCommands)
            {
                item->buf[0] = ID_DAP_ExecuteCommands;
            }

            resLength = DAP_ProcessCommand((uint8_t *)item->buf, (uint8_t *)DAPDataProcessed.buf); // use first 4 byte to save length 
            resLength &= 0xFFFF; // res length in lower 16 bits

            vRingbufferReturnItem(dap_dataIN_handle, (void *)item); // process done.

            // now prepare to reply
        #if (USE_WINUSB == 1)
            DAPDataProcessed.length = resLength;
        #endif
            xRingbufferSend(dap_dataOUT_handle, (void *)&DAPDataProcessed, DAP_HANDLE_SIZE, portMAX_DELAY);

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
            DAPPacetDataType *item;
            size_t packetSize = 0;
            item = (DAPPacetDataType *)xRingbufferReceiveUpTo(dap_dataOUT_handle, &packetSize,
                                                     (10 / portTICK_RATE_MS), DAP_HANDLE_SIZE);
            if (packetSize == DAP_HANDLE_SIZE)
            {
                unpack((uint32_t *)buf, sizeof(usbip_stage2_header));

            #if (USE_WINUSB == 1)
                uint32_t resLength = item->length;
                send_stage2_submit_data((usbip_stage2_header *)buf, 0, item->buf, resLength);
            #else
                send_stage2_submit_data((usbip_stage2_header *)buf, 0, item->buf, DAP_HANDLE_SIZE);
            #endif
                

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
            //// TODO: ep0 dir 0 ? 
            buf[0x3] = 0x3; // command
            buf[0xF] = 0;  // direction
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
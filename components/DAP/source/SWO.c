/**
 * @file SWO.c
 * @author windowsair
 * @brief SWO support
 * @change: 2021-02-17: Add basic functions
 * @version 0.2
 *
 * @date 2021-02-17
 *
 * @copyright Copyright (c) 2021
 *
 */

/*
 * Copyright (c) 2013-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * $Date:        1. December 2017
 * $Revision:    V2.0.0
 *
 * Project:      CMSIS-DAP Source
 * Title:        SWO.c CMSIS-DAP SWO I/O
 *
 *---------------------------------------------------------------------------*/

#include "components/DAP/config/DAP_config.h"
#include "components/DAP/include/DAP.h"
#include "components/DAP/include/uart_modify.h"
#include "components/DAP/include/swo.h"

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


EventGroupHandle_t kSwoThreadEventGroup;


#if (SWO_STREAM != 0)
#ifdef DAP_FW_V1
#error "SWO Streaming Trace not supported in DAP V1!"
#endif
#endif

#if (SWO_UART != 0)

#ifndef USART_PORT
#define USART_PORT UART_NUM_0 /* USART Port Number */
#endif


// use in baudrate setting
static uint8_t USART_Ready = 0U;

#endif /* (SWO_UART != 0) */

#if ((SWO_UART != 0) || (SWO_MANCHESTER != 0))

#define SWO_STREAM_TIMEOUT pdMS_TO_TICKS(50) /* Stream timeout in ms */

#define USB_BLOCK_SIZE 512U  /* USB Block Size */
#define TRACE_BLOCK_SIZE 64U /* Trace Block Size (2^n: 32...512) */

// Trace State
static uint8_t  TraceTransport =  0U;       /* Trace Transport */
static uint8_t  TraceMode      =  0U;       /* Trace Mode */
static uint8_t  TraceStatus    =  0U;       /* Trace Status without Errors */
static uint8_t  TraceError[2]  = {0U, 0U};  /* Trace Error flags (banked) */
static uint8_t  TraceError_n   =  0U;       /* Active Trace Error bank */

// Trace Buffer
static uint8_t kSwoTraceBuf[SWO_BUFFER_SIZE];  /* Trace Buffer (must be 2^n) */
static volatile uint32_t TraceIndexI  = 0U; /* Incoming Trace Index */
static volatile uint32_t TraceIndexO  = 0U; /* Outgoing Trace Index */
static volatile uint8_t  TraceUpdate;       /* Trace Update Flag */
static          uint32_t TraceBlockSize;    /* Current Trace Block Size */

#if (TIMESTAMP_CLOCK != 0U)
// Trace Timestamp
static volatile struct
{
  uint32_t index;
  uint32_t tick;
} TraceTimestamp;
#endif

// Trace Helper functions
static void ClearTrace(void);
static void ResumeTrace(void);
static uint32_t GetTraceCount(void);
static uint8_t GetTraceStatus(void);

#if (SWO_STREAM != 0)

volatile uint8_t kSwoTransferBusy = 0U; /* Transfer Busy Flag */
static uint32_t TransferSize;           /* Current Transfer Size */
#endif

#if (SWO_UART != 0)



// Enable or disable UART SWO Mode
//   enable: enable flag
//   return: 1 - Success, 0 - Error
uint32_t UART_SWO_Mode(uint32_t enable)
{
  int32_t status;

  USART_Ready = 0U;
  uart_config_t uart_config = {
      .baud_rate = 74880,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
  my_uart_param_config(USART_PORT, &uart_config); // register setting

#define BUF_SIZE (1024)
//// TODO: remove this

  if (enable != 0U)
  {
    my_uart_param_config(USART_PORT, &uart_config);
    status = my_uart_driver_install(USART_PORT, BUF_SIZE, 0, 0, NULL, 0);
    if (status != ESP_OK)
    {
      my_uart_driver_delete(USART_PORT);
      return (0U);
    }
  }
  else
  {
    my_uart_driver_delete(USART_PORT);
  }

  return (1U);


}

// Configure UART SWO Baudrate
//   baudrate: requested baudrate
//   return:   actual baudrate or 0 when not configured
uint32_t UART_SWO_Baudrate(uint32_t baudrate)
{
  //// TODO: There may be bugs.
  //int32_t status;
  uint32_t index;
  uint32_t remain_trace_block_size;

  if (baudrate > SWO_UART_MAX_BAUDRATE)
  {
    baudrate = SWO_UART_MAX_BAUDRATE;
  }

  if (TraceStatus & DAP_SWO_CAPTURE_ACTIVE)
  {
    TraceIndexI += my_uart_get_rx_buffered_data_len(USART_PORT);
    my_uart_flush(USART_PORT);
    // pUSART->Control(ARM_USART_CONTROL_RX, 0U);
    // if (pUSART->GetStatus().rx_busy)
    // {
    //   TraceIndexI += pUSART->GetRxCount();
    //   pUSART->Control(ARM_USART_ABORT_RECEIVE, 0U);
    // }
  }

  /////////////
  my_uart_set_baudrate(USART_PORT, baudrate);

  USART_Ready = 1U;

  if (TraceStatus & DAP_SWO_CAPTURE_ACTIVE)
  {
    if ((TraceStatus & DAP_SWO_CAPTURE_PAUSED) == 0U)
    {
      index = TraceIndexI & (SWO_BUFFER_SIZE - 1U); // TraceIndexI % SWO_BUFFER_SIZE
      remain_trace_block_size = TRACE_BLOCK_SIZE - (index & (TRACE_BLOCK_SIZE - 1U)); // index % TRACE_BLOCK_SIZE
      TraceBlockSize = remain_trace_block_size;
      //pUSART->Receive(&kSwoTraceBuf[index], num);
      my_uart_read_bytes_async_swo(index, remain_trace_block_size);
    }
    //pUSART->Control(ARM_USART_CONTROL_RX, 1U);
  }

  return (baudrate);
}

// Control UART SWO Capture
//   active: active flag
//   return: 1 - Success, 0 - Error
uint32_t UART_SWO_Control(uint32_t active)
{
  int32_t status;

  if (active)
  {
    if (!USART_Ready)
    {
      return (0U);
    }
    TraceBlockSize = 1U;
    status = my_uart_read_bytes_async_swo(0, 1U);

    if (status == ESP_FAIL)
    {
      return (0U);
    }
    // status = pUSART->Control(ARM_USART_CONTROL_RX, 1U);
    // if (status != ARM_DRIVER_OK)
    // {
    //   return (0U);
    // }
  }
  else
  {
    // no active
    TraceIndexI += my_uart_get_rx_buffered_data_len(USART_PORT);
    my_uart_flush(USART_PORT);
    // pUSART->Control(ARM_USART_CONTROL_RX, 0U);
    // if (pUSART->GetStatus().rx_busy)
    // {
    //   TraceIndexI += pUSART->GetRxCount();
    //   pUSART->Control(ARM_USART_ABORT_RECEIVE, 0U);
    // }
  }
  return (1U);
}

// Start UART SWO Capture
//   index: trace buffer index to read
//   num: number of bytes to capture
static void UART_SWO_Capture(uint32_t index, uint32_t num)
{
  TraceBlockSize = num;
  my_uart_read_bytes_async_swo(index,  num);
}

// Get UART SWO Pending Trace Count
//   return: number of pending trace data bytes
static uint32_t UART_SWO_GetCount(void)
{
  //// TODO: There may be bugs.
  // if (pUSART->GetStatus().rx_busy)
  // {
  //   count = pUSART->GetRxCount();
  // }
  // else
  // {
  //   count = 0U;
  // }
  return my_uart_get_rx_buffered_data_len(USART_PORT);

}

#endif /* (SWO_UART != 0) */



//
// Trace status helper functions
//


// Clear Trace Errors and Data
static void ClearTrace(void)
{

#if (SWO_STREAM != 0)
  if (TraceTransport == 2U) // Indicates that we use WinUSB for transfer.
  {
    if (kSwoTransferBusy != 0U)
    {
      // Unfortunately, we cannot abort the transmission
      // SWO_AbortTransfer();
      kSwoTransferBusy = 0U;
    }
  }
#endif

  TraceError[0] = 0U;
  TraceError[1] = 0U;
  TraceError_n = 0U;
  TraceIndexI = 0U;
  TraceIndexO = 0U;

#if (TIMESTAMP_CLOCK != 0U)
  TraceTimestamp.index = 0U;
  TraceTimestamp.tick = 0U;
#endif
}

// Resume Trace Capture
static void ResumeTrace(void)
{
  uint32_t index_i;
  uint32_t index_o;

  if (TraceStatus == (DAP_SWO_CAPTURE_ACTIVE | DAP_SWO_CAPTURE_PAUSED))
  {
    index_i = TraceIndexI;
    index_o = TraceIndexO;
    if ((index_i - index_o) < SWO_BUFFER_SIZE)
    {
      index_i &= SWO_BUFFER_SIZE - 1U;
      switch (TraceMode)
      {
      case DAP_SWO_UART:
        TraceStatus = DAP_SWO_CAPTURE_ACTIVE;
        UART_SWO_Capture(index_i, 1U);
        break;
      default:
        break;
      }
    }
  }
}

// Get Trace Count
//   return: number of available data bytes in trace buffer
static uint32_t GetTraceCount(void)
{
  uint32_t count;

  if (TraceStatus == DAP_SWO_CAPTURE_ACTIVE)
  {
    do
    {
      TraceUpdate = 0U;
      count = TraceIndexI - TraceIndexO;
      switch (TraceMode)
      {
      case DAP_SWO_UART:
        count += UART_SWO_GetCount();
        break;
      default:
        break;
      }
      vTaskDelay(pdMS_TO_TICKS(10));
    } while (TraceUpdate != 0U);
    // Synchronously wait for the data to complete
  }
  else
  {
    count = TraceIndexI - TraceIndexO;
  }

  return (count);
}

// Get Trace Status (clear Error flags)
//   return: Trace Status (Active flag and Error flags)
static uint8_t GetTraceStatus(void)
{
  uint8_t status;
  uint32_t n;

  n = TraceError_n;
  TraceError_n ^= 1U;
  status = TraceStatus | TraceError[n];
  TraceError[n] = 0U;

  return (status);
}

// Set Trace Error flag(s)
//   flag:  error flag(s) to set
void SetTraceError(uint8_t flag)
{
  TraceError[TraceError_n] |= flag;
}

// Process SWO Transport command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_Transport(const uint8_t *request, uint8_t *response)
{
  uint8_t transport;
  uint32_t result;

  if ((TraceStatus & DAP_SWO_CAPTURE_ACTIVE) == 0U)
  {
    transport = *request;
    switch (transport)
    {
    case 0U:
    case 1U:
#if (SWO_STREAM != 0)
    case 2U:
#endif
      TraceTransport = transport;
      result = 1U;
      break;
    default:
      result = 0U;
      break;
    }
  }
  else
  {
    result = 0U;
  }

  *response = result ? DAP_OK : DAP_ERROR;

  return ((1U << 16) | 1U);
}

// Process SWO Mode command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_Mode(const uint8_t *request, uint8_t *response)
{
  uint8_t mode;
  uint32_t result;

  mode = *request;

  // disable swo mode
  switch (TraceMode)
  {
#if (SWO_UART != 0)
  case DAP_SWO_UART:
    UART_SWO_Mode(0U);
    break;
#endif

  default:
    break;
  }

  switch (mode)
  {
  case DAP_SWO_OFF:
    result = 1U;
    break;
#if (SWO_UART != 0)
  case DAP_SWO_UART:
    result = UART_SWO_Mode(1U);
    break;
#endif

  default:
    result = 0U;
    break;
  }

  // DAP_SWO_OFF -> has error
  TraceMode = result ? mode : DAP_SWO_OFF;

  TraceStatus = 0U;

  *response = result ? DAP_OK : DAP_ERROR;

  return ((1U << 16) | 1U);
}

// Process SWO Baudrate command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_Baudrate(const uint8_t *request, uint8_t *response)
{
  uint32_t baudrate;

  baudrate = (uint32_t)(*(request + 0) << 0) |
             (uint32_t)(*(request + 1) << 8) |
             (uint32_t)(*(request + 2) << 16) |
             (uint32_t)(*(request + 3) << 24);

  switch (TraceMode)
  {
#if (SWO_UART != 0)
  case DAP_SWO_UART:
    baudrate = UART_SWO_Baudrate(baudrate);
    break;
#endif

  default:
    baudrate = 0U;
    break;
  }

  if (baudrate == 0U)
  {
    TraceStatus = 0U;
  }

  *response++ = (uint8_t)(baudrate >> 0);
  *response++ = (uint8_t)(baudrate >> 8);
  *response++ = (uint8_t)(baudrate >> 16);
  *response = (uint8_t)(baudrate >> 24);

  return ((4U << 16) | 4U);
}

// Process SWO Control command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_Control(const uint8_t *request, uint8_t *response)
{
  uint8_t active;
  uint32_t result;

  active = *request & DAP_SWO_CAPTURE_ACTIVE;

  if (active != (TraceStatus & DAP_SWO_CAPTURE_ACTIVE))
  {
    // active status: request != now status

    if (active) // request: active
    {
      ClearTrace();
    }
    switch (TraceMode)
    {
#if (SWO_UART != 0)
    case DAP_SWO_UART:
      result = UART_SWO_Control(active);
      break;
#endif

    default:
      result = 0U;
      break;
    }

    if (result != 0U)
    {
      // success done
      TraceStatus = active;
#if (SWO_STREAM != 0)
      if (TraceTransport == 2U) // Indicates that we use WinUSB for transfer.
      {
        xEventGroupSetBits(kSwoThreadEventGroup, SWO_GOT_DATA);
      }
#endif
    }
  }
  else
  {
    // request: active but already actived
    result = 1U;
  }


  *response = result ? DAP_OK : DAP_ERROR;

  return ((1U << 16) | 1U);
}

// Process SWO Status command and prepare response
//   response: pointer to response data
//   return:   number of bytes in response
uint32_t SWO_Status(uint8_t *response)
{
  uint8_t status;
  uint32_t count;

  status = GetTraceStatus();
  count = GetTraceCount();

  *response++ = status;
  *response++ = (uint8_t)(count >> 0);
  *response++ = (uint8_t)(count >> 8);
  *response++ = (uint8_t)(count >> 16);
  *response = (uint8_t)(count >> 24);

  return (5U);
}

// Process SWO Extended Status command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_ExtendedStatus(const uint8_t *request, uint8_t *response)
{
  uint8_t cmd;
  uint8_t status;
  uint32_t count;
#if (TIMESTAMP_CLOCK != 0U)
  uint32_t index;
  uint32_t tick;
#endif
  uint32_t num;

  num = 0U;
  cmd = *request;

  if (cmd & 0x01U)
  {
    status = GetTraceStatus();
    *response++ = status;
    num += 1U;
  }

  if (cmd & 0x02U)
  {
    count = GetTraceCount();
    *response++ = (uint8_t)(count >> 0);
    *response++ = (uint8_t)(count >> 8);
    *response++ = (uint8_t)(count >> 16);
    *response++ = (uint8_t)(count >> 24);
    num += 4U;
  }

#if (TIMESTAMP_CLOCK != 0U)
  if (cmd & 0x04U)
  {
    do
    {
      TraceUpdate = 0U;
      index = TraceTimestamp.index;
      tick = TraceTimestamp.tick;
      vTaskDelay(pdMS_TO_TICKS(10));
    } while (TraceUpdate != 0U);
    *response++ = (uint8_t)(index >> 0);
    *response++ = (uint8_t)(index >> 8);
    *response++ = (uint8_t)(index >> 16);
    *response++ = (uint8_t)(index >> 24);
    *response++ = (uint8_t)(tick >> 0);
    *response++ = (uint8_t)(tick >> 8);
    *response++ = (uint8_t)(tick >> 16);
    *response++ = (uint8_t)(tick >> 24);
    num += 4U;
  }
#endif

  return ((1U << 16) | num);
}

// Process SWO Data command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response (lower 16 bits)
//             number of bytes in request (upper 16 bits)
uint32_t SWO_Data(const uint8_t *request, uint8_t *response)
{
  uint8_t status;
  uint32_t count;
  uint32_t index;
  uint32_t n, i;

  status = GetTraceStatus();
  count = GetTraceCount();

  // transport 1: use DAP SWO command
  // transport 2: WinUSB
  if (TraceTransport == 1U)
  {
    n = (uint32_t)(*(request + 0) << 0) |
        (uint32_t)(*(request + 1) << 8);
    if (n > (DAP_PACKET_SIZE - 4U))
    {
      n = DAP_PACKET_SIZE - 4U;
    }
    if (count > n)
    {
      count = n;
    }
  }
  else
  {
    // if use winusb, then nothing to do.
    count = 0U;
  }

  *response++ = status;
  *response++ = (uint8_t)(count >> 0);
  *response++ = (uint8_t)(count >> 8);

  if (TraceTransport == 1U)
  {
    index = TraceIndexO;
    for (i = index, n = count; n; n--)
    {
      i &= SWO_BUFFER_SIZE - 1U;
      *response++ = kSwoTraceBuf[i++];
    }
    TraceIndexO = index + count;
    ResumeTrace();
  }

  return ((2U << 16) | (3U + count));
}

#if (SWO_STREAM != 0)

// SWO Data Transfer complete callback
void SWO_TransferComplete(void)
{
  TraceIndexO += TransferSize;
  kSwoTransferBusy = 0U;
  ResumeTrace();
  xEventGroupSetBits(kSwoThreadEventGroup, SWO_GOT_DATA);
}

// SWO Thread
void SWO_Thread()
{
  uint32_t flags;
  uint32_t count;
  uint32_t index;
  uint32_t i, n;

  uint32_t index_i;
  uint32_t index_o;
  uint32_t remain_trace_block_size;

  TickType_t timeout = portMAX_DELAY;

  kSwoThreadEventGroup = xEventGroupCreate();
  for (;;)
  {
    /*
      Here, our event group handles two main types of events:
      1. `SWO_GOT_DATA`  : The SWO package may be ready to send.
      2. `UART_GOT_DATA` : The uart transfer event is complete and we can start reading.

      Note that the `SWO_ERROR_TIME_OUT` flag is only used for internal processing, only when the status is
      inactive can it be set.

      We deal with uart events first.
      Currently, SWO events are always handled.
    */
    flags = xEventGroupWaitBits(kSwoThreadEventGroup, SWO_GOT_DATA | UART_GOT_DATA | SWO_ERROR_TIME_OUT,
                                pdTRUE, pdFALSE, timeout);

    if (!(flags & UART_GOT_DATA) && !(flags & SWO_GOT_DATA))
    {
      if (TraceMode != DAP_SWO_OFF && my_uart_get_rx_buffered_data_len(USART_PORT) >= kSWO_read_num)
      {
        flags = UART_GOT_DATA;
      }
    }

    if (flags & UART_GOT_DATA)
    {
      // The data is still in the uart buffer,
      // and the data in the uart needs to be read into the SWO buffer
      my_uart_read_bytes(USART_PORT, &kSwoTraceBuf[kSWO_read_index], kSWO_read_num, pdMS_TO_TICKS(20));


      index_o = TraceIndexO;

      TraceIndexI += TraceBlockSize;
	    index_i = TraceIndexI;

#if (TIMESTAMP_CLOCK != 0U)
      TraceTimestamp.tick = TIMESTAMP_GET();
      TraceTimestamp.index = index_i;
#endif
      remain_trace_block_size = TRACE_BLOCK_SIZE - (index_i & (TRACE_BLOCK_SIZE - 1U));
      // remain_trace_block_size is the number of bytes we need to read
      // (to achieve the size of TRACE_BLOCK_SIZE)
      count = index_i - index_o;
      // Amount of data that has not been processed yet

      // (SWO_BUFFER_SIZE-num): the remaining usable length of the buffer after reading this data
      if (count <= (SWO_BUFFER_SIZE - remain_trace_block_size))
      {
        // index_i % 4096
        index_i &= SWO_BUFFER_SIZE - 1U;
        TraceBlockSize = remain_trace_block_size;
        my_uart_read_bytes_async_swo(index_i, remain_trace_block_size);
        //pUSART->Receive(&kSwoTraceBuf[index_i], num);
      }
      else
      {
        // Not enough buffers
        TraceStatus = DAP_SWO_CAPTURE_ACTIVE | DAP_SWO_CAPTURE_PAUSED;
      }
      TraceUpdate = 1U;
#if (SWO_STREAM != 0)
      if (TraceTransport == 2U) // Indicates that we use WinUSB for transfer.
      {
        // unsent length >= remaining length of the USB block
        //// TODO:  may be we can remove this
        if (count >= (USB_BLOCK_SIZE - (index_o & (USB_BLOCK_SIZE - 1U))))
        {
          flags = SWO_GOT_DATA;
        }
      }
#endif
    }

    // xEventGroupClearBits

    /*
      `SWO_GOT_DATA` may be set in the following situations:
      - The SWO status requested by `SWD_contorl` is different from the current SWO status.
      - When `SWO_TransferComplete` is called, and there may be potentially unsent data in the buffer.
      - monitor task believes we need to send data that is already long enough.

      Note: The processing of the following code segment will always be executed,
      and should be ensured to be stable enough.


      TODO: The time interval for executing the code is not guaranteed.
    */

    if (!(TraceStatus & DAP_SWO_CAPTURE_ACTIVE))
    {
      flags = SWO_ERROR_TIME_OUT;
      timeout = portMAX_DELAY;
    }
    else
    {
      timeout = pdMS_TO_TICKS(150);
    }

    if (kSwoTransferBusy)
    {
      continue;
    }

    count = GetTraceCount();
    if (count != 0U)
    {
      index = TraceIndexO & (SWO_BUFFER_SIZE - 1U);
      n = SWO_BUFFER_SIZE - index;
      if (count > n)
      {
        count = n;
      }
      if ((flags & SWO_ERROR_TIME_OUT) == 0)
      {
        i = index & (USB_BLOCK_SIZE - 1U);
        if (i == 0U)
        {
          count &= ~(USB_BLOCK_SIZE - 1U); // Take down to the nearest number that is a multiple of USB_BLOCK_SIZE
        }
        else
        {
          n = USB_BLOCK_SIZE - i;
          if (count >= n)
          {
            count = n; // The number of bytes to be sent exceeds the remain USB block size.
          }
          else
          {
            count = 0U; // Haven't received a full USB block yet.
          }
        }
      }
      // Notify that there is data available for transmission
      if (count != 0U)
      {
        TransferSize = count;
        kSwoTransferBusy = 1U;
        SWO_QueueTransfer(&kSwoTraceBuf[index], count); // through USB
      }
    }

  }

  // For all exceptions, we have handled them directly in the interrupt handlers.
}

#endif /* (SWO_STREAM != 0) */

#endif /* ((SWO_UART != 0) || (SWO_MANCHESTER != 0)) */

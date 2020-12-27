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
 * Title:        SW_DP.c CMSIS-DAP SW DP I/O
 *
 *---------------------------------------------------------------------------*/

#include "DAP_config.h"
#include "DAP.h"

#include "spi_op.h"
#include "spi_switch.h"
#include "dap_utility.h"

// Debug 
#define PRINT_SWD_PROTOCOL 0

// SW Macros

#define PIN_SWCLK_SET PIN_SWCLK_TCK_SET
#define PIN_SWCLK_CLR PIN_SWCLK_TCK_CLR

#define SW_CLOCK_CYCLE()                \
  PIN_SWCLK_CLR();                      \
  PIN_DELAY();                          \
  PIN_SWCLK_SET();                      \
  PIN_DELAY()

#define SW_WRITE_BIT(bit)               \
  PIN_SWDIO_OUT(bit);                   \
  PIN_SWCLK_CLR();                      \
  PIN_DELAY();                          \
  PIN_SWCLK_SET();                      \
  PIN_DELAY()

#define SW_READ_BIT(bit)                \
  PIN_SWCLK_CLR();                      \
  PIN_DELAY();                          \
  bit = PIN_SWDIO_IN();                 \
  PIN_SWCLK_SET();                      \
  PIN_DELAY()

//#define PIN_DELAY() PIN_DELAY_SLOW(DAP_Data.clock_delay)
#define PIN_DELAY() PIN_DELAY_FAST()


// Generate SWJ Sequence
//   count:  sequence bit count
//   data:   pointer to sequence bit data
//   return: none
#if ((DAP_SWD != 0) || (DAP_JTAG != 0))
void SWJ_Sequence (uint32_t count, const uint8_t *data) {
  if (count != 8 && count != 16 && count!= 51)
  {
    printf("[ERROR] wrong SWJ Swquence length:%d\r\n", (int)count);
    return;
  }
  DAP_SPI_Enable();
  DAP_SPI_WriteBits(count, data);
}
#endif


// Generate SWD Sequence
//   info:   sequence information
//   swdo:   pointer to SWDIO generated data
//   swdi:   pointer to SWDIO captured data
//   return: none
#if (DAP_SWD != 0)
void SWD_Sequence (uint32_t info, const uint8_t *swdo, uint8_t *swdi) {
  uint32_t val;
  uint32_t bit;
  uint32_t n, k;

  n = info & SWD_SEQUENCE_CLK;
  if (n == 0U) {
    n = 64U;
  }

  if (info & SWD_SEQUENCE_DIN) {
    while (n) {
      val = 0U;
      for (k = 8U; k && n; k--, n--) {
        SW_READ_BIT(bit);
        val >>= 1;
        val  |= bit << 7;
      }
      val >>= k;
      *swdi++ = (uint8_t)val;
    }
  } else {
    while (n) {
      val = *swdo++;
      for (k = 8U; k && n; k--, n--) {
        SW_WRITE_BIT(val);
        val >>= 1;
      }
    }
  }
}
#endif


#if (DAP_SWD != 0)


// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]

//// TODO: low speed
#define SWD_TransferFunction(speed)     /* Speed may be useless, because all use this function */                            \
static uint8_t SWD_Transfer##speed (uint32_t request, uint32_t *data) {         \
          SWD_Transfer_Common(request,data);                                    \
					return 1;																															\
}

static uint8_t SWD_Transfer_Common (uint32_t request, uint32_t *data) {         
  uint8_t ack;                                                                 
  // uint32_t bit;                                                                 
  uint32_t val;                                                                 
  uint8_t parity;
  uint8_t computedParity;                                               
                                                                                
  uint32_t n;

  int retryCount = 0;                                            
  const uint8_t constantBits = 0b10000001U; /* Start Bit  & Stop Bit & Park Bit is fixed. */  
  uint8_t requestByte;  /* LSB */


  DAP_SPI_Enable();
  do {
    requestByte = constantBits | (((uint8_t)(request & 0xFU)) << 1U) | (ParityEvenUint8(request & 0xFU) << 5U);
    /* For 4bit, Parity can be equivalent to 8bit with all 0 high bits */
    
    #if (PRINT_SWD_PROTOCOL == 1)
    switch (requestByte)
      {
      case 0xA5U:
        printf("IDCODE\r\n");
        break;
      case 0xA9U:
        printf("W CTRL/STAT\r\n");
        break;
      case 0xBDU:
        printf("RDBUFF\r\n");
        break;
      case 0x8DU:
        printf("R CTRL/STAT\r\n");
        break;
      case 0x81U:
        printf("W ABORT\r\n");
        break;
      case 0xB1U:
        printf("W SELECT\r\n");
        break;
      case 0xBBU:
        printf("W APc\r\n");
        break;
      case 0x9FU:
        printf("R APc\r\n");
        break;
      case 0x8BU:
        printf("W AP4\r\n");
        break;
      case 0xA3U:
        printf("W AP0\r\n");
        break;
      case 0X87U:
        printf("R AP0\r\n");
        break;
      case 0xB7U:
        printf("R AP8\r\n");
        break;
      default:
      //W AP8
        printf("Unknown:%08x\r\n", requestByte);
        break;
      }
    #endif



    if (request & DAP_TRANSFER_RnW) {
      /* Read data */

      DAP_SPI_Send_Header(requestByte, &ack, 0); // 0 Trn After ACK
      if (ack == DAP_TRANSFER_OK) {
        DAP_SPI_Read_Data(&val, &parity);
        computedParity = ParityEvenUint32(val);

        if ((computedParity ^ parity) & 1U) {
          ack = DAP_TRANSFER_ERROR;
        }
        if (data) { *data = val; }

        /* Capture Timestamp */
        if (request & DAP_TRANSFER_TIMESTAMP) {
          DAP_Data.timestamp = TIMESTAMP_GET();
        }

      } 
      else if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT)) {
        DAP_SPI_Generate_Cycle(1);
        #if (PRINT_SWD_PROTOCOL == 1)
        printf("WAIT\r\n");
        #endif 
        
        continue;
        // return DAP_TRANSFER_WAIT;
      } 
      else {
        /* Protocol error */
        DAP_SPI_Disable();
        PIN_SWDIO_TMS_SET();

        DAP_SPI_Enable();
        DAP_SPI_Protocol_Error_Read();
        
        DAP_SPI_Disable();
        PIN_SWDIO_TMS_SET();
        printf("Protocol Error: Read\r\n");
      }

      return ((uint8_t)ack);
    }
    else {
      /* Write data */
      parity = ParityEvenUint32(*data);
      DAP_SPI_Send_Header(requestByte, &ack, 1); // 1 Trn After ACK
      if (ack == DAP_TRANSFER_OK) {
        DAP_SPI_Write_Data(*data, parity);
        /* Capture Timestamp */
        if (request & DAP_TRANSFER_TIMESTAMP) {
          DAP_Data.timestamp = TIMESTAMP_GET();
        }
        /* Idle cycles */
        n = DAP_Data.transfer.idle_cycles;
        if (n) { DAP_SPI_Generate_Cycle(n); }
      
        DAP_SPI_Disable();
        PIN_SWDIO_TMS_SET();

        return ((uint8_t)ack);
      }
      else if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT)) {
        /* already turnaround. */

        /* TODO: overrun transfer -> for read */
        #if (PRINT_SWD_PROTOCOL == 1)
        printf("WAIT\r\n");
        #endif

        continue;

      }
      else {
        //// FIXME: bug
        /* Protocol error */
        DAP_SPI_Disable();
        PIN_SWDIO_TMS_SET();

        DAP_SPI_Enable();
        DAP_SPI_Protocol_Error_Write();
        
        DAP_SPI_Disable();
        PIN_SWDIO_TMS_SET();
        printf("Protocol Error: Write\r\n");
      }

      return ((uint8_t)ack);

    }
  } while (retryCount++ < 99);

  return DAP_TRANSFER_ERROR;
  
                                                                                                                          
}


#undef  PIN_DELAY
#define PIN_DELAY() PIN_DELAY_FAST()
SWD_TransferFunction(Fast)

#undef  PIN_DELAY
#define PIN_DELAY() PIN_DELAY_SLOW(DAP_Data.clock_delay)
SWD_TransferFunction(Slow)


// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]
uint8_t  SWD_Transfer(uint32_t request, uint32_t *data) {
  if (DAP_Data.fast_clock) {
    return SWD_TransferFast(request, data);
  } else {
    return SWD_TransferSlow(request, data);
  }
}


#endif  /* (DAP_SWD != 0) */

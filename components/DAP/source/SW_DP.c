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

/**
 * @file SW_DP.c
 * @author windowsair
 * @brief Adaptation of GPIO and SPI
 * @change:
 *    2021-2-10 Support GPIO and SPI for SWD sequence / SWJ sequence / SWD transfer
 *              Note: SWD sequence not yet tested
 * @version 0.1
 * @date 2021-2-10
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdio.h>

#include "components/DAP/config/DAP_config.h"
#include "components/DAP/include/DAP.h"
#include "components/DAP/include/spi_op.h"
#include "components/DAP/include/spi_switch.h"
#include "components/DAP/include/dap_utility.h"


// Debug
#define PRINT_SWD_PROTOCOL 0

// SW Macros

#define PIN_DELAY() PIN_DELAY_SLOW(DAP_Data.clock_delay)

#define PIN_SWCLK_SET PIN_SWCLK_TCK_SET
#define PIN_SWCLK_CLR PIN_SWCLK_TCK_CLR

// Space for time in the original version,
// and time for space in our implementation

#define SW_CLOCK_CYCLE()                \
  PIN_SWCLK_CLR();                      \
  if (need_delay) { PIN_DELAY(); }      \
  PIN_SWCLK_SET();                      \
  if (need_delay) { PIN_DELAY(); }

#define SW_WRITE_BIT(bit)               \
  PIN_SWDIO_OUT(bit);                   \
  PIN_SWCLK_CLR();                      \
  if (need_delay) { PIN_DELAY(); }      \
  PIN_SWCLK_SET();                      \
  if (need_delay) { PIN_DELAY(); }

#define SW_READ_BIT(bit)                \
  PIN_SWCLK_CLR();                      \
  if (need_delay) { PIN_DELAY(); }      \
  bit = PIN_SWDIO_IN();                 \
  PIN_SWCLK_SET();                      \
  if (need_delay) { PIN_DELAY(); }



uint8_t SWD_TransferSpeed = kTransfer_GPIO_normal;


void SWJ_Sequence_GPIO (uint32_t count, const uint8_t *data, uint8_t need_delay);
void SWJ_Sequence_SPI (uint32_t count, const uint8_t *data);

void SWD_Sequence_GPIO (uint32_t info, const uint8_t *swdo, uint8_t *swdi);
void SWD_Sequence_SPI (uint32_t info, const uint8_t *swdo, uint8_t *swdi);


// Generate SWJ Sequence
//   count:  sequence bit count
//   data:   pointer to sequence bit data
//   return: none
#if ((DAP_SWD != 0) || (DAP_JTAG != 0))
void SWJ_Sequence (uint32_t count, const uint8_t *data) {
  // if (count != 8 && count != 16 && count!= 51)
  // {
  //   os_printf("[ERROR] wrong SWJ Swquence length:%d\r\n", (int)count);
  //   return;
  // }

  if(SWD_TransferSpeed == kTransfer_SPI) {
    SWJ_Sequence_SPI(count, data);
  } else {
    SWJ_Sequence_GPIO(count, data, 1);
  }

}


void SWJ_Sequence_GPIO (uint32_t count, const uint8_t *data, uint8_t need_delay) {
    uint32_t val;
    uint32_t n;

    val = 0U;
    n = 0U;
    while (count--) {
      if (n == 0U) {
        val = *data++;
        n = 8U;
      }
      if (val & 1U) {
        PIN_SWDIO_TMS_SET();
      } else {
        PIN_SWDIO_TMS_CLR();
      }
      SW_CLOCK_CYCLE();
      val >>= 1;
      n--;
    }
}

void SWJ_Sequence_SPI (uint32_t count, const uint8_t *data) {
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
  if (SWD_TransferSpeed == kTransfer_SPI) {
    SWD_Sequence_SPI(info, swdo, swdi);
  } else {
    SWD_Sequence_GPIO(info, swdo, swdi);
  }
}

void SWD_Sequence_GPIO (uint32_t info, const uint8_t *swdo, uint8_t *swdi) {
  const uint8_t need_delay = 1;

  uint32_t val;
  uint32_t bit;
  uint32_t n, k;

  n = info & SWD_SEQUENCE_CLK;
  if (n == 0U) {
    n = 64U;
  }
  // n = 1 ~ 64

  // LSB
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

void SWD_Sequence_SPI (uint32_t info, const uint8_t *swdo, uint8_t *swdi) {
  uint32_t n;
  n = info & SWD_SEQUENCE_CLK;
  if (n == 0U) {
    n = 64U;
  }
  // n = 1 ~ 64

  if (info & SWD_SEQUENCE_DIN) {
    DAP_SPI_ReadBits(n, swdi);
  } else {
    DAP_SPI_WriteBits(n, swdo);
  }
}

#endif


#if (DAP_SWD != 0)


// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]
static uint8_t SWD_Transfer_SPI (uint32_t request, uint32_t *data) {
  //// FIXME: overrun detection
  // SPI transfer mode does not require operations such as PIN_DELAY
  uint8_t ack;
  // uint32_t bit;
  uint32_t val;
  uint8_t parity;
  uint8_t computedParity;

  uint32_t n;

  const uint8_t constantBits = 0b10000001U; /* Start Bit  & Stop Bit & Park Bit is fixed. */
  uint8_t requestByte;  /* LSB */


  DAP_SPI_Enable();

  requestByte = constantBits | (((uint8_t)(request & 0xFU)) << 1U) | (ParityEvenUint8(request & 0xFU) << 5U);
  /* For 4bit, Parity can be equivalent to 8bit with all 0 high bits */

#if (PRINT_SWD_PROTOCOL == 1)
  switch (requestByte)
    {
    case 0xA5U:
      os_printf("IDCODE\r\n");
      break;
    case 0xA9U:
      os_printf("W CTRL/STAT\r\n");
      break;
    case 0xBDU:
      os_printf("RDBUFF\r\n");
      break;
    case 0x8DU:
      os_printf("R CTRL/STAT\r\n");
      break;
    case 0x81U:
      os_printf("W ABORT\r\n");
      break;
    case 0xB1U:
      os_printf("W SELECT\r\n");
      break;
    case 0xBBU:
      os_printf("W APc\r\n");
      break;
    case 0x9FU:
      os_printf("R APc\r\n");
      break;
    case 0x8BU:
      os_printf("W AP4\r\n");
      break;
    case 0xA3U:
      os_printf("W AP0\r\n");
      break;
    case 0X87U:
      os_printf("R AP0\r\n");
      break;
    case 0xB7U:
      os_printf("R AP8\r\n");
      break;
    default:
    //W AP8
      os_printf("Unknown:%08x\r\n", requestByte);
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
#if defined CONFIG_IDF_TARGET_ESP8266 || defined CONFIG_IDF_TARGET_ESP32
      DAP_SPI_Generate_Cycle(1);
#elif defined CONFIG_IDF_TARGET_ESP32C3 || defined CONFIG_IDF_TARGET_ESP32S3
      DAP_SPI_Fast_Cycle();
#endif

#if (PRINT_SWD_PROTOCOL == 1)
      os_printf("WAIT\r\n");
#endif

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
#if (PRINT_SWD_PROTOCOL == 1)
      os_printf("Protocol Error: Read\r\n");
#endif
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
      os_printf("WAIT\r\n");
#endif

    }
    else {
      /* Protocol error */
      DAP_SPI_Disable();
      PIN_SWDIO_TMS_SET();

      DAP_SPI_Enable();
      DAP_SPI_Protocol_Error_Write();

      DAP_SPI_Disable();
      PIN_SWDIO_TMS_SET();

#if (PRINT_SWD_PROTOCOL == 1)
      os_printf("Protocol Error: Write\r\n");
#endif
    }

    return ((uint8_t)ack);

  }

  return DAP_TRANSFER_ERROR;
}



static uint8_t SWD_Transfer_GPIO (uint32_t request, uint32_t *data, uint8_t need_delay) {
  uint32_t ack;
  uint32_t bit;
  uint32_t val;
  uint32_t parity;

  uint32_t n;

  /* Packet Request */
  parity = 0U;
  SW_WRITE_BIT(1U);                     /* Start Bit */
  bit = request >> 0;
  SW_WRITE_BIT(bit);                    /* APnDP Bit */
  parity += bit;
  bit = request >> 1;
  SW_WRITE_BIT(bit);                    /* RnW Bit */
  parity += bit;
  bit = request >> 2;
  SW_WRITE_BIT(bit);                    /* A2 Bit */
  parity += bit;
  bit = request >> 3;
  SW_WRITE_BIT(bit);                    /* A3 Bit */
  parity += bit;
  SW_WRITE_BIT(parity);                 /* Parity Bit */
  SW_WRITE_BIT(0U);                     /* Stop Bit */
  SW_WRITE_BIT(1U);                     /* Park Bit */

  /* Turnaround */
  PIN_SWDIO_OUT_DISABLE();
  for (n = DAP_Data.swd_conf.turnaround; n; n--) {
    SW_CLOCK_CYCLE();
  }

  /* Acknowledge response */
  SW_READ_BIT(bit);
  ack  = bit << 0;
  SW_READ_BIT(bit);
  ack |= bit << 1;
  SW_READ_BIT(bit);
  ack |= bit << 2;

  if (ack == DAP_TRANSFER_OK) {         /* OK response */
    /* Data transfer */
    if (request & DAP_TRANSFER_RnW) {
      /* Read data */
      val = 0U;
      parity = 0U;
      for (n = 32U; n; n--) {
        SW_READ_BIT(bit);               /* Read RDATA[0:31] */
        parity += bit;
        val >>= 1;
        val  |= bit << 31;
      }
      SW_READ_BIT(bit);                 /* Read Parity */
      if ((parity ^ bit) & 1U) {
        ack = DAP_TRANSFER_ERROR;
      }
      if (data) { *data = val; }
      /* Turnaround */
      for (n = DAP_Data.swd_conf.turnaround; n; n--) {
        SW_CLOCK_CYCLE();
      }
      PIN_SWDIO_OUT_ENABLE();
    } else {
      /* Turnaround */
      for (n = DAP_Data.swd_conf.turnaround; n; n--) {
        SW_CLOCK_CYCLE();
      }
      PIN_SWDIO_OUT_ENABLE();
      /* Write data */
      val = *data;
      parity = 0U;
      for (n = 32U; n; n--) {
        SW_WRITE_BIT(val);              /* Write WDATA[0:31] */
        parity += val;
        val >>= 1;
      }
      SW_WRITE_BIT(parity);             /* Write Parity Bit */
    }
    /* Capture Timestamp */
    if (request & DAP_TRANSFER_TIMESTAMP) {
      DAP_Data.timestamp = TIMESTAMP_GET();
    }
    /* Idle cycles */
    n = DAP_Data.transfer.idle_cycles;
    if (n) {
      PIN_SWDIO_OUT(0U);
      for (; n; n--) {
        SW_CLOCK_CYCLE();
      }
    }
    PIN_SWDIO_OUT(1U);
    return ((uint8_t)ack);
  }

  if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT)) {
    /* WAIT or FAULT response */
    if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) != 0U)) {
      for (n = 32U+1U; n; n--) {
        SW_CLOCK_CYCLE();               /* Dummy Read RDATA[0:31] + Parity */
      }
    }
    /* Turnaround */
    for (n = DAP_Data.swd_conf.turnaround; n; n--) {
      SW_CLOCK_CYCLE();
    }
    PIN_SWDIO_OUT_ENABLE();
    if (DAP_Data.swd_conf.data_phase && ((request & DAP_TRANSFER_RnW) == 0U)) {
      PIN_SWDIO_OUT(0U);
      for (n = 32U+1U; n; n--) {
        SW_CLOCK_CYCLE();               /* Dummy Write WDATA[0:31] + Parity */
      }
    }
    PIN_SWDIO_OUT(1U);
    return ((uint8_t)ack);
  }

  /* Protocol error */
  for (n = DAP_Data.swd_conf.turnaround + 32U + 1U; n; n--) {
    SW_CLOCK_CYCLE();                   /* Back off data phase */
  }
  PIN_SWDIO_OUT_ENABLE();
  PIN_SWDIO_OUT(1U);
  return ((uint8_t)ack);
}


// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]
uint8_t  SWD_Transfer(uint32_t request, uint32_t *data) {
  switch (SWD_TransferSpeed) {
    case kTransfer_SPI:
      return SWD_Transfer_SPI(request, data);
    case kTransfer_GPIO_fast:
      return SWD_Transfer_GPIO(request, data, 0);
    case kTransfer_GPIO_normal:
      return SWD_Transfer_GPIO(request, data, 1);
    default:
      return SWD_Transfer_GPIO(request, data, 1);
  }
}


#endif  /* (DAP_SWD != 0) */

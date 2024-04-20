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
 * $Date:        16. June 2021
 * $Revision:    V2.1.0
 *
 * Project:      CMSIS-DAP Configuration
 * Title:        DAP_config.h CMSIS-DAP Configuration File (Template)
 *
 *---------------------------------------------------------------------------*/

/**
 * @file DAP_config.h
 * @author windowsair
 * @brief Adaptation of GPIO and SPI pin
 * @change: 2021-2-10 Support GPIO and SPI
 *          2021-2-18 Try to support SWO
 *          2024-1-28 Update to CMSIS-DAP v2.1.0
 * @version 0.3
 * @date 2024-1-28
 *
 * @copyright Copyright (c) 2021-2024
 *
 */


#ifndef __DAP_CONFIG_H__
#define __DAP_CONFIG_H__

#include <stdint.h>
#include <string.h>

#include "main/dap_configuration.h"
#include "main/wifi_configuration.h"
#include "main/timer.h"

#include "components/DAP/include/cmsis_compiler.h"
#include "components/DAP/include/gpio_op.h"
#include "components/DAP/include/spi_switch.h"


#ifdef CONFIG_IDF_TARGET_ESP8266
  #include "gpio.h"
  #include "esp8266/include/esp8266/gpio_struct.h"
  #include "esp8266/pin_mux_register.h"
#elif defined CONFIG_IDF_TARGET_ESP32
#elif defined CONFIG_IDF_TARGET_ESP32C3
#elif defined CONFIG_IDF_TARGET_ESP32S3
#else
  #error unknown hardware
#endif



//**************************************************************************************************
/**
\defgroup DAP_Config_Debug_gr CMSIS-DAP Debug Unit Information
\ingroup DAP_ConfigIO_gr
@{
Provides definitions about the hardware and configuration of the Debug Unit.

This information includes:
 - Definition of Cortex-M processor parameters used in CMSIS-DAP Debug Unit.
 - Debug Unit Identification strings (Vendor, Product, Serial Number).
 - Debug Unit communication packet size.
 - Debug Access Port supported modes and settings (JTAG/SWD and SWO).
 - Optional information about a connected Target Device (for Evaluation Boards).
*/

//#ifdef _RTE_
//#include "RTE_Components.h"
//#include CMSIS_device_header
//#else
//#include "device.h"                             // Debug Unit Cortex-M Processor Header File
//#endif

/// Processor Clock of the Cortex-M MCU used in the Debug Unit.
/// This value is used to calculate the SWD/JTAG clock speed.
#ifdef CONFIG_IDF_TARGET_ESP8266
  #define CPU_CLOCK 160000000 ///< Specifies the CPU Clock in Hz.
  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<160MHz
#elif defined CONFIG_IDF_TARGET_ESP32
  #define CPU_CLOCK 240000000
  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<240MHz
#elif defined CONFIG_IDF_TARGET_ESP32C3
  #define CPU_CLOCK 16000000
  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<160MHz
#elif defined CONFIG_IDF_TARGET_ESP32S3
  #define CPU_CLOCK 240000000
#endif



//#define MAX_USER_CLOCK 16000000 ///< Specifies the max Debug Clock in Hz.

/// Number of processor cycles for I/O Port write operations.
/// This value is used to calculate the SWD/JTAG clock speed that is generated with I/O
/// Port write operations in the Debug Unit by a Cortex-M MCU. Most Cortex-M processors
/// require 2 processor cycles for a I/O Port Write operation.  If the Debug Unit uses
/// a Cortex-M0+ processor with high-speed peripheral I/O only 1 processor cycle might be
/// required.
#define IO_PORT_WRITE_CYCLES 2U ///< I/O Cycles: 2=default, 1=Cortex-M0+ fast I/0.

/// Indicate that Serial Wire Debug (SWD) communication mode is available at the Debug Access Port.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define DAP_SWD 1 ///< SWD Mode:  1 = available, 0 = not available.

/// Indicate that JTAG communication mode is available at the Debug Port.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define DAP_JTAG 1 ///< JTAG Mode: 1 = available, 0 = not available.

/// Configure maximum number of JTAG devices on the scan chain connected to the Debug Access Port.
/// This setting impacts the RAM requirements of the Debug Unit. Valid range is 1 .. 255.
#define DAP_JTAG_DEV_CNT 8U ///< Maximum number of JTAG devices on scan chain.

/// Default communication mode on the Debug Access Port.
/// Used for the command \ref DAP_Connect when Port Default mode is selected.
#define DAP_DEFAULT_PORT 1U ///< Default JTAG/SWJ Port Mode: 1 = SWD, 2 = JTAG.

/// Default communication speed on the Debug Access Port for SWD and JTAG mode.
/// Used to initialize the default SWD/JTAG clock frequency.
/// The command \ref DAP_SWJ_Clock can be used to overwrite this default setting.
#define DAP_DEFAULT_SWJ_CLOCK 1000000U ///< Default SWD/JTAG clock frequency in Hz.
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<1MHz

/// Maximum Package Buffers for Command and Response data.
/// This configuration settings is used to optimize the communication performance with the
/// debugger and depends on the USB peripheral. For devices with limited RAM or USB buffer the
/// setting can be reduced (valid range is 1 .. 255).
#define DAP_PACKET_COUNT 255 ///< Specifies number of packets buffered.

/// Indicates that the SWO function(UART SWO & Streaming Trace) is available
#define SWO_FUNCTION_ENABLE 0 ///< SWO function:  1 = available, 0 = not available.


/// Indicate that UART Serial Wire Output (SWO) trace is available.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define SWO_UART SWO_FUNCTION_ENABLE ///< SWO UART:  1 = available, 0 = not available.

/// USART Driver instance number for the UART SWO.
#define SWO_UART_DRIVER 0 ///< USART Driver instance number (Driver_USART#).

/// Maximum SWO UART Baudrate.
#define SWO_UART_MAX_BAUDRATE (115200U * 40U) ///< SWO UART Maximum Baudrate in Hz.
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<< 5MHz
//// TODO: uncertain value

/// Indicate that Manchester Serial Wire Output (SWO) trace is available.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define SWO_MANCHESTER 0 ///< SWO Manchester:  1 = available, 0 = not available.
// (windowsair)Do not modify. Not support.


/// SWO Trace Buffer Size.
#define SWO_BUFFER_SIZE 2048U ///< SWO Trace Buffer Size in bytes (must be 2^n).

/// SWO Streaming Trace.
#define SWO_STREAM SWO_FUNCTION_ENABLE ///< SWO Streaming Trace: 1 = available, 0 = not available.

/// Clock frequency of the Test Domain Timer. Timer value is returned with \ref TIMESTAMP_GET.
#define TIMESTAMP_CLOCK 5000000U ///< Timestamp clock in Hz (0 = timestamps not supported).
// <<<<<<<<<<<<<<<<<<<<<5MHz

/// Indicate that UART Communication Port is available.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define DAP_UART                0               ///< DAP UART:  1 = available, 0 = not available.

/// USART Driver instance number for the UART Communication Port.
#define DAP_UART_DRIVER         1               ///< USART Driver instance number (Driver_USART#).

/// UART Receive Buffer Size.
#define DAP_UART_RX_BUFFER_SIZE 1024U           ///< Uart Receive Buffer Size in bytes (must be 2^n).

/// UART Transmit Buffer Size.
#define DAP_UART_TX_BUFFER_SIZE 1024U           ///< Uart Transmit Buffer Size in bytes (must be 2^n).

/// Indicate that UART Communication via USB COM Port is available.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define DAP_UART_USB_COM_PORT   0               ///< USB COM Port:  1 = available, 0 = not available.

/// Debug Unit is connected to fixed Target Device.
/// The Debug Unit may be part of an evaluation board and always connected to a fixed
/// known device. In this case a Device Vendor, Device Name, Board Vendor and Board Name strings
/// are stored and may be used by the debugger or IDE to configure device parameters.
#define TARGET_FIXED            1               ///< Target: 1 = known, 0 = unknown;

#define TARGET_DEVICE_VENDOR    ""                 ///< String indicating the Silicon Vendor
#define TARGET_DEVICE_NAME      ""                 ///< String indicating the Target Device
#define TARGET_BOARD_VENDOR     "windowsair"       ///< String indicating the Board Vendor
#define TARGET_BOARD_NAME       "ESP wireless DAP" ///< String indicating the Board Name

#if TARGET_FIXED != 0
#include <string.h>
static const char TargetDeviceVendor [] = TARGET_DEVICE_VENDOR;
static const char TargetDeviceName   [] = TARGET_DEVICE_NAME;
static const char TargetBoardVendor  [] = TARGET_BOARD_VENDOR;
static const char TargetBoardName    [] = TARGET_BOARD_NAME;
#endif

/**
 * @brief Get Vendor ID string.
 *
 * @param str Pointer to buffer to store the string (max 60 characters).
 * @return String length. (including terminating NULL character) or 0 (no string).
 */
__STATIC_INLINE uint8_t DAP_GetVendorString(char *str)
{
  // In fact, Keil can get the corresponding information through USB
  // without filling in this information.
  // (void)str;
  strcpy(str, "windowsair");
  return (sizeof("windowsair"));
}

/**
 * @brief Get Product ID string.
 *
 * @param str Pointer to buffer to store the string (max 60 characters).
 * @return String length. (including terminating NULL character) or 0 (no string).
 */
__STATIC_INLINE uint8_t DAP_GetProductString(char *str)
{
  //(void)str;
  strcpy(str, "CMSIS-DAP v2");
  return (sizeof("CMSIS-DAP v2"));
}

/**
 * @brief Get Serial Number string.
 *
 * @param str Pointer to buffer to store the string (max 60 characters).
 * @return String length. (including terminating NULL character) or 0 (no string).
 */
__STATIC_INLINE uint8_t DAP_GetSerNumString(char *str)
{
  strcpy(str, "1234");
  return (sizeof("1234"));
}

/**
 * @brief Get Target Device Vendor string.
 *
 * @param str Pointer to buffer to store the string (max 60 characters).
 * @return String length (including terminating NULL character) or 0 (no string).
 */
__STATIC_INLINE uint8_t DAP_GetTargetDeviceVendorString (char *str) {
#if TARGET_FIXED != 0
  uint8_t len;

  strcpy(str, TargetDeviceVendor);
  len = (uint8_t)(strlen(TargetDeviceVendor) + 1U);
  return (len);
#else
  (void)str;
  return (0U);
#endif
}

/**
 * @brief Get Target Device Name string.
 *
 * @param str Pointer to buffer to store the string (max 60 characters).
 * @return String length (including terminating NULL character) or 0 (no string).
 */
__STATIC_INLINE uint8_t DAP_GetTargetDeviceNameString (char *str) {
#if TARGET_FIXED != 0
  uint8_t len;

  strcpy(str, TargetDeviceName);
  len = (uint8_t)(strlen(TargetDeviceName) + 1U);
  return (len);
#else
  (void)str;
  return (0U);
#endif
}

/**
 * @brief Get Target Board Vendor string.
 *
 * @param str Pointer to buffer to store the string (max 60 characters).
 * @return String length (including terminating NULL character) or 0 (no string).
 */
__STATIC_INLINE uint8_t DAP_GetTargetBoardVendorString (char *str) {
#if TARGET_FIXED != 0
  uint8_t len;

  strcpy(str, TargetBoardVendor);
  len = (uint8_t)(strlen(TargetBoardVendor) + 1U);
  return (len);
#else
  (void)str;
  return (0U);
#endif
}

/**
 * @brief Get Target Board Name string.
 *
 * @param str Pointer to buffer to store the string (max 60 characters).
 * @return String length (including terminating NULL character) or 0 (no string).
 */
__STATIC_INLINE uint8_t DAP_GetTargetBoardNameString (char *str) {
#if TARGET_FIXED != 0
  uint8_t len;

  strcpy(str, TargetBoardName);
  len = (uint8_t)(strlen(TargetBoardName) + 1U);
  return (len);
#else
  (void)str;
  return (0U);
#endif
}

/**
 * @brief Get Product Firmware Version string.
 *
 * @param str Pointer to buffer to store the string (max 60 characters).
 * @return String length (including terminating NULL character) or 0 (no string).
 */
__STATIC_INLINE uint8_t DAP_GetProductFirmwareVersionString (char *str) {
  (void)str;
  return (0U);
}

///@}


// Note: DO NOT modify these pins: PIN_SWDIO  PIN_SWDIO_MOSI  PIN_SWCLK
// Modify the following pins carefully: PIN_TDO
#ifdef CONFIG_IDF_TARGET_ESP8266
  #define PIN_SWDIO 12      // SPI MISO
  #define PIN_SWDIO_MOSI 13 // SPI MOSI
  #define PIN_SWCLK 14
  #define PIN_TDO 16        // device TDO -> Host Data Input (use RTC pin 16)
  #define PIN_TDI 4
  #define PIN_nTRST 0       // optional
  #define PIN_nRESET 5

  #define PIN_LED_CONNECTED _ // won't be used
  #define PIN_LED_RUNNING _ // won't be used
#elif defined CONFIG_IDF_TARGET_ESP32
  #define PIN_SWDIO 12      // SPI MISO
  #define PIN_SWDIO_MOSI 13 // SPI MOSI
  #define PIN_SWCLK 14
  #define PIN_TDO 19        // device TDO -> Host Data Input
  #define PIN_TDI 18
  #define PIN_nTRST 25       // optional
  #define PIN_nRESET 26

  #define PIN_LED_CONNECTED _ // won't be used
  #define PIN_LED_RUNNING _ // won't be used
#elif defined CONFIG_IDF_TARGET_ESP32C3
  #define PIN_SWDIO _      // SPI MISO
  #define PIN_SWDIO_MOSI 7 // SPI MOSI
  #define PIN_SWCLK 6
  #define PIN_TDO 8        // device TDO -> Host Data Input
  #define PIN_TDI 9
  #define PIN_nTRST 4       // optional
  #define PIN_nRESET 5

  #define PIN_LED_CONNECTED _ // won't be used
  #define PIN_LED_RUNNING _ // won't be used
#elif defined CONFIG_IDF_TARGET_ESP32S3
  #define PIN_SWDIO _      // SPI MISO
  #define PIN_SWDIO_MOSI 11 // SPI MOSI
  #define PIN_SWCLK 12
  #define PIN_TDO 9        // device TDO -> Host Data Input
  #define PIN_TDI 10
  #define PIN_nTRST 14       // optional
  #define PIN_nRESET 13

  #define PIN_LED_CONNECTED _ // won't be used
  #define PIN_LED_RUNNING _ // won't be used
#else
#error "not a supported target"
#endif


//**************************************************************************************************
/**
\defgroup DAP_Config_PortIO_gr CMSIS-DAP Hardware I/O Pin Access
\ingroup DAP_ConfigIO_gr
@{

Standard I/O Pins of the CMSIS-DAP Hardware Debug Port support standard JTAG mode
and Serial Wire Debug (SWD) mode. In SWD mode only 2 pins are required to implement the debug
interface of a device. The following I/O Pins are provided:

JTAG I/O Pin                 | SWD I/O Pin          | CMSIS-DAP Hardware pin mode
---------------------------- | -------------------- | ---------------------------------------------
TCK: Test Clock              | SWCLK: Clock         | Output Push/Pull
TMS: Test Mode Select        | SWDIO: Data I/O      | Output Push/Pull; Input (for receiving data)
TDI: Test Data Input         |                      | Output Push/Pull
TDO: Test Data Output        |                      | Input
nTRST: Test Reset (optional) |                      | Output Open Drain with pull-up resistor
nRESET: Device Reset         | nRESET: Device Reset | Output Open Drain with pull-up resistor


DAP Hardware I/O Pin Access Functions
-------------------------------------
The various I/O Pins are accessed by functions that implement the Read, Write, Set, or Clear to
these I/O Pins.

For the SWDIO I/O Pin there are additional functions that are called in SWD I/O mode only.
This functions are provided to achieve faster I/O that is possible with some advanced GPIO
peripherals that can independently write/read a single I/O pin without affecting any other pins
of the same I/O port. The following SWDIO I/O Pin functions are provided:
 - \ref PIN_SWDIO_OUT_ENABLE to enable the output mode from the DAP hardware.
 - \ref PIN_SWDIO_OUT_DISABLE to enable the input mode to the DAP hardware.
 - \ref PIN_SWDIO_IN to read from the SWDIO I/O pin with utmost possible speed.
 - \ref PIN_SWDIO_OUT to write to the SWDIO I/O pin with utmost possible speed.
*/

/**
 * @brief Setup JTAG I/O pins: TCK, TMS, TDI, TDO, nTRST, and nRESET.
 * Configures the DAP Hardware I/O pins for JTAG mode:
 * - TCK, TMS, TDI, nTRST, nRESET to ***output*** mode and set to high level.
 * - TDO to ***input*** mode.
 *
 */
#ifdef CONFIG_IDF_TARGET_ESP8266
__STATIC_INLINE void PORT_JTAG_SETUP(void)
{
  gpio_pin_reg_t pin_reg;


  // set TCK, TMS pin
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14); // GPIO14 is SPI CLK pin (Clock)
  GPIO.enable_w1ts |= (0x1 << 14); // PP Output
  pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(14));
  pin_reg.pullup = 1;
  WRITE_PERI_REG(GPIO_PIN_REG(14), pin_reg.val);

  PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13); // GPIO13 is SPI MOSI pin (Master Data Out)
  GPIO.enable_w1ts |= (0x1 << 13);
  GPIO.pin[13].driver = 1; // OD output
  pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(13));
  pin_reg.pullup = 0;
  WRITE_PERI_REG(GPIO_PIN_REG(13), pin_reg.val);


  // use RTC pin 16
  // output disable
  WRITE_PERI_REG(PAD_XPD_DCDC_CONF, ((READ_PERI_REG(PAD_XPD_DCDC_CONF) & (uint32_t)0xffffffbc)) | (uint32_t)0x1); 	// mux configuration for XPD_DCDC and rtc_gpio0 connection
  CLEAR_PERI_REG_MASK(RTC_GPIO_CONF, 0x1);    // mux configuration for out enable
  CLEAR_PERI_REG_MASK(RTC_GPIO_ENABLE, 0x1);  // out disable
  // pulldown disable
  pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(PIN_TDO));
  pin_reg.rtc_pin.pulldown = 0;
  WRITE_PERI_REG(GPIO_PIN_REG(PIN_TDO), pin_reg.val);



  // gpio_set_direction(PIN_TDI, GPIO_MODE_OUTPUT);
  GPIO.enable_w1ts |= (0x1 << PIN_TDI);
  GPIO.pin[PIN_TDI].driver = 0;
  pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(PIN_TDI));
  pin_reg.pullup = 0;
  WRITE_PERI_REG(GPIO_PIN_REG(PIN_TDI), pin_reg.val);

  // gpio_set_direction(PIN_nTRST, GPIO_MODE_OUTPUT_OD);
  // gpio_set_direction(PIN_nRESET, GPIO_MODE_OUTPUT_OD);
  GPIO.enable_w1tc |= (0x1 << PIN_nTRST);
  GPIO.pin[PIN_nTRST].driver = 1;
  GPIO.enable_w1tc |= (0x1 << PIN_nRESET);
  GPIO.pin[PIN_nRESET].driver = 1;

  // gpio_set_pull_mode(PIN_nTRST, GPIO_PULLUP_ONLY);
  // gpio_set_pull_mode(PIN_nRESET, GPIO_PULLUP_ONLY);
  GPIO_PULL_UP_ONLY_SET(PIN_nTRST);
  GPIO_PULL_UP_ONLY_SET(PIN_nRESET);
}
#elif defined CONFIG_IDF_TARGET_ESP32
__STATIC_INLINE void PORT_JTAG_SETUP(void)
{
  // set TCK, TMS pin
  //// FIXME: esp32
  //DAP_SPI_Deinit();


  // PIN_TDO output disable
  GPIO.enable_w1tc = (0x1 << PIN_TDO);
  // PIN_TDO input enable
  PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[PIN_TDO]);



  // gpio_set_direction(PIN_TDI, GPIO_MODE_OUTPUT);
  GPIO.enable_w1ts = (0x1 << PIN_TDI);
  GPIO.pin[PIN_TDI].pad_driver = 0;
  REG_CLR_BIT(GPIO_PIN_MUX_REG[PIN_TDI], FUN_PD); // disable pull down

  // gpio_set_direction(PIN_nTRST, GPIO_MODE_OUTPUT_OD);
  // gpio_set_direction(PIN_nRESET, GPIO_MODE_OUTPUT_OD);
  GPIO.enable_w1tc = (0x1 << PIN_nTRST);
  GPIO.pin[PIN_nTRST].pad_driver = 1;
  GPIO.enable_w1tc = (0x1 << PIN_nRESET);
  GPIO.pin[PIN_nRESET].pad_driver = 1;

  // gpio_set_pull_mode(PIN_nTRST, GPIO_PULLUP_ONLY);
  // gpio_set_pull_mode(PIN_nRESET, GPIO_PULLUP_ONLY);
  GPIO_PULL_UP_ONLY_SET(PIN_nTRST);
  GPIO_PULL_UP_ONLY_SET(PIN_nRESET);
}
#elif defined CONFIG_IDF_TARGET_ESP32C3
__STATIC_INLINE void PORT_JTAG_SETUP(void)
{
  // set TCK, TMS pin


  // PIN_TDO output disable
  GPIO.enable_w1tc.enable_w1tc = (0x1 << PIN_TDO);
  // PIN_TDO input enable
  PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[PIN_TDO]);



  // gpio_set_direction(PIN_TDI, GPIO_MODE_OUTPUT);
  GPIO.enable_w1ts.enable_w1ts = (0x1 << PIN_TDI);
  GPIO.pin[PIN_TDI].pad_driver = 0;
  REG_CLR_BIT(GPIO_PIN_MUX_REG[PIN_TDI], FUN_PD); // disable pull down

  // gpio_set_direction(PIN_nTRST, GPIO_MODE_OUTPUT_OD);
  // gpio_set_direction(PIN_nRESET, GPIO_MODE_OUTPUT_OD);
  GPIO.enable_w1tc.enable_w1tc = (0x1 << PIN_nTRST);
  GPIO.pin[PIN_nTRST].pad_driver = 1;
  GPIO.enable_w1tc.enable_w1tc = (0x1 << PIN_nRESET);
  GPIO.pin[PIN_nRESET].pad_driver = 1;

  // gpio_set_pull_mode(PIN_nTRST, GPIO_PULLUP_ONLY);
  // gpio_set_pull_mode(PIN_nRESET, GPIO_PULLUP_ONLY);
  GPIO_PULL_UP_ONLY_SET(PIN_nTRST);
  GPIO_PULL_UP_ONLY_SET(PIN_nRESET);
}
#elif defined CONFIG_IDF_TARGET_ESP32S3
__STATIC_INLINE void PORT_JTAG_SETUP(void)
{
  // set TCK, TMS pin

  // PIN_TDO output disable
  gpio_ll_output_disable(&GPIO, PIN_TDO);
  // PIN_TDO input enable
  gpio_ll_input_enable(&GPIO, PIN_TDO);

  // PIN_TDI output
  gpio_ll_output_enable(&GPIO, PIN_TDI);
  gpio_ll_od_disable(&GPIO, PIN_TDI);
  gpio_ll_pulldown_dis(&GPIO, PIN_TDI);

  gpio_ll_output_enable(&GPIO, PIN_nTRST);
  gpio_ll_od_enable(&GPIO, PIN_nTRST);
  gpio_ll_output_enable(&GPIO, PIN_nRESET);
  gpio_ll_od_enable(&GPIO, PIN_nRESET);

  GPIO_PULL_UP_ONLY_SET(PIN_nTRST);
  GPIO_PULL_UP_ONLY_SET(PIN_nRESET);
}
#endif

/**
 * @brief Setup SWD I/O pins: SWCLK, SWDIO, and nRESET.
 * Configures the DAP Hardware I/O pins for Serial Wire Debug (SWD) mode:
 * - SWCLK, SWDIO, nRESET to output mode and set to default high level.
 * - TDI, nTRST to HighZ mode (pins are unused in SWD mode).
 *
 */
__STATIC_INLINE void PORT_SWD_SETUP(void)
{
  // We will switch to the specific mode when setting the transfer rate.

  // Now we need to set it to ordinary GPIO mode. In most implementations,
  // the DAP will then read the status of the PIN via the `SWJ_PIN` command.
  DAP_SPI_Deinit();
}

/**
 * @brief Disable JTAG/SWD I/O Pins.
 * Disables the DAP Hardware I/O pins which configures:
 * - TCK/SWCLK, TMS/SWDIO, TDI, TDO, nTRST, nRESET to High-Z mode.
 *
 */
__STATIC_INLINE void PORT_OFF(void)
{
  // Will be called when the DAP disconnected
  DAP_SPI_Disable();

#if defined CONFIG_IDF_TARGET_ESP8266
  // gpio_set_direction(PIN_nRESET, GPIO_MODE_OUTPUT_OD);
  GPIO.enable_w1tc |= (0x1 << PIN_nRESET);
  GPIO.pin[PIN_nRESET].driver = 1;

  // gpio_set_pull_mode(PIN_nRESET, GPIO_PULLUP_ONLY);
  GPIO_PULL_UP_ONLY_SET(PIN_nRESET);
#elif defined CONFIG_IDF_TARGET_ESP32
  // gpio_set_direction(PIN_nRESET, GPIO_MODE_OUTPUT_OD);
  GPIO.enable_w1tc = (0x1 << PIN_nRESET);
  GPIO.pin[PIN_nRESET].pad_driver = 1;

  // gpio_set_pull_mode(PIN_nRESET, GPIO_PULLUP_ONLY);
  GPIO_PULL_UP_ONLY_SET(PIN_nRESET);
#elif defined CONFIG_IDF_TARGET_ESP32C3
  // gpio_set_direction(PIN_nRESET, GPIO_MODE_OUTPUT_OD);
  GPIO.enable_w1tc.enable_w1tc = (0x1 << PIN_nRESET);
  GPIO.pin[PIN_nRESET].pad_driver = 1;

  // gpio_set_pull_mode(PIN_nTRST, GPIO_PULLUP_ONLY);
  GPIO_PULL_UP_ONLY_SET(PIN_nRESET);
#elif defined CONFIG_IDF_TARGET_ESP32S3
  gpio_ll_output_enable(&GPIO, PIN_nRESET);
  gpio_ll_od_enable(&GPIO, PIN_nRESET);
  GPIO_PULL_UP_ONLY_SET(PIN_nRESET);
  gpio_ll_set_level(&GPIO, PIN_nRESET, 1);
#endif
}

// SWCLK/TCK I/O pin -------------------------------------

/**
 * @brief SWCLK/TCK I/O pin: Get Input.
 *
 * @return Current status of the SWCLK/TCK DAP hardware I/O pin.
 */
__STATIC_FORCEINLINE uint32_t PIN_SWCLK_TCK_IN(void)
{
  ////TODO: can we set to 0?
  return 0;
}

/**
 * @brief SWCLK/TCK I/O pin: Set Output to High.
 *
 *  Set the SWCLK/TCK DAP hardware I/O pin to high level.
 */
__STATIC_FORCEINLINE void PIN_SWCLK_TCK_SET(void)
{
  GPIO_SET_LEVEL_HIGH(PIN_SWCLK);
}

/**
 * @brief SWCLK/TCK I/O pin: Set Output to Low.
 *
 *  Set the SWCLK/TCK DAP hardware I/O pin to low level.
 */
__STATIC_FORCEINLINE void PIN_SWCLK_TCK_CLR(void)
{
  GPIO_SET_LEVEL_LOW(PIN_SWCLK);
}

// SWDIO/TMS Pin I/O --------------------------------------

/**
 * @brief SWDIO/TMS I/O pin: Get Input.
 *
 * @return Current status of the SWDIO/TMS DAP hardware I/O pin.
 */
__STATIC_FORCEINLINE uint32_t PIN_SWDIO_TMS_IN(void)
{
  // Note that we only use mosi in GPIO mode
  return GPIO_GET_LEVEL(PIN_SWDIO_MOSI);
}

/**
 * @brief SWDIO/TMS I/O pin: Set Output to High.
 *
 * Set the SWDIO/TMS DAP hardware I/O pin to high level.
 */
__STATIC_FORCEINLINE void PIN_SWDIO_TMS_SET(void)
{
  GPIO_SET_LEVEL_HIGH(PIN_SWDIO_MOSI);
}

/**
 * @brief SWDIO/TMS I/O pin: Set Output to Low.
 *
 * Set the SWDIO/TMS DAP hardware I/O pin to low level.
 */
__STATIC_FORCEINLINE void PIN_SWDIO_TMS_CLR(void)
{
  GPIO_SET_LEVEL_LOW(PIN_SWDIO_MOSI);
}

/**
 * @brief SWDIO I/O pin: Get Input (used in SWD mode only).
 *
 * @return Current status of the SWDIO DAP hardware I/O pin.
 */
__STATIC_FORCEINLINE uint32_t PIN_SWDIO_IN(void)
{
  // Note that we only use mosi in GPIO mode
  return GPIO_GET_LEVEL(PIN_SWDIO_MOSI);
}

/**
 * @brief SWDIO I/O pin: Set Output (used in SWD mode only).
 *
 * @param bit Output value for the SWDIO DAP hardware I/O pin.
 *
 */
__STATIC_FORCEINLINE void PIN_SWDIO_OUT(uint32_t bit)
{
  /**
    * Important: Use only one bit (bit0) of param!
	  * Sometimes the func "SWD_TransferFunction" of SW_DP.c will
	  * issue "2" as param instead of "0". Zach Lee
	  */
  if ((bit & 1U) == 1)
  {
    //set bit
    GPIO_SET_LEVEL_HIGH(PIN_SWDIO_MOSI);

  }
  else
  {
    //reset bit
    GPIO_SET_LEVEL_LOW(PIN_SWDIO_MOSI);

  }
}

/**
 * @brief SWDIO I/O pin: Switch to Output mode (used in SWD mode only).
 * Configure the SWDIO DAP hardware I/O pin to output mode. This function is
 * called prior \ref PIN_SWDIO_OUT function calls.
 */
__STATIC_FORCEINLINE void PIN_SWDIO_OUT_ENABLE(void)
{
  // set \ref gpio_set_direction -> OUTPUT
  // GPIO.enable_w1ts |= (0x1 << PIN_SWDIO_MOSI);
  // GPIO.pin[PIN_SWDIO_MOSI].driver = 0;
  do {}while (0);

}

/**
 * @brief SWDIO I/O pin: Switch to Input mode (used in SWD mode only).
 * Configure the SWDIO DAP hardware I/O pin to input mode. This function is
 * called prior \ref PIN_SWDIO_IN function calls.
 */
__STATIC_FORCEINLINE void PIN_SWDIO_OUT_DISABLE(void)
{
  // may be unuse.
  // set \ref gpio_set_dircetion -> INPUT
  // esp8266 input is always connected
  // GPIO.enable_w1tc |= (0x1 << PIN_SWDIO_MOSI);
  // GPIO.pin[PIN_SWDIO_MOSI].driver = 0;
#ifdef CONFIG_IDF_TARGET_ESP8266
  GPIO.out_w1ts |= (0x1 << PIN_SWDIO_MOSI);
#elif defined CONFIG_IDF_TARGET_ESP32
  // Note that the input of esp32 is not always connected.
  PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[PIN_SWDIO_MOSI]);
  GPIO.out_w1ts = (0x1 << PIN_SWDIO_MOSI);
#elif defined CONFIG_IDF_TARGET_ESP32C3
  // Note that the input of esp32c3 is not always connected.
  PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[PIN_SWDIO_MOSI]);
  GPIO.out_w1ts.out_w1ts = (0x1 << PIN_SWDIO_MOSI);
#elif defined CONFIG_IDF_TARGET_ESP32S3
  // Note that the input is not always connected.
  gpio_ll_input_enable(&GPIO, PIN_SWDIO_MOSI);
  gpio_ll_set_level(&GPIO, PIN_SWDIO_MOSI, 1);
#endif
}

// TDI Pin I/O ---------------------------------------------

/**
 * @brief TDI I/O pin: Get Input.
 *
 * @return Current status of the TDI DAP hardware I/O pin.
 */
__STATIC_FORCEINLINE uint32_t PIN_TDI_IN(void)
{
  return GPIO_GET_LEVEL(PIN_TDI);
}

/**
 * @brief TDI I/O pin: Set Output.
 *
 * @param bit Output value for the TDI DAP hardware I/O pin.
 *
 */
__STATIC_FORCEINLINE void PIN_TDI_OUT(uint32_t bit)
{
  if ((bit & 1U) == 1)
  {
    //set bit
    GPIO_SET_LEVEL_HIGH(PIN_TDI);
  }
  else
  {
    //reset bit
    GPIO_SET_LEVEL_LOW(PIN_TDI);
  }
}

// TDO Pin I/O ---------------------------------------------

/**
 * @brief TDO I/O pin: Get Input.
 *
 * @return Current status of the TDO DAP hardware I/O pin.
 */
__STATIC_FORCEINLINE uint32_t PIN_TDO_IN(void)
{
#ifdef CONFIG_IDF_TARGET_ESP8266
  return READ_PERI_REG(RTC_GPIO_IN_DATA) & 0x1;
#elif defined CONFIG_IDF_TARGET_ESP32
  return ((GPIO.in >> PIN_TDO) & 0x1) ? 1 : 0;
#elif defined CONFIG_IDF_TARGET_ESP32C3 || defined CONFIG_IDF_TARGET_ESP32S3
  return GPIO_GET_LEVEL(PIN_TDO);
#endif
}

// nTRST Pin I/O -------------------------------------------

/**
 * @brief nTRST I/O pin: Get Input.
 *
 * @return Current status of the nTRST DAP hardware I/O pin.
 */
__STATIC_FORCEINLINE uint32_t PIN_nTRST_IN(void)
{
  return 0;  // not available
}

/**
 * @brief nTRST I/O pin: Set Output.
 *
 * @param bit JTAG TRST Test Reset pin status:
 *         - 0: issue a JTAG TRST Test Reset.
           - 1: release JTAG TRST Test Reset.
 */
__STATIC_FORCEINLINE void PIN_nTRST_OUT(uint32_t bit)
{
  // Vendor reset sequence
  ; // not available
}

// nRESET Pin I/O------------------------------------------

/**
 * @brief nRESET I/O pin: Get Input.
 *
 * @return Current status of the nRESET DAP hardware I/O pin.
 */
__STATIC_FORCEINLINE uint32_t PIN_nRESET_IN(void)
{
  return GPIO_GET_LEVEL(PIN_nRESET);
}

/**
 * @brief nRESET I/O pin: Set Output.
 *
 * @param bit target device hardware reset pin status:
 *            - 0: issue a device hardware reset.
 *            - 1: release device hardware reset.
 */
__STATIC_FORCEINLINE void PIN_nRESET_OUT(uint32_t bit)
{
  // Vendor reset sequence
  //// FIXME: unavailable
  if ((bit & 1U) == 1)
  {
    //set bit
    GPIO_SET_LEVEL_HIGH(PIN_nRESET);
#if defined CONFIG_IDF_TARGET_ESP8266 || defined CONFIG_IDF_TARGET_ESP32
    GPIO.enable_w1tc |= (0x01 << PIN_nRESET);
#elif defined CONFIG_IDF_TARGET_ESP32C3
    GPIO.enable_w1tc.enable_w1tc |= (0x01 << PIN_nRESET);
#elif defined CONFIG_IDF_TARGET_ESP32S3
    gpio_ll_output_disable(&GPIO, PIN_nRESET);
#endif
  }
  else
  {
    //reset bit
#if defined CONFIG_IDF_TARGET_ESP8266 || defined CONFIG_IDF_TARGET_ESP32
    GPIO.enable_w1ts |= (0x01 << PIN_nRESET);
#elif defined CONFIG_IDF_TARGET_ESP32C3
    GPIO.enable_w1ts.enable_w1ts |= (0x01 << PIN_nRESET);
#elif defined CONFIG_IDF_TARGET_ESP32S3
    gpio_ll_output_enable(&GPIO, PIN_nRESET);
#endif
    GPIO_SET_LEVEL_LOW(PIN_nRESET);
  }
}

///@}

//**************************************************************************************************
/**
\defgroup DAP_Config_LEDs_gr CMSIS-DAP Hardware Status LEDs
\ingroup DAP_ConfigIO_gr
@{

CMSIS-DAP Hardware may provide LEDs that indicate the status of the CMSIS-DAP Debug Unit.

It is recommended to provide the following LEDs for status indication:
 - Connect LED: is active when the DAP hardware is connected to a debugger.
 - Running LED: is active when the debugger has put the target device into running state.
*/

/** Debug Unit: Set status of Connected LED.
\param bit status of the Connect LED.
           - 1: Connect LED ON: debugger is connected to CMSIS-DAP Debug Unit.
           - 0: Connect LED OFF: debugger is not connected to CMSIS-DAP Debug Unit.
*/

/**
 * @brief Debug Unit: Set status of Connected LED.
 *
 * @param bit status of the Connect LED.
 *        - 1: Connect LED ON: debugger is connected to CMSIS-DAP Debug Unit.
 *        - 0: Connect LED OFF: debugger is not connected to CMSIS-DAP Debug Unit.
 */
__STATIC_INLINE void LED_CONNECTED_OUT(uint32_t bit)
{
  (void)(bit);
}

/**
 * @brief Debug Unit: Set status Target Running LED.
 *
 * @param bit status of the Target Running LED.
 *        - 1: Target Running LED ON: program execution in target started.
 *        - 0: Target Running LED OFF: program execution in target stopped.
 */
__STATIC_INLINE void LED_RUNNING_OUT(uint32_t bit)
{
  (void)(bit);
  // if (bit)
  // {
  //   //set bit
  //   GPIO.out_w1ts |= (0x1 << PIN_LED_RUNNING);
  // }
  // else
  // {
  //   //reset bit
  //   GPIO.out_w1tc |= (0x1 << PIN_LED_RUNNING);
  // }
}

///@}

//**************************************************************************************************
/**
\defgroup DAP_Config_Timestamp_gr CMSIS-DAP Timestamp
\ingroup DAP_ConfigIO_gr
@{
Access function for Test Domain Timer.

The value of the Test Domain Timer in the Debug Unit is returned by the function \ref TIMESTAMP_GET. By
default, the DWT timer is used.  The frequency of this timer is configured with \ref TIMESTAMP_CLOCK.

*/

/**
 * @brief Get timestamp of Test Domain Timer.
 *
 * @return Current timestamp value.
 */
__STATIC_INLINE uint32_t TIMESTAMP_GET(void)
{
  return get_timer_count();
}

///@}

//**************************************************************************************************
/**
\defgroup DAP_Config_Initialization_gr CMSIS-DAP Initialization
\ingroup DAP_ConfigIO_gr
@{

CMSIS-DAP Hardware I/O and LED Pins are initialized with the function \ref DAP_SETUP.
*/

/** Setup of the Debug Unit I/O pins and LEDs (called when Debug Unit is initialized).
This function performs the initialization of the CMSIS-DAP Hardware I/O Pins and the
Status LEDs. In detail the operation of Hardware I/O and LED pins are enabled and set:
 - I/O clock system enabled.
 - all I/O pins: input buffer enabled, output pins are set to HighZ mode.
 - for nTRST, nRESET a weak pull-up (if available) is enabled.
 - LED output pins are enabled and LEDs are turned off.
*/
__STATIC_INLINE void DAP_SETUP(void)
{
  // Connecting non-SWD pins to GPIO
  GPIO_FUNCTION_SET(PIN_TDO);
  GPIO_FUNCTION_SET(PIN_TDI);
  GPIO_FUNCTION_SET(PIN_nTRST);
  GPIO_FUNCTION_SET(PIN_nRESET);

  // GPIO_FUNCTION_SET(PIN_LED_RUNNING);


  // Configure: LED as output (turned off)

  // GPIO_SET_DIRECTION_NORMAL_OUT(PIN_LED_RUNNING);

  // LED_CONNECTED_OUT(0);
  // LED_RUNNING_OUT(0);

  PORT_OFF();
}

extern void dap_os_delay(int ms);
/** Reset Target Device with custom specific I/O pin or command sequence.
This function allows the optional implementation of a device specific reset sequence.
It is called when the command \ref DAP_ResetTarget and is for example required
when a device needs a time-critical unlock sequence that enables the debug port.
\return 0 = no device specific reset sequence is implemented.\n
        1 = a device specific reset sequence is implemented.
*/
__STATIC_INLINE uint8_t RESET_TARGET(void)
{

  PIN_nRESET_OUT(0);
  dap_os_delay(2);
  PIN_nRESET_OUT(1);
  dap_os_delay(2);
  return (1U); // OK
}

///@}

#endif /* __DAP_CONFIG_H__ */

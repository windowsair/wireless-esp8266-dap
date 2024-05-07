#ifndef __DAP_CONFIGURATION_H__
#define __DAP_CONFIGURATION_H__

/**
 * @brief Specify the use of WINUSB
 *
 */
#define USE_WINUSB 1

/**
 * @brief Enable this option, no need to physically connect MOSI and MISO
 *
 */
#define USE_SPI_SIO 1


/**
 * @brief Specify to enable USB 3.0
 *
 */
#define USE_USB_3_0 0


// For USB 3.0, it must be 1024 byte.
#if (USE_USB_3_0 == 1)
    #define USB_ENDPOINT_SIZE 1024U
#else
    #define USB_ENDPOINT_SIZE 512U
#endif

/// Maximum Package Size for Command and Response data.
/// This configuration settings is used to optimize the communication performance with the
/// debugger and depends on the USB peripheral. Typical vales are 64 for Full-speed USB HID or WinUSB,
/// 1024 for High-speed USB HID and 512 for High-speed USB WinUSB.

#if (USE_WINUSB == 1)
    #define DAP_PACKET_SIZE 512U // 512 for WinUSB.
#else
    #define DAP_PACKET_SIZE 255U // 255 for USB HID
#endif

/**
 * @brief Enable this option to force a software reset when resetting the device
 *
 * Some debugger software (e.g. Keil) does not perform a software reset when
 * resetting the target. When this option enabled, a soft reset is attempted
 * when DAP_ResetTarget() is performed. This is done by writing to the
 * SYSRESETREQ field of the AIRCR register in the Cortex-M architecture.
 *
 * This should work for ARMv6-m, ARMv7-m and ARMv8-m architecture. However,
 * there is no guarantee that the reset operation will be executed correctly.
 *
 * Only available for SWD.
 *
 */
#define USE_FORCE_SYSRESETREQ_AFTER_FLASH 0

#endif

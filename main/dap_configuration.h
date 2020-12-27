#ifndef __DAP_CONFIGURATION_H__
#define __DAP_CONFIGURATION_H__

/**
 * @brief Specify the use of WINUSB
 * 
 */
#define USE_WINUSB 1

/// Maximum Package Size for Command and Response data.
/// This configuration settings is used to optimize the communication performance with the
/// debugger and depends on the USB peripheral. Typical vales are 64 for Full-speed USB HID or WinUSB,
/// 1024 for High-speed USB HID and 512 for High-speed USB WinUSB.

#if (USE_WINUSB == 1)
    #define DAP_PACKET_SIZE 512U // 512 for WinUSB.
#else
    #define DAP_PACKET_SIZE 255U // 255 for USB HID
#endif


#endif

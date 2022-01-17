/**
 * @file wifi_configuration.h
 * @brief Fill in your wifi configuration information here.
 * @version 0.1
 * @date 2020-01-22
 *
 * @copyright Copyright (c) 2020
 *
 */
#ifndef __WIFI_CONFIGURATION__
#define __WIFI_CONFIGURATION__

#define WIFI_SSID "DAP"
#define WIFI_PASS "12345678"


#define USE_MDNS 1
// Use the address "dap.local" to access the device
#define MDNS_HOSTNAME "dap"
#define MDNS_INSTANCE "DAP mDNS"
//

#define USE_STATIC_IP 1
// If you don't want to specify the ip configuration, then ignore the following items.
#define DAP_IP_ADDRESS 192, 168, 137, 123
#define DAP_IP_GATEWAY 192, 168, 137, 1
#define DAP_IP_NETMASK 255, 255, 255, 0
//

#define USE_OTA 1


#define USE_UART_BRIDGE 0
#define UART_BRIDGE_PORT 1234
//

#define USE_TCP_NETCONN 0

// DO NOT CHANGE
#define PORT 3240
#define CONFIG_EXAMPLE_IPV4 1
#define USE_KCP 0
#define MTU_SIZE 1500
//

#if (USE_TCP_NETCONN == 1 && USE_KCP == 1)
#error Can not use KCP and TCP at the same time!
#endif

#if (USE_KCP == 1)
#warning KCP is a very experimental feature, and it should not be used under any circumstances. Please make sure what you are doing. Related usbip version: https://github.com/windowsair/usbip-win
#endif

#endif
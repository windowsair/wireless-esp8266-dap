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


#define USE_STATIC_IP 1
// If you don't want to specify the ip configuration, then ignore the following items.
#define DAP_IP_ADDRESS 192, 168, 137, 123
#define DAP_IP_GATEWAY 192, 168, 137, 1
#define DAP_IP_NETMASK 255, 255, 255, 0

// DO NOT CHANGE
#define PORT 3240
#define CONFIG_EXAMPLE_IPV4 1
#define USE_KCP 0
#define MTU_SIZE 1500

#endif
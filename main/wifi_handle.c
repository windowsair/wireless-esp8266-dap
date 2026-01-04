#include "sdkconfig.h"

#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "main/wifi_configuration.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_netif.h"
#include "lwip/ip4_addr.h"

#ifdef CONFIG_IDF_TARGET_ESP8266
    #define PIN_LED_WIFI_STATUS 15
#elif defined CONFIG_IDF_TARGET_ESP32
    #define PIN_LED_WIFI_STATUS 27
#elif defined CONFIG_IDF_TARGET_ESP32C3
    #define PIN_LED_WIFI_STATUS 10
#elif defined CONFIG_IDF_TARGET_ESP32S3
    #define PIN_LED_WIFI_STATUS 4
#else
    #error unknown hardware
#endif

static EventGroupHandle_t wifi_event_group;
static int ssid_index = 0;

const int IPV4_GOTIP_BIT = BIT0;
#ifdef CONFIG_EXAMPLE_IPV6
const int IPV6_GOTIP_BIT = BIT1;
#endif

static void ssid_change();

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_CONNECTED:
#ifdef CONFIG_EXAMPLE_IPV6
            /* enable ipv6 */
            esp_netif_create_ip6_linklocal(esp_netif_get_handle_from_ifkey("WIFI_STA"));
#endif
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            gpio_set_level(PIN_LED_WIFI_STATUS, 0);

            wifi_event_sta_disconnected_t *disconnected = (wifi_event_sta_disconnected_t*) event_data;
            os_printf("Disconnect reason : %d\r\n", (int)disconnected->reason);

#ifdef CONFIG_IDF_TARGET_ESP8266
            if (disconnected->reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
                /*Switch to 802.11 bgn mode */
                esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
            }
#endif
            ssid_change();
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
#ifdef CONFIG_EXAMPLE_IPV6
            xEventGroupClearBits(wifi_event_group, IPV6_GOTIP_BIT);
#endif


            break;
        default:
            break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            gpio_set_level(PIN_LED_WIFI_STATUS, 1);

            ip_event_got_ip_t *got_ip = (ip_event_got_ip_t*) event_data;
            xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
            char ip_str[16];
            esp_ip4addr_ntoa(&got_ip->ip_info.ip, ip_str, sizeof(ip_str));
            os_printf("SYSTEM EVENT STA GOT IP : %s\r\n", ip_str);
            break;
        case IP_EVENT_GOT_IP6:
#ifdef CONFIG_EXAMPLE_IPV6
            xEventGroupSetBits(wifi_event_group, IPV6_GOTIP_BIT);
            os_printf("SYSTEM_EVENT_STA_GOT_IP6\r\n");

            ip_event_got_ip6_t *got_ip6 = (ip_event_got_ip6_t*) event_data;
            char *ip6 = ip6addr_ntoa(&got_ip6->ip6_info.ip);
            os_printf("IPv6: %s\r\n", ip6);
#endif
            break;
        default:
            break;
        }
    }
}

static void ssid_change() {
    if (ssid_index > WIFI_LIST_SIZE - 1) {
        ssid_index = 0;
    }

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };

    strcpy((char *)wifi_config.sta.ssid, wifi_list[ssid_index].ssid);
    strcpy((char *)wifi_config.sta.password, wifi_list[ssid_index].password);
    ssid_index++;
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
}

static void wait_for_ip() {
#ifdef CONFIG_EXAMPLE_IPV6
    uint32_t bits = IPV4_GOTIP_BIT | IPV6_GOTIP_BIT;
#else
    uint32_t bits = IPV4_GOTIP_BIT;
#endif

    os_printf("Waiting for AP connection...\r\n");
    xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
    os_printf("Connected to AP\r\n");
}

void wifi_ap_init(void) {
    // Initialize network interface
    esp_netif_init();

    wifi_event_group = xEventGroupCreate();

    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    
    // Create default WiFi AP interface
    esp_netif_create_default_wifi_ap();
    
    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Configure AP settings
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32S3_AP",
            .ssid_len = 0,
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

#if (USE_STATIC_IP == 1)
    // Configure static IP for AP
    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP");
    
    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(ip_info));
    
    // Set IP address, gateway and netmask
    ip_info.ip.addr = ESP_IP4TOADDR(192, 168, 4, 1);
    ip_info.gw.addr = ESP_IP4TOADDR(192, 168, 4, 1);
    ip_info.netmask.addr = ESP_IP4TOADDR(255, 255, 255, 0);

    esp_netif_set_ip_info(ap_netif, &ip_info);
#endif // (USE_STATIC_IP == 1)

    // Set WiFi mode and power save
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    
    // Set WiFi configuration and start
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    os_printf("AP started with SSID: ESP32S3_AP, IP: 192.168.4.1\r\n");
}

void wifi_init(void) {
    // Initialize network interface
    esp_netif_init();

#if (USE_STATIC_IP == 1)
    // Configure static IP
    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA");
    esp_netif_dhcpc_stop(sta_netif);

    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(ip_info));
    
    // Set IP address, gateway and netmask
    ip_info.ip.addr = ESP_IP4TOADDR(192, 168, 137, 123);
    ip_info.gw.addr = ESP_IP4TOADDR(192, 168, 137, 1);
    ip_info.netmask.addr = ESP_IP4TOADDR(255, 255, 255, 0);

    esp_netif_set_ip_info(sta_netif, &ip_info);
#endif // (USE_STATIC_IP == 1)

    wifi_event_group = xEventGroupCreate();

    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    
    // Create default WiFi station interface
    esp_netif_create_default_wifi_sta();
    
    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Set WiFi mode and power save
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    
    // Set WiFi configuration and start
    ssid_change();
    ESP_ERROR_CHECK(esp_wifi_start());


    wait_for_ip();
}

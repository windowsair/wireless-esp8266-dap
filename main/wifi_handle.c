#include "sdkconfig.h"

#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "main/wifi_configuration.h"
#include "main/uart_bridge.h"

#include "components/DAP/include/gpio_op.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#ifdef CONFIG_IDF_TARGET_ESP8266
    #define PIN_LED_WIFI_STATUS 15
#elif defined CONFIG_IDF_TARGET_ESP32
    #define PIN_LED_WIFI_STATUS 27
#elif defined CONFIG_IDF_TARGET_ESP32C3
    #define PIN_LED_WIFI_STATUS 12
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

static esp_err_t event_handler(void *ctx, system_event_t *event) {
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
#ifdef CONFIG_EXAMPLE_IPV6
        /* enable ipv6 */
        tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
#endif
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        GPIO_SET_LEVEL_HIGH(PIN_LED_WIFI_STATUS);

        xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
        os_printf("SYSTEM EVENT STA GOT IP : %s\r\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        GPIO_SET_LEVEL_LOW(PIN_LED_WIFI_STATUS);

        os_printf("Disconnect reason : %d\r\n", (int)info->disconnected.reason);

#ifdef CONFIG_IDF_TARGET_ESP8266
        if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
            /*Switch to 802.11 bgn mode */
            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
        }
#endif
        ssid_change();
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
#ifdef CONFIG_EXAMPLE_IPV6
        xEventGroupClearBits(wifi_event_group, IPV6_GOTIP_BIT);
#endif

#if (USE_UART_BRIDGE == 1)
        uart_bridge_close();
#endif
        break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
#ifdef CONFIG_EXAMPLE_IPV6
        xEventGroupSetBits(wifi_event_group, IPV6_GOTIP_BIT);
        os_printf("SYSTEM_EVENT_STA_GOT_IP6\r\n");

        char *ip6 = ip6addr_ntoa(&event->event_info.got_ip6.ip6_info.ip);
        os_printf("IPv6: %s\r\n", ip6);
#endif
    default:
        break;
    }
    return ESP_OK;
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

void wifi_init(void) {
    GPIO_FUNCTION_SET(PIN_LED_WIFI_STATUS);
    GPIO_SET_DIRECTION_NORMAL_OUT(PIN_LED_WIFI_STATUS);

    tcpip_adapter_init();

#if (USE_STATIC_IP == 1)
    tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_STA);

    tcpip_adapter_ip_info_t ip_info;

#define MY_IP4_ADDR(...) IP4_ADDR(__VA_ARGS__)
    MY_IP4_ADDR(&ip_info.ip, DAP_IP_ADDRESS);
    MY_IP4_ADDR(&ip_info.gw, DAP_IP_GATEWAY);
    MY_IP4_ADDR(&ip_info.netmask, DAP_IP_NETMASK);
#undef MY_IP4_ADDR

    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
#endif // (USE_STATIC_IP == 1)

    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // os_printf("Setting WiFi configuration SSID %s...\r\n", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ssid_change();
    ESP_ERROR_CHECK(esp_wifi_start());


    wait_for_ip();
}

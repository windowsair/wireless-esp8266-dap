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
#include "esp_log.h"

#include "lwip/ip4_addr.h"
#include "lwip/ip6_addr.h"

#ifdef CONFIG_IDF_TARGET_ESP8266
    #include "esp_event_loop.h"
    #define WIFI_STA_IF ESP_IF_WIFI_STA
#else
    #include "esp_event.h"
    #include "esp_netif.h"
    /* ESP-IDF v6 renamed ESP_IF_WIFI_STA to WIFI_IF_STA */
    #define WIFI_STA_IF WIFI_IF_STA
#endif

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

#ifdef CONFIG_IDF_TARGET_ESP8266
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

        if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
            /*Switch to 802.11 bgn mode */
            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
        }
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

#else /* CONFIG_IDF_TARGET_ESP8266 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
#ifdef CONFIG_EXAMPLE_IPV6
        /* enable ipv6 */
        esp_netif_create_ip6_linklocal(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
#endif
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *disconnected = (wifi_event_sta_disconnected_t *)event_data;

        GPIO_SET_LEVEL_LOW(PIN_LED_WIFI_STATUS);
        os_printf("Disconnect reason : %d\r\n", (int)disconnected->reason);

        ssid_change();
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
#ifdef CONFIG_EXAMPLE_IPV6
        xEventGroupClearBits(wifi_event_group, IPV6_GOTIP_BIT);
#endif

#if (USE_UART_BRIDGE == 1)
        uart_bridge_close();
#endif
    }
}

static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data) {
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        GPIO_SET_LEVEL_HIGH(PIN_LED_WIFI_STATUS);
        xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
        char ip_str[16] = {0};
        esp_ip4addr_ntoa(&event->ip_info.ip, ip_str, sizeof(ip_str));
        os_printf("SYSTEM EVENT STA GOT IP : %s\r\n", ip_str);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_GOT_IP6) {
#ifdef CONFIG_EXAMPLE_IPV6
        ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;

        xEventGroupSetBits(wifi_event_group, IPV6_GOTIP_BIT);
        os_printf("SYSTEM_EVENT_STA_GOT_IP6\r\n");
        /* esp_ip6_addr_t and lwip ip6_addr_t share the same addr[4] layout */
        char ip6_str[48] = {0};
        ip6addr_ntoa_r((const ip6_addr_t *)&event->ip6_info.ip, ip6_str, sizeof(ip6_str));
        os_printf("IPv6: %s\r\n", ip6_str);
#endif
    }
}

#endif /* CONFIG_IDF_TARGET_ESP8266 */

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
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_STA_IF, &wifi_config));
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

#ifdef CONFIG_IDF_TARGET_ESP8266
    tcpip_adapter_init();
#else
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
#endif

#if (USE_STATIC_IP == 1)
#ifdef CONFIG_IDF_TARGET_ESP8266
    tcpip_adapter_ip_info_t ip_info;
#else
    esp_netif_ip_info_t ip_info;
#endif
#define MY_IP4_ADDR(...) IP4_ADDR(__VA_ARGS__)
    MY_IP4_ADDR(&ip_info.ip, DAP_IP_ADDRESS);
    MY_IP4_ADDR(&ip_info.gw, DAP_IP_GATEWAY);
    MY_IP4_ADDR(&ip_info.netmask, DAP_IP_NETMASK);
#undef MY_IP4_ADDR

#ifdef CONFIG_IDF_TARGET_ESP8266
    tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA);
    tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
#else
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
    ESP_ERROR_CHECK(esp_netif_set_ip_info(netif, &ip_info));
#endif
#endif // (USE_STATIC_IP == 1)

    wifi_event_group = xEventGroupCreate();

#ifdef CONFIG_IDF_TARGET_ESP8266
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
#else
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
#ifdef CONFIG_EXAMPLE_IPV6
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &ip_event_handler, NULL));
#endif
#endif

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

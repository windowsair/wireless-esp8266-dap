/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "main/tcp_server.h"
#include "main/tcp_netconn.h"
#include "main/kcp_server.h"
#include "main/timer.h"
#include "main/wifi_configuration.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


extern void DAP_Setup(void);
extern void DAP_Thread(void *argument);
extern void SWO_Thread();

TaskHandle_t kDAPTaskHandle = NULL;



/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

const int IPV4_GOTIP_BIT = BIT0;
#ifdef CONFIG_EXAMPLE_IPV6
const int IPV6_GOTIP_BIT = BIT1;
#endif


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        os_printf("SYSTEM_EVENT_STA_START\r\n");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
#ifdef CONFIG_EXAMPLE_IPV6
        /* enable ipv6 */
        tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
#endif
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
        os_printf("SYSTEM EVENT STA GOT IP : %s\r\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        os_printf("Disconnect reason : %d\r\n", info->disconnected.reason);
        if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT)
        {
            /*Switch to 802.11 bgn mode */
            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
        }
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
#ifdef CONFIG_EXAMPLE_IPV6
        xEventGroupClearBits(wifi_event_group, IPV6_GOTIP_BIT);
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

static void initialise_wifi(void)
{
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
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    os_printf("Setting WiFi configuration SSID %s...\r\n", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void wait_for_ip()
{
#ifdef CONFIG_EXAMPLE_IPV6
    uint32_t bits = IPV4_GOTIP_BIT | IPV6_GOTIP_BIT;
#else
    uint32_t bits = IPV4_GOTIP_BIT;
#endif

    os_printf("Waiting for AP connection...\r\n");
    xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
    os_printf("Connected to AP\r\n");
}


void app_main()
{
    // struct rst_info *rtc_info = system_get_rst_info();

    // os_printf("reset reason: %x\n", rtc_info->reason);

    // if (rtc_info->reason == REASON_WDT_RST ||
    //     rtc_info->reason == REASON_EXCEPTION_RST ||
    //     rtc_info->reason == REASON_SOFT_WDT_RST)
    // {
    // if (rtc_info->reason == REASON_EXCEPTION_RST)
    // {
    //     os_printf("Fatal exception (%d):\n", rtc_info->exccause);
    // }
    // os_printf("epc1=0x%08x, epc2=0x%08x, epc3=0x%08x,excvaddr=0x%08x, depc=0x%08x\n",
    //             rtc_info->epc1, rtc_info->epc2, rtc_info->epc3,
    //             rtc_info->excvaddr, rtc_info->depc);
    // }

    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi();
    wait_for_ip();
    DAP_Setup();
    timer_init();

    // Specify the usbip server task
#if (USE_KCP == 1)
    xTaskCreate(kcp_server_task, "kcp_server", 4096, NULL, 7, NULL);
#elif (USE_TCP_NETCONN == 1)
    xTaskCreate(tcp_netconn_task, "tcp_server", 4096, NULL, 14, NULL);
#else // BSD style
    xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 14, NULL);
#endif

    // DAP handle task
    xTaskCreate(DAP_Thread, "DAP_Task", 2048, NULL, 10, &kDAPTaskHandle);

    // SWO Trace Task
#if (SWO_FUNCTION_ENABLE == 1)
    xTaskCreate(SWO_Thread, "SWO_Task", 512, NULL, 10, NULL);
#endif
    // It seems that the task is overly stressful...
}

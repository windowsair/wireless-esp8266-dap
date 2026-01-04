/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <stdint.h>
#include <sys/param.h>

#include "sdkconfig.h"
#include "main/tcp_server.h"
#include "main/tcp_netconn.h"
#include "main/wifi_configuration.h"
#include "main/wifi_handle.h"



#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

void app_main() {
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

    wifi_ap_init();
    //wifi_init();


    // Specify the usbip server task
#if (USE_TCP_NETCONN == 1)
    xTaskCreatePinnedToCore(tcp_netconn_task, "tcp_server", 4096, NULL, 14, NULL, 0);
#else // BSD style
    xTaskCreatePinnedToCore(tcp_server_task, "tcp_server", 4096, NULL, 14, NULL, 0);
#endif

    

#if defined CONFIG_IDF_TARGET_ESP8266
    #define UART_BRIDGE_TASK_STACK_SIZE 1024
#else
    #define UART_BRIDGE_TASK_STACK_SIZE 2048
#endif

    //// FIXME: potential stack overflow
#if (USE_UART_BRIDGE == 1)
    xTaskCreate(uart_bridge_task, "uart_server", UART_BRIDGE_TASK_STACK_SIZE, NULL, 2, NULL);
#endif
}

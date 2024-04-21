/**
 * @file timer.c
 * @brief Hardware timer for DAP timestamp
 * @change: 2021-02-18 Using the FRC2 timer
 *
 * @version 0.2
 * @date 2020-01-22
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <stdint.h>
#include <stdbool.h>

#include "main/timer.h"

#ifdef CONFIG_IDF_TARGET_ESP8266
    #include "hw_timer.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/*
    Note:
        System bus frequency is 80MHz, will not be affected by CPU frequency. The
        frequency of UART, SPI, or other peripheral devices, are divided from system
        bus frequency, so they will not be affected by CPU frequency either.

*/
#ifdef CONFIG_IDF_TARGET_ESP8266
#define TIMER_BASE 0x60000600
volatile frc2_struct_t * frc2  = (frc2_struct_t *)(TIMER_BASE + (1) * 0x20);
#endif

void timer_init()
{
#ifdef CONFIG_IDF_TARGET_ESP8266
    vPortEnterCritical();
    frc2->ctrl.div = TIMER_CLKDIV_16;  // 80MHz / 16 = 5MHz
    frc2->ctrl.intr_type = TIMER_EDGE_INT;
    frc2->ctrl.reload = 0x01;          // enable auto reload
    frc2->load.val = 0x7FFFFFFF;      // 31bit max
    frc2->ctrl.en = 0x01;
    vPortExitCritical();
#endif
}

// Timing up to 2147483647(0x7FFFFFFF) / 5000000(5MHz) = 429s
// 0.2 micro-second resolution
uint32_t get_timer_count()
{


#ifdef CONFIG_IDF_TARGET_ESP8266
    return (uint32_t)frc2->count.data;
#elif defined CONFIG_IDF_TARGET_ESP32 || defined CONFIG_IDF_TARGET_ESP32C3 || defined CONFIG_IDF_TARGET_ESP32S3
    return 0;
#else
    #error unknown hardware
#endif
}
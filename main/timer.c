/**
 * @file timer.c
 * @brief Hardware timer for DAP timestamp
 * @version 0.1
 * @date 2020-01-22
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include <stdint.h>
#include <stdbool.h>
#include "timer.h"
#include "hw_timer.h"
#include "timer_struct.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
void timer_create_task()
{
    // FRC1 frequency 80MHz
    vPortEnterCritical();
    frc1.ctrl.div = TIMER_CLKDIV_16;  // 80MHz / 16 = 5MHz
    frc1.ctrl.intr_type = TIMER_EDGE_INT;
    frc1.ctrl.reload = 0x01; 
    frc1.load.data = 0x1000000U - 1U; 
    frc1.ctrl.en = 0x01;
    vPortExitCritical();
    vTaskDelete(NULL);
}
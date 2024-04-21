#ifndef __GPIO_COMMON_H__
#define __GPIO_COMMON_H__

#include "sdkconfig.h"
#include "esp_idf_version.h"

#ifdef CONFIG_IDF_TARGET_ESP8266
    #include "esp8266/spi_struct.h"
    #include "gpio.h"
    #include "esp8266/include/esp8266/gpio_struct.h"
    #include "esp8266/include/esp8266/timer_struct.h"
    #include "esp8266/pin_mux_register.h"

#elif defined CONFIG_IDF_TARGET_ESP32
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
        #include "soc/esp32/include/soc/gpio_struct.h"
        #include "soc/esp32/include/soc/dport_access.h"
        #include "soc/esp32/include/soc/dport_reg.h"
        #include "soc/esp32/include/soc/periph_defs.h"
        #include "soc/esp32/include/soc/spi_struct.h"
        #include "soc/esp32/include/soc/spi_reg.h"
    #else
        #include "soc/soc/esp32/include/soc/gpio_struct.h"
        #include "soc/soc/esp32/include/soc/dport_access.h"
        #include "soc/soc/esp32/include/soc/dport_reg.h"
        #include "soc/soc/esp32/include/soc/periph_defs.h"
        #include "soc/soc/esp32/include/soc/spi_struct.h"
        #include "soc/soc/esp32/include/soc/spi_reg.h"
    #endif
    #include "hal/gpio_types.h"
#elif defined CONFIG_IDF_TARGET_ESP32C3
    #include "soc/esp32c3/include/soc/gpio_struct.h"
    #include "hal/esp32c3/include/hal/gpio_ll.h"
    #include "hal/esp32c3/include/hal/clk_gate_ll.h"
    #include "soc/esp32c3/include/soc/gpio_struct.h"
    #include "soc/esp32c3/include/soc/dport_access.h"
    #include "soc/esp32c3/include/soc/periph_defs.h"
    #include "soc/esp32c3/include/soc/usb_serial_jtag_reg.h"
    #include "soc/esp32c3/include/soc/io_mux_reg.h"
    #include "soc/esp32c3/include/soc/spi_struct.h"
    #include "soc/esp32c3/include/soc/spi_reg.h"
#elif defined CONFIG_IDF_TARGET_ESP32S3
    #include "soc/esp32s3/include/soc/gpio_struct.h"
    #include "hal/esp32s3/include/hal/gpio_ll.h"
    #include "hal/esp32s3/include/hal/clk_gate_ll.h"
    #include "soc/esp32s3/include/soc/gpio_struct.h"
    #include "soc/esp32s3/include/soc/dport_access.h"
    #include "soc/esp32s3/include/soc/periph_defs.h"
    #include "soc/esp32s3/include/soc/usb_serial_jtag_reg.h"
    #include "soc/esp32s3/include/soc/io_mux_reg.h"
    #include "soc/esp32s3/include/soc/spi_struct.h"
    #include "soc/esp32s3/include/soc/spi_reg.h"
#else
    #error unknown hardware
#endif


#endif
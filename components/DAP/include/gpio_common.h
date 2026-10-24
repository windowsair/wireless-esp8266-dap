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
    #include "soc/gpio_struct.h"
    #include "soc/dport_access.h"
    #include "soc/dport_reg.h"
    #include "soc/periph_defs.h"
    #include "soc/spi_struct.h"
    #include "soc/spi_reg.h"
    #include "soc/io_mux_reg.h"
    #include "soc/gpio_sig_map.h"
    #include "soc/gpio_periph.h"
    #include "hal/gpio_types.h"
#elif defined CONFIG_IDF_TARGET_ESP32C3
    #include "soc/gpio_struct.h"
    #include "hal/gpio_ll.h"
    #include "hal/clk_gate_ll.h"
    #include "hal/dedic_gpio_cpu_ll.h"
    #include "soc/dport_access.h"
    #include "soc/periph_defs.h"
    #include "soc/usb_serial_jtag_reg.h"
    #include "soc/io_mux_reg.h"
    #include "soc/gpio_sig_map.h"
    #include "soc/spi_struct.h"
    #include "soc/spi_reg.h"
    #include "soc/gpio_periph.h"
    #include "soc/system_struct.h"
#elif defined CONFIG_IDF_TARGET_ESP32S3
    #include "soc/gpio_struct.h"
    #include "hal/gpio_ll.h"
    #include "hal/clk_gate_ll.h"
    #include "hal/dedic_gpio_cpu_ll.h"
    #include "soc/dport_access.h"
    #include "soc/periph_defs.h"
    #include "soc/usb_serial_jtag_reg.h"
    #include "soc/io_mux_reg.h"
    #include "soc/gpio_sig_map.h"
    #include "soc/spi_struct.h"
    #include "soc/spi_reg.h"
    #include "soc/gpio_periph.h"
    #include "soc/system_struct.h"
#else
    #error unknown hardware
#endif


#endif
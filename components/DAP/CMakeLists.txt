set(COMPONENT_ADD_INCLUDEDIRS "config include $ENV{IDF_PATH}/components/esp_ringbuf/include/ $ENV{IDF_PATH}/components/")
set(COMPONENT_SRCS
    "./source/DAP.c"
    "./source/DAP_vendor.c"
    "./source/JTAG_DP.c"
    "./source/SW_DP.c"
    "./source/SWO.c"
    "./source/spi_op.c"
    "./source/spi_switch.c"
    "./source/dap_utility.c"
    "./source/swd_host.c")

if(NOT IDF_TARGET STREQUAL "esp8266")
    set(COMPONENT_REQUIRES esp_hal_gpio esp_hal_clock esp_hal_gpspi)
endif()

register_component()
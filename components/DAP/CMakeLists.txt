set(COMPONENT_ADD_INCLUDEDIRS "config include $ENV{IDF_PATH}/components/esp8266/include/esp8266/ $ENV{IDF_PATH}//components/esp_ringbuf/include/")
set(COMPONENT_SRCS "./source/DAP.c ./source/DAP_vendor.c ./source/JTAG_DP.c ./source/SW_DP.c ./source/SWO.c ./source/uart_modify.c ./source/spi_op.c ./source/spi_switch.c ./source/dap_utility.c")



register_component()
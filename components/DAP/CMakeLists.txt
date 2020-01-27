set(COMPONENT_ADD_INCLUDEDIRS "config include $ENV{IDF_PATH}/components/esp8266/include/esp8266/")
set(COMPONENT_SRCS "./source/DAP.c ./source/DAP_vendor.c ./source/JTAG_DP.c ./source/SW_DP.c ")
# ./source/SWO.c

register_component()
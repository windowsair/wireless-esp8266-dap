set(COMPONENT_ADD_INCLUDEDIRS ". $ENV{IDF_PATH}/components/esp8266/include/esp8266/ ../components/USBIP")
set(COMPONENT_SRCS "main.c timer.c tcp_server.c usbip_server.c DAP_handle.c")

register_component()

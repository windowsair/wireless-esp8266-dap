set(COMPONENT_SRCS "tcp_server.c")
set(COMPONENT_ADD_INCLUDEDIRS ". $ENV{IDF_PATH}/components/esp8266/include/esp8266/")

register_component()

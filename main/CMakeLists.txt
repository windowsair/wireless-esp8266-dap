set(COMPONENT_ADD_INCLUDEDIRS "${PROJECT_PATH}")
set(COMPONENT_SRCS
    main.c timer.c tcp_server.c usbip_server.c DAP_handle.c
    uart_bridge.c wifi_handle.c)

register_component()

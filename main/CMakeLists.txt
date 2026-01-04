set(COMPONENT_ADD_INCLUDEDIRS "${PROJECT_PATH}")
set(COMPONENT_SRCS
    main.c tcp_server.c usbip_server.c
    wifi_handle.c)

register_component()

#ifndef __OS_PRINTF_H__
#define __OS_PRINTF_H__

#include "sdkconfig.h"

#ifdef CONFIG_IDF_TARGET_ESP8266
    #include "rom/ets_sys.h" /* os_printf */
#else
    #include <stdio.h>
    #define os_printf printf
#endif

#endif /* __OS_PRINTF_H__ */

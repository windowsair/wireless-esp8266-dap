/**
 * @file gpio_op.h
 * @author windowsair
 * @brief esp GPIO operation
 * @version 0.1
 * @date 2021-03-03
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __GPIO_OP_H__
#define __GPIO_OP_H__

#include "sdkconfig.h"
#include "components/DAP/include/cmsis_compiler.h"
#ifdef CONFIG_IDF_TARGET_ESP8266
  #include "gpio.h"
  #include "esp8266/include/esp8266/gpio_struct.h"
  #include "esp8266/include/esp8266/timer_struct.h"
  #include "esp8266/pin_mux_register.h"
#elif defined CONFIG_IDF_TARGET_ESP32
  // soc register
  // #include "soc/soc/esp32/rom/gpio.h"
  #include "soc/soc/esp32/include/soc/gpio_struct.h"
  #include "hal/gpio_types.h"
#else
  #error unknown hardware
#endif



#ifdef CONFIG_IDF_TARGET_ESP8266
__STATIC_INLINE __UNUSED void GPIO_FUNCTION_SET(int io_num)
{
  gpio_pin_reg_t pin_reg;

  pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(io_num));

  // It should be noted that GPIO0, 2, 4, and 5 need to set the func register to 0,
  // and the other GPIO needs to be set to 3 so that IO can be GPIO function.
  if ((0x1 << io_num) & (GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5)) {
      pin_reg.rtc_pin.func_low_bit = 0;
      pin_reg.rtc_pin.func_high_bit = 0;
  } else {
      pin_reg.func_low_bit = 3;
      pin_reg.func_high_bit = 0;
  }

  WRITE_PERI_REG(GPIO_PIN_REG(io_num), pin_reg.val);
}
#elif defined CONFIG_IDF_TARGET_ESP32
__STATIC_INLINE void GPIO_FUNCTION_SET(int io_num)
{
    // function number 2 is GPIO_FUNC for each pin
    // Note that the index starts at 0, so we are using function 3.
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[io_num], PIN_FUNC_GPIO);
}
#endif

#ifdef CONFIG_IDF_TARGET_ESP8266
__UNUSED static void GPIO_SET_DIRECTION_NORMAL_OUT(int io_num)
{
  GPIO.enable_w1ts |= (0x1 << io_num);
  // PP out
  GPIO.pin[io_num].driver = 0;
}
#elif defined CONFIG_IDF_TARGET_ESP32
static void GPIO_SET_DIRECTION_NORMAL_OUT(int io_num)
{
    GPIO.enable_w1ts = (0x1 << io_num);
    // PP out
    GPIO.pin[io_num].pad_driver = 0;
}
#endif



__STATIC_INLINE __UNUSED void GPIO_SET_LEVEL_HIGH(int io_num)
{
  GPIO.out_w1ts |= (0x1 << io_num);

}


#ifdef CONFIG_IDF_TARGET_ESP32
__STATIC_INLINE void GPIO_PULL_UP_ONLY_SET(int io_num)
{
  // disable pull down
  REG_CLR_BIT(GPIO_PIN_MUX_REG[io_num], FUN_PD);
  // enable pull up
  REG_SET_BIT(GPIO_PIN_MUX_REG[io_num], FUN_PU);
}
#endif


//FIXME: esp32
__STATIC_INLINE __UNUSED void GPIO_SET_LEVEL_LOW(int io_num)
{
  GPIO.out_w1tc |= (0x1 << io_num);
}

// static void GPIO_SET_DIRECTION_NORMAL_IN(int io_num)
// {
//   GPIO.enable_w1tc |= (0x1 << io_num);
// }



#endif
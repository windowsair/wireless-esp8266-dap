#ifndef __GPIO_OP_H__
#define __GPIO_OP_H__

#include "components/DAP/include/cmsis_compiler.h"

#include "gpio.h"
#include "gpio_struct.h"
#include "timer_struct.h"
#include "esp8266/pin_mux_register.h"


__STATIC_INLINE void GPIO_FUNCTION_SET(int io_num)
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


static void GPIO_SET_DIRECTION_NORMAL_OUT(int io_num)
{
  GPIO.enable_w1ts |= (0x1 << io_num);
  // PP out
  GPIO.pin[io_num].driver = 0;
}


// static void GPIO_SET_DIRECTION_NORMAL_IN(int io_num)
// {
//   GPIO.enable_w1tc |= (0x1 << io_num);
// }



#endif
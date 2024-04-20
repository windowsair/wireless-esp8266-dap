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
#include "components/DAP/include/gpio_common.h"



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
__STATIC_INLINE __UNUSED void GPIO_FUNCTION_SET(int io_num)
{
    // function number 2 is GPIO_FUNC for each pin
    // Note that the index starts at 0, so we are using function 3.
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[io_num], PIN_FUNC_GPIO);
}
#elif defined CONFIG_IDF_TARGET_ESP32C3
__STATIC_INLINE __UNUSED void GPIO_FUNCTION_SET(int io_num)
{
  // Disable USB Serial JTAG if pins 18 or pins 19 needs to select an IOMUX function
  if (io_num == IO_MUX_GPIO18_REG || io_num == IO_MUX_GPIO19_REG) {
      CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_USB_PAD_ENABLE);
  }
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[io_num], PIN_FUNC_GPIO);
}
#elif defined CONFIG_IDF_TARGET_ESP32S3
__STATIC_INLINE __UNUSED void GPIO_FUNCTION_SET(int io_num)
{
  gpio_ll_iomux_func_sel(GPIO_PIN_MUX_REG[io_num], PIN_FUNC_GPIO);
}
#endif


#ifdef CONFIG_IDF_TARGET_ESP8266
__STATIC_INLINE __UNUSED void GPIO_SET_DIRECTION_NORMAL_OUT(int io_num)
{
  GPIO.enable_w1ts |= (0x1 << io_num);
  // PP out
  GPIO.pin[io_num].driver = 0;
}
#elif defined CONFIG_IDF_TARGET_ESP32
__STATIC_INLINE __UNUSED void GPIO_SET_DIRECTION_NORMAL_OUT(int io_num)
{
    GPIO.enable_w1ts = (0x1 << io_num);
    // PP out
    GPIO.pin[io_num].pad_driver = 0;
}
#elif defined CONFIG_IDF_TARGET_ESP32C3
__STATIC_INLINE __UNUSED void GPIO_SET_DIRECTION_NORMAL_OUT(int io_num)
{
    GPIO.enable_w1ts.enable_w1ts = (0x1 << io_num);
    // PP out
    GPIO.pin[io_num].pad_driver = 0;
}
#elif defined CONFIG_IDF_TARGET_ESP32S3
__STATIC_INLINE __UNUSED void GPIO_SET_DIRECTION_NORMAL_OUT(int io_num)
{
    gpio_ll_output_enable(&GPIO, io_num);
    // PP out
    gpio_ll_od_disable(&GPIO, io_num);
}
#endif


#if defined CONFIG_IDF_TARGET_ESP8266 || defined CONFIG_IDF_TARGET_ESP32
__STATIC_INLINE __UNUSED void GPIO_SET_LEVEL_HIGH(int io_num)
{
  GPIO.out_w1ts |= (0x1 << io_num);
}
//FIXME: esp32
__STATIC_INLINE __UNUSED void GPIO_SET_LEVEL_LOW(int io_num)
{
  GPIO.out_w1tc |= (0x1 << io_num);
}
#elif defined CONFIG_IDF_TARGET_ESP32C3 || defined CONFIG_IDF_TARGET_ESP32S3
__STATIC_INLINE __UNUSED void GPIO_SET_LEVEL_HIGH(int io_num)
{
  gpio_ll_set_level(&GPIO, io_num, 1);
}
__STATIC_INLINE __UNUSED void GPIO_SET_LEVEL_LOW(int io_num)
{
  gpio_ll_set_level(&GPIO, io_num, 0);
}
#endif


#if defined CONFIG_IDF_TARGET_ESP8266 || defined CONFIG_IDF_TARGET_ESP32
__STATIC_INLINE __UNUSED int GPIO_GET_LEVEL(int io_num)
{
  return ((GPIO.in >> io_num) & 0x1) ? 1 : 0;
}
#elif defined CONFIG_IDF_TARGET_ESP32C3 || defined CONFIG_IDF_TARGET_ESP32S3
__STATIC_INLINE __UNUSED int GPIO_GET_LEVEL(int io_num)
{
  return gpio_ll_get_level(&GPIO, io_num);
}
#endif




#if defined CONFIG_IDF_TARGET_ESP32 || defined CONFIG_IDF_TARGET_ESP32C3
__STATIC_INLINE __UNUSED void GPIO_PULL_UP_ONLY_SET(int io_num)
{
  // disable pull down
  REG_CLR_BIT(GPIO_PIN_MUX_REG[io_num], FUN_PD);
  // enable pull up
  REG_SET_BIT(GPIO_PIN_MUX_REG[io_num], FUN_PU);
}
#elif defined CONFIG_IDF_TARGET_ESP32S3
__STATIC_INLINE __UNUSED void GPIO_PULL_UP_ONLY_SET(int io_num)
{
  // disable pull down
  gpio_ll_pulldown_dis(&GPIO, io_num);
  // enable pull up
  gpio_ll_pullup_en(&GPIO, io_num);
}
#elif defined CONFIG_IDF_TARGET_ESP8266
__STATIC_INLINE __UNUSED void GPIO_PULL_UP_ONLY_SET(int io_num)
{
  gpio_pin_reg_t pin_reg;

  pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(io_num));
  pin_reg.pullup = 1;
  WRITE_PERI_REG(GPIO_PIN_REG(io_num), pin_reg.val);
}
#endif


// static void GPIO_SET_DIRECTION_NORMAL_IN(int io_num)
// {
//   GPIO.enable_w1tc |= (0x1 << io_num);
// }



#endif
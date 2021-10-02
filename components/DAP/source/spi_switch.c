/**
 * @file spi_switch.c
 * @author windowsair
 * @brief Switching between SPI mode and IO mode
 * @change: 2020-11-25 first version
 *          2021-2-11 Transmission mode switching test passed
 * @version 0.2
 * @date 2021-2-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <stdbool.h>

#include "components/DAP/include/cmsis_compiler.h"
#include "components/DAP/include/spi_switch.h"

#include "esp8266/spi_struct.h"
#include "esp8266/pin_mux_register.h"
#include "esp8266/gpio_struct.h"


#define DAP_SPI SPI1

#define ENTER_CRITICAL() portENTER_CRITICAL()
#define EXIT_CRITICAL() portEXIT_CRITICAL()

typedef enum {
    SPI_40MHz_DIV = 2,
    // SPI_80MHz_DIV = 1, //// FIXME: high speed clock
} spi_clk_div_t;


/**
 * @brief Initialize on first use
 *
 */
void DAP_SPI_Init()
{
    // The driving of GPIO should be stopped,
    // otherwise SPI has potential timing issues (This issue has been identified in OpenOCD)
    GPIO.out_w1tc = (0x1 << 13);
    GPIO.out_w1tc = (0x1 << 14);

    // Disable flash operation mode
    DAP_SPI.user.flash_mode = false;

    // Set to Master mode
    DAP_SPI.pin.slave_mode = false;
    DAP_SPI.slave.slave_mode = false;

    // Master uses the entire hardware buffer to improve transmission speed
    // If the following fields are enabled, only a part of the buffer is used
    DAP_SPI.user.usr_mosi_highpart = false;
    DAP_SPI.user.usr_miso_highpart = false;

    // Disable cs pin
    DAP_SPI.user.cs_setup = false;
    DAP_SPI.user.cs_hold = false;

    // Duplex transmit
    DAP_SPI.user.duplex = true;

    // SCLK delay setting
    DAP_SPI.user.ck_i_edge = true;
    DAP_SPI.ctrl2.mosi_delay_num = 0;
    DAP_SPI.ctrl2.miso_delay_num = 0;

    // DIO & QIO SPI disable
    DAP_SPI.user.fwrite_dual = false;
    DAP_SPI.user.fwrite_quad = false;
    DAP_SPI.user.fwrite_dio  = false;
    DAP_SPI.user.fwrite_qio  = false;
    DAP_SPI.ctrl.fread_dual  = false;
    DAP_SPI.ctrl.fread_quad  = false;
    DAP_SPI.ctrl.fread_dio   = false;
    DAP_SPI.ctrl.fread_qio   = false;
    DAP_SPI.ctrl.fastrd_mode = true;

    // Enable soft reset
    DAP_SPI.slave.sync_reset = true;

    // Set the clock polarity and phase CPOL = CPHA = 0
    DAP_SPI.pin.ck_idle_edge = 1;  // HIGH while idle
    DAP_SPI.user.ck_out_edge = 0;


    // Set data bit order
    DAP_SPI.ctrl.wr_bit_order = 1; // SWD -> LSB
    DAP_SPI.ctrl.rd_bit_order = 1; // SWD -> LSB
    // Set data byte order
    DAP_SPI.user.wr_byte_order = 0;  // SWD -> litte_endian && Risc V -> litte_endian
    DAP_SPI.user.rd_byte_order = 0;  // SWD -> litte_endian && Risc V -> litte_endian

    // Set dummy
    DAP_SPI.user.usr_dummy = 0;

    // Initialize HSPI IO
    gpio_pin_reg_t pin_reg;

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_HSPI_CLK); // GPIO14 is SPI CLK pin (Clock)
    GPIO.enable_w1ts |= (0x1 << 14); // PP Output
    pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(14));
    pin_reg.pullup = 1;
    WRITE_PERI_REG(GPIO_PIN_REG(14), pin_reg.val);

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_HSPID_MOSI); // GPIO13 is SPI MOSI pin (Master Data Out)
    GPIO.enable_w1ts |= (0x1 << 13);
    GPIO.pin[13].driver = 0; // PP Output or OD output
    pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(13));
    pin_reg.pullup = 0;
    WRITE_PERI_REG(GPIO_PIN_REG(13), pin_reg.val);

#if (USE_SPI_SIO != 1)
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_HSPIQ_MISO); // GPIO12 is SPI MISO pin (Master Data In)
    // esp8266 in is always connected
    pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(12));
    pin_reg.pullup = 0;
    WRITE_PERI_REG(GPIO_PIN_REG(12), pin_reg.val);
#endif // (USE_SPI_SIO != 1)



    // Set spi clk div
    CLEAR_PERI_REG_MASK(PERIPHS_IO_MUX_CONF_U, SPI1_CLK_EQU_SYS_CLK);

    DAP_SPI.clock.clk_equ_sysclk = false;
    DAP_SPI.clock.clkdiv_pre = 0;
    DAP_SPI.clock.clkcnt_n = SPI_40MHz_DIV - 1;
    DAP_SPI.clock.clkcnt_h = SPI_40MHz_DIV / 2 - 1;
    DAP_SPI.clock.clkcnt_l = SPI_40MHz_DIV - 1;

    // Do not use command and addr
    DAP_SPI.user.usr_command = 0;
    DAP_SPI.user.usr_addr = 0;
}

/**
 * @brief Switch to GPIO
 * Note: You may be able to pull the pin high in SPI mode, though you cannot set it to LOW
 */
__FORCEINLINE void DAP_SPI_Deinit()
{
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13); // MOSI
#if (USE_SPI_SIO != 1)
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); // MISO

    // disable MISO output connect
    GPIO.enable_w1tc |= (0x1 << 12);
#endif // (USE_SPI_SIO != 1)

    // enable SWCLK output
    GPIO.enable_w1ts |= (0x01 << 14);

    gpio_pin_reg_t pin_reg;
    GPIO.enable_w1ts |= (0x1 << 13);
    GPIO.pin[13].driver = 1; // OD output
    pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(13));
    pin_reg.pullup = 0;
    WRITE_PERI_REG(GPIO_PIN_REG(13), pin_reg.val);
}


/**
 * @brief Use SPI acclerate
 *
 */
void DAP_SPI_Enable()
{
    // may be unuse
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_HSPID_MOSI); // GPIO13 is SPI MOSI pin (Master Data Out)
}


/**
 * @brief Disable SPI
 * Drive capability not yet known
 */
__FORCEINLINE void DAP_SPI_Disable()
{
    ;
    //CLEAR_PERI_REG_MASK(PERIPHS_IO_MUX_MTCK_U, (PERIPHS_IO_MUX_FUNC << PERIPHS_IO_MUX_FUNC_S));
    // may be unuse
    // gpio_pin_reg_t pin_reg;
    // GPIO.enable_w1ts |= (0x1 << 13);
    // GPIO.pin[13].driver = 0; // OD Output
    // pin_reg.val = READ_PERI_REG(GPIO_PIN_REG(13));
    // pin_reg.pullup = 1;
    // WRITE_PERI_REG(GPIO_PIN_REG(13), pin_reg.val);
}


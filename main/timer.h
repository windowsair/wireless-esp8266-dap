/**
 * @file timer.h
 * @author windowsair
 * @brief esp8266 hardware timer
 * @change: 2021-02-18 Add frc2 timer
 * @note
 *  FRC2 is not officially documented. There is no guarantee of its behavior.
 *  FRC2 may be used for RTC functions. May be reserved for other functions in the future.
 *
 *
 * @version 0.1
 * @date 2021-02-18
 *
 * @copyright Copyright (c) 2021
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */
#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

#include "sdkconfig.h"

#ifdef CONFIG_IDF_TARGET_ESP8266
// FRC2 is a 32-bit countup timer
typedef struct {
    union {
        struct {
            uint32_t data:        32;
        };
        uint32_t val;
    } load;

    union {
        struct {
            uint32_t data:        31;
            uint32_t reserved23:   1;
        };
        uint32_t val;
    } count;

    union {
        struct {
            uint32_t div:          6;
            uint32_t reload:       1;
            uint32_t en:           1;
            uint32_t intr_type:    1;
            uint32_t reserved24:  23;
        };
        uint32_t val;
    } ctrl;

    union {
        struct {
            uint32_t clr:          1;
            uint32_t reserved1:   31;
        };
        uint32_t val;
    } intr;
} frc2_struct_t;

extern volatile frc2_struct_t* frc2;
#endif

extern void timer_init();
extern uint32_t get_timer_count();

#endif
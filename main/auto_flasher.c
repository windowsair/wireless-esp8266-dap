/**
 * @file auto_flasher.c
 * @brief Offline auto-flash engine implementation
 *
 * Core state machine that:
 *  1. Polls SWD connection to detect target MCU
 *  2. Loads flash algorithm into target RAM
 *  3. Executes init → erase → program → verify → uninit
 *  4. Controls LED for visual status indication
 */
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "main/auto_flasher.h"
#include "main/firmware_store.h"
#include "main/wifi_configuration.h"
#include "main/dap_configuration.h"

#include "components/DAP/include/swd_host.h"
#include "components/DAP/include/DAP.h"
#include "components/DAP/include/debug_cm.h"
#include "components/flash_algo/flash_algo.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/gpio.h"

/* ---- LED Configuration ---- */
/* Use existing LED_WIFI_STATUS pin (GPIO10 on ESP32C3) for status */
#ifndef LED_WIFI_STATUS_GPIO
#define LED_WIFI_STATUS_GPIO GPIO_NUM_10
#endif

static flash_status_t s_flash_status;
static volatile int    s_trigger_flash = 0;
static volatile int    s_flash_enabled = 1;
static uint32_t        s_target_idcode = 0;
static uint32_t        s_last_idcode   = 0;   /* for 2-step confirm */
static uint32_t        s_done_idcode   = 0;   /* last flushed target — skip until disconnected */
static target_chip_t   s_target_chip   = TARGET_UNKNOWN;

/* ---- Internal helpers ---- */

static void led_slow_blink(void) {
    /* Will be set via GPIO toggle in loop — handled in task */
}

static void set_led(int on) {
    gpio_set_level(LED_WIFI_STATUS_GPIO, on ? 1 : 0);
}

static int target_connect(void) {
    /* Initialize SWD and detect target */
    DAP_Setup();
    PORT_SWD_SETUP();

    /* Try to connect: send 50+ SWCLK cycles with SWDIO high for line reset */
    uint8_t reset_seq[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    SWJ_Sequence(51, reset_seq);

    /* Read DP IDCODE */
    uint32_t idcode = 0;
    if (!swd_read_dp(DP_IDCODE, &idcode)) {
        return 0;
    }

    /* Valid IDCODE: not 0 and not all Fs */
    if (idcode == 0 || idcode == 0xFFFFFFFF) {
        return 0;
    }

    s_target_idcode = idcode;
    s_target_chip = flash_algo_detect(idcode);
    return (s_target_chip != TARGET_UNKNOWN);
}

static int flash_execute(void) {
    const program_target_t *algo = flash_algo_get(s_target_chip);
    const target_cfg_t *tgt = flash_algo_get_target_config(s_target_chip);

    if (algo == NULL || tgt == NULL) {
        snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                 "No flash algorithm for chip %s", flash_algo_get_name(s_target_chip));
        return 0;
    }

    size_t fw_size = fs_get_stored_size();
    if (fw_size == 0) {
        snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                 "No firmware stored");
        return 0;
    }

    uint32_t flash_start = tgt->flash_regions[0].start;
    uint32_t flash_end   = tgt->flash_regions[0].end;
    uint32_t flash_total = flash_end - flash_start;

    if (fw_size > flash_total) {
        snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                 "Firmware (%u bytes) exceeds target flash (%lu bytes)",
                 (unsigned int)fw_size, (unsigned long)flash_total);
        return 0;
    }

    /* --- Phase 1: Reset & halt target --- */
    s_flash_status.state = FLASH_STATE_LOADING_ALGO;
    s_flash_status.state_name = "Loading flash algorithm";
    if (!swd_set_target_state_hw(RESET_PROGRAM)) {
        snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                 "Failed to reset target");
        goto fail;
    }

    /* Load flash algorithm blob into target RAM */
    if (!swd_flash_load_algo(algo)) {
        snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                 "Failed to load flash algorithm");
        goto fail;
    }

    /* --- Phase 2: Init flash controller --- */
    s_flash_status.state = FLASH_STATE_INIT;
    s_flash_status.state_name = "Initializing flash";
    /* Init function code: 1 = erase, 2 = program, 3 = verify */
    if (!swd_flash_syscall_exec(&algo->sys_call_s, algo->init,
                                 0, 0, 0, 0, FLASHALGO_RETURN_BOOL)) {
        snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                 "Flash init failed");
        goto fail_uninit;
    }

    /* --- Phase 3: Erase --- */
    s_flash_status.state = FLASH_STATE_ERASING;
    s_flash_status.state_name = "Erasing chip";
    if (!swd_flash_syscall_exec(&algo->sys_call_s, algo->erase_chip,
                                 0, 0, 0, 0, FLASHALGO_RETURN_BOOL)) {
        snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                 "Chip erase failed");
        goto fail_uninit;
    }

    /* --- Phase 4: Program --- */
    s_flash_status.state = FLASH_STATE_PROGRAMMING;
    s_flash_status.state_name = "Programming";
    s_flash_status.total_bytes = fw_size;

    uint32_t page_size = algo->program_buffer_size;
    uint8_t *page_buf = (uint8_t *)malloc(page_size);
    if (page_buf == NULL) {
        snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                 "Out of memory");
        goto fail_uninit;
    }

    size_t offset = 0;
    while (offset < fw_size) {
        size_t chunk = fw_size - offset;
        if (chunk > page_size) chunk = page_size;

        /* Read firmware page from ESP flash */
        if (fs_read(offset, page_buf, chunk) != ESP_OK) {
            snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                     "Failed to read firmware from ESP flash");
            free(page_buf);
            goto fail_uninit;
        }

        /* Write page data to target RAM buffer */
        if (!swd_write_memory(algo->program_buffer, page_buf, chunk)) {
            snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                     "Failed to write page data to target RAM");
            free(page_buf);
            goto fail_uninit;
        }

        /* Execute program_page(flash_addr, size, buffer_addr) */
        uint32_t flash_addr = flash_start + offset;
        if (!swd_flash_syscall_exec(&algo->sys_call_s, algo->program_page,
                                     flash_addr, chunk, algo->program_buffer, 0,
                                     FLASHALGO_RETURN_BOOL)) {
            snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                     "Program page failed at offset %u", (unsigned int)offset);
            free(page_buf);
            goto fail_uninit;
        }

        offset += chunk;
        s_flash_status.current_bytes = offset;
        /* Brief delay to prevent watchdog issues */
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    free(page_buf);

    /* --- Phase 5: Verify (optional, via re-read) --- */
    s_flash_status.state = FLASH_STATE_VERIFYING;
    s_flash_status.state_name = "Verifying";

    {
        uint8_t *verify_buf = (uint8_t *)malloc(page_size);
        uint8_t *source_buf = (uint8_t *)malloc(page_size);
        if (verify_buf && source_buf) {
            offset = 0;
            while (offset < fw_size) {
                size_t chunk = fw_size - offset;
                if (chunk > page_size) chunk = page_size;

                if (fs_read(offset, source_buf, chunk) != ESP_OK) break;
                if (!swd_read_memory(flash_start + offset, verify_buf, chunk)) break;
                if (memcmp(source_buf, verify_buf, chunk) != 0) {
                    snprintf(s_flash_status.last_error, sizeof(s_flash_status.last_error),
                             "Verification failed at offset %u", (unsigned int)offset);
                    free(verify_buf);
                    free(source_buf);
                    goto fail_uninit;
                }
                offset += chunk;
            }
        }
        if (verify_buf) free(verify_buf);
        if (source_buf) free(source_buf);
    }

    /* --- Phase 6: Uninit and run --- */
    s_flash_status.state = FLASH_STATE_UNINIT;
    s_flash_status.state_name = "Finalizing";
    swd_flash_syscall_exec(&algo->sys_call_s, algo->uninit,
                            0, 0, 0, 0, FLASHALGO_RETURN_BOOL);

    /* Release target from reset */
    swd_set_target_state_hw(RUN);
    swd_off();

    s_flash_status.state = FLASH_STATE_DONE;
    s_flash_status.state_name = "Flash complete";
    return 1;

fail_uninit:
    swd_flash_syscall_exec(&algo->sys_call_s, algo->uninit,
                            0, 0, 0, 0, FLASHALGO_RETURN_BOOL);
fail:
    s_flash_status.state = FLASH_STATE_FAILED;
    s_flash_status.state_name = "Failed";
    s_flash_status.error_count++;
    return 0;
}

/* ---- Public API ---- */

void auto_flasher_init(void) {
    memset(&s_flash_status, 0, sizeof(s_flash_status));
    s_flash_status.state = FLASH_STATE_IDLE;
    s_flash_status.state_name = "Idle";

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_WIFI_STATUS_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    set_led(0);

    os_printf("[AUTO_FLASH] Initialized\n");
}

void auto_flasher_trigger(void) {
    s_trigger_flash = 1;
}

flash_status_t auto_flasher_get_status(void) {
    return s_flash_status;
}

void auto_flasher_task(void *pvParameters) {
    uint32_t detect_ticks  = 0;
    uint32_t result_ticks  = 0;
    uint32_t blink_phase   = 0;
    int      target_was_lost = 0;

    auto_flasher_init();

    while (1) {
        switch (s_flash_status.state) {

        case FLASH_STATE_IDLE:
            /* Slow blink: 100ms on, 1900ms off — "ready, waiting for target" */
            set_led(blink_phase % 20 == 0);

            /* Periodic target detection */
            if (++detect_ticks >= (TARGET_DETECT_INTERVAL / 500)) {
                detect_ticks = 0;
                s_flash_status.state = FLASH_STATE_DETECTING;
                s_flash_status.state_name = "Detecting";
            }
            break;

        case FLASH_STATE_DETECTING:
            set_led(1);
            if (target_connect()) {
                /* Guard: skip target that was just flashed — user must disconnect it first */
                if (s_done_idcode != 0 && s_target_idcode == s_done_idcode) {
                    os_printf("[AUTO_FLASH] Target 0x%08lX already flashed, "
                             "waiting for disconnect...\n",
                             (unsigned long)s_done_idcode);
                    s_flash_status.state = FLASH_STATE_IDLE;
                    s_flash_status.state_name = "Waiting disconnect";
                    s_last_idcode = 0;
                    vTaskDelay(pdMS_TO_TICKS(TARGET_DETECT_INTERVAL));
                    break;
                }
                /* Clear done guard when a different target appears */
                if (s_target_idcode != s_done_idcode) {
                    s_done_idcode = 0;
                }
                s_last_idcode = s_target_idcode;
                s_flash_status.state = FLASH_STATE_TARGET_FOUND;
                s_flash_status.state_name = "Target found";
                s_flash_status.target_name = flash_algo_get_name(s_target_chip);
                s_flash_status.target_idcode = s_target_idcode;
                os_printf("[AUTO_FLASH] Target detected: %s (IDCODE: 0x%08lX), "
                         "confirming...\n",
                         s_flash_status.target_name,
                         (unsigned long)s_target_idcode);
            } else {
                /* Target disconnected — clear done guard so next connection triggers flash */
                if (s_done_idcode != 0) {
                    os_printf("[AUTO_FLASH] Target disconnected, guard cleared\n");
                    s_done_idcode = 0;
                }
                s_flash_status.state = FLASH_STATE_IDLE;
                s_flash_status.state_name = "Idle";
                s_last_idcode = 0;
                vTaskDelay(pdMS_TO_TICKS(TARGET_DETECT_INTERVAL));
            }
            break;

        case FLASH_STATE_TARGET_FOUND:
            /* Re-detect: must see the same IDCODE twice before arming */
            s_flash_status.state_name = "Confirming target";
            vTaskDelay(pdMS_TO_TICKS(500));  /* brief settle time */
            if (target_connect() && s_target_idcode == s_last_idcode) {
                os_printf("[AUTO_FLASH] Target confirmed: %s (IDCODE: 0x%08lX)\n",
                         flash_algo_get_name(s_target_chip),
                         (unsigned long)s_target_idcode);
                s_flash_status.state = FLASH_STATE_ARMED;
                s_flash_status.state_name = "Armed";
            } else {
                os_printf("[AUTO_FLASH] Target confirmation failed, back to idle\n");
                s_flash_status.state = FLASH_STATE_IDLE;
                s_flash_status.state_name = "Idle";
                s_last_idcode = 0;
            }
            break;

        case FLASH_STATE_ARMED:
            set_led(1);
            /* Auto-flash immediately if firmware is present */
            if (fs_get_stored_size() > 0) {
                os_printf("[AUTO_FLASH] Armed + firmware present, auto-flashing...\n");
                if (flash_execute()) {
                    result_ticks = 0;
                }
            } else {
                s_flash_status.state_name = "Armed (no firmware)";
                if (s_trigger_flash) {
                    s_trigger_flash = 0;
                    os_printf("[AUTO_FLASH] Manual flash triggered\n");
                    if (flash_execute()) {
                        result_ticks = 0;
                    }
                }
            }
            break;

        case FLASH_STATE_DONE:
            /* Solid LED for 3 seconds, then go to IDLE.
               Guard prevents re-flashing the same target until it's disconnected. */
            set_led(1);
            if (++result_ticks >= 6) {  /* 3s / 500ms = 6 */
                result_ticks = 0;
                s_done_idcode = s_target_idcode;  /* lock this target from re-flash */
                s_flash_status.state = FLASH_STATE_IDLE;
                s_flash_status.state_name = "Idle";
                s_last_idcode = 0;
                os_printf("[AUTO_FLASH] Done. Disconnect target to re-arm.\n");
            }
            break;

        case FLASH_STATE_FAILED:
            /* Fast blink: 200ms on, 200ms off — error indication */
            set_led((blink_phase % 2) == 0);
            /* Auto-recover after 10 seconds */
            if (++result_ticks >= 20) {  /* 10s / 500ms = 20 */
                result_ticks = 0;
                s_flash_status.state = FLASH_STATE_IDLE;
                s_flash_status.state_name = "Idle";
                s_last_idcode = 0;
                os_printf("[AUTO_FLASH] Error timeout, returning to idle\n");
            }
            break;

        /* Intermediate states handled synchronously by flash_execute() */
        case FLASH_STATE_LOADING_ALGO:
        case FLASH_STATE_INIT:
        case FLASH_STATE_ERASING:
        case FLASH_STATE_PROGRAMMING:
        case FLASH_STATE_VERIFYING:
        case FLASH_STATE_UNINIT:
            break;
        }

        blink_phase++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

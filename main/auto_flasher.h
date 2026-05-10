/**
 * @file auto_flasher.h
 * @brief Offline auto-flash engine
 *
 * Detects target MCU via SWD, loads flash algorithm,
 * and executes erase → program → verify sequence
 * using firmware stored in ESP flash partition.
 */
#ifndef AUTO_FLASHER_H
#define AUTO_FLASHER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Flash state machine states */
typedef enum {
    FLASH_STATE_IDLE,
    FLASH_STATE_DETECTING,
    FLASH_STATE_TARGET_FOUND,
    FLASH_STATE_CONFIRM,     /* second detection before auto-flash */
    FLASH_STATE_ARMED,       /* confirmed, ready to auto-flash */
    FLASH_STATE_LOADING_ALGO,
    FLASH_STATE_INIT,
    FLASH_STATE_ERASING,
    FLASH_STATE_PROGRAMMING,
    FLASH_STATE_VERIFYING,
    FLASH_STATE_UNINIT,
    FLASH_STATE_DONE,
    FLASH_STATE_FAILED,
} flash_state_t;

/** Progress information for status reporting */
typedef struct {
    flash_state_t state;
    const char   *state_name;
    uint32_t      total_bytes;
    uint32_t      current_bytes;
    const char   *target_name;
    uint32_t      target_idcode;
    int           error_count;
    char          last_error[64];
} flash_status_t;

/**
 * @brief Initialize the auto-flasher module.
 */
void auto_flasher_init(void);

/**
 * @brief FreeRTOS task entry for the auto-flasher.
 * Handles target detection loop and flash state machine.
 */
void auto_flasher_task(void *pvParameters);

/**
 * @brief Trigger a manual flash sequence.
 * Only effective when target is already detected.
 */
void auto_flasher_trigger(void);

/**
 * @brief Get current flash status for external reporting.
 */
flash_status_t auto_flasher_get_status(void);

#ifdef __cplusplus
}
#endif

#endif /* AUTO_FLASHER_H */

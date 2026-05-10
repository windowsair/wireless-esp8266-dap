/**
 * @file flash_algo.h
 * @brief Flash algorithm definitions for offline programmer
 *
 * Provides target chip detection and algorithm retrieval.
 * Flash algorithm blobs are ported from ARM DAPLink project.
 */
#ifndef FLASH_ALGO_H
#define FLASH_ALGO_H

#include <stdint.h>
#include "components/DAP/include/flash_blob.h"
#include "components/DAP/config/target_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Supported target chip families */
typedef enum {
    TARGET_STM32F1_MD,      /* STM32F103 中等密度 (64/128KB Flash) */
    TARGET_STM32F1_HD,      /* STM32F103 高密度 (256/512KB Flash) */
    TARGET_UNKNOWN,
} target_chip_t;

/**
 * @brief Detect target chip from SWD IDCODE.
 * Can be extended to detect specific MCU families by reading
 * additional debug registers (DBGMCU_IDCODE, etc.)
 */
target_chip_t flash_algo_detect(uint32_t idcode);

/**
 * @brief Get flash algorithm for a given target chip.
 * @return pointer to program_target_t, or NULL if not supported
 */
const program_target_t *flash_algo_get(target_chip_t chip);

/**
 * @brief Get target configuration for a given target chip.
 * @return pointer to target_cfg_t, or NULL if not supported
 */
const target_cfg_t *flash_algo_get_target_config(target_chip_t chip);

/**
 * @brief Get human-readable name for a target chip.
 */
const char *flash_algo_get_name(target_chip_t chip);

#ifdef __cplusplus
}
#endif

#endif /* FLASH_ALGO_H */

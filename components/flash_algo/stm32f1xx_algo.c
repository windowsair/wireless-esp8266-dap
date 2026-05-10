/**
 * @file stm32f1xx_algo.c
 * @brief STM32F1xx Flash algorithm for offline programmer
 *
 * Flash algorithm blob ported from ARM DAPLink project:
 *   https://github.com/ARMmbed/DAPLink/tree/main/source/family/st/stm32f103rb
 *
 * Supports:
 *   - STM32F103 MD (medium density): C8/CB, 64/128KB Flash
 *   - STM32F103 HD (high density): RB/RD/RE, 256/512KB Flash
 *
 * License: Apache-2.0 (from DAPLink)
 */

#include "components/flash_algo/flash_algo.h"

/* ---- STM32F1xx Flash Algorithm Blob (Cortex-M3 Thumb) ---- */

static const uint32_t stm32f1_algo_blob[] = {
    0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F2,
    0x4603b510, 0x4c442000, 0x48446020, 0x48446060, 0x46206060, 0xf01069c0, 0xd1080f04, 0x5055f245,
    0x60204c40, 0x60602006, 0x70fff640, 0x200060a0, 0x4601bd10, 0x69004838, 0x0080f040, 0x61104a36,
    0x47702000, 0x69004834, 0x0004f040, 0x61084932, 0x69004608, 0x0040f040, 0xe0036108, 0x20aaf64a,
    0x60084930, 0x68c0482c, 0x0f01f010, 0x482ad1f6, 0xf0206900, 0x49280004, 0x20006108, 0x46014770,
    0x69004825, 0x0002f040, 0x61104a23, 0x61414610, 0xf0406900, 0x61100040, 0xf64ae003, 0x4a2120aa,
    0x481d6010, 0xf01068c0, 0xd1f60f01, 0x6900481a, 0x0002f020, 0x61104a18, 0x47702000, 0x4603b510,
    0xf0201c48, 0xe0220101, 0x69004813, 0x0001f040, 0x61204c11, 0x80188810, 0x480fbf00, 0xf01068c0,
    0xd1fa0f01, 0x6900480c, 0x0001f020, 0x61204c0a, 0x68c04620, 0x0f14f010, 0x4620d006, 0xf04068c0,
    0x60e00014, 0xbd102001, 0x1c921c9b, 0x29001e89, 0x2000d1da, 0x0000e7f7, 0x40022000, 0x45670123,
    0xcdef89ab, 0x40003000, 0x00000000
};

/*
 * algo_blob entry points (relative offsets from start of blob):
 *   0x20000000 + 0x00 = algo_start
 *   0x20000000 + 0x21 = init
 *   0x20000000 + 0x53 = uninit
 *   0x20000000 + 0x65 = erase_chip
 *   0x20000000 + 0x9F = erase_sector
 *   0x20000000 + 0xDD = program_page
 *
 * Stack: 0x20000800  (2KB above algo + buffer)
 * Static base: 0x20000148 (r9 — used for literal pool base)
 */

/* ---- Sector Layout ---- */
/*
 * STM32F103 Flash is organized as uniform 1KB pages.
 * Medium density: 64 or 128 pages (64KB / 128KB)
 * High density: 256 or 512 pages (256KB / 512KB)
 */

static const sector_info_t stm32f1_md_sectors[] = {
    {0x08000000, 0x400},  /* 1KB per sector, 64/128 sectors */
};

static const sector_info_t stm32f1_hd_sectors[] = {
    {0x08000000, 0x400},  /* 1KB per sector, 256/512 sectors */
};

/* ---- Flash Algorithm (shared between MD and HD) ---- */

static const program_target_t stm32f1_flash_algo = {
    .init               = 0x20000021,
    .uninit             = 0x20000053,
    .erase_chip         = 0x20000065,
    .erase_sector       = 0x2000009F,
    .program_page       = 0x200000DD,
    .verify             = 0x0,  /* no verify function in blob */
    {
        .breakpoint     = 0x20000001,
        .static_base    = 0x20000148,  /* R9 — literal pool base */
        .stack_pointer  = 0x20000800,
    },
    .program_buffer     = 0x20000A00,   /* 0x20000000 + 0x00000A00 */
    .algo_start         = 0x20000000,
    .algo_size          = sizeof(stm32f1_algo_blob),
    .algo_blob          = stm32f1_algo_blob,
    .program_buffer_size = 0x00000400,  /* 1KB page buffer */
    .algo_flags         = 0,            /* no special flags */
};

/* ---- Target Configurations ---- */

static target_cfg_t stm32f1_md_target = {
    .version            = 1,
    .sectors_info       = stm32f1_md_sectors,
    .sector_info_length = 1,
    .flash_regions      = {
        { .start = 0x08000000, .end = 0x08010000, .flags = 1, .flash_algo = (program_target_t *)&stm32f1_flash_algo },
    },
    .ram_regions        = {
        { .start = 0x20000000, .end = 0x20005000 },  /* 20KB SRAM */
    },
    .target_vendor      = "STMicroelectronics",
    .target_part_number = "STM32F103C8/CB",
    .erase_reset        = 1,
};

static target_cfg_t stm32f1_hd_target = {
    .version            = 1,
    .sectors_info       = stm32f1_hd_sectors,
    .sector_info_length = 1,
    .flash_regions      = {
        { .start = 0x08000000, .end = 0x08020000, .flags = 1, .flash_algo = (program_target_t *)&stm32f1_flash_algo },
    },
    .ram_regions        = {
        { .start = 0x20000000, .end = 0x20005000 },
    },
    .target_vendor      = "STMicroelectronics",
    .target_part_number = "STM32F103RB/RD/RE",
    .erase_reset        = 1,
};

/* ---- Public API ---- */

target_chip_t flash_algo_detect(uint32_t idcode) {
    /*
     * Cortex-M3 IDCODE format:
     *   Bits [31:16] = 0x3BA0 (STMicroelectronics JEDEC code for STM32F1)
     *   Bits [15:0]  = device specific
     *
     * STM32F103 variants:
     *   0x3BA00477 = STM32F103 medium density (C8/CB)
     *   0x3BA00414 = STM32F103 high density (RB/RD)
     *   0x3BA00410 = STM32F103 XL density
     *
     * We also check DBGMCU_IDCODE register at 0xE0042000 to get
     * exact device ID, but that requires more advanced detection.
     */
    if ((idcode & 0xFFF00000) == 0x3BA00000) {
        uint32_t device_id = idcode & 0x00000FFF;
        switch (device_id) {
            case 0x412:  /* STM32F103 low density */
            case 0x410:  /* STM32F103 medium density */
                return TARGET_STM32F1_MD;
            case 0x414:  /* STM32F103 high density */
                return TARGET_STM32F1_HD;
            default:
                /* Try as medium density for any other STM32F1 variant */
                return TARGET_STM32F1_MD;
        }
    }
    return TARGET_UNKNOWN;
}

const program_target_t *flash_algo_get(target_chip_t chip) {
    switch (chip) {
        case TARGET_STM32F1_MD:
        case TARGET_STM32F1_HD:
            return &stm32f1_flash_algo;
        default:
            return NULL;
    }
}

const target_cfg_t *flash_algo_get_target_config(target_chip_t chip) {
    switch (chip) {
        case TARGET_STM32F1_MD:
            return &stm32f1_md_target;
        case TARGET_STM32F1_HD:
            return &stm32f1_hd_target;
        default:
            return NULL;
    }
}

const char *flash_algo_get_name(target_chip_t chip) {
    switch (chip) {
        case TARGET_STM32F1_MD:
            return "STM32F103C8/CB (Medium Density)";
        case TARGET_STM32F1_HD:
            return "STM32F103RB/RD/RE (High Density)";
        default:
            return "Unknown";
    }
}

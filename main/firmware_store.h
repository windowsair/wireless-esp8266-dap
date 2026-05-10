/**
 * @file firmware_store.h
 * @brief Target firmware storage management on ESP flash
 *
 * Layout:
 *   Sector 0 (4KB): metadata only
 *   Sector 1+:     firmware payload
 *
 * This ensures fs_finalize() can safely erase/write metadata
 * without touching the firmware data.
 */
#ifndef FIRMWARE_STORE_H
#define FIRMWARE_STORE_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Offset where firmware payload begins (1 full 4KB sector for metadata) */
#define FS_DATA_OFFSET 4096

/** Firmware metadata stored in the first sector of the partition */
typedef struct __attribute__((__packed__)) {
    uint32_t magic;         /* 0x46574D50 ("FWMP") */
    uint32_t version;       /* metadata format version */
    uint32_t firmware_size; /* size of stored firmware in bytes */
    uint32_t crc32;         /* CRC32 of firmware data */
    uint32_t target_family; /* target chip family (reserved, set to 0) */
    uint32_t flash_address; /* target flash start address */
    uint32_t flags;         /* reserved flags */
    uint32_t timestamp;     /* Unix timestamp of last write */
    uint8_t  reserved[32];  /* padding to 64 bytes */
} firmware_meta_t;

#define FW_META_MAGIC   0x46574D50
#define FW_META_VERSION 2

esp_err_t fs_init(void);
esp_err_t fs_erase(void);

/**
 * @brief Write a chunk of firmware payload data.
 * offset is relative to firmware payload start (after metadata sector).
 */
esp_err_t fs_write(size_t offset, const uint8_t *data, size_t len);
esp_err_t fs_read(size_t offset, uint8_t *buf, size_t len);

size_t    fs_get_stored_size(void);
uint32_t  fs_get_checksum(void);

/**
 * @brief Finalize after all payload written.
 * Erases ONLY the metadata sector (first 4KB) and writes metadata.
 * Firmware payload is untouched.
 */
esp_err_t fs_finalize(size_t total_size, uint32_t crc);
size_t    fs_get_capacity(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRMWARE_STORE_H */

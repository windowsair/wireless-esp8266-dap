/**
 * @file firmware_store.c
 * @brief Target firmware storage — metadata sector + payload
 *
 * Partition layout:
 *   [ Sector 0 (4KB): firmware_meta_t  ][ Sector 1..N: firmware payload ]
 *
 * fs_finalize() erases only sector 0, so payload is never destroyed
 * by a metadata update.
 */
#include <string.h>
#include "main/firmware_store.h"
#include "main/wifi_configuration.h"

#include "esp_partition.h"
#include "esp_log.h"
#include "rom/crc.h"

static const esp_partition_t *s_partition = NULL;
static firmware_meta_t        s_meta;
static size_t                 s_written = 0;   /* bytes written so far in current upload */

esp_err_t fs_init(void) {
    s_partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        ESP_PARTITION_SUBTYPE_DATA_FAT,
        FIRMWARE_PARTITION_NAME
    );

    if (s_partition == NULL) {
        os_printf("[FW_STORE] Partition '%s' not found\n", FIRMWARE_PARTITION_NAME);
        return ESP_ERR_NOT_FOUND;
    }

    os_printf("[FW_STORE] Partition '%s' at 0x%lx, %luKB total, %luKB payload\n",
              FIRMWARE_PARTITION_NAME,
              (unsigned long)s_partition->address,
              (unsigned long)(s_partition->size / 1024),
              (unsigned long)((s_partition->size - FS_DATA_OFFSET) / 1024));

    /* Read metadata from sector 0 */
    esp_err_t err = esp_partition_read(s_partition, 0, &s_meta, sizeof(s_meta));
    if (err != ESP_OK || s_meta.magic != FW_META_MAGIC) {
        os_printf("[FW_STORE] No valid firmware stored\n");
        memset(&s_meta, 0, sizeof(s_meta));
        s_written = 0;
        return ESP_OK;   /* partition exists, just empty */
    }

    /* Validate stored size does not exceed capacity */
    if (s_meta.firmware_size > (s_partition->size - FS_DATA_OFFSET)) {
        os_printf("[FW_STORE] Stored size %lu exceeds capacity %lu, discarding\n",
                  (unsigned long)s_meta.firmware_size,
                  (unsigned long)(s_partition->size - FS_DATA_OFFSET));
        memset(&s_meta, 0, sizeof(s_meta));
        s_written = 0;
        return ESP_OK;
    }

    os_printf("[FW_STORE] Firmware: %lu bytes, CRC32 0x%08lX\n",
              (unsigned long)s_meta.firmware_size,
              (unsigned long)s_meta.crc32);
    s_written = s_meta.firmware_size;
    return ESP_OK;
}

esp_err_t fs_erase(void) {
    if (s_partition == NULL) return ESP_ERR_INVALID_STATE;

    /* Erase entire partition (metadata + payload) */
    esp_err_t err = esp_partition_erase_range(s_partition, 0, s_partition->size);
    if (err != ESP_OK) {
        os_printf("[FW_STORE] Erase failed: %d\n", err);
        return err;
    }

    memset(&s_meta, 0, sizeof(s_meta));
    s_written = 0;
    os_printf("[FW_STORE] Partition erased\n");
    return ESP_OK;
}

esp_err_t fs_write(size_t offset, const uint8_t *data, size_t len) {
    if (s_partition == NULL) return ESP_ERR_INVALID_STATE;

    size_t part_off = FS_DATA_OFFSET + offset;
    if (part_off + len > s_partition->size) {
        os_printf("[FW_STORE] Write exceeds partition boundary\n");
        return ESP_ERR_INVALID_SIZE;
    }

    esp_err_t err = esp_partition_write(s_partition, part_off, data, len);
    if (err == ESP_OK) {
        if (offset + len > s_written) {
            s_written = offset + len;
        }
    }
    return err;
}

esp_err_t fs_read(size_t offset, uint8_t *buf, size_t len) {
    if (s_partition == NULL) return ESP_ERR_INVALID_STATE;

    size_t part_off = FS_DATA_OFFSET + offset;
    if (part_off + len > s_partition->size) return ESP_ERR_INVALID_SIZE;

    return esp_partition_read(s_partition, part_off, buf, len);
}

size_t fs_get_stored_size(void) {
    if (s_meta.magic != FW_META_MAGIC) return 0;
    return s_meta.firmware_size;
}

uint32_t fs_get_checksum(void) {
    if (s_meta.magic != FW_META_MAGIC) return 0;
    return s_meta.crc32;
}

esp_err_t fs_finalize(size_t total_size, uint32_t crc) {
    if (s_partition == NULL) return ESP_ERR_INVALID_STATE;

    if (total_size > (s_partition->size - FS_DATA_OFFSET)) {
        os_printf("[FW_STORE] Firmware too large: %lu > %lu\n",
                  (unsigned long)total_size,
                  (unsigned long)(s_partition->size - FS_DATA_OFFSET));
        return ESP_ERR_INVALID_SIZE;
    }

    s_meta.magic         = FW_META_MAGIC;
    s_meta.version       = FW_META_VERSION;
    s_meta.firmware_size = total_size;
    s_meta.crc32         = crc;
    s_meta.target_family = 0;
    s_meta.flash_address = 0;
    s_meta.timestamp     = 0;
    memset(s_meta.reserved, 0, sizeof(s_meta.reserved));

    /*
     * Erase ONLY sector 0 (the metadata sector).
     * Firmware payload starts at FS_DATA_OFFSET (4096) —
     * it lives in separate sectors and is untouched.
     */
    esp_err_t err = esp_partition_erase_range(s_partition, 0, FS_DATA_OFFSET);
    if (err != ESP_OK) {
        os_printf("[FW_STORE] Metadata sector erase failed: %d\n", err);
        return err;
    }

    err = esp_partition_write(s_partition, 0, &s_meta, sizeof(s_meta));
    if (err == ESP_OK) {
        s_written = total_size;
        os_printf("[FW_STORE] Finalized: %lu bytes, CRC32 0x%08lX\n",
                  (unsigned long)total_size, (unsigned long)crc);
    }
    return err;
}

size_t fs_get_capacity(void) {
    if (s_partition == NULL) return 0;
    return s_partition->size - FS_DATA_OFFSET;
}

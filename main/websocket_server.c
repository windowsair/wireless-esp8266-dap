/**
 * Copyright (c) 2021 windowsair <msdn_02 at sina.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdint.h>
#include <string.h>
#include <sys/param.h>

// share header file
#include "components/corsacOTA/src/corsacOTA.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mbedtls/base64.h"
#include "mbedtls/sha1.h"

#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "sdkconfig.h"

static const char *CO_TAG = "corsacOTA";

#define CONFIG_CO_SOCKET_BUFFER_SIZE  1500
#define CONFIG_CO_WS_TEXT_BUFFER_SIZE 100

#define LOG_FMT(x)                    "%s: " x, __func__

#define min(a, b)                     ((a) < (b) ? (a) : (b))

#define CO_NO_RETURN                  __attribute__((noreturn))
#define CO_INLINE                     __attribute__((always_inline))

#define CO_TEST_MODE                  0

#if (CO_TEST_MODE == 1)
#warning corsacOTA test mode is in use
#endif

extern void free_dap_ringbuf();
extern uint32_t DAP_ExecuteCommand(const uint8_t *request, uint8_t *response);

static void co_websocket_process_dap(uint8_t *data, size_t len);

uint8_t* ws_process_buffer = NULL;

/**
 * @brief corsacOTA websocket control block
 *
 */
typedef struct co_websocket_cb {
    uint8_t FIN;
    uint8_t OPCODE;

    uint8_t MASK;
    size_t payload_len; // it is used not only for the 7-bit payload len,
                        // but also for the total length of the payload after the extended payload length is included.

    size_t payload_read_len; // the number of payload bytes already read

    union {
        uint32_t val;
        uint8_t data[4];
    } mask;

    bool skip_frame; // skip too long text frames
} co_websocket_cb_t;

/**
 * @brief corsacOTA socket control block
 *
 */
typedef struct co_socket_cb {
    int fd; // The file descriptor for this socket
    enum co_socket_status {
        CO_SOCKET_ACCEPT = 0,
        CO_SOCKET_HANDSHAKE,               // not handshake, or in progress
        CO_SOCKET_WEBSOCKET_HEADER,        // already handshake, now reading the header of websocket frame
        CO_SOCKET_WEBSOCKET_EXTEND_LENGTH, // reading the extended length of websocket header
        CO_SOCKET_WEBSOCKET_MASK,          // reading the mask part of websocket header
        CO_SOCKET_WEBSOCKET_PAYLOAD,       // reading the payload of websocket frame
        CO_SOCKET_CLOSING                  // waiting to close
    } status;

    char *buf;            // data from raw socket
    size_t remaining_len; // the number of available bytes remaining in buf
    size_t read_len;      // the number of bytes that have been processed

    co_websocket_cb_t wcb; // websocket control block

} co_socket_cb_t;

/**
 * @brief corsacOTA OTA control block
 *
 */
typedef struct co_ota_cb {
    enum co_ota_status {
        CO_OTA_INIT = 0,
        CO_OTA_LOAD,
        CO_OTA_DONE,
        CO_OTA_STOP,
        CO_OTA_ERROR,
        CO_OTA_FATAL_ERROR,
    } status;
    int32_t error_code; //// TODO: ?

    const esp_partition_t *update_ptn;
    const esp_partition_t *running_ptn;
    esp_ota_handle_t update_handle;

    int32_t total_size;        // Total firmware size
    int32_t offset;            // Current processed size
    int32_t chunk_size;        // The response will be made every time the chunk size is reached
    int32_t last_index_offset; // The offset recorded in the last response

} co_ota_cb_t;

/**
 * @brief corsacOTA http control block
 *
 */
typedef struct co_cb {
    int listen_fd;        // server listener FD
    int websocket_fd;     // only one websocket is allowed.
    uint8_t *recv_data;   // recv buffer at websocket stage (text mode)
    int recv_data_offset; // (text mode)
    int max_listen_num;   // maxium number of connections. In fact, after the handshake is complete, there is only one connection to provide services

    int wait_timeout_sec;  // timeout (in seconds)
    int wait_timeout_usec; // timeout (in microseconds)

    co_socket_cb_t **socket_list; // socket control block list
    co_socket_cb_t *websocket;    // the only valid socket in the list

    int accept_num; // current number of established connections

    int closing_num; // current number of closing socket

    co_ota_cb_t ota; // ota control block

} co_cb_t;

static co_cb_t *global_cb = NULL;

/*  RFC 6455: The WebSocket Protocol

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-------+-+-------------+-------------------------------+
    |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
    |N|V|V|V|       |S|             |   (if payload len==126/127)   |
    | |1|2|3|       |K|             |                               |
    +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    |     Extended payload length continued, if payload len == 127  |
    + - - - - - - - - - - - - - - - +-------------------------------+
    |                               |Masking-key, if MASK set to 1  |
    +-------------------------------+-------------------------------+
    | Masking-key (continued)       |          Payload Data         |
    +-------------------------------- - - - - - - - - - - - - - - - +
    :                     Payload Data continued ...                :
    + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
    |                     Payload Data continued ...                |
    +---------------------------------------------------------------+
*/

#define WS_FIN                 0x80
#define WS_RSV1                0x40
#define WS_RSV2                0x20
#define WS_RSV3                0x10
#define WS_OPCODE_CONTINUTAION 0x00
#define WS_OPCODE_TEXT         0x01
#define WS_OPCODE_BINARY       0x02
#define WS_OPCODE_CLOSE        0x08
#define WS_OPCODE_PING         0x09
#define WS_OPCODE_PONG         0x0A

#define WS_MASK                0x80

static inline int co_websocket_get_res_payload_offset(int payload_len) {
    //  promise: payload_len <= 65535
    return 2 + (payload_len >= 126 ? 2 : 0);
}

static co_err_t co_websocket_process_header(co_cb_t *cb, co_socket_cb_t *scb) {
    uint8_t opcode, fin, mask;
    uint64_t payload_len;
    uint8_t *data;

    data = (uint8_t *)scb->buf;

    if (scb->status == CO_SOCKET_WEBSOCKET_HEADER) {
        if (scb->remaining_len < 2) {
            return CO_OK;
        }

        // check RSV
        if (data[0] & 0b1110000) {
            return CO_FAIL; // no extension defining RSV
        }

        // first byte
        fin = (data[0] & WS_FIN) == WS_FIN;
        opcode = data[0] & 0b1111;
        // second byte
        mask = (data[1] & WS_MASK) == WS_MASK;
        payload_len = data[1] & 0x7F;

        switch (opcode) {
        case WS_OPCODE_CONTINUTAION:
            // nothing to do
            break;
        case WS_OPCODE_TEXT:
        case WS_OPCODE_BINARY:
            scb->wcb.OPCODE = opcode;
            break;
        case WS_OPCODE_PING:
        case WS_OPCODE_PONG:
            scb->wcb.OPCODE = opcode;
            break;
        case WS_OPCODE_CLOSE:
            scb->wcb.OPCODE = opcode;
            break;
        default:
            return CO_FAIL;
            break;
        }

        scb->wcb.FIN = fin;
        scb->wcb.MASK = mask;
        scb->wcb.payload_len = payload_len;

        // extended payload length should be read
        if (payload_len == 126 || payload_len == 127) {
            scb->status = CO_SOCKET_WEBSOCKET_EXTEND_LENGTH;
        } else if (mask == 1) {
            scb->status = CO_SOCKET_WEBSOCKET_MASK;
        }

        scb->read_len = 2; // first 2 byte header
    }

    if (scb->status == CO_SOCKET_WEBSOCKET_EXTEND_LENGTH) {
        if (scb->wcb.payload_len == 126) {
            if (scb->remaining_len < scb->read_len + 2) { // 2 byte extended length
                return CO_OK;
            }

            payload_len = data[2] << 8 | data[3]; // 0 + scb->read_len == 2

            scb->read_len += 2;
        } else if (scb->wcb.payload_len == 127) { // 8 byte extended length
            if (scb->remaining_len < scb->read_len + 8) {
                return CO_OK;
            }

            payload_len = ((uint64_t)(data[9]) << 0);
            payload_len |= ((uint64_t)(data[8]) << 8);
            payload_len |= ((uint64_t)(data[7]) << 16);
            payload_len |= ((uint64_t)(data[6]) << 24);
            payload_len |= ((uint64_t)(data[5]) << 32);
            payload_len |= ((uint64_t)(data[4]) << 40);
            payload_len |= ((uint64_t)(data[3]) << 48);
            payload_len |= ((uint64_t)(data[2]) << 56);

            // most significant bit MUST be 0
            if (((payload_len >> 63) & 0b1) == 0x1) {
                ESP_LOGE(CO_TAG, "wrong payload length");
                return CO_FAIL;
            }

            scb->read_len += 8;
        } else {
            payload_len = scb->wcb.payload_len;
        }

        scb->wcb.payload_len = payload_len;

        scb->status = scb->wcb.MASK == 1 ? CO_SOCKET_WEBSOCKET_MASK : CO_SOCKET_WEBSOCKET_PAYLOAD;
    }

    if (scb->status == CO_SOCKET_WEBSOCKET_MASK) {
        if (scb->remaining_len < scb->read_len + 4) { // 4 byte mask
            return CO_OK;
        }

        memcpy(&scb->wcb.mask.data[0], &data[scb->read_len], 4);
        scb->read_len += 4;
        scb->status = CO_SOCKET_WEBSOCKET_PAYLOAD;
    } else {
        scb->status = CO_SOCKET_WEBSOCKET_PAYLOAD;
    }

    return CO_OK;
}

// We promise that the length of the payload should not exceed 65535
static co_err_t co_websocket_send_frame(void *frame_buffer, size_t payload_len, int frame_type) {
    int sz;
    uint16_t payload_length;
    uint8_t *p;

    payload_length = payload_len;
    sz = co_websocket_get_res_payload_offset(payload_len) + payload_len;

    p = frame_buffer;
    // 2 bytes
    *p++ = WS_FIN | frame_type; // frame_type
    *p++ = (payload_length >= 126 ? 126 : payload_length);

    // extended length
    if (payload_length >= 126) {
        payload_length = htons(payload_length);
        memcpy(p, &payload_length, 2);
        p += 2;
    }

    // no mask

    send(global_cb->websocket->fd, frame_buffer, sz, 0);

    return CO_OK;
}

// Create a new frame buffer, construct text and send frame.
static co_err_t co_websocket_send_msg_with_code(int code, const char *msg) {
    char *buffer;
    int len, ret;
    int offset;

    len = strlen(msg);
    offset = co_websocket_get_res_payload_offset(len);
    buffer = malloc(offset + len + 25);
    if (buffer == NULL) {
        ret = CO_ERROR_NO_MEM;
        goto cleanup;
    }

    if (code == CO_RES_SUCCESS) {
        ret = snprintf(buffer + offset, len + 24, "code=%d&data=\"%s\"", code, msg);
    } else {
        ret = snprintf(buffer + offset, len + 24, "code=%d&data=\"msg=%s\"", code, msg);
    }

    if (ret < 0) {
        ESP_LOGE(CO_TAG, "invalid arg");
        ret = CO_ERROR_INVALID_ARG;
        goto cleanup;
    }

    ret = co_websocket_send_frame(buffer, ret, WS_OPCODE_TEXT);

cleanup:
    free(buffer);
    return ret;
}

#if (CO_TEST_MODE == 1)
// use for test
static co_err_t co_websocket_send_echo(void *data, size_t len, int frame_type) {
    char *buffer;
    int ret;
    int offset;

    offset = co_websocket_get_res_payload_offset(len);
    buffer = malloc(offset + len);
    if (buffer == NULL) {
        ret = CO_ERROR_NO_MEM;
        goto cleanup;
    }
    memcpy(buffer + offset, data, len);

    ret = co_websocket_send_frame(buffer, len, frame_type);

cleanup:
    free(buffer);
    return ret;
}
#endif // (CO_TEST_MODE == 1)

static void co_websocket_process_binary(uint8_t *data, size_t len) {
    co_websocket_process_dap(data, len);
}

static void co_websocket_process_text(uint8_t *data, size_t len) {
}

// send pong response
// TODO: too long ping frame
static void co_websocket_process_ping(co_cb_t *cb, co_socket_cb_t *scb) {
    int len;

    // control frame max payload length: 125 -> 0 byte extended length
    len = 2 + scb->wcb.payload_len + (scb->wcb.MASK ? 4 : 0);

    scb->buf[0] = WS_FIN | WS_OPCODE_PONG;

    send(scb->fd, scb->buf, len, 0);
}

// close handshake
// TODO: array
static void co_websocket_process_close(co_cb_t *cb, co_socket_cb_t *scb) {
    uint8_t buf[4];
    uint8_t *p = buf;

    *p++ = WS_FIN | WS_OPCODE_CLOSE;
    *p++ = 0x02; // 2 byte status code
    // normal closure
    *p++ = 0x03;
    *p = 0xe8;

    send(scb->fd, buf, 4, 0);
}

static inline CO_INLINE uint32_t co_rotr32(uint32_t n, unsigned int c) {
    const unsigned int mask = (CHAR_BIT * sizeof(n) - 1);
    c &= mask;
    return (n >> c) | (n << ((-c) & mask));
}

/**
 * @brief Quick calculation WebSocket. The process of calculating the mask is one of the performance bottlenecks
 * of the entire websocket. The performance between the optimized version and the version without mask is not significant.
 *
 * We assume: the natural machine word length is 4byte (32bits) and the endianness is little-endian
 * For xtensa: single fetch: 4 byte(32bit)
 *
 * @param data data buffer ptr
 * @param mask websocket mask. Little-endian 32bis mask.
 * @param len data length
 */
static void co_websocket_fast_mask(uint8_t *data, uint32_t mask, size_t len) {
    uint32_t new_mask;
    int align_len;
    size_t i;

    const uint8_t *p_mask = (uint8_t *)&mask;

    unsigned long int dst = (long int)data;

    if (len >= 8) {
        // copy just a few bytes to make dst aligned.
        align_len = (-dst) % 4;
        len -= align_len;

        for (i = 0; i < align_len; i++) {
            data[i] ^= p_mask[i];
        }

        // use the new mask on the aligned address
        switch (align_len) {
        case 1:
            new_mask = co_rotr32(mask, 8U);
            break;
        case 2:
            new_mask = co_rotr32(mask, 16U);
            break;
        case 3:
            new_mask = co_rotr32(mask, 24U);
            break;
        default: // 0
            new_mask = mask;
            break;
        }

        p_mask = (uint8_t *)&new_mask;

        dst += align_len;

        for (i = 0; i < len / 4; i++) {
            *((uint32_t *)dst) ^= new_mask;
            dst += 4;
        }

        len %= 4;
    }

    // There are just a few bytes to process
    for (i = 0; i < len; i++) {
        *((uint8_t *)dst) ^= p_mask[i % 4];
        dst += 1;
    }
}

static inline uint32_t co_websocket_get_new_mask(uint32_t mask, size_t len) {
    switch (len & 0b11) {
    case 1:
        return co_rotr32(mask, 8U);
    case 2:
        return co_rotr32(mask, 16U);
    case 3:
        return co_rotr32(mask, 24U);
    default:
        return mask;
    }
}

/**
 * @brief Process websocket payload
 *
 * @param cb corsacOTA control block
 * @param scb corsacOTA socket control block
 * @return co_err_t
 * - CO_OK: Successful processing of all payloads
 * - CO_ERROR_IO_PENDING: There are still new frames to be processed
 */
static co_err_t co_websocket_process_payload(co_cb_t *cb, co_socket_cb_t *scb) {
    int len, new_len;
    uint8_t *data;
    uint32_t mask;

    data = (uint8_t *)scb->buf + scb->read_len;
    // May be possible to read the complete frame and maybe a new frame rate afterwards
    len = min(scb->remaining_len - scb->read_len, scb->wcb.payload_len);

    // For ping frames, we will directly change their opcode and send.
    if (scb->wcb.MASK == 1 && scb->wcb.OPCODE != WS_OPCODE_PING) {
        mask = scb->wcb.mask.val;
        co_websocket_fast_mask(data, mask, len);

        scb->wcb.mask.val = co_websocket_get_new_mask(mask, len);
    }

    // In the previous processing, we can ensure that each new frame can begin in a place where the Buffer offset is 0.
    switch (scb->wcb.OPCODE) {
    case WS_OPCODE_TEXT:
#if (CO_TEST_MODE == 1)
        co_websocket_send_echo(data, len, WS_OPCODE_TEXT);
        break;
#endif
        // case 0: This frame should be skip
        if (scb->wcb.skip_frame) {
            if (len == scb->wcb.payload_len) { // The last part of the data in this frame has been received
                scb->wcb.skip_frame = false;
            }
            break;
        }

        // case 1: Receive the entire payload
        if (len == scb->wcb.payload_len && cb->recv_data_offset == 0) {
            co_websocket_process_text(data, len);
            break;
        }

        // case 2: Part of the payload has been received before
        if (len > CONFIG_CO_WS_TEXT_BUFFER_SIZE - cb->recv_data_offset) { // overflow
            if (len < scb->wcb.payload_len) {                             // This frame has not yet been received
                scb->wcb.skip_frame = true;
            }

            co_websocket_send_msg_with_code(CO_RES_INVALID_SIZE, "request too long");
            cb->recv_data_offset = 0;
            break;
        }

        memcpy(cb->recv_data + cb->recv_data_offset, data, len);
        cb->recv_data_offset += len;

        if (len == scb->wcb.payload_len) {
            co_websocket_process_text(cb->recv_data, len);
            cb->recv_data_offset = 0;
        }
        break;
    case WS_OPCODE_BINARY:
#if (CO_TEST_MODE == 1)
        co_websocket_send_echo(data, len, WS_OPCODE_BINARY);
        break;
#endif
        //// TODO: check return val
        co_websocket_process_binary(data, len);
        break;
    case WS_OPCODE_PING:
        co_websocket_process_ping(cb, scb);
        break;
    case WS_OPCODE_PONG:
        break;
    case WS_OPCODE_CLOSE:
        co_websocket_process_close(cb, scb);
        return CO_FAIL; // close by server
    default:
        ESP_LOGE(CO_TAG, "unknow opcode: %d", scb->wcb.OPCODE);
        break;
    }

    new_len = scb->remaining_len - scb->read_len - len;
    // case 0: New frames still exist
    if (new_len > 0) {
        // For simplicity, we make sure that the websocket header is always at the beginning of the buf.
        memcpy(scb->buf, data + len, new_len);

        scb->read_len = 0;
        scb->remaining_len = new_len;

        scb->status = CO_SOCKET_WEBSOCKET_HEADER;
        scb->wcb.payload_len = 0;
        scb->wcb.payload_read_len = 0;
        return CO_ERROR_IO_PENDING;
    }
    // case 1: The payload part is not yet fully read.
    else if (scb->wcb.payload_len > len) {
        scb->wcb.payload_len -= len;

        scb->read_len = 0;
        scb->remaining_len = 0;

        return CO_OK;
    }
    // case 2: Exactly one complete frame is read and there is no remaining available data in buf.
    else {
        scb->read_len = 0;
        scb->remaining_len = 0;

        scb->status = CO_SOCKET_WEBSOCKET_HEADER;
        scb->wcb.payload_len = 0;
        scb->wcb.payload_read_len = 0;
        return CO_OK;
    }
}

static esp_err_t co_websocket_process(co_cb_t *cb, co_socket_cb_t *scb) {
    if (cb->websocket != scb) {
        return ESP_FAIL;
    }

    int fd, ret, offset;

    fd = scb->fd;
    offset = scb->remaining_len;

    ret = recv(fd, scb->buf + offset, CONFIG_CO_SOCKET_BUFFER_SIZE - offset, 0);
    if (ret <= 0) {
        return ESP_FAIL;
    }
    scb->remaining_len += ret;

    do {
        // After we process a partial or complete payload,
        // we always receive a new payload or header starting from the head of the buf.
        if (scb->status != CO_SOCKET_WEBSOCKET_PAYLOAD) {
            ret = co_websocket_process_header(cb, scb);
            if (ret != CO_OK) {
                return CO_FAIL;
            }
        }

        // Perhaps we have already read the header section, and if there are extra bytes left over,
        // we continue reading the payload section.
        if (scb->status == CO_SOCKET_WEBSOCKET_PAYLOAD) {
            ret = co_websocket_process_payload(cb, scb);
            if (ret == CO_FAIL) {
                return CO_FAIL;
            }
        }
    } while (ret == CO_ERROR_IO_PENDING);

    return ESP_OK;
}

/**
 * @brief parse HTTP header lines of format:
 *        \r\nfield_name: value1, value2, ... \r\n
 *
 * @param header_start
 * @param header_end
 * @param field_name
 * @param value Optional value
 * @return const char* The specific value starts, or the beginning of value in field.
 */
static const char *co_http_header_find_field_value(const char *header_start, const char *header_end, const char *field_name, const char *value) {
    const char *field_start, *field_end, *next_crlf, *value_start;
    int field_name_len;

    field_name_len = (int)strlen(field_name);

    field_start = header_start;
    do {
        field_start = strcasestr(field_start + 1, field_name);
        field_end = field_start + field_name_len - 1;

        if (field_start != NULL && field_start - header_start >= 2 && field_start[-2] == '\r' && field_start[-1] == '\n') { // is start with "\r\n" ?
            if (header_end - field_end >= 1 && field_end[1] == ':') {                                                       // is end with ':' ?
                break;
            }
        }
    } while (field_start != NULL);

    if (field_start == NULL) {
        return NULL;
    }

    // find the field terminator
    next_crlf = strcasestr(field_start, "\r\n");
    if (next_crlf == NULL) {
        return NULL; // Malformed HTTP header!
    }

    // If not looking for a value, then return a pointer to the start of values string
    if (value == NULL) {
        return field_end + 2; // 2 for ':'  ' '(blank)
    }

    value_start = strcasestr(field_start, value);
    if (value_start == NULL) {
        return NULL;
    }

    if (value_start > next_crlf) {
        return NULL; // encounter with unanticipated CRLF
    }

    // The value we found should be properly delineated from the other tokens
    if (isalnum((unsigned char)value_start[-1]) || isalnum((unsigned char)value_start[strlen(value)])) {
        // "field_name: value1, value2,"
        // Consecutive tokens will be considered as errors.
        return NULL;
    }

    return value_start;
}

static void co_http_error_400_response(co_cb_t *cb, co_socket_cb_t *scb) {
    const char *error = "HTTP/1.1 400 Bad Request\r\n\r\n";
    send(scb->fd, error, strlen(error), 0);
}

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

static int co_sha1(const unsigned char *src, size_t src_len, unsigned char *dst) {
    return mbedtls_sha1_ret(src, src_len, dst);
}

static int co_base64_encode(unsigned char *dst, size_t dst_len, size_t *written_len, unsigned char *src, size_t src_len) {
    return mbedtls_base64_encode(dst, dst_len, written_len, src, src_len);
}

static esp_err_t co_websocket_create_accept_key(char *dst, size_t dst_len, const char *client_key) {
    uint8_t sha1buf[20], key_src[60];

    memcpy(key_src, client_key, 24);
    memcpy(key_src + 24, WS_GUID, 36);

    if (co_sha1(key_src, sizeof(key_src), sha1buf) != 0) {
        return ESP_FAIL;
    }

    size_t base64_encode_len;
    if (co_base64_encode((unsigned char *)dst, dst_len, &base64_encode_len, sha1buf, sizeof(sha1buf)) != 0) {
        return ESP_FAIL;
    }

    // add terminator
    dst[base64_encode_len] = '\0';

    return ESP_OK;
}

static esp_err_t co_websocket_handshake_send_key(int fd, const char *client_key) {
    char res_header[256], accept_key[29];
    int res_header_length;

    if (co_websocket_create_accept_key(accept_key, sizeof(accept_key), client_key) != ESP_OK) {
        ESP_LOGE(CO_TAG, LOG_FMT("fail to create accept key"));
        return ESP_FAIL;
    }

    snprintf(res_header, sizeof(res_header),
             "HTTP/1.1 101 Switching Protocols\r\n"
             "Server: corsacOTA server\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Accept: %s\r\n"
             "\r\n",
             accept_key);

    res_header_length = strlen(res_header);
    send(fd, res_header, res_header_length, 0);

    return ESP_OK;
}

static esp_err_t co_websocket_handshake_process(co_cb_t *cb, co_socket_cb_t *scb) {
    if (scb->remaining_len == 0) {
        memset(scb->buf, 0, CONFIG_CO_SOCKET_BUFFER_SIZE);
    }

    int offset = scb->remaining_len;
    int fd = scb->fd;

    int ret = recv(fd, scb->buf + offset, CONFIG_CO_SOCKET_BUFFER_SIZE - offset, 0);
    if (ret <= 0) {
        co_http_error_400_response(cb, scb);
        return ESP_FAIL;
    }

    scb->remaining_len += ret;

    // Already received the entire http header?
    if (scb->remaining_len < 4 || memcmp(scb->buf + scb->remaining_len - 4, "\r\n\r\n", 4) != 0) {
        return ESP_OK; // Not yet received
    }

    const char *header_start = scb->buf, *header_end = scb->buf + scb->remaining_len - 1;
    const char *ws_key_start, *ws_key_end;

    if (co_http_header_find_field_value(header_start, header_end, "Upgrade", "websocket") == NULL ||
        co_http_header_find_field_value(header_start, header_end, "Connection", "Upgrade") == NULL ||
        (ws_key_start = co_http_header_find_field_value(header_start, header_end, "Sec-WebSocket-Key", NULL)) == NULL) {
        co_http_error_400_response(cb, scb);
        return ESP_FAIL;
    }

    /* example:
     Sec-WebSocket-Key: c2REMVVpRXJRQWJ0Q1dKeQ==\r\n
                       |
                    ws_key_start
    */

    // skip the extra blank
    for (; *ws_key_start == ' '; ws_key_start++) {
        ;
    }

    // find the end of ws key
    for (ws_key_end = ws_key_start; *ws_key_end != '\r' && *ws_key_end != ' '; ws_key_end++) {
        ;
    }

    /* example:
     Sec-WebSocket-Key: c2REMVVpRXJRQWJ0Q1dKeQ==\r\n
                        |                       ||
                    ws_key_start            ws_key_end
    */
    if (ws_key_end - ws_key_start != 24) {
        co_http_error_400_response(cb, scb);
        return ESP_FAIL;
    }

    if (co_websocket_handshake_send_key(scb->fd, ws_key_start) != ESP_OK) {
        co_http_error_400_response(cb, scb);
        return ESP_FAIL;
    }

    ESP_LOGD(CO_TAG, "websocket handshake success");

    cb->websocket = scb;
    scb->status = CO_SOCKET_WEBSOCKET_HEADER;
    scb->remaining_len = 0;

    memset(scb->buf, 0, CONFIG_CO_SOCKET_BUFFER_SIZE);

    return ESP_OK;
}

static void websocket_buffer_malloc() {
    if (ws_process_buffer != NULL)
        return;

    free_dap_ringbuf();
    ws_process_buffer = malloc(1200);
}

static void websocket_buffer_free() {
    if (ws_process_buffer != NULL) {
        free(ws_process_buffer);
        ws_process_buffer = NULL;
    }
}

static void co_websocket_process_dap(uint8_t *data, size_t len) {
    uint8_t *buf;
    int max_offset, res, offset;

    max_offset = co_websocket_get_res_payload_offset(1500);
    buf = ws_process_buffer + max_offset;

    res = DAP_ExecuteCommand(data, buf);
    res &= 0xFFFF;

    offset = co_websocket_get_res_payload_offset(res);
    buf -= offset;

    co_websocket_send_frame(buf, res, WS_OPCODE_BINARY);
}

int websocket_worker(int fd, uint8_t *base, uint32_t length) {
    co_cb_t cb;
    co_socket_cb_t scb;
    esp_err_t ret;

    memset(&cb, 0, sizeof(co_cb_t));
    memset(&scb, 0, sizeof(co_socket_cb_t));

    cb.recv_data = NULL; // used in websocket text mode
    cb.websocket = &scb;

    scb.fd = fd;
    scb.status = CO_SOCKET_HANDSHAKE;
    scb.buf = (char *)base;
    scb.remaining_len = 4; // already read 4 byte

    global_cb = &cb;

    // handshake
    do {
        ret = co_websocket_handshake_process(&cb, &scb);
        if (ret != ESP_OK)
            return ret;
    } while (scb.status == CO_SOCKET_HANDSHAKE);

    websocket_buffer_malloc();

    // websocket data process
    do {
        ret = co_websocket_process(&cb, &scb);
        if (ret != ESP_OK)
            goto out;
    } while (1);


out:
    websocket_buffer_free();
    return 0;
}

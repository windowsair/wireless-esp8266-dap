#ifndef __ELAPHURELINK_PROTOCOL_H__
#define __ELAPHURELINK_PROTOCOL_H__

#include <stdint.h>
#include <stddef.h>

#define EL_LINK_IDENTIFIER 0x8a656c70

#define EL_DAP_VERSION 0x10000 // v1.0.0 support vendor command

#define EL_COMMAND_HANDSHAKE 0x00000000

#define EL_VENDOR_COMMAND_PERFIX        0x88
#define EL_NATIVE_COMMAND_PASSTHROUGH   0x1
#define EL_VENDOR_SCOPE_ENTER           0x2
#define EL_VENDOR_SCOPE_EXIT            0x3

typedef struct
{
    uint32_t el_link_identifier;
    uint32_t command;
    uint32_t el_proxy_version
} __attribute__((packed)) el_request_handshake;


typedef struct
{
    uint32_t el_link_identifier;
    uint32_t command;
    uint32_t el_dap_version
} __attribute__((packed)) el_response_handshake;


/**
 * @brief elahpureLink Proxy handshake phase process
 *
 * @param fd socket fd
 * @param buffer packet buffer
 * @param len packet length
 * @return 0 on Success, other on failed.
 */
int el_handshake_process(int fd, void* buffer, size_t len);


/**
 * @brief Process dap data and send to socket
 *
 * @param buffer dap data buffer
 * @param len dap data length
 */
void el_dap_data_process(void* buffer, size_t len);


uint32_t el_vendor_command(const uint8_t *request, uint8_t *response);

int el_dap_work(uint8_t* base, size_t len);

void el_process_buffer_malloc();
void el_process_buffer_free();

#endif
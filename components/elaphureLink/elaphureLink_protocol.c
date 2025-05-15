#include "components/elaphureLink/elaphureLink_protocol.h"

#include "main/DAP_handle.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

extern int kRestartDAPHandle;
extern int kSock;
extern int usbip_network_send(int s, const void *dataptr, size_t size, int flags);

extern void malloc_dap_ringbuf();
extern void free_dap_ringbuf();

extern uint32_t DAP_ExecuteCommand(const uint8_t *request, uint8_t *response);

struct el_context {
    bool is_async;
};

static struct el_context k_el_context;
uint8_t* el_process_buffer = NULL;

void el_process_buffer_malloc() {
    if (el_process_buffer != NULL)
        return;

    free_dap_ringbuf();

    el_process_buffer = malloc(1500);
}

void el_process_buffer_free() {
    if (el_process_buffer != NULL) {
        free(el_process_buffer);
        el_process_buffer = NULL;
    }
}

int el_handshake_process(int fd, void *buffer, size_t len) {
    if (len != sizeof(el_request_handshake)) {
        return -1;
    }

    el_request_handshake* req = (el_request_handshake*)buffer;

    if (ntohl(req->el_link_identifier) != EL_LINK_IDENTIFIER) {
        return -1;
    }

    if (ntohl(req->command) != EL_COMMAND_HANDSHAKE) {
        return -1;
    }

    el_response_handshake res;
    res.el_link_identifier = htonl(EL_LINK_IDENTIFIER);
    res.command = htonl(EL_COMMAND_HANDSHAKE);
    res.el_dap_version = htonl(EL_DAP_VERSION);

    usbip_network_send(fd, &res, sizeof(el_response_handshake), 0);

    return 0;
}

void el_dap_data_process(void* buffer, size_t len) {
    int res = DAP_ExecuteCommand(buffer, (uint8_t *)el_process_buffer);
    res &= 0xFFFF;

    usbip_network_send(kSock, el_process_buffer, res, 0);
}

static inline int recv_all(int fd, uint8_t *buf, size_t size, int flag)
{
    const size_t total = size;
    int ret;

    if (size == 0)
        return 0;

    do {
        ret = recv(fd, buf, size, flag);
        if (ret <= 0)
            return ret;

        buf += ret;
        size -= ret;
    } while (size);

    return total;
}

static int el_vendor_command_pre_process(uint8_t *base, int recved_len)
{
    int offset = 0;
    int payload_len, remain_len, packet_len;
    uint16_t *payload;
    int ret;

    while (recved_len - offset >= 4) {
        payload = (uint16_t *)(base + offset + 2);
        payload_len = ntohs(*payload);
        packet_len = 4 + payload_len;

        if (offset + packet_len > recved_len)
            break;

        el_dap_data_process(base + offset, packet_len);
        offset += packet_len;
    }

    // already process done
    remain_len = recved_len - offset;
    if (remain_len == 0)
        return 1;

    memmove(base, base + offset, remain_len);
    if (remain_len < 4) {
        ret = recv(kSock, base + remain_len, 4 - remain_len, 0);
        if (ret <= 0)
            return ret;
        offset = 4;
        remain_len = 0;
    } else {
        offset = remain_len;
        remain_len -= 4;
    }

    payload = (uint16_t *)(base + 2);
    payload_len = ntohs(*payload);
    if (payload_len - remain_len > 0) {
        ret = recv(kSock, base + offset, payload_len - remain_len, 0);
        if (ret <= 0)
            return ret;
    }

    el_dap_data_process(base, 4 + payload_len);

    return 1;
}

int el_dap_work(uint8_t* base, size_t len)
{
    uint16_t *length, payload_len;
    uint8_t *data;
    int sz, ret;

    memset(&k_el_context, 0, sizeof(struct el_context));

    // read command code and protocol version
    data = base + 4;
    sz = 8;
    do {
        ret = recv(kSock, data, sz, 0);
        if (ret <= 0)
            return ret;
        sz -= ret;
        data += ret;
    } while (sz > 0);

    ret = el_handshake_process(kSock, base, 12);
    if (ret)
        return ret;

    kRestartDAPHandle = DELETE_HANDLE;
    el_process_buffer_malloc();
    // data process
    while(1) {
        ret = recv(kSock, base, len, 0);
        if (ret <= 0)
            return ret;

        if (*base == EL_VENDOR_COMMAND_PERFIX) {
            ret = el_vendor_command_pre_process(base, ret);
            if (ret <= 0)
                return ret;
        } else {
            el_dap_data_process(base, ret);
        }

        if (k_el_context.is_async) {
            do {
                ret = recv_all(kSock, base, 4, 0);
                if (ret <= 0)
                    return ret;
                length = (uint16_t *)(base + 2);
                payload_len = ntohs(*length);

                ret = recv_all(kSock, base + 4, payload_len, 0);
                if (ret <= 0)
                    return ret;
                el_dap_data_process(base, 4 + ntohs(*length));
            } while (k_el_context.is_async);
        }
    }

    return 0;
}

uint32_t el_native_command_passthrough(const uint8_t *request, uint8_t *response)
{
    int ret;

    request += 2; // skip header (length field)

    ret = DAP_ExecuteCommand(request, response + 3);
    ret &= 0xFFFF;

    response[0] = 0x00; // status
    response[1] = (ret >> 8) & 0xFF;
    response[2] = ret & 0xFF;

    return ret + 4; // header + payload
}

uint32_t el_vendor_command(const uint8_t *request, uint8_t *response)
{
    uint8_t type;
    uint32_t ret = 0;

    type = *request++;

    switch (type) {
    case EL_NATIVE_COMMAND_PASSTHROUGH:
        ret = el_native_command_passthrough(request, response);
        break;
    case EL_VENDOR_SCOPE_ENTER:
        memset(&k_el_context, 0, sizeof(struct el_context));
        k_el_context.is_async = true;
        *response++ = 0; // status
        *response++ = 0;
        *response++ = 0;
        ret = 4;
        break;
    case EL_VENDOR_SCOPE_EXIT:
        k_el_context.is_async = false;
        *response++ = 0; // status
        *response++ = 0;
        *response++ = 0;
        ret = 4;
        break;
    default:
        break;
    }

    return ret;
}


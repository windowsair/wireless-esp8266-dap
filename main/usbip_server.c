#include <stdint.h>
#include <string.h>

#include "main/usbip_server.h"
#include "main/kcp_server.h"
#include "main/tcp_netconn.h"
#include "main/DAP_handle.h"
#include "main/wifi_configuration.h"

#include "components/USBIP/usb_handle.h"
#include "components/USBIP/usb_descriptor.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#ifndef likely
#define likely(x)      __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

// attach helper function
static int read_stage1_command(uint8_t *buffer, uint32_t length);
static void handle_device_list(uint8_t *buffer, uint32_t length);
static void handle_device_attach(uint8_t *buffer, uint32_t length);
static void send_stage1_header(uint16_t command, uint32_t status);
static void send_device_list();
static void send_device_info();
static void send_interface_info();

// emulate helper function
static void pack(void *data, int size);
static void unpack(void *data, int size);

static void handle_unlink(usbip_stage2_header *header);
// unlink helper function
static void send_stage2_unlink(usbip_stage2_header *req_header);

int usbip_network_send(int s, const void *dataptr, size_t size, int flags) {
#if (USE_KCP == 1)
    return kcp_network_send(dataptr, size);
#elif (USE_TCP_NETCONN == 1)
    return tcp_netconn_send(dataptr, size);
#else // BSD style
    return send(s, dataptr, size, flags);
#endif
}

static int attach(uint8_t *buffer, uint32_t length)
{
    int command = read_stage1_command(buffer, length);
    if (command < 0)
    {
        return -1;
    }

    switch (command)
    {
    case USBIP_STAGE1_CMD_DEVICE_LIST: // OP_REQ_DEVLIST
        handle_device_list(buffer, length);
        break;

    case USBIP_STAGE1_CMD_DEVICE_ATTACH: // OP_REQ_IMPORT
        handle_device_attach(buffer, length);
        break;

    default:
        os_printf("attach Unknown command: %d\r\n", command);
        break;
    }
    return 0;
}

static int read_stage1_command(uint8_t *buffer, uint32_t length)
{
    if (length < sizeof(usbip_stage1_header))
    {
        return -1;
    }
    usbip_stage1_header *req = (usbip_stage1_header *)buffer;
    return (ntohs(req->command) & 0xFF); // 0x80xx low bit
}

static void handle_device_list(uint8_t *buffer, uint32_t length)
{
    os_printf("Handling dev list request...\r\n");
    send_stage1_header(USBIP_STAGE1_CMD_DEVICE_LIST, 0);
    send_device_list();
}

static void handle_device_attach(uint8_t *buffer, uint32_t length)
{
    os_printf("Handling dev attach request...\r\n");

    //char bus[USBIP_BUSID_SIZE];
    if (length < sizeof(USBIP_BUSID_SIZE))
    {
        os_printf("handle device attach failed!\r\n");
        return;
    }
    //client.readBytes((uint8_t *)bus, USBIP_BUSID_SIZE);

    send_stage1_header(USBIP_STAGE1_CMD_DEVICE_ATTACH, 0);

    send_device_info();
}

static void send_stage1_header(uint16_t command, uint32_t status)
{
    os_printf("Sending header...\r\n");
    usbip_stage1_header header;
    header.version = htons(273); ////TODO:  273???
    // may be : https://github.com/Oxalin/usbip_windows/issues/4

    header.command = htons(command);
    header.status = htonl(status);

    usbip_network_send(kSock, (uint8_t *)&header, sizeof(usbip_stage1_header), 0);
}

static void send_device_list()
{
    os_printf("Sending device list...\r\n");

    // send device list size:
    os_printf("Sending device list size...\r\n");
    usbip_stage1_response_devlist response_devlist;

    // we have only 1 device, so:
    response_devlist.list_size = htonl(1);

    usbip_network_send(kSock, (uint8_t *)&response_devlist, sizeof(usbip_stage1_response_devlist), 0);

    // may be foreach:

    {
        // send device info:
        send_device_info();
        // send device interfaces: // (1)
        send_interface_info();
    }
}

static void send_device_info()
{
    os_printf("Sending device info...\r\n");
    usbip_stage1_usb_device device;

    strcpy(device.path, "/sys/devices/pci0000:00/0000:00:01.2/usb1/1-1");
    strcpy(device.busid, "1-1");

    device.busnum = htonl(1);
    device.devnum = htonl(1);
    device.speed = htonl(3); // See usb_device_speed enum

    device.idVendor = htons(USBD0_DEV_DESC_IDVENDOR);
    device.idProduct = htons(USBD0_DEV_DESC_IDPRODUCT);
    device.bcdDevice = htons(USBD0_DEV_DESC_BCDDEVICE);

    device.bDeviceClass = 0x00; // We need to use a device other than the USB-IF standard, set to 0x00
    device.bDeviceSubClass = 0x00;
    device.bDeviceProtocol = 0x00;

    device.bConfigurationValue = 1;
    device.bNumConfigurations = 1;
    device.bNumInterfaces = 1;

    usbip_network_send(kSock, (uint8_t *)&device, sizeof(usbip_stage1_usb_device), 0);
}

static void send_interface_info()
{
    os_printf("Sending interface info...\r\n");
    usbip_stage1_usb_interface interface;
    interface.bInterfaceClass = USBD_CUSTOM_CLASS0_IF0_CLASS;
    interface.bInterfaceSubClass = USBD_CUSTOM_CLASS0_IF0_SUBCLASS;
    interface.bInterfaceProtocol = USBD_CUSTOM_CLASS0_IF0_PROTOCOL;
    interface.padding = 0; // shall be set to zero

    usbip_network_send(kSock, (uint8_t *)&interface, sizeof(usbip_stage1_usb_interface), 0);
}

static int usbip_urb_process(uint8_t *base, uint32_t length)
{
    usbip_stage2_header *header = (usbip_stage2_header *)base;
    uint8_t *data;
    uint32_t command, dir, ep;
    uint32_t unlink_count = 0;
    bool may_has_data;
    int sz, ret;
    int dap_req_num = 0;

    while (1) {
        // header
        data = base;
        sz = 48; // for USBIP_CMD_SUBMIT/USBIP_CMD_UNLINK
        do {
            ret = recv(kSock, data, sz, 0);
            if (ret <= 0)
                goto out;
            sz -= ret;
            data += ret;
        } while (sz > 0);

        command = ntohl(header->base.command);
        dir = ntohl(header->base.direction);
        ep = ntohl(header->base.ep);
        may_has_data = (command == USBIP_STAGE2_REQ_SUBMIT && dir == USBIP_DIR_OUT);
        sz = may_has_data ? ntohl(header->u.cmd_submit.data_length) : 0;

        while (sz) {
                ret = recv(kSock, data, sz, 0);
                if (ret <= 0)
                    goto out;
                sz -= ret;
                data += ret;
        }

        if (likely(command == USBIP_STAGE2_REQ_SUBMIT)) {
            if (likely(ep == 1 && dir == USBIP_DIR_IN)) {
                fast_reply(base, sizeof(usbip_stage2_header), dap_req_num);
                if (dap_req_num > 0)
                    dap_req_num--;
            } else if (likely(ep == 1 && dir == USBIP_DIR_OUT)) {
                dap_req_num++;
                handle_dap_data_request(header, length);
            } else if (ep == 0) {
                unpack(base, sizeof(usbip_stage2_header));
                handleUSBControlRequest(header);
            } else {
                // ep3 reserved for SWO
                os_printf("ep reserved:%d\r\n", ep);
                send_stage2_submit(header, 0, 0);
            }
        } else if (command == USBIP_STAGE2_REQ_UNLINK) {
            if (unlink_count == 0 || unlink_count % 100 == 0)
                os_printf("unlink\r\n");
            unlink_count++;
            unpack(base, sizeof(usbip_stage2_header));
            handle_unlink(header);
        } else {
            os_printf("emulate unknown command:%d\r\n", command);
            return -1;
        }
    }

out:
    if (ret < 0)
        os_printf("recv failed: errno %d\r\n", errno);
    return ret;
}

int usbip_worker(uint8_t *base, uint32_t length, enum usbip_server_state_t *state)
{
    uint8_t *data;
    int pre_read_sz = 4;
    int sz, ret;

    // OP_REQ_DEVLIST status field
    if (*state == WAIT_DEVLIST) {
        data = base + 4;
        sz = 8 - pre_read_sz;
        do {
            ret = recv(kSock, data, sz, 0);
            if (ret <= 0)
                return ret;
            sz -= ret;
            data += ret;
        } while (sz > 0);

        ret = attach(base, 8);
        if (ret)
            return ret;

        pre_read_sz = 0;
    }

    *state = WAIT_IMPORT;
    // OP_REQ_IMPORT
    data = base + pre_read_sz;
    sz = 40 - pre_read_sz;
    do {
        ret = recv(kSock, data, sz, 0);
        if (ret <= 0)
            return ret;
        sz -= ret;
        data += ret;
    } while (sz > 0);

    ret = attach(base, 40);
    if (ret)
        return ret;

    // URB process
    *state = WAIT_URB;
    ret = usbip_urb_process(base, length);
    if (ret) {
        *state = WAIT_DEVLIST;
        return ret;
    }

    return 0;
}

/**
 * @brief Pack the following packets(Offset 0x00 - 0x28):
 *       - cmd_submit
 *       - ret_submit
 *       - cmd_unlink
 *       - ret_unlink
 *
 * @param data Point to packets header
 * @param size Packets header size
 */
static void pack(void *data, int size)
{

    // Ignore the setup field
    int sz = (size / sizeof(uint32_t)) - 2;
    uint32_t *ptr = (uint32_t *)data;

    for (int i = 0; i < sz; i++)
    {

        ptr[i] = htonl(ptr[i]);
    }
}

/**
 * @brief Unack the following packets(Offset 0x00 - 0x28):
 *       - cmd_submit
 *       - ret_submit
 *       - cmd_unlink
 *       - ret_unlink
 *
 * @param data Point to packets header
 * @param size  packets header size
 */
static void unpack(void *data, int size)
{

    // Ignore the setup field
    int sz = (size / sizeof(uint32_t)) - 2;
    uint32_t *ptr = (uint32_t *)data;

    for (int i = 0; i < sz; i++)
    {
        ptr[i] = ntohl(ptr[i]);
    }
}

void send_stage2_submit(usbip_stage2_header *req_header, int32_t status, int32_t data_length)
{

    req_header->base.command = USBIP_STAGE2_RSP_SUBMIT;
    req_header->base.direction = !(req_header->base.direction);

    memset(&(req_header->u.ret_submit), 0, sizeof(usbip_stage2_header_ret_submit));

    req_header->u.ret_submit.status = status;
    req_header->u.ret_submit.data_length = data_length;
    // already unpacked
    pack(req_header, sizeof(usbip_stage2_header));
    usbip_network_send(kSock, req_header, sizeof(usbip_stage2_header), 0);
}

void send_stage2_submit_data(usbip_stage2_header *req_header, int32_t status, const void *const data, int32_t data_length)
{

    send_stage2_submit(req_header, status, data_length);

    if (data_length)
    {
        usbip_network_send(kSock, data, data_length, 0);
    }
}

void send_stage2_submit_data_fast(usbip_stage2_header *req_header, const void *const data, int32_t data_length)
{
    uint8_t * send_buf = (uint8_t *)req_header;

    req_header->base.command = PP_HTONL(USBIP_STAGE2_RSP_SUBMIT);
    req_header->base.direction = htonl(!(req_header->base.direction));

    memset(&(req_header->u.ret_submit), 0, sizeof(usbip_stage2_header_ret_submit));
    req_header->u.ret_submit.data_length = htonl(data_length);

    // payload
    if (data)
        memcpy(&send_buf[sizeof(usbip_stage2_header)], data, data_length);
    usbip_network_send(kSock, send_buf, sizeof(usbip_stage2_header) + data_length, 0);
}


static void handle_unlink(usbip_stage2_header *header)
{
    handle_dap_unlink();
    send_stage2_unlink(header);
}

static void send_stage2_unlink(usbip_stage2_header *req_header)
{

    req_header->base.command = USBIP_STAGE2_RSP_UNLINK;
    req_header->base.direction = USBIP_DIR_OUT;

    memset(&(req_header->u.ret_unlink), 0, sizeof(usbip_stage2_header_ret_unlink));

    // To be more precise, the value is `-ECONNRESET`, but usbip-win only cares if it is a
    // non zero value. A non-zero value indicates that our UNLINK operation was "successful",
    // but the host driver's may behave differently, or may even ignore this state. For consistent
    // behavior, we use non-zero value here. See also comments regarding `handle_dap_unlink()`.
    req_header->u.ret_unlink.status = -1;

    pack(req_header, sizeof(usbip_stage2_header));

    usbip_network_send(kSock, req_header, sizeof(usbip_stage2_header), 0);
}

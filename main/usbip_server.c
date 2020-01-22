#include <stdint.h>
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "usbip_server.h"
#include "usbip_defs.h"
#include "usb_defs.h"

static int read_stage1_command(uint8_t *buffer, uint32_t length);
static int handle_device_list(uint8_t *buffer, uint32_t length);
static int handle_device_attach(uint8_t *buffer, uint32_t length);
static void send_stage1_header(uint16_t command, uint32_t status);
static void send_device_list();
static void send_device_info();



int attach(uint8_t *buffer, uint32_t length)
{
    uint32_t command = read_stage1_command(buffer, length);
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
        os_printf("s1 unknown command: %d\r\n", command);
        break;
    }
}

static int read_stage1_command(uint8_t *buffer, uint32_t length)
{
    if (length < sizeof(usbip_stage1_header))
    {
        return -1;
    }
    usbip_stage1_header *req = (usbip_stage1_header *)buffer;
    return ntohs(req->command) & 0xFF; // 0x80xx low bit
}

static int handle_device_list(uint8_t *buffer, uint32_t length)
{
    os_printf("Handling dev list request...\r\n");
    send_stage1_header(USBIP_STAGE1_CMD_DEVICE_LIST, 0);
    send_device_list();
}

static int handle_device_attach(uint8_t *buffer, uint32_t length)
{
    int
}

static void send_stage1_header(uint16_t command, uint32_t status)
{
    os_printf("Sending header...\r\n");
    usbip_stage1_header header;
    header.version = htons(273); // 273???
    header.command = htons(command);
    header.status = htonl(status);

    send(socket, (uint8_t *)&header, sizeof(usbip_stage1_header), 0);
}

static void send_device_list()
{
    os_printf("Sending device list...\r\n");

    // send device list size:
    os_printf("Sending device list size...\r\n");
    usbip_stage1_response_devlist response_devlist;

    // we have only 1 device, so:
    response_devlist.list_size = htonl(1);

    send(socket, (uint8_t *)&response_devlist, sizeof(usbip_stage1_response_devlist), 0);

    // may be foreach:

    {
        // send device info:
        send_device_info();
        // send device interfaces: // (1)
        send_interface_info();
    }
}

void send_device_info()
{
    //// TODO:fill this
    os_printf("Sending device info...");
    usbip_stage1_usb_device device;

    strcpy(device.path, "/sys/devices/pci0000:00/0000:00:01.2/usb1/1-1");
    strcpy(device.busid, "1-1");

    device.busnum = htonl(1);
    device.devnum = htonl(2);
    device.speed = htonl(2); // what is this???
    //// TODO: 0200H for USB2.0

    //// TODO: fill Vendor Product Device-version
    device.idVendor = htons(USB_DEVICE_VENDOR_ID);
    device.idProduct = htons(USB_DEVICE_PRODUCT_ID);
    device.bcdDevice = htons(USB_DEVICE_VERSION);

    device.bDeviceClass = 0x00; // <defined at interface level>
    device.bDeviceSubClass = 0x00;
    device.bDeviceProtocol = 0x00;

    device.bConfigurationValue = 1;
    device.bNumConfigurations = 1;
    device.bNumInterfaces = 1;

    write(&device, sizeof(usbip_stage1_usb_device));
}

int emulate(uint8_t *buffer, uint32_t length)
{
}
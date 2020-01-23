/**
 * @file USB_handle.c
 * @brief Handle all Standard Device Requests
 * @version 0.1
 * @date 2020-01-23
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#include <stdint.h>
#include "USB_handle.h"

// handle functions
static void handleGetDescriptor(usbip_stage2_header *header);
static void handle_get_device_descriptor(usbip_stage2_header *header);

////TODO: fill this
int handleUSBControlRequest(usbip_stage2_header *header)
{
    // Table 9-3. Standard Device Requests

    switch (header->u.cmd_submit.request.bmRequestType)
    {
    case 0x00:
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_CLEAR_FEATURE:
            /* code */
            break;
        case USB_REQ_SET_FEATURE:
            /* code */
            break;
        case USB_REQ_SET_ADDRESS:
            /* code */
            break;
        case USB_REQ_SET_DESCRIPTOR:
            /* code */
            break;
        case USB_REQ_SET_CONFIGURATION:
            /* code */
            break;
        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;
    case 0x01:
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_CLEAR_FEATURE:
            /* code */
            break;
        case USB_REQ_SET_FEATURE:
            /* code */
            break;
        case USB_REQ_SET_INTERFACE:
            /* code */
            break;

        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;
    case 0x02:
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_CLEAR_FEATURE:
            /* code */
            break;
        case USB_REQ_SET_FEATURE:
            /* code */
            break;

        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;

    case 0x80:
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_GET_CONFIGURATION:
            handleGetDescriptor(header);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            /* code */
            break;
        case USB_REQ_GET_STATUS:
            /* code */
            break;
        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;
    case 0x81:
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_GET_INTERFACE:
            /* code */
            break;
        case USB_REQ_SET_SYNCH_FRAME:
            /* code */
            break;
        case USB_REQ_GET_STATUS:
            /* code */
            break;

        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;

    case 0x82:
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_GET_STATUS:
            /* code */
            break;

        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;
        /////////

    default:
        os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                  header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
        break;
    }
}

////TODO: fill this
static void handleGetDescriptor(usbip_stage2_header *header)
{
    // 9.4.3 Get Descriptor
    switch (header->u.cmd_submit.request.wValue.u8hi)
    {
    case USB_DT_DEVICE:
        handleGetDeviceDescriptor(header);
        break;

    case USB_DT_CONFIGURATION:
        handleGetConfigurationDescriptor(header);
        break;

    case USB_DT_STRING:
        handleGetStringDescriptor(header);
        break;

    case USB_DT_INTERFACE:
        handleGetInterfaceDescriptor(header);
        break;

    case USB_DT_ENDPOINT:
        handleGetEndpointDescriptor(header);
        break;

    case USB_DT_DEVICE_QUALIFIER:
        handleGetDeviceQualifierDescriptor(header);
        break;

    case USB_DT_OTHER_SPEED_CONFIGURATION:
        os_printf("GET 0x07 [UNIMPLEMENTED] USB_DT_OTHER_SPEED_CONFIGURATION");
        break;

    case USB_DT_INTERFACE_POWER:
        os_printf("GET 0x08 [UNIMPLEMENTED] USB_DT_INTERFACE_POWER");
        break;

    case USB_DT_REPORT:
        handle_get_hid_report_descriptor(header);
        break;

    default:
        os_printf("USB unknown Get Descriptor requested:%d", header->u.cmd_submit.request.wValue.u8lo);
        break;
    }
}

static void handle_get_device_descriptor(usbip_stage2_header *header)
{
    os_printf("* GET 0x01 DEVICE DESCRIPTOR\r\n");

    usb_device_descriptor desc;

    desc.bLength = USB_DT_DEVICE_SIZE;
    desc.bDescriptorType = USB_DT_DEVICE;

    desc.bcdUSB = 0x0110;

    // defined at interface level
    desc.bDeviceClass = 0x0;
    desc.bDeviceSubClass = 0x0;
    desc.bDeviceProtocol = 0x0;

    desc.bMaxPacketSize0 = USB_HID_MAX_PACKET_SIZE;

    desc.idVendor = USB_DEVICE_VENDOR_ID;
    desc.idProduct = USB_DEVICE_PRODUCT_ID;
    desc.bcdDevice = USB_DEVICE_VERSION;

    desc.iManufacturer = STR_IMANUFACTURER;
    desc.iProduct = STR_IPRODUCT;
    desc.iSerialNumber = STR_ISERIAL;

    desc.bNumConfigurations = 1;

    send_stage2_submit_data(header, 0, &desc, sizeof(usb_device_descriptor));
}
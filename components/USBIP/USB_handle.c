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
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "USB_handle.h"
#include "USBd_config.h"
#include "usbip_server.h"
#include "usb_defs.h"


// handle functions
static void handleGetDescriptor(struct usbip_stage2_header *header);

////TODO: fill this
int handleUSBControlRequest(struct usbip_stage2_header *header)
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
static void handleGetDescriptor(struct usbip_stage2_header *header)
{
    // 9.4.3 Get Descriptor
    switch (header->u.cmd_submit.request.wValue.u8hi)
    {
    case USB_DT_DEVICE: // get device descriptor
        os_printf("* GET 0x01 DEVICE DESCRIPTOR\r\n");
        send_stage2_submit_data(header, 0, kUSBd0DeviceDescriptor, sizeof(kUSBd0DeviceDescriptor));
        break;

    case USB_DT_CONFIGURATION: // get configuration descriptor
        os_printf("* GET 0x02 CONFIGURATION DESCRIPTOR\r\n");
        ////TODO: ?
        if (header->u.cmd_submit.data_length == USB_DT_CONFIGURATION_SIZE)
        {
            os_printf("Sending only first part of CONFIG\r\n");

            send_stage2_submit(header, 0, header->u.cmd_submit.data_length);
            send(kSock, kUSBd0ConfigDescriptor, sizeof(kUSBd0ConfigDescriptor), 0);

        }
        else
        {
            os_printf("Sending ALL CONFIG\r\n");

            send_stage2_submit(header, 0, header->u.cmd_submit.data_length);
            send(kSock, kUSBd0ConfigDescriptor, sizeof(kUSBd0ConfigDescriptor), 0);
            send(kSock, kUSBd0InterfaceDescriptor, sizeof(kUSBd0InterfaceDescriptor), 0);

        }
        break;

    case USB_DT_STRING:
        os_printf("* GET 0x03 STRING DESCRIPTOR\r\n");

        if (header->u.cmd_submit.request.wValue.u8lo == 0) {
            os_printf("** REQUESTED list of supported languages\r\n");
            send_stage2_submit_data(header, 0, kLangDescriptor, sizeof(kLangDescriptor));
        }
        else{
            os_printf("***Unsupported operation***\r\n");
        }
        break;

    case USB_DT_INTERFACE:
        os_printf("* GET 0x04 INTERFACE DESCRIPTOR (UNIMPLEMENTED)\r\n");
        ////TODO:UNIMPLEMENTED
        send_stage2_submit(header, 0, 0);
        break;

    case USB_DT_ENDPOINT:
        os_printf("* GET 0x05 ENDPOINT DESCRIPTOR (UNIMPLEMENTED)\r\n");
        ////TODO:UNIMPLEMENTED
        send_stage2_submit(header, 0, 0);
        break;

    case USB_DT_DEVICE_QUALIFIER:
        os_printf("* GET 0x06 DEVICE QUALIFIER DESCRIPTOR");

        usb_device_qualifier_descriptor desc;

        memset(&desc, 0, sizeof(usb_device_qualifier_descriptor));

        send_stage2_submit_data(header, 0, &desc, sizeof(usb_device_qualifier_descriptor));
        break;

    case USB_DT_OTHER_SPEED_CONFIGURATION:
        os_printf("GET 0x07 [UNIMPLEMENTED] USB_DT_OTHER_SPEED_CONFIGURATION");
        ////TODO:UNIMPLEMENTED
        send_stage2_submit(header, 0, 0);
        break;

    case USB_DT_INTERFACE_POWER:
        os_printf("GET 0x08 [UNIMPLEMENTED] USB_DT_INTERFACE_POWER");
        ////TODO:UNIMPLEMENTED
        send_stage2_submit(header, 0, 0);
        break;


    default:
        os_printf("USB unknown Get Descriptor requested:%d", header->u.cmd_submit.request.wValue.u8lo);
        break;
    }
}

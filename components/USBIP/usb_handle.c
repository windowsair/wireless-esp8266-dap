/**
 * @file USB_handle.c
 * @brief Handle all Standard Device Requests on endpoint 0
 * @version 0.1
 * @date 2020-01-23
 *
 * @copyright Copyright (c) 2020
 *
 */
#include <stdint.h>
#include <string.h>

#include "main/usbip_server.h"
#include "main/wifi_configuration.h"

#include "components/USBIP/usb_handle.h"
#include "components/USBIP/usb_descriptor.h"
#include "components/USBIP/MSOS20_descriptor.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


const char *strings_list[] = {
        0, // reserved: available languages  -> iInterface
        "windowsair",
        "Wireless ESP CMSIS-DAP",
        "1234",
};
// handle functions
static void handleGetDescriptor(usbip_stage2_header *header);

////TODO: may be ok
void handleUSBControlRequest(usbip_stage2_header *header)
{
    // Table 9-3. Standard Device Requests

    switch (header->u.cmd_submit.request.bmRequestType)
    {
    case 0x00: // ignore..
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_CLEAR_FEATURE:
            os_printf("* CLEAR FEATURE\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        case USB_REQ_SET_FEATURE:
            os_printf("* SET FEATURE\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        case USB_REQ_SET_ADDRESS:
            os_printf("* SET ADDRESS\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        case USB_REQ_SET_DESCRIPTOR:
            os_printf("* SET DESCRIPTOR\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        case USB_REQ_SET_CONFIGURATION:
            os_printf("* SET CONFIGURATION\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;
    case 0x01: // ignore...
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_CLEAR_FEATURE:
            os_printf("* CLEAR FEATURE\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        case USB_REQ_SET_FEATURE:
            os_printf("* SET FEATURE\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        case USB_REQ_SET_INTERFACE:
            os_printf("* SET INTERFACE\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;

        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;
    case 0x02: // ignore..
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_CLEAR_FEATURE:
            os_printf("* CLEAR FEATURE\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        case USB_REQ_SET_FEATURE:
            os_printf("* SET INTERFACE\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;

        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;

    case 0x80: // *IMPORTANT*
#if (USE_WINUSB == 0)
    case 0x81:
#endif
    {
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_GET_CONFIGURATION:
            os_printf("* GET CONIFGTRATION\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            handleGetDescriptor(header); ////TODO: device_qualifier
            break;
        case USB_REQ_GET_STATUS:
            os_printf("* GET STATUS\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;
    }
#if (USE_WINUSB == 1)
    case 0x81: // ignore...
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_GET_INTERFACE:
            os_printf("* GET INTERFACE\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        case USB_REQ_SET_SYNCH_FRAME:
            os_printf("* SET SYNCH FRAME\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;
        case USB_REQ_GET_STATUS:
            os_printf("* GET STATUS\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;

        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;
#endif
    case 0x82: // ignore...
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_GET_STATUS:
            os_printf("* GET STATUS\r\n");
            send_stage2_submit_data(header, 0, 0, 0);
            break;

        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;
    case 0xC0: // Microsoft OS 2.0 vendor-specific descriptor
    {
        uint16_t *wIndex = (uint16_t *)(&(header->u.cmd_submit.request.wIndex));
        switch (*wIndex)
        {
        case MS_OS_20_DESCRIPTOR_INDEX:
            os_printf("* GET MSOS 2.0 vendor-specific descriptor\r\n");
            send_stage2_submit_data(header, 0, msOs20DescriptorSetHeader, sizeof(msOs20DescriptorSetHeader));
            break;
        case MS_OS_20_SET_ALT_ENUMERATION:
            // set alternate enumeration command
            // bAltEnumCode set to 0
            os_printf("Set alternate enumeration.This should not happen.\r\n");
            break;

        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d,wIndex:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest, *wIndex);
            break;
        }
        break;
    }
    case 0x21: // Set_Idle for HID
        switch (header->u.cmd_submit.request.bRequest)
        {
        case USB_REQ_SET_IDLE:
            os_printf("* SET IDLE\r\n");
            send_stage2_submit(header, 0, 0);
            break;

        default:
            os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                      header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
            break;
        }
        break;
    default:
        os_printf("USB unknown request, bmRequestType:%d,bRequest:%d\r\n",
                  header->u.cmd_submit.request.bmRequestType, header->u.cmd_submit.request.bRequest);
        break;
    }
}

static void handleGetDescriptor(usbip_stage2_header *header)
{
    // 9.4.3 Get Descriptor
    switch (header->u.cmd_submit.request.wValue.u8hi)
    {
    case USB_DT_DEVICE: // get device descriptor
        os_printf("* GET 0x01 DEVICE DESCRIPTOR\r\n");
        send_stage2_submit_data(header, 0, &kUSBd0DeviceDescriptor[0], sizeof(kUSBd0DeviceDescriptor));
        break;

    case USB_DT_CONFIGURATION: // get configuration descriptor
        os_printf("* GET 0x02 CONFIGURATION DESCRIPTOR\r\n");
        ////TODO: ?
        if (header->u.cmd_submit.data_length == USB_DT_CONFIGURATION_SIZE)
        {
            os_printf("Sending only first part of CONFIG\r\n");

            send_stage2_submit(header, 0, header->u.cmd_submit.data_length);
            usbip_network_send(kSock, kUSBd0ConfigDescriptor, sizeof(kUSBd0ConfigDescriptor), 0);
        }
        else
        {
            os_printf("Sending ALL CONFIG\r\n");
            send_stage2_submit(header, 0, sizeof(kUSBd0ConfigDescriptor) + sizeof(kUSBd0InterfaceDescriptor));
            usbip_network_send(kSock, kUSBd0ConfigDescriptor, sizeof(kUSBd0ConfigDescriptor), 0);
            usbip_network_send(kSock, kUSBd0InterfaceDescriptor, sizeof(kUSBd0InterfaceDescriptor), 0);
        }
        break;

    case USB_DT_STRING:
        //os_printf("* GET 0x03 STRING DESCRIPTOR\r\n");

        if (header->u.cmd_submit.request.wValue.u8lo == 0)
        {
            os_printf("** REQUESTED list of supported languages\r\n");
            send_stage2_submit_data(header, 0, kLangDescriptor, sizeof(kLangDescriptor));
        }
        else if (header->u.cmd_submit.request.wValue.u8lo != 0xee)
        {
            //os_printf("low bit : %d\r\n", (int)header->u.cmd_submit.request.wValue.u8lo);
            //os_printf("high bit : %d\r\n", (int)header->u.cmd_submit.request.wValue.u8hi);
            int slen = strlen(strings_list[header->u.cmd_submit.request.wValue.u8lo]);
            int wslen = slen * 2;
            int buff_len = sizeof(usb_string_descriptor) + wslen;
            char temp_buff[64];
            usb_string_descriptor *desc = (usb_string_descriptor *)temp_buff;
            desc->bLength = buff_len;
            desc->bDescriptorType = USB_DT_STRING;
            for (int i = 0; i < slen; i++)
            {
                desc->wData[i] = strings_list[header->u.cmd_submit.request.wValue.u8lo][i];

            }
            send_stage2_submit_data(header, 0, (uint8_t *)temp_buff, buff_len);
        }
        else
        {
            os_printf("low bit : %d\r\n", (int)header->u.cmd_submit.request.wValue.u8lo);
            os_printf("high bit : %d\r\n", (int)header->u.cmd_submit.request.wValue.u8hi);
            os_printf("***Unsupported String descriptor***\r\n");
            send_stage2_submit(header, 0, 0);
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
        os_printf("* GET 0x06 DEVICE QUALIFIER DESCRIPTOR\r\n");

        usb_device_qualifier_descriptor desc;

        memset(&desc, 0, sizeof(usb_device_qualifier_descriptor));

        send_stage2_submit_data(header, 0, &desc, sizeof(usb_device_qualifier_descriptor));
        break;

    case USB_DT_OTHER_SPEED_CONFIGURATION:
        os_printf("GET 0x07 [UNIMPLEMENTED] USB_DT_OTHER_SPEED_CONFIGURATION\r\n");
        ////TODO:UNIMPLEMENTED
        send_stage2_submit(header, 0, 0);
        break;

    case USB_DT_INTERFACE_POWER:
        os_printf("GET 0x08 [UNIMPLEMENTED] USB_DT_INTERFACE_POWER\r\n");
        ////TODO:UNIMPLEMENTED
        send_stage2_submit(header, 0, 0);
        break;
#if (USE_WINUSB == 1)
    case USB_DT_BOS:
        os_printf("* GET 0x0F BOS DESCRIPTOR\r\n");
        uint32_t bos_len = header->u.cmd_submit.request.wLength.u8lo | ((uint32_t) header->u.cmd_submit.request.wLength.u8hi << 8);
        bos_len = (sizeof(bosDescriptor) < bos_len) ? sizeof(bosDescriptor) : bos_len;
        send_stage2_submit_data(header, 0, bosDescriptor, bos_len);
        break;
#else
    case USB_DT_HID_REPORT:
        os_printf("* GET 0x22 HID REPORT DESCRIPTOR\r\n");
        send_stage2_submit_data(header, 0, (void *)kHidReportDescriptor, sizeof(kHidReportDescriptor));
        break;
#endif
    default:
        //// TODO: ms os 1.0 descriptor
        os_printf("USB unknown Get Descriptor requested:%d\r\n", header->u.cmd_submit.request.wValue.u8lo);
        os_printf("low bit :%d\r\n",header->u.cmd_submit.request.wValue.u8lo);
        os_printf("high bit :%d\r\n",header->u.cmd_submit.request.wValue.u8hi);
        break;
    }
}

/**
 * @file usb_defs.h
 * @brief Modify
 * @version 0.1
 * @date 2020-01-22
 *
 * @copyright Copyright (c) 2020
 *
 */

//
// Created by thevoidnn on 10/25/17.
//

#ifndef __USB_DEFS_H__
#define __USB_DEFS_H__

#include <stdint.h>

#define USB_CLASS_MISCELLANEOUS_DEVICE 0xef
#define USB_MISC_SUBCLASS_COMMON 0x02
#define USB_MISC_PROTOCOL_INTERFACE_ASSOCIATION_DESCRIPTOR 0x01

typedef union {
    struct
    {
        uint8_t u8lo;
        uint8_t u8hi;
    } __attribute__((packed));
    uint16_t u16;
} word_t;

typedef struct
{
    uint8_t bmRequestType;
    uint8_t bRequest;
    word_t wValue; // 16bit
    word_t wIndex;
    word_t wLength;
} __attribute__((packed)) usb_standard_request;

//#define USB_CLASS_HID	3

#define USB_DT_HID 0x21
#define USB_DT_REPORT 0x22

//struct usb_hid_descriptor {
//    uint8_t bLength;
//    uint8_t bDescriptorType;
//    uint16_t bcdHID;
//    uint8_t bCountryCode;
//    uint8_t bNumDescriptors;
//} __attribute__((packed));
//#define USB_DT_HID_SIZE sizeof(struct usb_hid_descriptor)

//struct usb_hid_report_descriptor {
//    uint8_t bDescriptorType;
//    uint16_t wReportLength;
//} __attribute__((packed));

#define USB_DT_REPORT_SIZE sizeof(struct usb_hid_report_descriptor)

/* Class Definition */
#define USB_CLASS_VENDOR 0xFF

///////////////////////////////////////////////////////////////
/* Table 9-2. Format of Setup Data */
/* bmRequestType bit definitions */

/* bit 7 : Direction */
#define USB_REQ_TYPE_OUT 0x00 // Host-to-device
#define USB_REQ_TYPE_IN 0x80  // Device-to-host
/* bits 6..5 : Type */
#define USB_REQ_TYPE_STANDARD 0x00
#define USB_REQ_TYPE_CLASS 0x20
#define USB_REQ_TYPE_VENDOR 0x40
//#define USB_REQ_TYPE_RESERVED 0x60
/* bits 4..0 : Recipient */
#define USB_REQ_TYPE_DEVICE 0x00
#define USB_REQ_TYPE_INTERFACE 0x01
#define USB_REQ_TYPE_ENDPOINT 0x02
#define USB_REQ_TYPE_OTHER 0x03
//#define USB_REQ_TYPE_RESERVED 0x1F
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
/* USB Standard Request Codes - Table 9-4 */
#define USB_REQ_GET_STATUS 0
#define USB_REQ_CLEAR_FEATURE 1
/* Reserved for future use: 2 */
#define USB_REQ_SET_FEATURE 3
/* Reserved for future use: 3 */
#define USB_REQ_SET_ADDRESS 5
#define USB_REQ_GET_DESCRIPTOR 6
#define USB_REQ_SET_DESCRIPTOR 7
#define USB_REQ_GET_CONFIGURATION 8
#define USB_REQ_SET_CONFIGURATION 9
#define USB_REQ_GET_INTERFACE 10
#define USB_REQ_SET_INTERFACE 11
#define USB_REQ_SET_SYNCH_FRAME 12

// USB HID Request
#define USB_REQ_GET_REPORT     0x01
#define USB_REQ_GET_IDLE       0x02
#define USB_REQ_GET_PROTOCOL   0x03
#define USB_REQ_SET_REPORT     0x09
#define USB_REQ_SET_IDLE       0X0A
#define USB_REQ_SET_PROTOCOL   0X0B
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
/* USB Descriptor Types - Table 9-5 */
#define USB_DT_DEVICE 1
#define USB_DT_CONFIGURATION 2
#define USB_DT_STRING 3
#define USB_DT_INTERFACE 4
#define USB_DT_ENDPOINT 5
#define USB_DT_DEVICE_QUALIFIER 6
#define USB_DT_OTHER_SPEED_CONFIGURATION 7
#define USB_DT_INTERFACE_POWER 8
#define USB_DT_BOS 15
#define USB_DT_SUPERSPEED_USB_ENDPOINT_COMPANION 48
/* From ECNs */
#define USB_DT_OTG 9
#define USB_DT_DEBUG 10
#define USB_DT_INTERFACE_ASSOCIATION 11
/* USB HID */
#define USB_DT_HID        0x21
#define USB_DT_HID_REPORT 0x22
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
/* USB Standard Feature Selectors - Table 9-6 */
#define USB_FEAT_ENDPOINT_HALT 0        // Recipient: Device
#define USB_FEAT_DEVICE_REMOTE_WAKEUP 1 // Recipient: Endpoint
#define USB_FEAT_TEST_MODE 2            // Recipient: Device

/* Information Returned by a GetStatus() Request to a Device - Figure 9-4 */
#define USB_DEV_STATUS_SELF_POWERED 0x01
#define USB_DEV_STATUS_REMOTE_WAKEUP 0x02
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
/* USB Standard Device Descriptor - Table 9-8 */
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __attribute__((packed)) usb_device_descriptor;
#define USB_DT_DEVICE_SIZE sizeof(usb_device_descriptor)
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
/* USB Device_Qualifier Descriptor - Table 9-9
 * Not used in this implementation.
 */
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint8_t bNumConfigurations;
    uint8_t bReserved;
} __attribute__((packed)) usb_device_qualifier_descriptor;
///////////////////////////////////////////////////////////////

/* This is only defined as a top level named struct to improve c++
 * compatibility.  You should never need to instance this struct
 * in user code! */
typedef struct
{
    uint8_t *cur_altsetting;
    uint8_t num_altsetting;
    const struct usb_iface_assoc_descriptor *iface_assoc;
    const struct usb_interface_descriptor *altsetting;
} __attribute__((packed)) usb_interface;

///////////////////////////////////////////////////////////////
/* USB Standard Configuration Descriptor - Table 9-10 */
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
} __attribute__((packed)) usb_config_descriptor;
#define USB_DT_CONFIGURATION_SIZE sizeof(usb_config_descriptor)
///////////////////////////////////////////////////////////////

/* USB Configuration Descriptor *bmAttributes* bit definitions */
#define USB_CONFIG_ATTR_DEFAULT 0x80 /** always required (USB2.0 table 9-10) */
#define USB_CONFIG_ATTR_SELF_POWERED 0x40
#define USB_CONFIG_ATTR_REMOTE_WAKEUP 0x20

/* Other Speed Configuration is the same as Configuration Descriptor.
 *  - Table 9-11
 */

///////////////////////////////////////////////////////////////
/* USB Standard Interface Descriptor - Table 9-12 */
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} __attribute__((packed)) usb_interface_descriptor;
#define USB_DT_INTERFACE_SIZE sizeof(usb_interface_descriptor)
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
/* USB Standard Endpoint Descriptor - Table 9-13 */
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} __attribute__((packed))usb_endpoint_descriptor;
#define USB_DT_ENDPOINT_SIZE sizeof(usb_endpoint_descriptor)
///////////////////////////////////////////////////////////////

/* USB bEndpointAddress helper macros */
#define USB_ENDPOINT_ADDR_OUT(x) (x)
#define USB_ENDPOINT_ADDR_IN(x) (0x80 | (x))

///////////////////////////////////////////////////////////////
/* USB Endpoint Descriptor bmAttributes bit definitions - Table 9-13 */
/* bits 1..0 : Transfer type */
#define USB_ENDPOINT_ATTR_CONTROL 0x00
#define USB_ENDPOINT_ATTR_ISOCHRONOUS 0x01
#define USB_ENDPOINT_ATTR_BULK 0x02
#define USB_ENDPOINT_ATTR_INTERRUPT 0x03
#define USB_ENDPOINT_ATTR_TYPE 0x03
// If not an isochronous endpoint, bits 5..2 are reserved
// and must be set to zero.
/* bits 3..2 : Sync type (only if ISOCHRONOUS) */
#define USB_ENDPOINT_ATTR_NOSYNC 0x00
#define USB_ENDPOINT_ATTR_ASYNC 0x04
#define USB_ENDPOINT_ATTR_ADAPTIVE 0x08
#define USB_ENDPOINT_ATTR_SYNC 0x0C
#define USB_ENDPOINT_ATTR_SYNCTYPE 0x0C
/* bits 5..4 : Usage type (only if ISOCHRONOUS) */
#define USB_ENDPOINT_ATTR_DATA 0x00
#define USB_ENDPOINT_ATTR_FEEDBACK 0x10
#define USB_ENDPOINT_ATTR_IMPLICIT_FEEDBACK_DATA 0x20
#define USB_ENDPOINT_ATTR_USAGETYPE 0x30
///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
/* Table 9-15 specifies String Descriptor Zero.
 * Table 9-16 specified UNICODE String Descriptor.
 */
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wData[];
} __attribute__((packed)) usb_string_descriptor;

/* From ECN: Interface Association Descriptors, Table 9-Z */
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bFirstInterface;
    uint8_t bInterfaceCount;
    uint8_t bFunctionClass;
    uint8_t bFunctionSubClass;
    uint8_t bFunctionProtocol;
    uint8_t iFunction;
} __attribute__((packed)) usb_iface_assoc_descriptor;
#define USB_DT_INTERFACE_ASSOCIATION_SIZE \
    sizeof(usb_iface_assoc_descriptor)

enum usb_language_id
{
    USB_LANGID_ENGLISH_US = 0x409,
};
///////////////////////////////////////////////////////////////

#endif

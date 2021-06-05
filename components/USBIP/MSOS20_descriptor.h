/**
 * @file MSOS20_descriptor.h
 * @author windowsair
 * @brief
 * @version 0.2
 * @date 2021-5-12
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __MSOS20_DESCRIPTOR_H__
#define __MSOS20_DESCRIPTOR_H__


#define kLengthOfMsOS20 0xA2

#if (USE_USB_3_0 == 1)
#define kLengthOfBos 0x32
#else
#define kLengthOfBos 0x21
#endif // USE_USB_3_0 == 1

#define kValueOfbMS_VendorCode  0x01// Just set to 0x01
extern const uint8_t bosDescriptor[kLengthOfBos];
extern const uint8_t msOs20DescriptorSetHeader[kLengthOfMsOS20];

/* Microsoft OS 2.0 Descriptors BEGIN */

// Platform capability BOS descriptor, Table 1.
#define USB_DEVICE_CAPABILITY_TYPE_PLATFORM 5

// USB 2.0 Extension Descriptor, USB3.0 Specification Table 9-11
#define USB_DEVICE_CAPABILITY_TYPE_USB2_0_EXTENSION 2
//  SuperSpeed USB specific device level capabilities, USB3.0 Specification Table 9-11
#define USB_DEVICE_CAPABILITY_TYPE_SUPERSPEED_USB 3

// Platform capability UUID,  Table 3.
// {D8DD60DF-4589-4CC7-9CD2-659D9E648A9F}
#define USB_DEVICE_CAPABILITY_UUID  0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C, 0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F


// Microsoft OS 2.0 descriptor wIndex values enum, Table 8.
#define MS_OS_20_DESCRIPTOR_INDEX 7
#define MS_OS_20_SET_ALT_ENUMERATION 8


// Microsoft OS 2.0 descriptor types enum for wDescriptorType values, Table 9.
#define MS_OS_20_SET_HEADER_DESCRIPTOR 0x00
#define MS_OS_20_SUBSET_HEADER_CONFIGURATION 0x01
#define MS_OS_20_SUBSET_HEADER_FUNCTION 0x02
#define MS_OS_20_FEATURE_COMPATIBLE_ID 0x03
#define MS_OS_20_FEATURE_REG_PROPERTY 0x04
#define MS_OS_20_FEATURE_MIN_RESUME_TIME 0x05
#define MS_OS_20_FEATURE_MODEL_ID 0x06
#define MS_OS_20_FEATURE_CCGP_DEVICE 0x07

/* Microsoft OS 2.0 Descriptors END */



/* Wireless USB Standard Extension Descriptor Types BEGIN */

// Wireless USB Specification 1.1 revison 1.1, Table 7-21.
#define USB_DESCRIPTOR_TYPE_SECURITY 12
#define USB_DESCRIPTOR_TYPE_KEY 13
#define USB_DESCRIPTOR_TYPE_ENCRYPTION_TYPE 14
#define USB_DESCRIPTOR_TYPE_BOS 15
#define USB_DESCRIPTOR_TYPE_DEVICE_CAPABILITY 16
#define USB_DESCRIPTOR_TYPE_WIRELESS_ENDPOINT_COMPANION 17

/* Wireless USB Standard Extension Descriptor Types END */


/* Microsoft Extended Compat ID OS Feature Descriptor BEGIN */

#define USB_MS_EXTENDED_COMPAT_ID_VERSION 0x0100
#define USB_MS_EXTENDED_COMPAT_ID_TYPE    0x04

#define USB_COMPATID_NONE                {0}
#define USB_SUBCOMPATID_NONE             {0}
#define USB_COMPATID_WINUSB              "WINUSB\0"
#define USB_COMPATID_RNDIS               "RNDIS\0\0"
#define USB_COMPATID_PTP                 "PTP\0\0\0\0"
#define USB_COMPATID_MTP                 "MTP\0\0\0\0"
#define USB_COMPATID_BLUETOOTH           "BLUTUTH"
#define USB_SUBCOMPATID_BT_V11           "11\0\0\0\0\0"
#define USB_SUBCOMPATID_BT_V12           "12\0\0\0\0\0"
#define USB_SUBCOMPATID_BT_V20EDR        "EDR\0\0\0\0"

/* Microsoft Extended Compat ID OS Feature Descriptor END */

#endif
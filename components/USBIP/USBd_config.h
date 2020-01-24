#ifndef __USBD_CONFIG_H__
#define __USBD_CONFIG_H__

// Vendor ID assigned by USB-IF (idVendor).
#define USBD0_DEV_DESC_IDVENDOR         0xC251
// Product ID assigned by manufacturer (idProduct).
#define USBD0_DEV_DESC_IDPRODUCT        0xF00A
// Device Release Number in binary-coded decimal (bcdDevice).
#define USBD0_DEV_DESC_BCDDEVICE        0x0100

// Maximum packet size for Endpoint 0 (bMaxPacketSize0).
#define USBD0_MAX_PACKET0               64

// If disabled Serial Number String will not be assigned to USB Device.
#define USBD0_STR_DESC_SER_EN           1

// bmAttributes
#define USBD0_CFG_DESC_BMATTRIBUTES     0x80

// bMaxPower
#define USBD0_CFG_DESC_BMAXPOWER        250


// Interface Number
#define USBD_CUSTOM_CLASS0_IF0_NUM      0

// Alternate Setting
#define USBD_CUSTOM_CLASS0_IF0_ALT      0

// Class Code
#define USBD_CUSTOM_CLASS0_IF0_CLASS    0xFF // 0xFF: Vendor Specific

// Subclass Code
#define USBD_CUSTOM_CLASS0_IF0_SUBCLASS 0x00

// Protocol Code
#define USBD_CUSTOM_CLASS0_IF0_PROTOCOL 0x00





/////////////////////////////////////////////
extern const uint8_t kUSBd0DeviceDescriptor[0x12];
extern const uint8_t kUSBd0InterfaceDescriptor[0x1E];
extern const uint8_t kUSBd0ConfigDescriptor[0x09];
extern const uint8_t kLangDescriptor[0x04];
extern const uint8_t kManufacturerString[0x28];
extern const uint8_t kProductString[0x18];
extern const uint8_t kSerialNumberString[0x1A];
extern const uint8_t kInterfaceString[0x2C];


#endif
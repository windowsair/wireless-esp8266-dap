 ////TODO: refactoring into structure
/**
 * @file USBd_config.c
 * @brief Standard USB Descriptor Definitions
          fix bugs 2020-1-23
 * @version 0.2
 * @date 2020-1-23
 * 
 * 
 */
#include <stdint.h>
#include <stdbool.h>
#include "USBd_config.h"
#include "usb_defs.h"

#define USBShort(ui16Value)     ((ui16Value) & 0xff), ((ui16Value) >> 8) //((ui16Value) & 0xFF),(((ui16Value) >> 8) & 0xFF)


/**
 * @brief step 1. Build Standard Device Descriptor
 * 
 */

// Standard Device Descriptor
const uint8_t kUSBd0DeviceDescriptor[0x12] =
{
    0x12,             // bLength
    USB_DT_DEVICE, // bDescriptorType (constant)
    USBShort(0x0210), // bcdUSB 
    ////TODO: Is it also available elsewhere?

    // We need to use a device other than the USB-IF standard, set to 0x00
    0x00, // bDeviceClass
    0x00, // bDeviceSubClass
    0x00, // bDeviceProtocol
    
    USBD0_MAX_PACKET0,                  // Maximum packet size for default pipe.
    USBShort(USBD0_DEV_DESC_IDVENDOR),  // Vendor ID (VID).
    USBShort(USBD0_DEV_DESC_IDPRODUCT), // Product ID (PID).
    USBShort(USBD0_DEV_DESC_BCDDEVICE), // Device Version BCD.
    0x01,                               // Index of Manufacturer string identifier.
    0x02,                               // Index of Product string identifier.
    0x03 * USBD0_STR_DESC_SER_EN,       // Index of Product serial number.
    0x01                                // Number of configurations.
};




/**
 * @brief step 2. Buid Standard Configuration Descriptor
 * 
 */


// Standard Interface Descriptor
const uint8_t kUSBd0InterfaceDescriptor[0x1E]=
{
    0x09,                                   // bLength
    USB_DT_INTERFACE,                       // bDescriptorType
    USBD_CUSTOM_CLASS0_IF0_NUM,             // bInterfaceNumber                                    
    USBD_CUSTOM_CLASS0_IF0_ALT,             // bAlternateSetting                                     
    0x03,                                   // bNumEndpoints(we will use three endpoints)
                                            // 
    USBD_CUSTOM_CLASS0_IF0_CLASS,           // bInterfaceClass                                      
    USBD_CUSTOM_CLASS0_IF0_SUBCLASS,        // bInterfaceSubClass                                    
    USBD_CUSTOM_CLASS0_IF0_PROTOCOL,        // bInterfaceProtocol
    0x00,                                   // iInterface 
                                            // Index of string descriptor describing this interface
    ////TODO: fix this 0x04 ?


    // Standard Endpoint Descriptor

    // Endpoint 1: Bulk Out – used for commands received from host PC.
    // Endpoint 2: Bulk In – used for responses send to host PC.
    // Endpoint 3: Bulk In (optional) – used for streaming SWO trace
    
    // ATTENTION: 
    // physical endpoint 1 indeed included two "endpoints": Bulk OUT and Bulk IN
    // physical endpoint 1 -> Endpoint 1 & Endpoint 2
    // physical endpoint 2 -> Endpoint 3

    // See also :
    // http://www.keil.com/pack/doc/CMSIS/DAP/html/group__DAP__ConfigUSB__gr.html 
    
    /*                 Pysical endpoint 1                 */

    // "Endpoint 1: Bulk Out – used for commands received from host PC."  PC -> Device
    0x07,                                                      // bLength                                          
    USB_DT_ENDPOINT,                                           // bDescriptorType                                      
    0x01,                                                      // bEndpointAddress
    USB_ENDPOINT_ATTR_BULK,                                    // bmAttributes           
    USBShort(512),    // wMaxPacketSize   
    // We assume that it always runs in High Speed.
    0x00, // bInterval 

    /*                 Pysical endpoint 1                 */
    
    // "Endpoint 2: Bulk In – used for responses send to host PC." Device -> PC
    0x07,                                                      // bLength                                          
    USB_DT_ENDPOINT,                                           // bDescriptorType                                      
    0x81,                                                      // bEndpointAddress
    USB_ENDPOINT_ATTR_BULK,                                    // bmAttributes           
    USBShort(512),    // wMaxPacketSize   
    0x00, // bInterval 

    /*                 Pysical endpoint 2                */
    // "Endpoint 3: Bulk In (optional) – used for streaming SWO trace" Device -> PC
    0x07,                                                      // bLength                                          
    USB_DT_ENDPOINT,                                           // bDescriptorType                                      
    0x82,                                                      // bEndpointAddress
    USB_ENDPOINT_ATTR_BULK,                                    // bmAttributes           
    USBShort(512),    // wMaxPacketSize   
    0x00,                                                      // bInterval 


};

// Standard Configuration Descriptor
#define LENGTHOFCONFIGDESCRIPTOR 9
const uint8_t kUSBd0ConfigDescriptor[LENGTHOFCONFIGDESCRIPTOR] =
{
    // Configuration descriptor header.

    0x09,            // bLength -> 0x09?? may be ok... 1-23
    0x03,                                   // bDescriptorType 
                                            // constant, set to 0x03
                                       
    USBShort((sizeof(kUSBd0InterfaceDescriptor)) + (LENGTHOFCONFIGDESCRIPTOR)),  
                                            // wTotalLength

    0x01,                                   // bNumInterfaces 
                                            // There is only one interface in the CMSIS-DAP project
    0x01,                                   // bConfigurationValue: 0x01 is used to select this configuration */
    0x00,                                   // iConfiguration: no string to describe this configuration */
    USBD0_CFG_DESC_BMATTRIBUTES,            // bmAttributes

    USBD0_CFG_DESC_BMAXPOWER,               // bMaxPower
};



/**
 * @brief step 3. Build String Descriptor
 * 
 */


const uint8_t kLangDescriptor[0x04] =
{
    4,
    USB_DT_STRING,
    USBShort(USB_LANGID_ENGLISH_US)
};


/**
 * @brief We will use these string descriptor:
 *        1. Manufacturer string    -> "KEIL - Tools By ARM"
 *        2. Product string         -> "LPC-Link-II"
 *        3. Serial number string   -> "0001A0000000"
 *        4. Interface string       -> "LPC-Link-II CMSIS-DAP"
 *
 * 
 */

const uint8_t kManufacturerString[0x28] =
{
    0x28, // bLength
    0x03, // bDescriptorType
    // "KEIL - Tools By ARM"
    'K', 0, 'E', 0, 'I', 0, 'L', 0, ' ', 0, '-', 0, ' ', 0, 'T', 0, 'o', 0, 'o', 0,
    'l', 0, 's', 0, ' ', 0, 'B', 0, 'y', 0, ' ', 0, 'A', 0, 'R', 0, 'M', 0
};

const uint8_t kProductString[0x18] =
{
    0x18, // bLength
    0x03, // bDescriptorType
    // "LPC-Link-II"
    'L', 0, 'P', 0, 'C', 0, '-', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0, '-', 0, 'I', 0,
    'I', 0
};

const uint8_t kSerialNumberString[0x1A] = 
{
    0x1A, // bLength
    0x03, // bDescriptorType
    // "0001A0000000"
    '0', 0, '0', 0, '0', 0, '1', 0, 'A', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0,
    '0', 0, '0', 0
};

const uint8_t kInterfaceString[0x2C] =
{
    0x2C, // bLength
    0x03, // bDescriptorType
    // "LPC-Link-II CMSIS-DAP"
    'L', 0, 'P', 0, 'C', 0, '-', 0, 'L', 0, 'i', 0, 'n', 0, 'k', 0, '-', 0, 'I', 0,
    'I', 0, ' ', 0, 'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0,   
    'P', 0
};

const uint8_t * const kUSBd0StringDescriptorsSet[0x05] =
{
    kLangDescriptor,
    kManufacturerString,
    kProductString,
    kSerialNumberString,
    kInterfaceString
};
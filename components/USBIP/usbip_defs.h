/**
 * @file usbip_defs.h
 * @brief Simple modification
 * @version 0.1
 * @date 2020-01-22
 *
 * @copyright Copyright (c) 2020
 *
 */

// Focus on the following structures in this file:
// usbip_stage2_header
// usbip_stage1_response_devlist

//
// Created by thevoidnn on 10/25/17.
//

#ifndef __USBIP_DEFS_H__
#define __USBIP_DEFS_H__

#include <stdint.h>

#include "components/USBIP/usb_defs.h"

#define USBIP_SYSFS_PATH_SIZE 256
#define USBIP_BUSID_SIZE 32

enum usbip_stage1_command
{
    // Offset 2
    USBIP_STAGE1_CMD_DEVICE_LIST = 0x05,   // OP_REQ_DEVLIST
    USBIP_STAGE1_CMD_DEVICE_ATTACH = 0x03, // OP_REQ_IMPORT
};

enum usbip_stager2_command
{
    //Offset 0
    USBIP_STAGE2_REQ_SUBMIT = 0x0001,
    USBIP_STAGE2_REQ_UNLINK = 0x0002,
    USBIP_STAGE2_RSP_SUBMIT = 0x0003,
    USBIP_STAGE2_RSP_UNLINK = 0x0004,
};

enum usbip_stage2_direction
{
    USBIP_DIR_OUT = 0x00,
    USBIP_DIR_IN = 0x01,
};

typedef struct
{
    uint16_t version;
    uint16_t command;
    uint32_t status;
} __attribute__((__packed__)) usbip_stage1_header;
/////////////////////////////////////////////////////////////

// Device description
typedef struct
{
    char path[USBIP_SYSFS_PATH_SIZE];
    char busid[USBIP_BUSID_SIZE];

    uint32_t busnum;
    uint32_t devnum;
    uint32_t speed;

    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;

    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;

    uint8_t bConfigurationValue;
    uint8_t bNumConfigurations;
    uint8_t bNumInterfaces;
} __attribute__((packed)) usbip_stage1_usb_device;

// Interface description
typedef struct
{
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t padding;
} __attribute__((packed)) usbip_stage1_usb_interface;

typedef struct
{
    usbip_stage1_usb_device udev;
    usbip_stage1_usb_interface uinf[];
} __attribute__((packed)) usbip_stage1_response_devlist_entry;

typedef struct
{
    uint32_t list_size;
    usbip_stage1_response_devlist_entry devices[];
} __attribute__((__packed__)) usbip_stage1_response_devlist;

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

/**
 * struct usbip_header_basic - data pertinent to every URB request
 * RESPONSE & REQUEST
 *
 * @command: the usbip request type
 * @seqnum: sequential number that identifies requests; incremented per
 *	    connection
 * @devid: specifies a remote USB device uniquely instead of busnum and devnum;
 *	   in the stub driver, this value is ((busnum << 16) | devnum)
 * @direction: direction of the transfer
 * @ep: endpoint number
 */
typedef struct
{
    uint32_t command;
    uint32_t seqnum;
    uint32_t devid;
    uint32_t direction;
    uint32_t ep;
} __attribute__((packed)) usbip_stage2_header_basic;

/**
 * struct usbip_header_cmd_submit - USBIP_CMD_SUBMIT packet header
 * >>>REQUEST
 *
 * @transfer_flags: URB flags
 * @transfer_buffer_length: the data size for (in) or (out) transfer
 * @start_frame: initial frame for isochronous or interrupt transfers
 * @number_of_packets: number of isochronous packets
 * @interval: maximum time for the request on the server-side host controller
 * @setup: setup data for a control request
 */
typedef struct
{
    uint32_t transfer_flags;
    int32_t data_length;

    /* it is difficult for usbip to sync frames (reserved only?) */
    int32_t start_frame;
    int32_t number_of_packets;
    int32_t interval;

    union {
        uint8_t setup[8];
        usb_standard_request request;
    };
} __attribute__((packed)) usbip_stage2_header_cmd_submit;

/**
 * struct usbip_header_ret_submit - USBIP_RET_SUBMIT packet header
 * <<<RESPONSE
 *
 * @status: return status of a non-iso request
 * @actual_length: number of bytes transferred
 * @start_frame: initial frame for isochronous or interrupt transfers
 * @number_of_packets: number of isochronous packets
 * @error_count: number of errors for isochronous transfers
 */
typedef struct
{
    int32_t status;
    int32_t data_length; //actual_length
    int32_t start_frame;
    int32_t number_of_packets;
    int32_t error_count;
} __attribute__((packed)) usbip_stage2_header_ret_submit;

/**
 * struct usbip_header_cmd_unlink - USBIP_CMD_UNLINK packet header
 * >>>REQUEST
 * @seqnum: the URB seqnum to unlink
 */
typedef struct
{
    uint32_t seqnum;
} __attribute__((packed)) usbip_stage2_header_cmd_unlink;

/**
 * struct usbip_header_ret_unlink - USBIP_RET_UNLINK packet header
 * <<<RESPONSE
 * @status: return status of the request
 */
typedef struct
{
    int32_t status;
    uint8_t padding[24]; // Linux only. For usbip-win, it ignores this field.
} __attribute__((packed)) usbip_stage2_header_ret_unlink;

/**
 * struct usbip_header - common header for all usbip packets
 * @base: the basic header
 * @u: packet type dependent header
 */
typedef struct
{
    usbip_stage2_header_basic base;

    union {
        usbip_stage2_header_cmd_submit cmd_submit;
        usbip_stage2_header_ret_submit ret_submit;
        usbip_stage2_header_cmd_unlink cmd_unlink;
        usbip_stage2_header_ret_unlink ret_unlink;
    } u;
} __attribute__((packed)) usbip_stage2_header;

#endif

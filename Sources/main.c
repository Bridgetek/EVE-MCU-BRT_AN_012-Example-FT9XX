/**
  @file main.c
 */
/*
 * ============================================================================
 * History
 * =======
 * 2017-03-15 : Created
 *
 * (C) Copyright Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 * ("Bridgetek") subject to the licence terms set out
 * http://brtchip.com/BRTSourceCodeLicenseAgreement/ ("the Licence Terms").
 * You must read the Licence Terms before downloading or using the Software.
 * By installing or using the Software you agree to the Licence Terms. If you
 * do not agree to the Licence Terms then do not download or use the Software.
 *
 * Without prejudice to the Licence Terms, here is a summary of some of the key
 * terms of the Licence Terms (and in the event of any conflict between this
 * summary and the Licence Terms then the text of the Licence Terms will
 * prevail).
 *
 * The Software is provided "as is".
 * There are no warranties (or similar) in relation to the quality of the
 * Software. You use it at your own risk.
 * The Software should not be used in, or for, any medical device, system or
 * appliance. There are exclusions of Bridgetek liability for certain types of loss
 * such as: special loss or damage; incidental loss or damage; indirect or
 * consequential loss or damage; loss of income; loss of business; loss of
 * profits; loss of revenue; loss of contracts; business interruption; loss of
 * the use of money or anticipated savings; loss of information; loss of
 * opportunity; loss of goodwill or reputation; and/or loss of, damage to or
 * corruption of data.
 * There is a monetary cap on Bridgetek's liability.
 * The Software may have subsequently been amended by another user and then
 * distributed by that other user ("Adapted Software").  If so that user may
 * have additional licence terms that apply to those amendments. However, Bridgetek
 * has no liability in relation to those amendments.
 * ============================================================================
 */

/* INCLUDES ************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <eve_keyboard.h>

#include <ft900.h>
#include <ft900_uart_simple.h>

#include <ft900_usb.h>
#include <ft900_usbd.h>
#include <ft900_usb_hid.h>
#include <ft900_startup_dfu.h>

#include "EVE_config.h"
#include "EVE.h"

#include "eve_ui.h"

#include "keyboard.h"

/**
 @brief Include a DFU Interface in the configuration.
 @details This adds an interface to the USB configuration descriptor
  to present a DFU option to the host. It will add in WCID descriptors
  so that a WinUSB-based device driver can be used with a DFU program
  such as dfu-util.
 */
#undef USB_INTERFACE_USE_DFU

/**
 @brief Start DFU Interface briefly before the Touch Panel.
 @details This enables the DFU interface to appear for a short period
  of time to present a DFU option to the host. After this, the
  configuration will be removed and the main program's HID configuration
  will start. A DFU program can start when the DFU configuration is
  active and update firmware.
 */
#undef USB_INTERFACE_USE_STARTUPDFU

#define BRIDGE_DEBUG
#ifdef BRIDGE_DEBUG
#define BRIDGE_DEBUG_PRINTF(...) do {printf(__VA_ARGS__);} while (0)
#else
#define BRIDGE_DEBUG_PRINTF(...)
#endif

/* For MikroC const qualifier will place variables in Flash
 * not just make them constant.
 */
#if defined(__GNUC__)
#define DESCRIPTOR_QUALIFIER const
#elif defined(__MIKROC_PRO_FOR_FT90x__)
#define DESCRIPTOR_QUALIFIER data
#endif

/* CONSTANTS ***********************************************************************/

/**
 @name USB and Hub Configuration
 @brief Indication of how the USB device is powered and the size of
 	 the control endpoint Max Packets.
 */
//@{
// USB Bus Powered - set to 1 for self-powered or 0 for bus-powered
#ifndef USB_SELF_POWERED
#define USB_SELF_POWERED 0
#endif // USB_SELF_POWERED
#if USB_SELF_POWERED == 1
#define USB_CONFIG_BMATTRIBUTES_VALUE (USB_CONFIG_BMATTRIBUTES_SELF_POWERED | USB_CONFIG_BMATTRIBUTES_RESERVED_SET_TO_1)
#else // USB_SELF_POWERED
#define USB_CONFIG_BMATTRIBUTES_VALUE USB_CONFIG_BMATTRIBUTES_RESERVED_SET_TO_1
#endif // USB_SELF_POWERED
// USB Endpoint Zero packet size (both must match)
#define USB_CONTROL_EP_MAX_PACKET_SIZE 64
#define USB_CONTROL_EP_SIZE USBD_EP_SIZE_64
//@}


/**
 @name DFU Configuration
 @brief Determines the parts of the DFU specification which are supported
        by the DFU library code. Features can be disabled if required.
 */
//@{
#define DFU_ATTRIBUTES USBD_DFU_ATTRIBUTES
//@}

/**
 @name Device Configuration Areas
 @brief Size and location reserved for string descriptors.
 Leaving the allocation size blank will make an array exactly the size
 of the string allocation.
 Note: Specifying the location is not supported by the GCC compiler.
 */
//@{
// String descriptors - allow a maximum of 256 bytes for this
#define STRING_DESCRIPTOR_LOCATION 0x80
#define STRING_DESCRIPTOR_ALLOCATION 0x100
//@}

/**
 @name DFU_TRANSFER_SIZE definition.
 @brief Number of bytes in block, sent in each DFU_DNLOAD request
 from the DFU update program on the host. This is simplified
 in that the meaning of a block is an arbitrary number of
 bytes. This is intentionally a multiple of the maximum
 packet size for the control endpoints.
 It is used in the DFU functional descriptor as wTransferSize.
 The maximum size supported by the DFU library is 256 bytes
 which is the size of a page of Flash.
 */
//@{
#define DFU_TRANSFER_SIZE USBD_DFU_MAX_BLOCK_SIZE
#define DFU_TIMEOUT USBD_DFU_TIMEOUT
//@}

/**
 @name USB_PID_KEYBOARD configuration.
 @brief Run Time Product ID for Keyboard function.
 */
//@{
#define USB_PID_KEYBOARD 0x0fda
//@}

/**
 @name DFU_USB_PID_DFUMODE configuration.
 @brief FTDI predefined DFU Mode Product ID.
 */
//@{
#define DFU_USB_PID_DFUMODE 0x0fde
//@}

/**
 @name DFU_USB_INTERFACE configuration..
 @brief Run Time and DFU Mode Interface Numbers.
 */
//@{
#define DFU_USB_INTERFACE_RUNTIME 2
#define DFU_USB_INTERFACE_DFUMODE 0
//@}

/**
 @name WCID_VENDOR_REQUEST_CODE for WCID.
 @brief Unique vendor request code for WCID OS Vendor Extension validation.
 */
//@{
#define WCID_VENDOR_REQUEST_CODE	 0xF1
//@}

/**
 @brief Endpoint definitions for HID device.
 */
//@{

/// Interrupt IN endpoint for Boot Interface HID keyboard.
#define HID_KEYBOARD_IN_EP			USBD_EP_1
/// Endpoint size for Boot Interface.
//@{
#define HID_KEYBOARD_IN_EP_SIZE			0x8
#define HID_KEYBOARD_IN_EP_USBD_SIZE	USBD_EP_SIZE_8
//@}
/// Interrupt IN endpoint for System and Consumer Control Interface HID device.
#define HID_CONTROL_IN_EP			USBD_EP_2
/// Endpoint size for System and Consumer Control Interface.
//@{
#define HID_CONTROL_IN_EP_SIZE			0x3
#define HID_CONTROL_IN_EP_USBD_SIZE		USBD_EP_SIZE_8
//@}
/// Endpoint polling intervals for Boot Interface and Consumer Control Interface.
//@{
#define HID_IN_EP_INTERVAL_HS		8 /// 8 ms polling interval
#define HID_IN_EP_INTERVAL_FS		10 /// 10 ms polling interval
//@}
//@}

/**
 @brief Change this value in order to modify the size of the Tx and Rx ring buffers
  used to implement UART buffering.
 */
#define RINGBUFFER_SIZE (128)

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/**
 @name hid_report_descriptor_keyboard
 @brief HID Report descriptor for Boot Report keyboard.

 See Device Class Definition for Human Interface Devices (HID) Version 1.11
 from USB Implementers� Forum USB.org

 0x05, 0x01,             Usage Page: Generic Desktop Controls
 0x09, 0x06,             Usage: Keyboard
 0xA1, 0x01,             Collection: Application
 0x05, 0x07,               Usage Page: Keyboard
 0x19, 0xE0,               Usage Minimum: Keyboard LeftControl
 0x29, 0xE7,               Usage Maximum: Keyboard Right GUI
 0x15, 0x00,               Logical Minimum: 0
 0x25, 0x01,               Logical Maximum: 1
 0x75, 0x01,               Report Size: 1
 0x95, 0x08,               Report Count: 8
 0x81, 0x02,               Input: Data (2)
 0x95, 0x01,               Report Count: 1
 0x75, 0x08,               Report Size: 8
 0x81, 0x01,               Input: Constant (1)
 0x95, 0x03,               Report Count: 3
 0x75, 0x01,               Report Size: 1
 0x05, 0x08,               Usage Page: LEDs
 0x19, 0x01,               Usage Minimum: Num Lock
 0x29, 0x03,               Usage Maximum: Scroll Lock
 0x91, 0x02,               Output: Data (2)
 0x95, 0x05,               Report Count: 5
 0x75, 0x01,               Report Size: 1
 0x91, 0x01,               Output: Constant (1)
 0x95, 0x06,               Report Count: 6
 0x75, 0x08,               Report Size: 8
 0x15, 0x00,               Logical Minimum: 0
 0x26, 0xFF, 0x00,         Logical Maximum: 255
 0x05, 0x07,               Usage Page: Keyboard/Keypad
 0x19, 0x00,               Usage Minimum: 0
 0x2A, 0xFF, 0x00,         Usage Maximum: 255
 0x81, 0x00,               Input: Data (0)
 0xC0                    End collection
 **/
//@{
DESCRIPTOR_QUALIFIER uint8_t hid_report_descriptor_keyboard[] =
{ 0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15,
		0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01, 0x75,
		0x08, 0x81, 0x01, 0x95, 0x03, 0x75, 0x01, 0x05, 0x08, 0x19, 0x01, 0x29,
		0x03, 0x91, 0x02, 0x95, 0x05, 0x75, 0x01, 0x91, 0x01, 0x95, 0x06, 0x75,
		0x08, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x05, 0x07, 0x19, 0x00, 0x2A, 0xFF,
		0x00, 0x81, 0x00, 0xC0, };
//@}

/**
 @name hid_report_descriptor_consumer_control
 @brief HID Report descriptor for Consumer Control Device (Media and Power keys).

 See Device Class Definition for Human Interface Devices (HID) Version 1.11
 from USB Implementers� Forum USB.org

 0x05, 0x01              Usage Page (Generic Desktop)
 0x09, 0x80              Usage (System Control)
 0xA1, 0x01              Collection (Application)
 0x85, 0x01                Report Id (1)
 0x19, 0x81                Usage Minimum (System Power Down)
 0x29, 0x83                Usage Maximum (System Wake Up)
 0x15, 0x00                Logical minimum (0)
 0x25, 0x01                Logical maximum (1)
 0x75, 0x01                Report Size (1)
 0x95, 0x03                Report Count (3)
 0x81, 0x02                Input (Data,Value,Absolute,Bit Field)
 0x95, 0x05                Report Count (5)
 0x81, 0x01                Input (Constant,Array,Absolute,Bit Field)
 0xC0                    End Collection
 0x05, 0x0C              Usage Page (Consumer)
 0x09, 0x01              Usage (Consumer Control)
 0xA1, 0x01              Collection (Application)
 0x85, 0x02                Report Id (2)
 0x19, 0x00                Usage Minimum (Unassigned)
 0x2A, 0x3C, 0x02          Usage Maximum (AC Format)
 0x15, 0x00                Logical minimum (0)
 0x26, 0x3C, 0x02          Logical maximum (572)
 0x95, 0x01                Report Count (1)
 0x75, 0x10                Report Size (16)
 0x81, 0x00                Input (Data,Array,Absolute,Bit Field)
 0xC0,                   End Collection
 **/
//@{
DESCRIPTOR_QUALIFIER uint8_t hid_report_descriptor_control[] =
{ 0x05, 0x01, 0x09, 0x80, 0xA1, 0x01, 0x85, 0x01, 0x19, 0x81, 0x29, 0x83, 0x15,
		0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x03, 0x81, 0x02, 0x95, 0x05, 0x81,
		0x01, 0xC0,
		0x05, 0x0C, 0x09, 0x01, 0xA1, 0x01, 0x85, 0x02, 0x19, 0x00, 0x2A, 0x3C,
		0x02, 0x15, 0x00, 0x26, 0x3C, 0x02, 0x95, 0x01, 0x75, 0x10, 0x81, 0x00,
		0xC0, };
//@}

/**
 @name string_descriptor
 @brief Table of USB String descriptors

 This is placed at a fixed location in the const section allowing
 up-to 256 bytes of string descriptors to be defined. These can be
 modified or replaced by a utility or binary editor without
 requiring a recompilation of the firmware.
 They are placed at offset 0x100 and reserve 0x100 bytes.
 The data is not stored in section and is therefore
 regarded as const.
 */
//@{
#define UNICODE_LEN(A) (((A * 2) + 2) | (USB_DESCRIPTOR_TYPE_STRING << 8))

const uint16_t string_descriptor[STRING_DESCRIPTOR_ALLOCATION/sizeof(uint16_t)] =
{
		UNICODE_LEN(1), 0x0409, // 0409 = English (US)
		// String 1 (Manufacturer): "FTDI"
		UNICODE_LEN(4), L'F', L'T', L'D', L'I',
		// String 2 (Product): Depends on settings
		UNICODE_LEN(26), L'B', L'R', L'T', L'_', L'A', L'N', L'_', L'x', L'x', L'x', L' ', L'H', L'I', L'D', L' ', L'T', L'o', L'u', L'c', L'h', L' ', L'P', L'a', L'n', L'e', L'l',
		// String 3 (Serial Number):
		UNICODE_LEN(8), L'F', L'T', L'0', L'0', L'0', L'0', L'0', L'0',
		// String 4 (DFU Product Name): "FT900 DFU Mode"
		UNICODE_LEN(14), L'F', L'T', L'9', L'0', L'0', L' ', L'D', L'F', L'U', L' ', L'M', L'o', L'd', L'e',
		// String 5 (Interface Name): "DFU Interface"
		UNICODE_LEN(13), L'D', L'F', L'U', L' ', L'I', L'n', L't', L'e', L'r', L'f', L'a', L'c', L'e',
		// END OF STRINGS
		0x0000,
};
//@}

/**
 @name wcid_string_descriptor
 @brief USB String descriptor for WCID identification.
 */
DESCRIPTOR_QUALIFIER uint8_t wcid_string_descriptor[USB_MICROSOFT_WCID_STRING_LENGTH] = {
		USB_MICROSOFT_WCID_STRING(WCID_VENDOR_REQUEST_CODE)
};

/**
 @name device_descriptor_panel
 @brief Device descriptor for Run Time mode.
 */
DESCRIPTOR_QUALIFIER USB_device_descriptor device_descriptor_panel =
{
		sizeof(USB_device_descriptor), /* bLength */
		USB_DESCRIPTOR_TYPE_DEVICE, /* bDescriptorType */
		USB_BCD_VERSION_2_0, /* bcdUSB */          // V2.0
		USB_CLASS_DEVICE, /* bDeviceClass */       // Defined in interface
		USB_SUBCLASS_DEVICE, /* bDeviceSubClass */ // Defined in interface
		USB_PROTOCOL_DEVICE, /* bDeviceProtocol */ // Defined in interface
		USB_CONTROL_EP_MAX_PACKET_SIZE, /* bMaxPacketSize0 */
		USB_VID_FTDI, /* idVendor */   // idVendor: 0x0403 (FTDI)
		USB_PID_KEYBOARD, /* idProduct */ // idProduct: 0x0fd5
		0x0101, /* bcdDevice */        // 1.1
		0x01, /* iManufacturer */      // Manufacturer
		0x02, /* iProduct */           // Product
		0x03, /* iSerialNumber */      // Serial Number
		0x01, /* bNumConfigurations */
};

/**
 @name device_qualifier_descriptor_panel
 @brief Device qualifier descriptor for Run Time mode.
 */
DESCRIPTOR_QUALIFIER USB_device_qualifier_descriptor device_qualifier_descriptor_panel =
{
		sizeof(USB_device_qualifier_descriptor), /* bLength */
		USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER, /* bDescriptorType */
		USB_BCD_VERSION_2_0, /* bcdUSB */          // V2.0
		USB_CLASS_MISCELLANEOUS, /* bDeviceClass */       // Defined in interface
		USB_SUBCLASS_COMMON_CLASS, /* bDeviceSubClass */ // Defined in interface
		USB_PROTOCOL_INTERFACE_ASSOCIATION, /* bDeviceProtocol */ // Defined in interface
		USB_CONTROL_EP_MAX_PACKET_SIZE, /* bMaxPacketSize0 */
		1, /* bNumConfigurations */
		0
};

/**
 @brief Configuration descriptor - High Speed and Full Speed
 */
struct PACK config_descriptor_panel
{
	USB_configuration_descriptor configuration;
	USB_interface_descriptor boot_report_interface;
	USB_hid_descriptor boot_report_hid;
	USB_endpoint_descriptor boot_report_endpoint;
	USB_interface_descriptor consumer_control_interface;
	USB_hid_descriptor consumer_control_hid;
	USB_endpoint_descriptor consumer_control_endpoint;
#ifdef USB_INTERFACE_USE_DFU
	USB_interface_descriptor dfu_interface;
	USB_dfu_functional_descriptor dfu_functional;
#endif // USB_INTERFACE_USE_DFU
};


/**
 @brief Configuration descriptor declaration and initialisation.
 */
DESCRIPTOR_QUALIFIER struct config_descriptor_panel config_descriptor_panel_hs =
{
		{
				sizeof(USB_configuration_descriptor), /* configuration.bLength */
				USB_DESCRIPTOR_TYPE_CONFIGURATION, /* configuration.bDescriptorType */
				sizeof(struct config_descriptor_panel), /* configuration.wTotalLength */
#ifdef USB_INTERFACE_USE_DFU
				0x03, /* configuration.bNumInterfaces */
#else
				0x02,
#endif
				0x01, /* configuration.bConfigurationValue */
				0x00, /* configuration.iConfiguration */
				USB_CONFIG_BMATTRIBUTES_VALUE, /* configuration.bmAttributes */
				0xFA /* configuration.bMaxPower */           // 500mA
		},

		// ---- INTERFACE DESCRIPTOR for Boot Report Keyboard ----
		{
				sizeof(USB_interface_descriptor), /* interface.bLength */
				USB_DESCRIPTOR_TYPE_INTERFACE, /* interface.bDescriptorType */
				0, /* interface.bInterfaceNumber */
				0x00, /* interface.bAlternateSetting */
				0x01, /* interface.bNumEndpoints */
				USB_CLASS_HID, /* interface.bInterfaceClass */ // HID Class
				USB_SUBCLASS_HID_BOOT_INTERFACE, /* interface.bInterfaceSubClass */ // Boot Protocol
				USB_PROTOCOL_HID_KEYBOARD, /* interface.bInterfaceProtocol */ // Keyboard
				0x02 /* interface.iInterface */               // "FT900 Keyboard"
		},

		// ---- HID DESCRIPTOR for Boot Report Keyboard ----
		{
				0x09, /* hid.bLength */
				USB_DESCRIPTOR_TYPE_HID, /* hid.bDescriptorType */
				USB_BCD_VERSION_HID_1_1 | 1, /* hid.bcdHID */
				USB_HID_LANG_NOT_SUPPORTED, /* hid.bCountryCode */
				0x01, /* hid.bNumDescriptors */
				USB_DESCRIPTOR_TYPE_REPORT, /* hid.bDescriptorType_0 */
				sizeof(hid_report_descriptor_keyboard) /* hid.wDescriptorLength_0 */
		},

		// ---- ENDPOINT DESCRIPTOR for Boot Report Keyboard ----
		{
				sizeof(USB_endpoint_descriptor), /* endpoint.bLength */
				USB_DESCRIPTOR_TYPE_ENDPOINT, /* endpoint.bDescriptorType */
				USB_ENDPOINT_DESCRIPTOR_EPADDR_IN | HID_KEYBOARD_IN_EP, /* endpoint.bEndpointAddress */
				USB_ENDPOINT_DESCRIPTOR_ATTR_INTERRUPT, /* endpoint.bmAttributes */
				HID_KEYBOARD_IN_EP_SIZE, /* endpoint.wMaxPacketSize */
				HID_IN_EP_INTERVAL_HS /* endpoint.bInterval */
		},

		// ---- INTERFACE DESCRIPTOR for Consumer Control Device ----
		{
				sizeof(USB_interface_descriptor), /* interface.bLength */
				USB_DESCRIPTOR_TYPE_INTERFACE, /* interface.bDescriptorType */
				1, /* interface.bInterfaceNumber */
				0x00, /* interface.bAlternateSetting */
				0x01, /* interface.bNumEndpoints */
				USB_CLASS_HID, /* interface.bInterfaceClass */ // HID Class
				USB_SUBCLASS_HID_NONE, /* interface.bInterfaceSubClass */ // None
				USB_PROTOCOL_HID_NONE, /* interface.bInterfaceProtocol */ // None
				0x00 /* interface.iInterface */               // None
		},

		// ---- HID DESCRIPTOR for Consumer Control Device  ----
		{
				0x09, /* hid.bLength */
				USB_DESCRIPTOR_TYPE_HID, /* hid.bDescriptorType */
				USB_BCD_VERSION_HID_1_1 | 1, /* hid.bcdHID */
				USB_HID_LANG_NOT_SUPPORTED, /* hid.bCountryCode */
				0x01, /* hid.bNumDescriptors */
				USB_DESCRIPTOR_TYPE_REPORT, /* hid.bDescriptorType_0 */
				sizeof(hid_report_descriptor_control) /* hid.wDescriptorLength_0 */
		},

		// ---- ENDPOINT DESCRIPTOR for Consumer Control Device  ----
		{
				sizeof(USB_endpoint_descriptor), /* endpoint.bLength */
				USB_DESCRIPTOR_TYPE_ENDPOINT, /* endpoint.bDescriptorType */
				USB_ENDPOINT_DESCRIPTOR_EPADDR_IN | HID_CONTROL_IN_EP, /* endpoint.bEndpointAddress */
				USB_ENDPOINT_DESCRIPTOR_ATTR_INTERRUPT, /* endpoint.bmAttributes */
				HID_CONTROL_IN_EP_SIZE, /* endpoint.wMaxPacketSize */
				HID_IN_EP_INTERVAL_HS /* endpoint.bInterval */
		},
#ifdef USB_INTERFACE_USE_DFU
		{
				sizeof(USB_interface_descriptor), /* dfu_interface.bLength */
				USB_DESCRIPTOR_TYPE_INTERFACE, /* dfu_interface.bDescriptorType */
				DFU_USB_INTERFACE_RUNTIME, /* dfu_interface.bInterfaceNumber */
				0x00, /* dfu_interface.bAlternateSetting */
				0x00, /* dfu_interface.bNumEndpoints */
				USB_CLASS_APPLICATION, /* dfu_interface.bInterfaceClass */ // bInterfaceClass: Application Specific Class
				USB_SUBCLASS_DFU, /* dfu_interface.bInterfaceSubClass */ // bInterfaceSubClass: Device Firmware Update
				USB_PROTOCOL_DFU_RUNTIME, /* dfu_interface.bInterfaceProtocol */ // bInterfaceProtocol: Runtime Protocol
				0x05 /* dfu_interface.iInterface */       // * iInterface: "DFU Interface"
		},

		// ---- FUNCTIONAL DESCRIPTOR for DFU Interface ----
		{
				sizeof(USB_dfu_functional_descriptor), /* dfu_functional.bLength */
				USB_DESCRIPTOR_TYPE_DFU_FUNCTIONAL, /* dfu_functional.bDescriptorType */
				DFU_ATTRIBUTES, /* dfu_functional.bmAttributes */  	// bmAttributes
				DFU_TIMEOUT, /* dfu_functional.wDetatchTimeOut */ // wDetatchTimeOut
				DFU_TRANSFER_SIZE, /* dfu_functional.wTransferSize */     // wTransferSize
				USB_BCD_VERSION_DFU_1_1 /* dfu_functional.bcdDfuVersion */ // bcdDfuVersion: DFU Version 1.1
		}
#endif // USB_INTERFACE_USE_DFU
};

DESCRIPTOR_QUALIFIER struct config_descriptor_panel config_descriptor_panel_fs =
{
		{
				sizeof(USB_configuration_descriptor), /* configuration.bLength */
				USB_DESCRIPTOR_TYPE_CONFIGURATION, /* configuration.bDescriptorType */
				sizeof(struct config_descriptor_panel), /* configuration.wTotalLength */
#ifdef USB_INTERFACE_USE_DFU
				0x03, /* configuration.bNumInterfaces */
#else
				0x02,
#endif
				0x01, /* configuration.bConfigurationValue */
				0x00, /* configuration.iConfiguration */
				USB_CONFIG_BMATTRIBUTES_VALUE, /* configuration.bmAttributes */
				0xFA /* configuration.bMaxPower */           // 500mA
		},

		// ---- INTERFACE DESCRIPTOR for Keyboard ----
		{
				sizeof(USB_interface_descriptor), /* interface.bLength */
				USB_DESCRIPTOR_TYPE_INTERFACE, /* interface.bDescriptorType */
				0, /* interface.bInterfaceNumber */
				0x00, /* interface.bAlternateSetting */
				0x01, /* interface.bNumEndpoints */
				USB_CLASS_HID, /* interface.bInterfaceClass */ // HID Class
				USB_SUBCLASS_HID_BOOT_INTERFACE, /* interface.bInterfaceSubClass */ // Boot Protocol
				USB_PROTOCOL_HID_KEYBOARD, /* interface.bInterfaceProtocol */ // Keyboard
				0x02 /* interface.iInterface */               // "FT900 Keyboard"
		},

		// ---- HID DESCRIPTOR for Keyboard ----
		{
				0x09, /* hid.bLength */
				USB_DESCRIPTOR_TYPE_HID, /* hid.bDescriptorType */
				USB_BCD_VERSION_HID_1_1 | 1, /* hid.bcdHID */
				USB_HID_LANG_NOT_SUPPORTED, /* hid.bCountryCode */
				0x01, /* hid.bNumDescriptors */
				USB_DESCRIPTOR_TYPE_REPORT, /* hid.bDescriptorType_0 */
				sizeof(hid_report_descriptor_keyboard) /* hid.wDescriptorLength_0 */
		},

		// ---- ENDPOINT DESCRIPTOR for Keyboard ----
		{
				sizeof(USB_endpoint_descriptor), /* endpoint.bLength */
				USB_DESCRIPTOR_TYPE_ENDPOINT, /* endpoint.bDescriptorType */
				USB_ENDPOINT_DESCRIPTOR_EPADDR_IN | HID_KEYBOARD_IN_EP, /* endpoint.bEndpointAddress */
				USB_ENDPOINT_DESCRIPTOR_ATTR_INTERRUPT, /* endpoint.bmAttributes */
				HID_KEYBOARD_IN_EP_SIZE, /* endpoint.wMaxPacketSize */
				HID_IN_EP_INTERVAL_FS /* endpoint.bInterval */
		},

		// ---- INTERFACE DESCRIPTOR for Consumer Control Device ----
		{
				sizeof(USB_interface_descriptor), /* interface.bLength */
				USB_DESCRIPTOR_TYPE_INTERFACE, /* interface.bDescriptorType */
				1, /* interface.bInterfaceNumber */
				0x00, /* interface.bAlternateSetting */
				0x01, /* interface.bNumEndpoints */
				USB_CLASS_HID, /* interface.bInterfaceClass */ // HID Class
				USB_SUBCLASS_HID_NONE, /* interface.bInterfaceSubClass */ // None
				USB_PROTOCOL_HID_NONE, /* interface.bInterfaceProtocol */ // None
				0x00 /* interface.iInterface */               // None
		},

		// ---- HID DESCRIPTOR for Consumer Control Device  ----
		{
				0x09, /* hid.bLength */
				USB_DESCRIPTOR_TYPE_HID, /* hid.bDescriptorType */
				USB_BCD_VERSION_HID_1_1 | 1, /* hid.bcdHID */
				USB_HID_LANG_NOT_SUPPORTED, /* hid.bCountryCode */
				0x01, /* hid.bNumDescriptors */
				USB_DESCRIPTOR_TYPE_REPORT, /* hid.bDescriptorType_0 */
				sizeof(hid_report_descriptor_control) /* hid.wDescriptorLength_0 */
		},

		// ---- ENDPOINT DESCRIPTOR for Consumer Control Device  ----
		{
				sizeof(USB_endpoint_descriptor), /* endpoint.bLength */
				USB_DESCRIPTOR_TYPE_ENDPOINT, /* endpoint.bDescriptorType */
				USB_ENDPOINT_DESCRIPTOR_EPADDR_IN | HID_CONTROL_IN_EP, /* endpoint.bEndpointAddress */
				USB_ENDPOINT_DESCRIPTOR_ATTR_INTERRUPT, /* endpoint.bmAttributes */
				HID_CONTROL_IN_EP_SIZE, /* endpoint.wMaxPacketSize */
				HID_IN_EP_INTERVAL_FS /* endpoint.bInterval */
		},

#ifdef USB_INTERFACE_USE_DFU
		{
				sizeof(USB_interface_descriptor), /* dfu_interface.bLength */
				USB_DESCRIPTOR_TYPE_INTERFACE, /* dfu_interface.bDescriptorType */
				DFU_USB_INTERFACE_RUNTIME, /* dfu_interface.bInterfaceNumber */
				0x00, /* dfu_interface.bAlternateSetting */
				0x00, /* dfu_interface.bNumEndpoints */
				USB_CLASS_APPLICATION, /* dfu_interface.bInterfaceClass */ // bInterfaceClass: Application Specific Class
				USB_SUBCLASS_DFU, /* dfu_interface.bInterfaceSubClass */ // bInterfaceSubClass: Device Firmware Update
				USB_PROTOCOL_DFU_RUNTIME, /* dfu_interface.bInterfaceProtocol */ // bInterfaceProtocol: Runtime Protocol
				0x05 /* dfu_interface.iInterface */       // * iInterface: "DFU Interface"
		},

		// ---- FUNCTIONAL DESCRIPTOR for DFU Interface ----
		{
				sizeof(USB_dfu_functional_descriptor), /* dfu_functional.bLength */
				USB_DESCRIPTOR_TYPE_DFU_FUNCTIONAL, /* dfu_functional.bDescriptorType */
				DFU_ATTRIBUTES, /* dfu_functional.bmAttributes */  	// bmAttributes
				DFU_TIMEOUT, /* dfu_functional.wDetatchTimeOut */ // wDetatchTimeOut
				DFU_TRANSFER_SIZE, /* dfu_functional.wTransferSize */     // wTransferSize
				USB_BCD_VERSION_DFU_1_1 /* dfu_functional.bcdDfuVersion */ // bcdDfuVersion: DFU Version 1.1
		}
#endif // USB_INTERFACE_USE_DFU
};
//@}

#ifdef USB_INTERFACE_USE_DFU
/**
 @name wcid_feature_runtime
 @brief WCID Compatible ID for DFU interface in runtime.
 */
//@{
DESCRIPTOR_QUALIFIER USB_WCID_feature_descriptor wcid_feature_runtime =
{
		sizeof(struct USB_WCID_feature_descriptor), /* dwLength */
		USB_MICROSOFT_WCID_VERSION, /* bcdVersion */
		USB_MICROSOFT_WCID_FEATURE_WINDEX_COMPAT_ID, /* wIndex */
		1, /* bCount */
		{0, 0, 0, 0, 0, 0, 0,}, /* rsv1 */
		DFU_USB_INTERFACE_RUNTIME, /* bFirstInterfaceNumber */
		1, /* rsv2 - set to 1 */
		{'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,}, /* compatibleID[8] */
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, /* subCompatibleID[8] */
		{0, 0, 0, 0, 0, 0,}, /* rsv3[6] */
};
#endif // USB_INTERFACE_USE_DFU

/**
 @name device_descriptor_dfumode
 @brief Device descriptor for DFU Mode.
 */
DESCRIPTOR_QUALIFIER USB_device_descriptor device_descriptor_dfumode =
{
		0x12, /* bLength */
		USB_DESCRIPTOR_TYPE_DEVICE, /* bDescriptorType */
		USB_BCD_VERSION_2_0, /* bcdUSB */          // V2.00
		USB_CLASS_DEVICE, /* bDeviceClass */       // Defined in interface
		USB_SUBCLASS_DEVICE, /* bDeviceSubClass */ // Defined in interface
		USB_PROTOCOL_DEVICE, /* bDeviceProtocol */ // Defined in interface
		USB_CONTROL_EP_MAX_PACKET_SIZE, /* bMaxPacketSize0 */    // 8
		USB_VID_FTDI, /* idVendor */   // idVendor: 0x0403 (FTDI)
		DFU_USB_PID_DFUMODE, /* idProduct */ // idProduct: 0x0fee
		0x0101, /* bcdDevice */        // 1.1
		0x01, /* iManufacturer */      // Manufacturer
		0x04, /* iProduct */           // Product
		0x03, /* iSerialNumber */      // Serial Number
		0x01, /* bNumConfigurations */
};

/**
 @name config_descriptor_dfumode
 @brief Config descriptor for DFU Mode.
 */
//@{
struct PACK config_descriptor_dfumode
{
	USB_configuration_descriptor configuration;
	USB_interface_descriptor dfu_interface;
	USB_dfu_functional_descriptor dfu_functional;
};

DESCRIPTOR_QUALIFIER struct config_descriptor_dfumode config_descriptor_dfumode =
{
		{
				0x09, /* configuration.bLength */
				USB_DESCRIPTOR_TYPE_CONFIGURATION, /* configuration.bDescriptorType */
				sizeof(struct config_descriptor_dfumode), /* configuration.wTotalLength */
				0x01, /* configuration.bNumInterfaces */
				0x01, /* configuration.bConfigurationValue */
				0x00, /* configuration.iConfiguration */
				USB_CONFIG_BMATTRIBUTES_VALUE, /* configuration.bmAttributes */
				0xFA /* configuration.bMaxPower */ // 500mA
		},

		// ---- INTERFACE DESCRIPTOR for DFU Interface ----
		{
				0x09, /* dfu_interface.bLength */
				USB_DESCRIPTOR_TYPE_INTERFACE, /* dfu_interface.bDescriptorType */
				DFU_USB_INTERFACE_DFUMODE, /* dfu_interface.bInterfaceNumber */
				0x00, /* dfu_interface.bAlternateSetting */
				0x00, /* dfu_interface.bNumEndpoints */
				USB_CLASS_APPLICATION, /* dfu_interface.bInterfaceClass */ // bInterfaceClass: Application Specific Class
				USB_SUBCLASS_DFU, /* dfu_interface.bInterfaceSubClass */ // bInterfaceSubClass: Device Firmware Update
				USB_PROTOCOL_DFU_DFUMODE, /* dfu_interface.bInterfaceProtocol */ // bInterfaceProtocol: Runtime Protocol
				0x05 /* dfu_interface.iInterface */
		},

		// ---- FUNCTIONAL DESCRIPTOR for DFU Interface ----
		{
				0x09, /* dfu_functional.bLength */
				USB_DESCRIPTOR_TYPE_DFU_FUNCTIONAL, /* dfu_functional.bDescriptorType */
				DFU_ATTRIBUTES, /* dfu_functional.bmAttributes */  	// bmAttributes
				DFU_TIMEOUT, /* dfu_functional.wDetatchTimeOut */ // wDetatchTimeOut
				DFU_TRANSFER_SIZE, /* dfu_functional.wTransferSize */     // wTransferSize
				USB_BCD_VERSION_DFU_1_1 /* dfu_functional.bcdDfuVersion */ // bcdDfuVersion: DFU Version 1.1
		}
};
//@}

/**
 @name wcid_feature_dfumode
 @brief WCID Compatible ID for DFU interface in DFU mode.
 */
//@{
DESCRIPTOR_QUALIFIER USB_WCID_feature_descriptor wcid_feature_dfumode =
{
		sizeof(struct USB_WCID_feature_descriptor), /* dwLength */
		USB_MICROSOFT_WCID_VERSION, /* bcdVersion */
		USB_MICROSOFT_WCID_FEATURE_WINDEX_COMPAT_ID, /* wIndex */
		1, /* bCount */
		{0, 0, 0, 0, 0, 0, 0,}, /* rsv1 */
		DFU_USB_INTERFACE_DFUMODE, /* bFirstInterfaceNumber */
		1, /* rsv2 - set to 1 */
		{'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,}, /* compatibleID[8] */
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,}, /* subCompatibleID[8] */
		{0, 0, 0, 0, 0, 0,}, /* rsv3[6] */
};

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/**
 @brief Millisecond counter
 @details Count-up timer to provide the elapsed time for network operations.
 */
static uint32_t milliseconds = 0;

/**
 @brief Active Alternate Setting
 @details Current active alternate setting for the USB interface.
 */
static uint8_t usb_alt = 0;

/**
 @brief Storage for Configuration Descriptors.
 @details Configuration descriptors may need to be modified to turn from type
 USB_DESCRIPTOR_TYPE_CONFIGURATION to USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION.
 */
union config_descriptor_buffer {
	struct config_descriptor_panel hs;
	struct config_descriptor_panel fs;
} config_descriptor_buffer;

#if 0
/**
 @name UART ring buffer variables.
 @brief Variables required to implement an Rx ring buffer for the UART.
 */
//@{
static uint8_t uart0BufferIn_data[RINGBUFFER_SIZE];
static volatile uint16_t uart0BufferIn_wr_idx = 0;
static volatile uint16_t uart0BufferIn_rd_idx = 0;
static volatile uint16_t uart0BufferIn_avail = RINGBUFFER_SIZE;
//@}
#endif

/**
 @breif Keyboard State Machine
 */
enum {
	keyboard_msg_idle, // No message to send (in Key Up state).
	keyboard_msg_waiting, // Message waiting to send.
	keyboard_msg_report_keydown, // Report for Key Down ready to send.
	keyboard_msg_sending, // Message in progress (in Key Down state).
	keyboard_msg_report_keyup, // Report for Key Up ready to send.
} keyboard_msg_state = keyboard_msg_idle;

/**
 @brief Keyboard report buffers
 @details Current state of keyboard report buffers. IN is keyscan information to
 host and OUT is LED information from host.
 */
//@{
keyboard_in_report_structure_t keyboard_report_in;
keyboard_out_report_structure_t keyboard_report_out;
control_in_report_structure_t control_report_in;
//@}

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

#if 0
/** @name my_putc
 *  @details Machine dependent putc function at Interrupt level.
 *  @param c The character to write.
 */
int8_t my_putc_isr(char c)
{
	int8_t copied = -1;

	// Do not overwrite data already in the buffer.
	if (uart0BufferIn_avail > 0)
	{
		uart0BufferIn_avail--;

		uart0BufferIn_data[uart0BufferIn_wr_idx] = c;

		/* Increment the pointer and wrap around */
		uart0BufferIn_wr_idx++;
		if (uart0BufferIn_wr_idx == RINGBUFFER_SIZE) uart0BufferIn_wr_idx = 0;

		copied = 0;
	}

	/* Report back how many bytes have been copied into the buffer...*/
	return copied;
}

/** @name my_putc
 *  @details Machine dependent putc function.
 *  @param c The character to write.
 */
int8_t my_putc(char c)
{
	int8_t copied;

	CRITICAL_SECTION_BEGIN
	copied = my_putc_isr(c);
	CRITICAL_SECTION_END

	/* Report back how many bytes have been copied into the buffer...*/
	return copied;
}

/** @name my_getc
 *  @details Machine dependent getc function/
 *  @param p Parameters (machine dependent)
 *  @param c The character to read.
 */
int8_t my_getc(char *c)
{
	int8_t copied = -1;

	CRITICAL_SECTION_BEGIN
	if (uart0BufferIn_avail < RINGBUFFER_SIZE)
	{
		uart0BufferIn_avail++;

		*c = uart0BufferIn_data[uart0BufferIn_rd_idx];

		/* Increment the pointer and wrap around */
		uart0BufferIn_rd_idx++;
		if (uart0BufferIn_rd_idx == RINGBUFFER_SIZE) uart0BufferIn_rd_idx = 0;

		copied = 0;
	}
	CRITICAL_SECTION_END

	/* Report back how many bytes have been copied into the buffer...*/
	return copied;
}

/**
 The Interrupt that allows echoing back over UART0
 */
/**
 The Interrupt which handles asynchronous transmission and reception
 of data into the ring buffer
 */
void uart0ISR()
{
	static uint8_t c;

	/* Receive interrupt... */
	if (uart_is_interrupted(UART0, uart_interrupt_rx))
	{
		/* Read a byte into the Ring Buffer... */
		uart_read(UART0, &c);
		my_putc_isr(c);
	}
}
#endif

/** @brief Returns the current millisecond country
 *  @returns A count of milliseconds
 */
uint32_t millis(void)
{
	return milliseconds;
}

void timer_ISR(void)
{
	if (timer_is_interrupted(timer_select_a))
	{
		milliseconds++;
		keyboard_timer();
	}

	if (timer_is_interrupted(timer_select_b))
	{
		//eve_tick();
	}
}

/* Power management ISR */
void powermanagement_ISR(void)
{
	if (SYS->PMCFG_H & MASK_SYS_PMCFG_HOST_RST_DEV)
	{
		// Clear Host Reset interrupt
		SYS->PMCFG_H = MASK_SYS_PMCFG_HOST_RST_DEV;
		USBD_wakeup();
	}

	if (SYS->PMCFG_H & MASK_SYS_PMCFG_HOST_RESUME_DEV)
	{
		// Clear Host Resume interrupt
		SYS->PMCFG_H = MASK_SYS_PMCFG_HOST_RESUME_DEV;
		USBD_resume();
	}
}

/**
 @brief      USB Set/Get Interface request handler
 @details    Handle standard requests from the host application
 for GetInterface and SetInterface requests.
 @param[in]	req - USB_device_request structure containing the
 SETUP portion of the request from the host.
 @return		status - USBD_OK if successful or USBD_ERR_*
 if there is an error or the bRequest is not
 supported.
 **/
//@{
int8_t setif_req_cb(USB_device_request *req)
{
	int8_t status = USBD_ERR_NOT_SUPPORTED;

	if (req->wIndex == 0)
	{
		// Interface 0 can only have an Alt Setting of zero.
		if (req->wValue == 0x00)
		{
			status = USBD_OK;
		}
	}

	return status;
}

int8_t getif_req_cb(USB_device_request *req, uint8_t *val)
{
	int8_t status = USBD_ERR_NOT_SUPPORTED;

	// Only interface 1 has Alt Settings in this application.
	if (req->wIndex == 0)
	{
		// Alt setting for interface 0 is always zero.
		*val = 0;
		status = USBD_OK;
	}

	return status;
}
//@}

int8_t class_req_interface_hid(USB_device_request *req)
{
	uint8_t requestType = req->bmRequestType;
	uint8_t interface = LSB(req->wIndex) & 0x0F;
	int8_t status = USBD_ERR_NOT_SUPPORTED;

	uint8_t protocol;
	uint8_t idle;

	(void)requestType;

	// Handle HID class requests
	switch (req->bRequest)
	{
	case USB_HID_REQUEST_CODE_SET_IDLE:
		status = keyboard_req_set_idle(interface, req->wValue >> 8);
		if (status == USBD_OK)
		{
			// ACK packet
			USBD_transfer_ep0(USBD_DIR_IN, NULL, 0, 0);
		}
		break;

	case USB_HID_REQUEST_CODE_GET_IDLE:
		status = keyboard_req_get_idle(interface, &idle);
		if (status == USBD_OK)
		{
			USBD_transfer_ep0(USBD_DIR_IN, &idle, 1, 1);
			// ACK packet
			USBD_transfer_ep0(USBD_DIR_OUT, NULL, 0, 0);
		}
		break;

	case USB_HID_REQUEST_CODE_SET_PROTOCOL:
		status = keyboard_req_set_protocol(interface, req->wValue >> 8);
		if (status == USBD_OK)
		{
			// ACK packet
			USBD_transfer_ep0(USBD_DIR_IN, NULL, 0, 0);
		}
		break;

	case USB_HID_REQUEST_CODE_GET_PROTOCOL:
		status = keyboard_req_get_protocol(interface, &protocol);
		if (status == USBD_OK)
		{
			USBD_transfer_ep0(USBD_DIR_IN, &protocol, 1, 1);
			// ACK packet
			USBD_transfer_ep0(USBD_DIR_OUT, NULL, 0, 0);
		}
		break;

	case USB_HID_REQUEST_CODE_GET_REPORT:
		// Do not support a GetReport request from the host on the control endpoints.
#if 0
		if (eve_ui_keyboard_loop(&keyboard_report_in, &keyboard_report_out, &control_report_in) != -1)
		{
			// There is a keypress ready to send.
			keyboard_msg_state = keyboard_msg_waiting;
		}

		// ACK packet
		USBD_transfer_ep0(USBD_DIR_OUT, NULL, 0, 0);
#endif // 0
		break;

	case USB_HID_REQUEST_CODE_SET_REPORT:
		if (interface == config_descriptor_panel_hs.boot_report_interface.bInterfaceNumber)
		{
			USBD_transfer_ep0(USBD_DIR_OUT, (uint8_t *)&keyboard_report_out, sizeof(keyboard_out_report_structure_t), sizeof(keyboard_out_report_structure_t));

			keyboard_req_set_report(interface, &keyboard_report_out);
			// ACK packet
			USBD_transfer_ep0(USBD_DIR_IN, NULL, 0, 0);
			// There is a keypress ready to send.
			keyboard_msg_state = keyboard_msg_waiting;
		}
		else
		{
			return USBD_ERR_INVALID_PARAMETER;
		}

		break;
	}
	return USBD_OK;
}

int8_t class_req_endpoint_hid_1(USB_device_request *req)
{
	return USBD_OK;
}

/**
 @brief      USB DFU class request handler (runtime)
 @details    Handle class requests from the host application.
 @param[in]	req - USB_device_request structure containing the
 SETUP portion of the request from the host.
 @return		status - USBD_OK if successful or USBD_ERR_*
 if there is an error or the bRequest is not
 supported.
 **/
int8_t class_req_dfu_interface_runtime(USB_device_request *req)
{
	int8_t status = USBD_ERR_NOT_SUPPORTED;

	// Handle only DFU_DETATCH, DFU_GETSTATE and DFU_GETSTATUS
	// when in Runtime mode. Table 3.2 DFU_DETACH is mandatory
	// in Runtime mode, DFU_GETSTATE and DFU_GETSTATUS are
	// optional.
	switch (req->bRequest)
	{
	case USB_CLASS_REQUEST_DETACH:
		USBD_DFU_class_req_detach(req->wValue);
		status = USBD_OK;
		break;
	case USB_CLASS_REQUEST_GETSTATUS:
		USBD_DFU_class_req_getstatus(req->wLength);
		status = USBD_OK;
		break;
	case USB_CLASS_REQUEST_GETSTATE:
		USBD_DFU_class_req_getstate(req->wLength);
		status = USBD_OK;
		break;
	}
	return status;
}

/**
 @brief      USB DFU class request handler (DFU mode)
 @details    Handle class requests from the host application.
 @param[in]	req - USB_device_request structure containing the
 SETUP portion of the request from the host.
 @return		status - USBD_OK if successful or USBD_ERR_*
 if there is an error or the bRequest is not
 supported.
 **/
int8_t class_req_dfu_interface_dfumode(USB_device_request *req)
{
	int8_t status = USBD_ERR_NOT_SUPPORTED;

	// Handle remaining DFU class requests when in DFU Mode.
	// Table 3.2 DFU_DETACH is not supported in DFU Mode.
	switch (req->bRequest)
	{
	case USB_CLASS_REQUEST_DNLOAD:
		/* Block number passed in wValue gives the start address of
		 * to program based on the size of the transfer size.
		 */
		USBD_DFU_class_req_download(req->wValue * DFU_TRANSFER_SIZE,
				req->wLength);
		status = USBD_OK;
		break;

	case USB_CLASS_REQUEST_UPLOAD:
		/* Block number passed in wValue gives the start address of
		 * to program based on the size of the transfer size.
		 */
		USBD_DFU_class_req_upload(req->wValue * DFU_TRANSFER_SIZE,
				req->wLength);
		status = USBD_OK;
		break;

	case USB_CLASS_REQUEST_GETSTATUS:
		USBD_DFU_class_req_getstatus(req->wLength);
		status = USBD_OK;
		break;

	case USB_CLASS_REQUEST_GETSTATE:
		USBD_DFU_class_req_getstate(req->wLength);
		status = USBD_OK;
		break;
	case USB_CLASS_REQUEST_CLRSTATUS:
		USBD_DFU_class_req_clrstatus();
		status = USBD_OK;
		break;
	case USB_CLASS_REQUEST_ABORT:
		USBD_DFU_class_req_abort();
		status = USBD_OK;
		break;

	default:
		// Unknown or unsupported request.
		break;
	}
	return status;
}


/**
 @brief      USB class request handler
 @details    Handle class requests from the host application.
 The bRequest value is parsed and the appropriate
 action or function is performed. Additional values
 from the USB_device_request structure are decoded
 and the DFU state machine and status updated as
 required.
 @param[in]	req - USB_device_request structure containing the
 SETUP portion of the request from the host.
 @return		status - USBD_OK if successful or USBD_ERR_*
 if there is an error or the bRequest is not
 supported.
 **/
int8_t class_req_cb(USB_device_request *req)
{
	int8_t status = USBD_ERR_NOT_SUPPORTED;
	uint8_t requestType = req->bmRequestType;
	uint8_t interface = LSB(req->wIndex) & 0x0F;

	// For DFU requests the SETUP packet must be addressed to the
	// the DFU interface on the device.

	// For DFU requests ensure the recipient is an interface...
	if ((requestType & USB_BMREQUESTTYPE_RECIPIENT_MASK) ==
			USB_BMREQUESTTYPE_RECIPIENT_INTERFACE)
	{
		// ...and that the interface is the correct Runtime interface
		if (USBD_DFU_is_runtime())
		{
			if ((interface == DFU_USB_INTERFACE_RUNTIME))
			{
				status = class_req_dfu_interface_runtime(req);
			}
			else
			{
				status = class_req_interface_hid(req);
			}
		}
		// ...or the correct DFU Mode interface
		else
		{
			if (interface == DFU_USB_INTERFACE_DFUMODE)
			{
				status = class_req_dfu_interface_dfumode(req);
			}
		}
	}
	else if ((requestType & USB_BMREQUESTTYPE_RECIPIENT_MASK) ==
			USB_BMREQUESTTYPE_RECIPIENT_ENDPOINT)
	{
		status = class_req_endpoint_hid_1(req);
	}

	return status;
}

/**
 @brief      USB standard request handler for GET_DESCRIPTOR
 @details    Handle standard GET_DESCRIPTOR requests from the host
 application.
 The hValue is parsed and the appropriate device,
 configuration or string descriptor is selected.
 For device and configuration descriptors the DFU
 state machine determines which descriptor to use.
 For string descriptors the lValue selects which
 string from the string_descriptors table to use.
 The string table is automatically traversed to find
 the correct string and the length is taken from the
 string descriptor.
 @param[in]	req - USB_device_request structure containing the
 SETUP portion of the request from the host.
 @return		status - USBD_OK if successful or USBD_ERR_*
 if there is an error or the bmRequestType is not
 supported.
 **/
int8_t standard_req_get_descriptor(USB_device_request *req, uint8_t **buffer, uint16_t *len)
{
	const uint8_t *src = NULL;
	uint16_t length = req->wLength;
	uint16_t dlen;
	uint8_t hValue = MSB(req->wValue);
	uint8_t lValue = LSB(req->wValue);
	uint8_t interface = LSB(req->wIndex) & 0x0F;
	uint8_t i;
	uint8_t slen;

	switch (hValue)
	{
	case USB_DESCRIPTOR_TYPE_DEVICE:
		if (USBD_DFU_is_runtime())
		{
			src = (uint8_t *) &device_descriptor_panel;
		}
		else
		{
			src = (uint8_t *) &device_descriptor_dfumode;
		}
		if (length > sizeof(USB_device_descriptor)) // too many bytes requested
			length = sizeof(USB_device_descriptor); // Entire structure.
		break;

	case USB_DESCRIPTOR_TYPE_CONFIGURATION:
		if (USBD_DFU_is_runtime())
		{
			if (USBD_get_bus_speed() == USBD_SPEED_HIGH)
			{
				memcpy((void *)&config_descriptor_buffer.hs, (void *)&config_descriptor_panel_hs, sizeof(config_descriptor_panel_hs));
				if (length > sizeof(config_descriptor_panel_hs)) // too many bytes requested
					length = sizeof(config_descriptor_panel_hs); // Entire structure.
			}
			else
			{
				memcpy((void *)&config_descriptor_buffer.fs, (void *)&config_descriptor_panel_fs, sizeof(config_descriptor_panel_fs));
				if (length > sizeof(config_descriptor_panel_fs)) // too many bytes requested
					length = sizeof(config_descriptor_panel_fs); // Entire structure.
			}
			src = (uint8_t *)&config_descriptor_buffer.hs;
			config_descriptor_buffer.hs.configuration.bDescriptorType = USB_DESCRIPTOR_TYPE_CONFIGURATION;
		}
		else
		{
			src = (uint8_t *) &config_descriptor_dfumode;
			if (length > sizeof(config_descriptor_dfumode)) // too many bytes requested
				length = sizeof(config_descriptor_dfumode); // Entire structure.
		}
		break;

	case USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION:
		if (USBD_DFU_is_runtime())
		{
			if (USBD_get_bus_speed() == USBD_SPEED_HIGH)
			{
				memcpy((void *)&config_descriptor_buffer.fs, (void *)&config_descriptor_panel_fs, sizeof(config_descriptor_panel_fs));
				if (length > sizeof(config_descriptor_panel_fs)) // too many bytes requested
					length = sizeof(config_descriptor_panel_fs); // Entire structure.
			}
			else
			{
				memcpy((void *)&config_descriptor_buffer.hs, (void *)&config_descriptor_panel_hs, sizeof(config_descriptor_panel_hs));
				if (length > sizeof(config_descriptor_panel_hs)) // too many bytes requested
					length = sizeof(config_descriptor_panel_hs); // Entire structure.
			}
			src = (uint8_t *)&config_descriptor_buffer.hs;
			config_descriptor_buffer.hs.configuration.bDescriptorType = USB_DESCRIPTOR_TYPE_OTHER_SPEED_CONFIGURATION;
		}
		break;

	case USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER:
		src = (uint8_t *) &device_qualifier_descriptor_panel;
		if (length > sizeof(USB_device_qualifier_descriptor)) // too many bytes requested
			length = sizeof(USB_device_qualifier_descriptor); // Entire structure.
		break;

	case USB_DESCRIPTOR_TYPE_STRING:
		// Special case: WCID descriptor
#ifdef USB_INTERFACE_USE_DFU
		if (lValue == USB_MICROSOFT_WCID_STRING_DESCRIPTOR)
		{
			src = (uint8_t *)wcid_string_descriptor;
			length = sizeof(wcid_string_descriptor);
			break;
		}
#endif // USB_INTERFACE_USE_DFU

		// Find the nth string in the string descriptor table
		i = 0;
		while ((slen = ((uint8_t *)string_descriptor)[i]) > 0)
		{
			// Point to start of string descriptor in __code segment
			// Typecast prevents warning for losing const when USBD_transfer
			// is called
			src = (uint8_t *) &((uint8_t *)string_descriptor)[i];
			if (lValue == 0)
			{
				break;
			}
			i += slen;
			lValue--;
		}
		if (lValue > 0)
		{
			// String not found
			return USBD_ERR_NOT_SUPPORTED;
		}
		// Update the length returned only if it is less than the requested
		// size
		if (length > slen)
		{
			length = slen;
		}

		break;

	case USB_DESCRIPTOR_TYPE_REPORT:
		keyboard_get_report_descriptor(interface, &src, &dlen);
		if (length > dlen)
		{
			length = dlen; // Entire structure.
		}
		break;

	default:
		return USBD_ERR_NOT_SUPPORTED;
	}

	*buffer = (uint8_t *)src;
	*len = length;

	return USBD_OK;
}

/**
 @brief      USB vendor request handler
 @details    Handle vendor requests from the host application.
 The bRequest value is parsed and the appropriate
 action or function is performed. Additional values
 from the USB_device_request structure are decoded
 and provided to other handlers.
 There are no vendor requests requiring handling in
 this firmware.
 @param[in]	req - USB_device_request structure containing the
 SETUP portion of the request from the host.
 @return		status - USBD_OK if successful or USBD_ERR_*
 if there is an error or the bRequest is not
 supported.
 **/
int8_t vendor_req_cb(USB_device_request *req)
{
	int8_t status = USBD_ERR_NOT_SUPPORTED;
#ifdef USB_INTERFACE_USE_DFU
	uint16_t length = req->wLength;
#endif // USB_INTERFACE_USE_DFU

	// For Microsoft WCID only.
	// Request for Compatible ID Feature Descriptors.
	// Check the request code and wIndex.
#ifdef USB_INTERFACE_USE_DFU
	if (req->bRequest == WCID_VENDOR_REQUEST_CODE)
	{
		if (req->wIndex == USB_MICROSOFT_WCID_FEATURE_WINDEX_COMPAT_ID)
		{
			if (req->bmRequestType & USB_BMREQUESTTYPE_DIR_DEV_TO_HOST)
			{
				if (length > sizeof(wcid_feature_runtime)) // too many bytes requested
					length = sizeof(wcid_feature_runtime); // Entire structure.
				// Return a compatible ID feature descriptor.
				if (USBD_DFU_is_runtime())
				{
					USBD_transfer_ep0(USBD_DIR_IN, (uint8_t *) &wcid_feature_runtime,
							length, length);
				}
				else
				{
					USBD_transfer_ep0(USBD_DIR_IN, (uint8_t *) &wcid_feature_dfumode,
							length, length);
				}
				// ACK packet
				USBD_transfer_ep0(USBD_DIR_OUT, NULL, 0, 0);
				status = USBD_OK;
			}
		}
	}
#endif // USB_INTERFACE_USE_DFU

	return status;
}

/**
 @brief      USB suspend callback
 @details    Called when the USB bus enters the suspend state.
 @param[in]	status - unused.
 @return		N/A
 **/
void suspend_cb(uint8_t status)
{
	(void) status;

	SYS->PMCFG_L |= MASK_SYS_PMCFG_HOST_RESUME_DEV;

	printf("Suspend\r\n");

	eve_keyboard_splash("Waiting for host...", 0);
	eve_ui_wait();

	return;
}

/**
 @brief      USB resume callback
 @details    Called when the USB bus enters the resume state
 prior to restating after a suspend.
 @param[in]  status - unused.
 @return     N/S
 **/
void resume_cb(uint8_t status)
{
	(void) status;

	SYS->PMCFG_L &= (~MASK_SYS_PMCFG_HOST_RESUME_DEV);

	keyboard_start();

	printf("Resume\r\n");
	return;
}

/**
 @brief      USB reset callback
 @details    Called when the USB bus enters the reset state.
 @param[in]	status - unused.
 @return		N/A
 **/
void reset_cb(uint8_t status)
{
	(void) status;

	USBD_set_state(USBD_STATE_DEFAULT);

	// If we are at DFUSTATE_MANIFEST_WAIT_RESET stage and do
	// not support detach then the DFU will reset the chip and
	// run the new firmware.
	// From the DFUSTATE_APPIDLE state advance to DFUSTATE_DFUIDLE.
	// Other states will not advance the state machine or may
	// move this to DFUSTATE_DFUERROR if it is inappropriate to
	// find a reset at that stage.
	USBD_DFU_reset();
	printf("Reset\r\n");

	keyboard_report_enable(0, 0);
	keyboard_report_enable(1, 0);

	return;
}

uint8_t usbd_testing(void)
{
	USBD_ctx usb_ctx;

	// Current device speed.
	USBD_DEVICE_SPEED speed;

	uint8_t not_connected = 1;
	uint8_t report;

	eve_ui_setup();
	keyboard_set_report_descriptor(0, hid_report_descriptor_keyboard, sizeof(hid_report_descriptor_keyboard));
	keyboard_set_report_descriptor(1, hid_report_descriptor_control, sizeof(hid_report_descriptor_control));

	memset(&usb_ctx, 0, sizeof(usb_ctx));

	usb_ctx.standard_req_cb = NULL;
	usb_ctx.get_descriptor_cb = standard_req_get_descriptor;
	usb_ctx.set_interface_cb = setif_req_cb;
	usb_ctx.get_interface_cb = getif_req_cb;
	usb_ctx.class_req_cb = class_req_cb;
	usb_ctx.vendor_req_cb = vendor_req_cb;
	usb_ctx.suspend_cb = suspend_cb;
	usb_ctx.resume_cb = resume_cb;
	usb_ctx.reset_cb = reset_cb;
	usb_ctx.lpm_cb = NULL;
	usb_ctx.speed = USBD_SPEED_HIGH;

	// Initialise the USB device with a control endpoint size
	// of 8 to 64 bytes. This must match the size for bMaxPacketSize0
	// defined in the device descriptor.
	usb_ctx.ep0_size = USB_CONTROL_EP_SIZE;

	USBD_initialise(&usb_ctx);

	for (;;)
	{
		keyboard_start();

		USBD_attach();

		if (USBD_connect() == USBD_OK)
		{
			speed = USBD_get_bus_speed();

			if (speed == USBD_SPEED_HIGH)
			{
				USBD_create_endpoint(HID_KEYBOARD_IN_EP, USBD_EP_INT, USBD_DIR_IN,
						HID_KEYBOARD_IN_EP_USBD_SIZE, USBD_DB_OFF, NULL /*ep_cb*/);
				USBD_create_endpoint(HID_CONTROL_IN_EP, USBD_EP_INT, USBD_DIR_IN,
						HID_CONTROL_IN_EP_USBD_SIZE, USBD_DB_OFF, NULL /*ep_cb*/);
			}
			else
			{
				USBD_create_endpoint(HID_KEYBOARD_IN_EP, USBD_EP_INT, USBD_DIR_IN,
						HID_KEYBOARD_IN_EP_USBD_SIZE, USBD_DB_OFF, NULL /*ep_cb*/);
				USBD_create_endpoint(HID_CONTROL_IN_EP, USBD_EP_INT, USBD_DIR_IN,
						HID_CONTROL_IN_EP_USBD_SIZE, USBD_DB_OFF, NULL /*ep_cb*/);
			}

			// Not used as there are no alternate interfaces.
			(void)usb_alt;

			if (USBD_DFU_is_runtime())
			{
				// Start the HID emulation code.
				while (1)
				{
					if (USBD_get_state() == USBD_STATE_CONFIGURED)
					{
						if (not_connected)
						{
							// Now we are connected, draw the keyboard.
							printf("Starting\r\n");
							keyboard_report_enable(1, 1);
							not_connected = 0;
						}
						else
						{
							// If the touchscreen has a keypress to send then send it.
							report = keyboard_loop(&keyboard_report_in, &keyboard_report_out, &control_report_in);

							if (report == REP_KB)
							{
								if (keyboard_report_enabled(0))
								{
									// There is a keypress ready to send.
									USBD_transfer(HID_KEYBOARD_IN_EP, (uint8_t *)&keyboard_report_in, sizeof(keyboard_in_report_structure_t));
								}
							}
							if ((report == REP_SC) || (report == REP_CC))
							{
								if (keyboard_report_enabled(1))
								{
									// There is a system or consumer event ready to send.
									USBD_transfer(HID_CONTROL_IN_EP, (uint8_t *)&control_report_in, sizeof(control_in_report_structure_t));
								}
							}
						}
					}
				}
			}
		}
		else
		{
			// In DFU mode. Process USB requests.
			USBD_attach();
			while (USBD_connect() == USBD_OK);
		}
		printf("Restarting\r\n");

		not_connected = 1;
	}

	return 0;
}

/* FUNCTIONS ***********************************************************************/

int main(void)
{
#ifdef USB_INTERFACE_USE_STARTUPDFU
	STARTUP_DFU();
#endif // USB_INTERFACE_USE_STARTUPDFU

	sys_reset_all();
	sys_disable(sys_device_usb_device);

    /* Enable the UART Device... */
    sys_enable(sys_device_uart0);
    /* Set UART0 GPIO functions to UART0_TXD and UART0_RXD... */
#if defined(__FT900__)
    gpio_function(48, pad_uart0_txd); /* UART0 TXD */
    gpio_function(49, pad_uart0_rxd); /* UART0 RXD */
#elif defined(__FT930__)
    gpio_function(23, pad_uart0_txd); /* UART0 TXD */
    gpio_function(22, pad_uart0_rxd); /* UART0 RXD */
#endif

	// Open the UART using the coding required.
	uart_open(UART0, 1, UART_DIVIDER_19200_BAUD, uart_data_bits_8, uart_parity_none, uart_stop_bits_1);

#ifdef BRIDGE_DEBUG
	/* Print out a welcome message... */
	BRIDGE_DEBUG_PRINTF("\x1B[2J" /* ANSI/VT100 - Clear the Screen */
			"\x1B[H"  /* ANSI/VT100 - Move Cursor to Home */
	);

	BRIDGE_DEBUG_PRINTF("(C) Copyright 2016, Bridgetek Pte Ltd. \r\n");
	BRIDGE_DEBUG_PRINTF("--------------------------------------------------------------------- \r\n");
	BRIDGE_DEBUG_PRINTF("Welcome to the USB HID Touch Panel... \r\n");
	BRIDGE_DEBUG_PRINTF("\r\n");
	BRIDGE_DEBUG_PRINTF("Emulate a USB HID Device with a Simple Display.\r\n");
	BRIDGE_DEBUG_PRINTF("--------------------------------------------------------------------- \r\n");
#endif // BRIDGE_DEBUG

	/* Attach the interrupt so it can be called... */
	//interrupt_attach(interrupt_uart0, (uint8_t) interrupt_uart0, uart0ISR);
	uart_disable_interrupt(UART0, uart_interrupt_tx);
	/* Enable the UART to fire interrupts when receiving buffer... */
	uart_enable_interrupt(UART0, uart_interrupt_rx);
	/* Enable interrupts to be fired... */
	uart_enable_interrupts_globally(UART0);

	sys_enable(sys_device_timer_wdt);
	interrupt_attach(interrupt_timers, (int8_t)interrupt_timers, timer_ISR);
	/* Timer A = 1ms */
	timer_prescaler(1000);
	timer_init(timer_select_a, 100, timer_direction_down, timer_prescaler_select_on, timer_mode_continuous);
	timer_enable_interrupt(timer_select_a);
	timer_start(timer_select_a);

	/* Enable power management interrupts. Primarily to detect resume signalling
	 * from the USB host. */
#if defined(__FT900__)
	interrupt_attach(interrupt_0, (int8_t)interrupt_0, powermanagement_ISR);
#endif // defined(__FT900__)

	interrupt_enable_globally();

	usbd_testing();

	interrupt_disable_globally();

	// Wait forever...
	for (;;);

	return 0;
}

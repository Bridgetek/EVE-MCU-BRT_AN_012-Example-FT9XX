/**
 @file keyboard.h
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
 * http://www.ftdichip.com/FTSourceCodeLicenceTerms.htm ("the Licence Terms").
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

#ifndef INCLUDES_KEYBOARD_H_
#define INCLUDES_KEYBOARD_H_

#include <eve_keyboard.h>


/* For MikroC const qualifier will place variables in Flash
 * not just make them constant.
 */
#if defined(__GNUC__)
#define DESCRIPTOR_QUALIFIER const
#elif defined(__MIKROC_PRO_FOR_FT90x__)
#define DESCRIPTOR_QUALIFIER data
#endif

#define PACK __attribute__ ((__packed__))

/**
 @struct keyboard_in_report_structure
 @brief HID Report structure to match IN report
 hid_report_descriptor_keyboard descriptor.
 **/
typedef struct PACK keyboard_in_report_structure_t
{
	union {
		struct {
			unsigned char kbdLeftControl :1;
			unsigned char kbdLeftShift :1;
			unsigned char kbdLeftAlt :1;
			unsigned char kbdLeftGUI :1;
			unsigned char kbdRightControl :1;
			unsigned char kbdRightShift :1;
			unsigned char kbdRightAlt :1;
			unsigned char kbdRightGUI :1;
		};
		unsigned char arrayMap;
	};

	unsigned char arrayNotUsed;  // [1]
	unsigned char arrayKeyboard; // [2]
	unsigned char arrayResv1;
	unsigned char arrayResv2;
	unsigned char arrayResv3;
	unsigned char arrayResv4;
	unsigned char arrayResv5;
} keyboard_in_report_structure_t;

/**
 @struct keyboard_out_report_structure
 @brief HID Report structure to match OUT report
 hid_report_descriptor_keyboard descriptor.
 **/
typedef struct PACK keyboard_out_report_structure_t
{
	union {
		struct {
			unsigned char ledNumLock :1;
			unsigned char ledCapsLock :1;
			unsigned char ledScrollLock :1;
			unsigned char LedCompose :1;
			unsigned char ledKana :1;
			unsigned char ledConstant :3;
		};
		unsigned char ledsMap;
	};
} keyboard_out_report_structure_t;

/**
 @struct hid_out_report_structure
 @brief HID Report structure to match IN report
  @name hid_report_descriptor_control descriptor.
 **/
typedef struct PACK control_in_report_structure_t
{
	unsigned char reportID;

	union {
		union {
			struct {
				unsigned char sysPowerDown :1;
				unsigned char sysSleep :1;
				unsigned char sysWake :1;
				unsigned char resv3 :1;
				unsigned char resv4 :1;
				unsigned char resv5 :1;
				unsigned char resv6 :1;
				unsigned char resv7 :1;
			};
			unsigned char sysMap;
		} system;
		struct {
			unsigned char arrayConsumer1;
			unsigned char arrayConsumer2;
		} consumer;
	};

} control_in_report_structure_t;

/** @name Key Scancode Definitions
 * @details Special key definitions for virtual keypad. These map
 * from keys drawn on the keyboard to scan codes defined in
 * the keyboard abstraction file.
 * Sourced from Microsoft Keyboard Scancode Specification and USB.org
 * USB HID Usage Tables.
 */
//@{
enum {
	KEY_A = 4, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I,
	KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
	KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

	KEY_1 = 30, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,	KEY_0,

	KEY_ENTER = 40,				// Keyboard Enter
	KEY_ESCAPE = 41,			// Escape key
	KEY_BACKSPACE = 42,			// Backspace key
	KEY_TAB = 43,				// Tab key
	KEY_SPACE = 44,				// Keyboard Spacebar
	KEY_MINUS, KEY_EQUALS, KEY_SQB_OPEN, KEY_SQB_CLS,
	KEY_BACKSLASH, KEY_HASH, KEY_SEMICOLON, KEY_SQUOTE,
	KEY_TILDE, KEY_COMMA, KEY_DOT, KEY_SLASH,
	KEY_CAPS_LOCK = 57,				// Caps Lock

	KEY_F1 = 58, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8,
	KEY_F9, KEY_F10, KEY_F11, KEY_F12,

	KEY_PRINT_SCREEN = 70,
	KEY_SCROLL_LOCK = 71,				// Scroll Lock
	KEY_PAUSE = 72,	KEY_INSERT,	KEY_HOME, KEY_PAGE_UP, KEY_DEL,	KEY_END,
	KEY_PAGE_DOWN, KEY_RIGHT_ARROW, KEY_LEFT_ARROW, KEY_DOWN_ARROW,
	KEY_UP_ARROW = 82,
	KEY_NUMBER_LOCK = 83,				// Number Lock

	KEY_PAD_DIV = 84, KEY_PAD_MUL, KEY_PAD_MINUS, KEY_PAD_PLUS, KEY_PAD_ENTER,
	KEY_PAD_1 = 89, KEY_PAD_2, KEY_PAD_3, KEY_PAD_4, KEY_PAD_5, KEY_PAD_6,
	KEY_PAD_7, KEY_PAD_8, KEY_PAD_9, KEY_PAD_0, KEY_PAD_DOT,

	KEY_APP = 101,				// Context menu key
};

enum {
	KEY_CTRLL = 224,				// Ctrl
	KEY_SHIFTL = 225,				// Shift
	KEY_ALT = 226,				// Alt
	KEY_WINL = 227,				// Win
	KEY_CTRLR = 228,				// Ctrl
	KEY_SHIFTR = 229,				// Shift
	KEY_ALTGR = 230,				// AltGr
	KEY_WINR = 231,				// Win
};
//@}

/** @name System Control Scancode Definitions
 * @details Sourced from USB.org USB HID Usage Tables.
 */
//@{
enum {
	KEY_SC_POWER = 81,
	KEY_SC_SLEEP = 82,
	KEY_SC_WAKEUP = 83,
};
//@}

/** @name Consumer Control Scancode Definitions
 * @details Sourced from USB.org USB HID Usage Tables.
 */
//@{
enum {
	KEY_CC_PLAY = 0xb0,
	KEY_CC_PAUSE = 0xb1,
	KEY_CC_RECORD = 0xb2,
	KEY_CC_FFWD = 0xb3,
	KEY_CC_RWD = 0xb4,
	KEY_CC_NEXT = 0xb5,
	KEY_CC_PREV = 0xb6,
	KEY_CC_STOP = 0xb7,

	KEY_CC_MUTE = 0xe2,
	KEY_CC_VOL_UP = 0xe9,
	KEY_CC_VOL_DOWN = 0xea,

	KEY_CC_CUT = 0x21c,
	KEY_CC_COPY = 0x21b,
	KEY_CC_PASTE = 0x21d,
	KEY_CC_UNDO = 0x21a,
	KEY_CC_REDO = 0x279,
	KEY_CC_FIND = 0x221,
};
//@}


/** @name Report Types
 * @details Used internally in keyboard.c to map tags to scancodes.
 */
//@{
/// No report
#define REPORT_ID_NONE 0
/// Keyboard Report
#define REPORT_ID_KEYBOARD 1
/// System Control Report
#define REPORT_ID_SYSTEM_CONTROL 2
/// Consumer Control Report
#define REPORT_ID_CONSUMER_CONTROL 3
//@}

/** @name Tag map structure
 * @details Used internally in keyboard.c to map tags to scancodes.
 */
struct tagmap_s {
	uint8_t tag;
	uint8_t report;
	uint16_t scancode;
};

#endif /* INCLUDES_KEYBOARD_H_ */

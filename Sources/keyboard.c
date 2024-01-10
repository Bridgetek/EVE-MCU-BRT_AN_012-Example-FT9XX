/**
  @file keyboard.c
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


#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <ft900.h>
#include <ft900_usb.h>
#include <ft900_usb_hid.h>
#include <ft900_usb_dfu.h>
#include <ft900_usbd_dfu.h>

#include "keyboard.h"
#include "eve_keyboard.h"
#include "eve_ui_keyboard.h"


/* For MikroC const qualifier will place variables in Flash
 * not just make them constant.
 */
#if defined(__GNUC__)
#define DESCRIPTOR_QUALIFIER const
#elif defined(__MIKROC_PRO_FOR_FT90x__)
#define DESCRIPTOR_QUALIFIER data
#endif

#define PACK __attribute__ ((__packed__))

/* CONSTANTS ***********************************************************************/

/** @name Tag map
 * @details Used to map tags to scancodes and report types.
 */
const struct tagmap_s tagmap[] = {
		{TAG_A, REP_KB, KEY_A}, {TAG_B, REP_KB, KEY_B}, {TAG_C, REP_KB, KEY_C}, {TAG_D, REP_KB, KEY_D},
		{TAG_E, REP_KB, KEY_E}, {TAG_F, REP_KB, KEY_F}, {TAG_G, REP_KB, KEY_G}, {TAG_H, REP_KB, KEY_H},
		{TAG_I, REP_KB, KEY_I}, {TAG_J, REP_KB, KEY_J}, {TAG_K, REP_KB, KEY_K}, {TAG_L, REP_KB, KEY_L},
		{TAG_M, REP_KB, KEY_M}, {TAG_N, REP_KB, KEY_N}, {TAG_O, REP_KB, KEY_O}, {TAG_P, REP_KB, KEY_P},
		{TAG_Q, REP_KB, KEY_Q}, {TAG_R, REP_KB, KEY_R}, {TAG_S, REP_KB, KEY_S}, {TAG_T, REP_KB, KEY_T},
		{TAG_U, REP_KB, KEY_U}, {TAG_V, REP_KB, KEY_V}, {TAG_W, REP_KB, KEY_W}, {TAG_X, REP_KB, KEY_X},
		{TAG_Y, REP_KB, KEY_Y}, {TAG_Z, REP_KB, KEY_Z},
		{TAG_1, REP_KB, KEY_1}, {TAG_2, REP_KB, KEY_2}, {TAG_3, REP_KB, KEY_3}, {TAG_4, REP_KB, KEY_4},
		{TAG_5, REP_KB, KEY_5}, {TAG_6, REP_KB, KEY_6}, {TAG_7, REP_KB, KEY_7}, {TAG_8, REP_KB, KEY_8},
		{TAG_9, REP_KB, KEY_9},	{TAG_0, REP_KB, KEY_0},
		{TAG_ENTER, REP_KB, KEY_ENTER},
		{TAG_ESCAPE, REP_KB, KEY_ESCAPE},
		{TAG_BACKSPACE, REP_KB, KEY_BACKSPACE},
		{TAG_TAB, REP_KB, KEY_TAB},
		{TAG_SPACE, REP_KB, KEY_SPACE},
		{TAG_MINUS, REP_KB, KEY_MINUS}, {TAG_EQUALS, REP_KB, KEY_EQUALS},
		{TAG_SQB_OPEN, REP_KB, KEY_SQB_OPEN}, {TAG_SQB_CLS, REP_KB, KEY_SQB_CLS},
		{TAG_BACKSLASH, REP_KB, KEY_BACKSLASH}, {TAG_HASH, REP_KB, KEY_HASH},
		{TAG_SEMICOLON, REP_KB, KEY_SEMICOLON}, {TAG_SQUOTE, REP_KB, KEY_SQUOTE},
		{TAG_TILDE, REP_KB, KEY_TILDE}, {TAG_COMMA, REP_KB, KEY_COMMA},
		{TAG_DOT, REP_KB, KEY_DOT}, {TAG_SLASH, REP_KB, KEY_SLASH},
		{TAG_CAPS_LOCK, REP_KB, KEY_CAPS_LOCK},
		{TAG_F1, REP_KB, KEY_F1}, {TAG_F2, REP_KB, KEY_F2}, {TAG_F3, REP_KB, KEY_F3},
		{TAG_F4, REP_KB, KEY_F4}, {TAG_F5, REP_KB, KEY_F5}, {TAG_F6, REP_KB, KEY_F6},
		{TAG_F7, REP_KB, KEY_F7}, {TAG_F8, REP_KB, KEY_F8}, {TAG_F9, REP_KB, KEY_F9},
		{TAG_F10, REP_KB, KEY_F10}, {TAG_F11, REP_KB, KEY_F11}, {TAG_F12, REP_KB, KEY_F12},
		{TAG_PRINT_SCREEN, REP_KB, KEY_PRINT_SCREEN}, {TAG_SCROLL_LOCK, REP_KB, KEY_SCROLL_LOCK},
		{TAG_PAUSE, REP_KB, KEY_PAUSE}, {TAG_INSERT, REP_KB, KEY_INSERT},
		{TAG_HOME, REP_KB, KEY_HOME}, {TAG_PAGE_UP, REP_KB, KEY_PAGE_UP},
		{TAG_DEL, REP_KB, KEY_DEL},	{TAG_END, REP_KB, KEY_END},
		{TAG_PAGE_DOWN, REP_KB, KEY_PAGE_DOWN}, {TAG_RIGHT_ARROW, REP_KB, KEY_RIGHT_ARROW},
		{TAG_LEFT_ARROW, REP_KB, KEY_LEFT_ARROW}, {TAG_DOWN_ARROW, REP_KB, KEY_DOWN_ARROW},
		{TAG_UP_ARROW, REP_KB, KEY_UP_ARROW},
		{TAG_NUMBER_LOCK, REP_KB, KEY_NUMBER_LOCK},
		{TAG_PAD_DIV, REP_KB, KEY_PAD_DIV}, {TAG_PAD_MUL, REP_KB, KEY_PAD_MUL},
		{TAG_PAD_MINUS, REP_KB, KEY_PAD_MINUS}, {TAG_PAD_PLUS, REP_KB, KEY_PAD_PLUS},
		{TAG_PAD_ENTER, REP_KB, KEY_PAD_ENTER},
		{TAG_PAD_1, REP_KB, KEY_PAD_1}, {TAG_PAD_2, REP_KB, KEY_PAD_2}, {TAG_PAD_3, REP_KB, KEY_PAD_3},
		{TAG_PAD_4, REP_KB, KEY_PAD_4}, {TAG_PAD_5, REP_KB, KEY_PAD_5}, {TAG_PAD_6, REP_KB, KEY_PAD_6},
		{TAG_PAD_7, REP_KB, KEY_PAD_7}, {TAG_PAD_8, REP_KB, KEY_PAD_8}, {TAG_PAD_9, REP_KB, KEY_PAD_9},
		{TAG_PAD_0, REP_KB, KEY_PAD_0}, {TAG_PAD_DOT, REP_KB, KEY_PAD_DOT},
		{TAG_APP, REP_KB, KEY_APP},
		{TAG_CTRLL, REP_KB, KEY_CTRLL}, {TAG_SHIFTL, REP_KB, KEY_SHIFTL}, {TAG_ALT, REP_KB, KEY_ALT},
		{TAG_WINL, REP_KB, KEY_WINL}, {TAG_CTRLR, REP_KB, KEY_CTRLR}, {TAG_SHIFTR, REP_KB, KEY_SHIFTR},
		{TAG_ALTGR, REP_KB, KEY_ALTGR}, {TAG_WINR, REP_KB, KEY_WINR},
		{TAG_SC_POWER, REP_SC, KEY_SC_POWER},
		{TAG_SC_SLEEP, REP_SC, KEY_SC_SLEEP},
		{TAG_SC_WAKEUP, REP_SC, KEY_SC_WAKEUP},
		{TAG_CC_PLAY, REP_CC, KEY_CC_PLAY}, {TAG_CC_PAUSE, REP_CC, KEY_CC_PAUSE},
		{TAG_CC_RECORD, REP_CC, KEY_CC_RECORD}, {TAG_CC_FFWD, REP_CC, KEY_CC_FFWD},
		{TAG_CC_RWD, REP_CC, KEY_CC_RWD},
		{TAG_CC_NEXT, REP_CC, KEY_CC_NEXT}, {TAG_CC_PREV, REP_CC, KEY_CC_PREV},
		{TAG_CC_STOP, REP_CC, KEY_CC_STOP},
		{TAG_CC_MUTE, REP_CC, KEY_CC_MUTE}, {TAG_CC_VOL_UP, REP_CC, KEY_CC_VOL_UP},
		{TAG_CC_VOL_DOWN, REP_CC, KEY_CC_VOL_DOWN},
		{TAG_CC_CUT, REP_CC, KEY_CC_CUT}, {TAG_CC_COPY, REP_CC, KEY_CC_COPY},
		{TAG_CC_PASTE, REP_CC, KEY_CC_PASTE}, {TAG_CC_UNDO, REP_CC, KEY_CC_UNDO},
		{TAG_CC_REDO, REP_CC, KEY_CC_REDO}, {TAG_CC_FIND, REP_CC, KEY_CC_FIND},
};

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

static struct key_report key_report;
static struct key_scan key_scan;

/**
 @name report_descriptor
 @brief Pointer to report descriptor.
 */
static const uint8_t *report_descriptor[2] = {0, 0};

/**
 @name report_descriptor
 @brief Pointer to report descriptor.
 */
static uint16_t report_descriptor_len[2] = {0, 0};

/**
 @name report_protocol
 @brief Flag to set report protocol (0 - boot, 1 - report)
 */
static uint8_t report_protocol[2] = {0, 0};

/**
 @name report_enable
 @brief Flag to enable report mode (i.e. reporting active).
 */
static uint8_t report_enable[2] = {0, 0};

/**
 @name report_idle
 @brief Timer to set idle report period.
 */
static uint8_t report_idle[2] = {0, 0};

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/


/* EXTERN FUNCTIONS ****************************************************************/

int8_t keyboard_get_report_descriptor(int interface, const uint8_t **descriptor, uint16_t *len)
{
	*descriptor = report_descriptor[interface];
	*len = report_descriptor_len[interface];
	return USBD_OK;
}

int8_t keyboard_req_set_idle(int interface, USB_device_request *req)
{
	report_enable[interface] = 1;
	report_idle[interface] = req->wValue >> 8;
	return USBD_OK;
}

int8_t keyboard_req_get_idle(int interface, USB_device_request *req, uint8_t *idle)
{
	*idle = report_idle[interface];
	return USBD_OK;
}

int8_t keyboard_req_set_protocol(int interface, USB_device_request *req)
{
	report_protocol[interface] = req->wValue & 0xff;
	return USBD_OK;
}

int8_t keyboard_req_get_protocol(int interface, USB_device_request *req, uint8_t *protocol)
{
	*protocol = report_protocol[interface];
	return USBD_OK;
}

int8_t keyboard_req_get_report(int interface, void *report)
{
	return USBD_OK;
}

int8_t keyboard_req_set_report(int interface, void *report)
{
	keyboard_out_report_structure_t *keyboard_report_out = (keyboard_out_report_structure_t *)report;

	key_report.Caps = keyboard_report_out->ledCapsLock;
	key_report.Numeric = keyboard_report_out->ledNumLock;
	key_report.Scroll = keyboard_report_out->ledScrollLock;

	eve_keyboard_loop(&key_report, &key_scan);
	return USBD_OK;
}

int8_t keyboard_report_enabled(int interface)
{
	return report_enable[interface];
}

void keyboard_report_enable(int interface, int8_t enable)
{
	report_enable[interface] = enable;
}

void keyboard_set_report_descriptor(int interface, const uint8_t *descriptor, uint16_t len)
{
	report_descriptor[interface] = descriptor;
	report_descriptor_len[interface] = len;
}

void keyboard_timer(void)
{
}

void keyboard_start()
{
	eve_keyboard_start();
	// Draw a waiting for host message.
	eve_splash("Waiting for host...", OPTIONS_HEADER_LOGO);
}

int keyboard_loop(keyboard_in_report_structure_t *keyboard_report_in,
		keyboard_out_report_structure_t *report_buffer_out,
		control_in_report_structure_t *control_report_in)
{
	static uint8_t report = REP_NONE;
	int i;
	uint8_t reportsend = REP_NONE;
	uint16_t scancode = 0;

	key_report.Numeric = report_buffer_out->ledNumLock;
	key_report.Caps = report_buffer_out->ledCapsLock;
	key_report.Scroll = report_buffer_out->ledScrollLock;

	if (eve_keyboard_loop(&key_report, &key_scan) == 0)
	{
		for (i = 0; i < sizeof(tagmap) / sizeof(tagmap[0]); i++)
		{
			if (tagmap[i].tag == key_scan.KeyTag)
			{
				report = tagmap[i].report;
				scancode = tagmap[i].scancode;
				break;
			}
		}

		if (scancode)
		{
			if (report == REP_KB)
			{
				keyboard_report_in->kbdLeftShift = key_scan.ShiftL;
				keyboard_report_in->kbdRightShift = key_scan.ShiftR;
				keyboard_report_in->kbdLeftControl = key_scan.CtrlL;
				keyboard_report_in->kbdRightControl = key_scan.CtrlR;
				keyboard_report_in->kbdLeftAlt = key_scan.Alt;
				keyboard_report_in->kbdRightAlt = key_scan.AltGr;
				keyboard_report_in->kbdLeftGUI = key_scan.WinL;
				keyboard_report_in->kbdRightGUI = key_scan.WinR;

				keyboard_report_in->arrayKeyboard = 0;

				keyboard_report_in->arrayKeyboard = scancode & 0xff;
			}
			else if (report == REP_SC)
			{
				control_report_in->reportID = 1;
				control_report_in->system.sysMap = 0;

				if (scancode == KEY_SC_POWER)
				{
					control_report_in->system.sysPowerDown = 1;
				}
				if (scancode == KEY_SC_SLEEP)
				{
					control_report_in->system.sysSleep = 1;
				}
				if (scancode == KEY_SC_WAKEUP)
				{
					control_report_in->system.sysWake = 1;
				}
			}
			else if (report == REP_CC)
			{
				control_report_in->reportID = 2;
				control_report_in->consumer.arrayConsumer1 = scancode & 0xff;
				control_report_in->consumer.arrayConsumer2 = scancode >> 8;
			}
			else
			{
				report = REP_NONE;
			}
			reportsend = report;
		}
		else
		{
			if (report == REP_KB)
			{
				// Pressing an alphanumeric or symbol key will clear
				// the state of the special function keys.
				key_scan.ShiftL = 0;
				key_scan.ShiftR = 0;
				key_scan.CtrlL = 0;
				key_scan.CtrlR = 0;
				key_scan.Alt = 0;
				key_scan.AltGr = 0;
				key_scan.WinL = 0;
				key_scan.WinR = 0;

				keyboard_report_in->kbdLeftShift = key_scan.ShiftL;
				keyboard_report_in->kbdRightShift = key_scan.ShiftR;
				keyboard_report_in->kbdLeftControl = key_scan.CtrlL;
				keyboard_report_in->kbdRightControl = key_scan.CtrlR;
				keyboard_report_in->kbdLeftAlt = key_scan.Alt;
				keyboard_report_in->kbdRightAlt = key_scan.AltGr;
				keyboard_report_in->kbdLeftGUI = key_scan.WinL;
				keyboard_report_in->kbdRightGUI = key_scan.WinR;

				keyboard_report_in->arrayKeyboard = 0;
			}
			else if (report == REP_SC)
			{
				control_report_in->system.sysMap = 0;
			}
			else if (report == REP_CC)
			{
				control_report_in->reportID = 2;
				control_report_in->consumer.arrayConsumer1 = 0;
				control_report_in->consumer.arrayConsumer2 = 0;
			}

			reportsend = report;
			report = REP_NONE;
		}
	}
	return reportsend;
}

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
		{TAG_A, REPORT_ID_KEYBOARD, KEY_A}, {TAG_B, REPORT_ID_KEYBOARD, KEY_B}, {TAG_C, REPORT_ID_KEYBOARD, KEY_C}, {TAG_D, REPORT_ID_KEYBOARD, KEY_D},
		{TAG_E, REPORT_ID_KEYBOARD, KEY_E}, {TAG_F, REPORT_ID_KEYBOARD, KEY_F}, {TAG_G, REPORT_ID_KEYBOARD, KEY_G}, {TAG_H, REPORT_ID_KEYBOARD, KEY_H},
		{TAG_I, REPORT_ID_KEYBOARD, KEY_I}, {TAG_J, REPORT_ID_KEYBOARD, KEY_J}, {TAG_K, REPORT_ID_KEYBOARD, KEY_K}, {TAG_L, REPORT_ID_KEYBOARD, KEY_L},
		{TAG_M, REPORT_ID_KEYBOARD, KEY_M}, {TAG_N, REPORT_ID_KEYBOARD, KEY_N}, {TAG_O, REPORT_ID_KEYBOARD, KEY_O}, {TAG_P, REPORT_ID_KEYBOARD, KEY_P},
		{TAG_Q, REPORT_ID_KEYBOARD, KEY_Q}, {TAG_R, REPORT_ID_KEYBOARD, KEY_R}, {TAG_S, REPORT_ID_KEYBOARD, KEY_S}, {TAG_T, REPORT_ID_KEYBOARD, KEY_T},
		{TAG_U, REPORT_ID_KEYBOARD, KEY_U}, {TAG_V, REPORT_ID_KEYBOARD, KEY_V}, {TAG_W, REPORT_ID_KEYBOARD, KEY_W}, {TAG_X, REPORT_ID_KEYBOARD, KEY_X},
		{TAG_Y, REPORT_ID_KEYBOARD, KEY_Y}, {TAG_Z, REPORT_ID_KEYBOARD, KEY_Z},
		{TAG_1, REPORT_ID_KEYBOARD, KEY_1}, {TAG_2, REPORT_ID_KEYBOARD, KEY_2}, {TAG_3, REPORT_ID_KEYBOARD, KEY_3}, {TAG_4, REPORT_ID_KEYBOARD, KEY_4},
		{TAG_5, REPORT_ID_KEYBOARD, KEY_5}, {TAG_6, REPORT_ID_KEYBOARD, KEY_6}, {TAG_7, REPORT_ID_KEYBOARD, KEY_7}, {TAG_8, REPORT_ID_KEYBOARD, KEY_8},
		{TAG_9, REPORT_ID_KEYBOARD, KEY_9},	{TAG_0, REPORT_ID_KEYBOARD, KEY_0},
		{TAG_ENTER, REPORT_ID_KEYBOARD, KEY_ENTER},
		{TAG_ESCAPE, REPORT_ID_KEYBOARD, KEY_ESCAPE},
		{TAG_BACKSPACE, REPORT_ID_KEYBOARD, KEY_BACKSPACE},
		{TAG_TAB, REPORT_ID_KEYBOARD, KEY_TAB},
		{TAG_SPACE, REPORT_ID_KEYBOARD, KEY_SPACE},
		{TAG_MINUS, REPORT_ID_KEYBOARD, KEY_MINUS}, {TAG_EQUALS, REPORT_ID_KEYBOARD, KEY_EQUALS},
		{TAG_SQB_OPEN, REPORT_ID_KEYBOARD, KEY_SQB_OPEN}, {TAG_SQB_CLS, REPORT_ID_KEYBOARD, KEY_SQB_CLS},
		{TAG_BACKSLASH, REPORT_ID_KEYBOARD, KEY_BACKSLASH}, {TAG_HASH, REPORT_ID_KEYBOARD, KEY_HASH},
		{TAG_SEMICOLON, REPORT_ID_KEYBOARD, KEY_SEMICOLON}, {TAG_SQUOTE, REPORT_ID_KEYBOARD, KEY_SQUOTE},
		{TAG_TILDE, REPORT_ID_KEYBOARD, KEY_TILDE}, {TAG_COMMA, REPORT_ID_KEYBOARD, KEY_COMMA},
		{TAG_DOT, REPORT_ID_KEYBOARD, KEY_DOT}, {TAG_SLASH, REPORT_ID_KEYBOARD, KEY_SLASH},
		{TAG_CAPS_LOCK, REPORT_ID_KEYBOARD, KEY_CAPS_LOCK},
		{TAG_F1, REPORT_ID_KEYBOARD, KEY_F1}, {TAG_F2, REPORT_ID_KEYBOARD, KEY_F2}, {TAG_F3, REPORT_ID_KEYBOARD, KEY_F3},
		{TAG_F4, REPORT_ID_KEYBOARD, KEY_F4}, {TAG_F5, REPORT_ID_KEYBOARD, KEY_F5}, {TAG_F6, REPORT_ID_KEYBOARD, KEY_F6},
		{TAG_F7, REPORT_ID_KEYBOARD, KEY_F7}, {TAG_F8, REPORT_ID_KEYBOARD, KEY_F8}, {TAG_F9, REPORT_ID_KEYBOARD, KEY_F9},
		{TAG_F10, REPORT_ID_KEYBOARD, KEY_F10}, {TAG_F11, REPORT_ID_KEYBOARD, KEY_F11}, {TAG_F12, REPORT_ID_KEYBOARD, KEY_F12},
		{TAG_PRINT_SCREEN, REPORT_ID_KEYBOARD, KEY_PRINT_SCREEN}, {TAG_SCROLL_LOCK, REPORT_ID_KEYBOARD, KEY_SCROLL_LOCK},
		{TAG_PAUSE, REPORT_ID_KEYBOARD, KEY_PAUSE}, {TAG_INSERT, REPORT_ID_KEYBOARD, KEY_INSERT},
		{TAG_HOME, REPORT_ID_KEYBOARD, KEY_HOME}, {TAG_PAGE_UP, REPORT_ID_KEYBOARD, KEY_PAGE_UP},
		{TAG_DEL, REPORT_ID_KEYBOARD, KEY_DEL},	{TAG_END, REPORT_ID_KEYBOARD, KEY_END},
		{TAG_PAGE_DOWN, REPORT_ID_KEYBOARD, KEY_PAGE_DOWN}, {TAG_RIGHT_ARROW, REPORT_ID_KEYBOARD, KEY_RIGHT_ARROW},
		{TAG_LEFT_ARROW, REPORT_ID_KEYBOARD, KEY_LEFT_ARROW}, {TAG_DOWN_ARROW, REPORT_ID_KEYBOARD, KEY_DOWN_ARROW},
		{TAG_UP_ARROW, REPORT_ID_KEYBOARD, KEY_UP_ARROW},
		{TAG_NUMBER_LOCK, REPORT_ID_KEYBOARD, KEY_NUMBER_LOCK},
		{TAG_PAD_DIV, REPORT_ID_KEYBOARD, KEY_PAD_DIV}, {TAG_PAD_MUL, REPORT_ID_KEYBOARD, KEY_PAD_MUL},
		{TAG_PAD_MINUS, REPORT_ID_KEYBOARD, KEY_PAD_MINUS}, {TAG_PAD_PLUS, REPORT_ID_KEYBOARD, KEY_PAD_PLUS},
		{TAG_PAD_ENTER, REPORT_ID_KEYBOARD, KEY_PAD_ENTER},
		{TAG_PAD_1, REPORT_ID_KEYBOARD, KEY_PAD_1}, {TAG_PAD_2, REPORT_ID_KEYBOARD, KEY_PAD_2}, {TAG_PAD_3, REPORT_ID_KEYBOARD, KEY_PAD_3},
		{TAG_PAD_4, REPORT_ID_KEYBOARD, KEY_PAD_4}, {TAG_PAD_5, REPORT_ID_KEYBOARD, KEY_PAD_5}, {TAG_PAD_6, REPORT_ID_KEYBOARD, KEY_PAD_6},
		{TAG_PAD_7, REPORT_ID_KEYBOARD, KEY_PAD_7}, {TAG_PAD_8, REPORT_ID_KEYBOARD, KEY_PAD_8}, {TAG_PAD_9, REPORT_ID_KEYBOARD, KEY_PAD_9},
		{TAG_PAD_0, REPORT_ID_KEYBOARD, KEY_PAD_0}, {TAG_PAD_DOT, REPORT_ID_KEYBOARD, KEY_PAD_DOT},
		{TAG_APP, REPORT_ID_KEYBOARD, KEY_APP},
		{TAG_CTRLL, REPORT_ID_KEYBOARD, KEY_CTRLL}, {TAG_SHIFTL, REPORT_ID_KEYBOARD, KEY_SHIFTL}, {TAG_ALT, REPORT_ID_KEYBOARD, KEY_ALT},
		{TAG_WINL, REPORT_ID_KEYBOARD, KEY_WINL}, {TAG_CTRLR, REPORT_ID_KEYBOARD, KEY_CTRLR}, {TAG_SHIFTR, REPORT_ID_KEYBOARD, KEY_SHIFTR},
		{TAG_ALTGR, REPORT_ID_KEYBOARD, KEY_ALTGR}, {TAG_WINR, REPORT_ID_KEYBOARD, KEY_WINR},
		{TAG_SC_POWER, REPORT_ID_SYSTEM_CONTROL, KEY_SC_POWER},
		{TAG_SC_SLEEP, REPORT_ID_SYSTEM_CONTROL, KEY_SC_SLEEP},
		{TAG_SC_WAKEUP, REPORT_ID_SYSTEM_CONTROL, KEY_SC_WAKEUP},
		{TAG_CC_PLAY, REPORT_ID_CONSUMER_CONTROL, KEY_CC_PLAY}, {TAG_CC_PAUSE, REPORT_ID_CONSUMER_CONTROL, KEY_CC_PAUSE},
		{TAG_CC_RECORD, REPORT_ID_CONSUMER_CONTROL, KEY_CC_RECORD}, {TAG_CC_FFWD, REPORT_ID_CONSUMER_CONTROL, KEY_CC_FFWD},
		{TAG_CC_RWD, REPORT_ID_CONSUMER_CONTROL, KEY_CC_RWD},
		{TAG_CC_NEXT, REPORT_ID_CONSUMER_CONTROL, KEY_CC_NEXT}, {TAG_CC_PREV, REPORT_ID_CONSUMER_CONTROL, KEY_CC_PREV},
		{TAG_CC_STOP, REPORT_ID_CONSUMER_CONTROL, KEY_CC_STOP},
		{TAG_CC_MUTE, REPORT_ID_CONSUMER_CONTROL, KEY_CC_MUTE}, {TAG_CC_VOL_UP, REPORT_ID_CONSUMER_CONTROL, KEY_CC_VOL_UP},
		{TAG_CC_VOL_DOWN, REPORT_ID_CONSUMER_CONTROL, KEY_CC_VOL_DOWN},
		{TAG_CC_CUT, REPORT_ID_CONSUMER_CONTROL, KEY_CC_CUT}, {TAG_CC_COPY, REPORT_ID_CONSUMER_CONTROL, KEY_CC_COPY},
		{TAG_CC_PASTE, REPORT_ID_CONSUMER_CONTROL, KEY_CC_PASTE}, {TAG_CC_UNDO, REPORT_ID_CONSUMER_CONTROL, KEY_CC_UNDO},
		{TAG_CC_REDO, REPORT_ID_CONSUMER_CONTROL, KEY_CC_REDO}, {TAG_CC_FIND, REPORT_ID_CONSUMER_CONTROL, KEY_CC_FIND},
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

int8_t keyboard_req_set_idle(int interface, uint8_t idle)
{
	report_enable[interface] = 1;
	report_idle[interface] = idle;
	return USBD_OK;
}

int8_t keyboard_req_get_idle(int interface, uint8_t *idle)
{
	*idle = report_idle[interface];
	return USBD_OK;
}

int8_t keyboard_req_set_protocol(int interface, uint8_t protocol)
{
	report_protocol[interface] = protocol;
	return USBD_OK;
}

int8_t keyboard_req_get_protocol(int interface, uint8_t *protocol)
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
	eve_keyboard_splash("Waiting for host...", OPTIONS_HEADER_LOGO);
}

int keyboard_loop(keyboard_in_report_structure_t *keyboard_report_in,
		keyboard_out_report_structure_t *report_buffer_out,
		control_in_report_structure_t *control_report_in)
{
	static uint8_t report = REPORT_ID_NONE;
	int i;
	uint8_t reportsend = REPORT_ID_NONE;
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
			if (report == REPORT_ID_KEYBOARD)
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
			else if (report == REPORT_ID_SYSTEM_CONTROL)
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
			else if (report == REPORT_ID_CONSUMER_CONTROL)
			{
				control_report_in->reportID = 2;
				control_report_in->consumer.arrayConsumer1 = scancode & 0xff;
				control_report_in->consumer.arrayConsumer2 = scancode >> 8;
			}
			else
			{
				report = REPORT_ID_NONE;
			}
			reportsend = report;
		}
		else
		{
			if (report == REPORT_ID_KEYBOARD)
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
			else if (report == REPORT_ID_SYSTEM_CONTROL)
			{
				control_report_in->system.sysMap = 0;
			}
			else if (report == REPORT_ID_CONSUMER_CONTROL)
			{
				control_report_in->reportID = 2;
				control_report_in->consumer.arrayConsumer1 = 0;
				control_report_in->consumer.arrayConsumer2 = 0;
			}

			reportsend = report;
			report = REPORT_ID_NONE;
		}
	}
	return reportsend;
}

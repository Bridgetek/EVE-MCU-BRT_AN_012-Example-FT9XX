/**
  @file eve_ui_keyboard.c
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


/* INCLUDES ************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "EVE_config.h"
#include "EVE.h"

#include "keyboard.h"
#include "eve_keyboard.h"
#include "eve_ui.h"
#include "eve_ui_keyboard.h"


#undef USE_EXTRA_SCREEN

#define PACK __attribute__ ((__packed__))

struct key_state
{
	uint8_t Layout;
	uint8_t Screen;
	uint32_t Components;

	struct key_scan scan;
	struct key_report report;
};

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

struct key_state key_state;

/**
 * @brief Map of ASCII codes assigned to tagged buttons.
 */
//@{
static const uint8_t keymap_num_row_non_us[] = {TAG_TILDE, TAG_1, TAG_2, TAG_3, TAG_4, TAG_5, TAG_6, TAG_7, TAG_8, TAG_9, TAG_0, TAG_MINUS, TAG_EQUALS, };
static const uint8_t keymap_top_row_non_us[] = {TAG_Q, TAG_W, TAG_E, TAG_R, TAG_T, TAG_Y, TAG_U, TAG_I, TAG_O, TAG_P, TAG_SQB_OPEN, TAG_SQB_CLS, };
static const uint8_t keymap_mid_row_non_us[] = {TAG_A, TAG_S, TAG_D, TAG_F, TAG_G, TAG_H, TAG_J, TAG_K, TAG_L, TAG_SEMICOLON, TAG_SQUOTE, TAG_HASH, };
static const uint8_t keymap_bot_row_non_us[] = {TAG_BACKSLASH, TAG_Z, TAG_X, TAG_C, TAG_V, TAG_B, TAG_N, TAG_M, TAG_COMMA, TAG_DOT, TAG_SLASH, };

static const uint8_t keymap_num_row_us[] = {TAG_TILDE, TAG_1, TAG_2, TAG_3, TAG_4, TAG_5, TAG_6, TAG_7, TAG_8, TAG_9, TAG_0, TAG_MINUS, TAG_EQUALS, };
static const uint8_t keymap_top_row_us[] = {TAG_Q, TAG_W, TAG_E, TAG_R, TAG_T, TAG_Y, TAG_U, TAG_I, TAG_O, TAG_P, TAG_SQB_OPEN, TAG_SQB_CLS, TAG_BACKSLASH, };
static const uint8_t keymap_mid_row_us[] = {TAG_A, TAG_S, TAG_D, TAG_F, TAG_G, TAG_H, TAG_J, TAG_K, TAG_L, TAG_SEMICOLON, TAG_SQUOTE, };
static const uint8_t keymap_bot_row_us[] = {TAG_Z, TAG_X, TAG_C, TAG_V, TAG_B, TAG_N, TAG_M,TAG_COMMA, TAG_DOT, TAG_SLASH, };
//@}

/** Font to be used for keyboard keys. */
static uint8_t font_normal = 0;
static uint32_t font_alt = 1;
static uint32_t screen_bg = KEY_COLOUR_BG_SCREEN;
static uint32_t led_on = KEY_COLOUR_FG_LEDS;
static uint32_t led_off = KEY_COLOUR_BG_LEDS;
static uint32_t button_fg = KEY_COLOUR_FG_BUTTONS;
static uint32_t button_fg_alt = KEY_COLOUR_FG_BUTTONS_ALT;
static uint32_t button_fg_highlight = KEY_COLOUR_FG_BUTTONS_HIGHLIGHT;
static uint32_t button_bg = KEY_COLOUR_BG_BUTTONS;
static uint32_t button_bg_alt = KEY_COLOUR_BG_BUTTONS_ALT;
static uint32_t button_bg_highlight = KEY_COLOUR_BG_BUTTONS_HIGHLIGHT;

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

static void draw_keypad(struct key_report *pReport, struct key_scan *pScan)
{
	uint32_t button_colour;

	if (key_state.Components & KEYBOARD_COMPONENTS_KEYPAD_CONTROL)
	{
		button_colour = (pScan->KeyTag == TAG_PRINT_SCREEN)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PRINT_SCREEN); // Print Screen
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(0), KEY_ROW_KEYPAD(0) + (KEY_HEIGHT_KEYPAD(1) * 1) / 6,
				KEY_WIDTH_KEYPAD(1), (KEY_HEIGHT_KEYPAD(1) *2) / 3,
				font_normal, EVE_OPT_FLAT, "PrtScr");

		button_colour = (pReport->Scroll)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_SCROLL_LOCK); // Scroll Lock
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(1), KEY_ROW_KEYPAD(0) + (KEY_HEIGHT_KEYPAD(1) * 1) / 6,
				KEY_WIDTH_KEYPAD(1), (KEY_HEIGHT_KEYPAD(1) *2) / 3,
				font_normal, EVE_OPT_FLAT, "ScrLock");

		button_colour = (pScan->KeyTag == TAG_PAUSE)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAUSE); // Pause
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(2), KEY_ROW_KEYPAD(0) + (KEY_HEIGHT_KEYPAD(1) * 1) / 6,
				KEY_WIDTH_KEYPAD(1), (KEY_HEIGHT_KEYPAD(1) *2) / 3,
				font_normal, EVE_OPT_FLAT, "Pause");

		button_colour = (pScan->KeyTag == TAG_INSERT)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_INSERT); // Insert
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(0), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "Ins");

		button_colour = (pScan->KeyTag == TAG_HOME)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_HOME); // Home
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(1), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "Home");

		button_colour = (pScan->KeyTag == TAG_PAGE_UP)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAGE_UP); // Page Up
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(2), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "PgUp");

		button_colour = (pScan->KeyTag == TAG_DEL)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_DEL); // Delete
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(0), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "Del");

		button_colour = (pScan->KeyTag == TAG_END)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_END); // End
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(1), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "End");

		button_colour = (pScan->KeyTag == TAG_PAGE_DOWN)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAGE_DOWN); // Page Down
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(2), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "PgDn");

		button_colour = (pScan->KeyTag == TAG_UP_ARROW)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_UP_ARROW); // Up Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(1), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_alt, EVE_OPT_FLAT, "\x02");

		button_colour = (pScan->KeyTag == TAG_LEFT_ARROW)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_LEFT_ARROW); // Left Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(0), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_alt, EVE_OPT_FLAT, "\x01");

		button_colour = (pScan->KeyTag == TAG_DOWN_ARROW)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_DOWN_ARROW); // Down Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(1), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_alt, EVE_OPT_FLAT, "\x04");

		button_colour = (pScan->KeyTag == TAG_RIGHT_ARROW)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_RIGHT_ARROW); // Right Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(2), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_alt, EVE_OPT_FLAT, "\x03");
	}
	else
	{
		button_colour = (pScan->KeyTag == TAG_BACKSPACE)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_BACKSPACE); // Backspace
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(8), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_alt, EVE_OPT_FLAT,"\x01");
	}

	if (key_state.Components & KEYBOARD_COMPONENTS_KEYPAD_CONTROL)
	{
		button_colour = (pReport->Numeric)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_NUMBER_LOCK); // Num Lock
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "Num");
	}

	if (key_state.Components & KEYBOARD_COMPONENTS_KEYPAD_ARITH)
	{
		button_colour = (pScan->KeyTag == TAG_PAD_DIV)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_DIV); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "/");

		button_colour = (pScan->KeyTag == TAG_PAD_MUL)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_MUL); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "*");

		button_colour = (pScan->KeyTag == TAG_PAD_MINUS)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_MINUS); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(7), KEY_ROW_KEYPAD(1),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "-");

		button_colour = (pScan->KeyTag == TAG_PAD_PLUS)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_PLUS); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(7), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(2),
				font_normal, EVE_OPT_FLAT, "+");
	}

	button_colour = (pScan->KeyTag == TAG_PAD_ENTER)?button_bg_highlight:button_bg_alt;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_PAD_ENTER); //
	EVE_CMD_BUTTON(
			KEY_COL_KEYPAD(7), KEY_ROW_KEYPAD(4),
			KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(2),
			font_normal, EVE_OPT_FLAT, "Enter");

	if (pReport->Numeric)
	{
		button_colour = (pScan->KeyTag == TAG_PAD_7)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_7); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "7");

		button_colour = (pScan->KeyTag == TAG_PAD_8)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_8); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "8");

		button_colour = (pScan->KeyTag == TAG_PAD_9)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_9); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "9");

		button_colour = (pScan->KeyTag == TAG_PAD_4)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_4); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "4");

		button_colour = (pScan->KeyTag == TAG_PAD_5)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_5); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "5");

		button_colour = (pScan->KeyTag == TAG_PAD_6)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_6); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "6");

		button_colour = (pScan->KeyTag == TAG_PAD_1)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_1); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "1");

		button_colour = (pScan->KeyTag == TAG_PAD_2)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_2); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "2");

		button_colour = (pScan->KeyTag == TAG_PAD_3)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_3); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "3");

		button_colour = (pScan->KeyTag == TAG_PAD_0)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_0); //
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(2), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "0");

		if (key_state.Components & KEYBOARD_COMPONENTS_KEYPAD_DOT)
		{
			button_colour = (pScan->KeyTag == TAG_PAD_DOT)?button_bg_highlight:button_bg;
			EVE_CMD_FGCOLOR(button_colour);
			EVE_TAG(TAG_PAD_DOT); //
			EVE_CMD_BUTTON(
					KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(5),
					KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
					font_normal, EVE_OPT_FLAT, ".");
		}
	}
	else
	{
		button_colour = (pScan->KeyTag == TAG_PAD_7)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_7); // Home
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "Home");

		button_colour = (pScan->KeyTag == TAG_PAD_8)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_8); // Up Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_alt, EVE_OPT_FLAT, "\x02");

		button_colour = (pScan->KeyTag == TAG_PAD_9)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_9); // Page Up
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(2),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "PgUp");

		button_colour = (pScan->KeyTag == TAG_PAD_4)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_4); // Left Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_alt, EVE_OPT_FLAT, "\x01");

		button_colour = (pScan->KeyTag == TAG_PAD_5)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_5); // Blank
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "");

		button_colour = (pScan->KeyTag == TAG_PAD_6)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_6); // Right Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(3),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_alt, EVE_OPT_FLAT, "\x03");

		button_colour = (pScan->KeyTag == TAG_PAD_1)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_1); // End
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "End");

		button_colour = (pScan->KeyTag == TAG_PAD_2)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_2); // Down Arrow
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(5), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_alt, EVE_OPT_FLAT, "\x04");

		button_colour = (pScan->KeyTag == TAG_PAD_3)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_3); // Page Down
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(4),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "PgDn");

		button_colour = (pScan->KeyTag == TAG_PAD_0)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_0); // Insert
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(4), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(2), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "Ins");

		button_colour = (pScan->KeyTag == TAG_PAD_DOT)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_PAD_DOT); // Delete
		EVE_CMD_BUTTON(
				KEY_COL_KEYPAD(6), KEY_ROW_KEYPAD(5),
				KEY_WIDTH_KEYPAD(1), KEY_HEIGHT_KEYPAD(1),
				font_normal, EVE_OPT_FLAT, "Del");
	}
}

static void draw_function_keys(struct key_scan *pScan)
{
	uint32_t button_colour;
	int i;

	if (key_state.Components & KEYBOARD_COMPONENTS_ESCAPE)
	{
		button_colour = (pScan->KeyTag == TAG_ESCAPE)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_ESCAPE); // Escape
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(0), KEY_ROW_ALPHA(0) + (KEY_HEIGHT_ALPHA * 1) / 6,
				KEY_WIDTH_ALPHA(1), (KEY_HEIGHT_ALPHA *2) / 3,
				font_normal, EVE_OPT_FLAT, "Esc");
	}

	if (key_state.Components & KEYBOARD_COMPONENTS_FUNCTION)
	{
		for (i = 0; i < 12; i++)
		{
			char name[4];
			sprintf(name, "F%d", i + 1);
			button_colour = (pScan->KeyTag == (TAG_F1 + i))?button_bg_highlight:button_bg_alt;
			EVE_CMD_FGCOLOR(button_colour);
			EVE_TAG(TAG_F1 + i); // F1 to F12
			EVE_CMD_BUTTON(
					KEY_COL_ALPHA(1 + i)
					+ (i>4?(KEY_WIDTH_ALPHA(1) * 0.5):0)
					+ (i>8?(KEY_WIDTH_ALPHA(1) * 0.5):0),
					KEY_ROW_ALPHA(0) + (KEY_HEIGHT_ALPHA * 1) / 6,
					KEY_WIDTH_ALPHA(1), (KEY_HEIGHT_ALPHA *2) / 3,
					font_normal, EVE_OPT_FLAT, name);
		}
	}
}

static void draw_leds(struct key_report *pReport)
{
	EVE_TAG(TAG_NO_ACTION);
	EVE_COLOR(button_bg_alt);
	EVE_BEGIN(EVE_BEGIN_RECTS);
	EVE_VERTEX2F((KEY_COL_STATUS(7) - KEY_HEIGHT_STATUS/2) * 16, KEY_ROW_STATUS(1) * 16);
	EVE_VERTEX2F((KEY_COL_STATUS(7) + KEY_HEIGHT_STATUS/2 + KEY_WIDTH_STATUS(3)) * 16, KEY_ROW_STATUS(2) * 16);

	if (pReport->Numeric)
	{
		EVE_COLOR(led_on);
	}
	else
	{
		EVE_COLOR(led_off);
	}
	EVE_POINT_SIZE(10 * 16);
	EVE_BEGIN(EVE_BEGIN_POINTS);
	EVE_VERTEX2F(
			KEY_COL_STATUS(7) * 16,
			(KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2)) * 16);

	if (pReport->Caps)
	{
		EVE_COLOR(led_on);
	}
	else
	{
		EVE_COLOR(led_off);
	}
	EVE_POINT_SIZE(10 * 16);
	EVE_BEGIN(EVE_BEGIN_POINTS);
	EVE_VERTEX2F(
			KEY_COL_STATUS(8) * 16,
			(KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2)) * 16);

	if (pReport->Scroll)
	{
		EVE_COLOR(led_on);
	}
	else
	{
		EVE_COLOR(led_off);
	}
	EVE_POINT_SIZE(10 * 16);
	EVE_BEGIN(EVE_BEGIN_POINTS);
	EVE_VERTEX2F(
			KEY_COL_STATUS(9) * 16,
			(KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2)) * 16);

	EVE_COLOR(button_fg);
	EVE_CMD_TEXT(
			KEY_COL_STATUS(7) + 36, KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2),
			font_normal, EVE_OPT_CENTERX|EVE_OPT_CENTERY,"Num");
	EVE_CMD_TEXT(
			KEY_COL_STATUS(8) + 36, KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2),
			font_normal, EVE_OPT_CENTERX|EVE_OPT_CENTERY,"Caps");
	EVE_CMD_TEXT(
			KEY_COL_STATUS(9) + 36, KEY_ROW_STATUS(1) + (KEY_HEIGHT_STATUS / 2),
			font_normal, EVE_OPT_CENTERX|EVE_OPT_CENTERY,"Scroll");
}

#ifdef KEYBOARD_USE_MEDIA_SCREEN
static void draw_media(struct key_report *pReport, struct key_scan *pScan)
{
	uint32_t button_colour;

	EVE_CMD_TEXT(
			KEY_COL_STATUS(0), KEY_ROW_STATUS(3) + (KEY_HEIGHT_STATUS / 2),
			font_normal, EVE_OPT_CENTERY,"Volume:");

	button_colour = (pScan->KeyTag == TAG_CC_MUTE)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CC_MUTE); // Volume mute
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(1), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "Mute");

	button_colour = (pScan->KeyTag == TAG_CC_VOL_DOWN)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CC_VOL_DOWN); // Volume down
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(3), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "Down");

	button_colour = (pScan->KeyTag == TAG_CC_VOL_UP)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CC_VOL_UP); // Volume up
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(2), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "Up");

	button_colour = (pScan->KeyTag == TAG_CC_PREV)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CC_PREV); // Previous Track
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(4.5), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "Prev");

	button_colour = (pScan->KeyTag == TAG_CC_STOP)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CC_STOP); // Stop Track
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(5.5), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "Stop");

	button_colour = (pScan->KeyTag == TAG_CC_PAUSE)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CC_PAUSE); // Pause Track
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(6.5), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "Pause");

	button_colour = (pScan->KeyTag == TAG_CC_PLAY)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CC_PLAY); // Play Track
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(7.5), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "Play");

	button_colour = (pScan->KeyTag == TAG_CC_NEXT)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CC_NEXT); // Next Track
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(8.5), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "Next");

	EVE_CMD_TEXT(
			KEY_COL_STATUS(0), KEY_ROW_STATUS(6) + (KEY_HEIGHT_STATUS / 2),
			font_normal, EVE_OPT_CENTERY,"Other:");

	button_colour = (pScan->KeyTag == TAG_CC_FIND)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CC_FIND); // Find
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(1), KEY_ROW_STATUS(6),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "Find...");

	EVE_CMD_TEXT(
			KEY_COL_STATUS(0), KEY_ROW_STATUS(5) + (KEY_HEIGHT_STATUS / 2),
			font_normal, EVE_OPT_CENTERY,"Power:");

	button_colour = (pScan->KeyTag == TAG_SC_SLEEP)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_SC_SLEEP); // Power sleep
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(1), KEY_ROW_STATUS(5),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "Sleep");

}
#endif // KEYBOARD_USE_MEDIA_SCREEN

static void draw_layout_selectors(struct key_state *pState)
{
	uint32_t button_colour;

	EVE_CMD_TEXT(
			KEY_COL_STATUS(0), KEY_ROW_STATUS(3) + (KEY_HEIGHT_STATUS / 2),
			font_normal, EVE_OPT_CENTERY,"Layout:");

	button_colour = (pState->Layout == KEYBOARD_LAYOUT_PC_US_ALPHA)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_US_ALPHA); // US Keyboard Layout
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(1), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "US");

	button_colour = (pState->Layout == KEYBOARD_LAYOUT_PC_UK_ALPHA)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_UK_ALPHA); // UK Keyboard Layout
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(2), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "UK");

	button_colour = (pState->Layout == KEYBOARD_LAYOUT_PC_DE_ALPHA)?button_bg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_DE_ALPHA); // German Keyboard Layout
	EVE_CMD_BUTTON(
			KEY_COL_STATUS(3), KEY_ROW_STATUS(3),
			KEY_WIDTH_STATUS(1), KEY_HEIGHT_STATUS,
			font_normal, EVE_OPT_FLAT, "DE");
}

static void draw_keyboard_fixed_keys(struct key_report *pReport, struct key_scan *pScan)
{
	uint32_t button_colour;

	button_colour = (pScan->KeyTag == TAG_BACKSPACE)?button_bg_highlight:button_bg_alt;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_BACKSPACE); // Backspace
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(13), KEY_ROW_ALPHA(1),
			KEY_WIDTH_ALPHA(2), KEY_HEIGHT_ALPHA,
			font_alt, EVE_OPT_FLAT,"\x01");

	if (key_state.Components & KEYBOARD_COMPONENTS_TAB)
	{
		button_colour = (pScan->KeyTag == TAG_TAB)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_TAB); // Tab
		if (pScan->ShiftL || pScan->ShiftR)
		{
			EVE_CMD_BUTTON(
					KEY_COL_ALPHA(0), KEY_ROW_ALPHA(2),
					KEY_WIDTH_ALPHA(1.5), KEY_HEIGHT_ALPHA,
					font_alt, EVE_OPT_FLAT,"\x05\x01");
		}
		else
		{
			EVE_CMD_BUTTON(
					KEY_COL_ALPHA(0), KEY_ROW_ALPHA(2),
					KEY_WIDTH_ALPHA(1.5), KEY_HEIGHT_ALPHA,
					font_alt, EVE_OPT_FLAT,"\x03\x05");
		}
	}

	button_colour = (pReport->Caps)?button_bg_highlight:button_bg_alt;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CAPS_LOCK); // Capslock
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(0), KEY_ROW_ALPHA(3),
			KEY_WIDTH_ALPHA(1.75), KEY_HEIGHT_ALPHA,
			font_normal, EVE_OPT_FLAT,"CapsLock");

	button_colour = (pScan->KeyTag == TAG_SPACE)?  button_fg_highlight:button_bg;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_SPACE); // Space
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(4), KEY_ROW_ALPHA(5),
			KEY_WIDTH_ALPHA(5.75), KEY_HEIGHT_ALPHA,
			font_normal, EVE_OPT_FLAT, "Space");

	button_colour = (pScan->ShiftR)?button_bg_highlight:button_bg_alt;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_SHIFTR); // Shift Right
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(12.25), KEY_ROW_ALPHA(4),
			KEY_WIDTH_ALPHA(2.75), KEY_HEIGHT_ALPHA,
			font_normal, EVE_OPT_FLAT,"Shift");

	if (key_state.Components & KEYBOARD_COMPONENTS_MODIFIERS)
	{
		button_colour = (pScan->CtrlL)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_CTRLL); // Ctrl Left
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(0), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.5), KEY_HEIGHT_ALPHA,
				font_normal, EVE_OPT_FLAT,"Ctrl");

		button_colour = (pScan->WinL)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_WINL); // Win Left
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(1.5), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
				font_normal, EVE_OPT_FLAT,"Gui");

		button_colour = (pScan->Alt)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_ALT); // Alt Left
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(2.75), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
				font_normal, EVE_OPT_FLAT,"Alt");

		button_colour = (pScan->AltGr)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_ALTGR); // AltGr
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(9.75), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
				font_normal, EVE_OPT_FLAT,"AltGr");

		button_colour = (pScan->WinR)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_WINR); // Win Right
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(11), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
				font_normal, EVE_OPT_FLAT, "Gui");

		button_colour = (pScan->KeyTag == TAG_APP)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_APP); // Context menu
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(12.25), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
				font_normal, EVE_OPT_FLAT, "Menu");

		button_colour = (pScan->CtrlR)?button_bg_highlight:button_bg_alt;
		EVE_CMD_FGCOLOR(button_colour);
		EVE_TAG(TAG_CTRLR); // Ctrl Right
		EVE_CMD_BUTTON(
				KEY_COL_ALPHA(13.5), KEY_ROW_ALPHA(5),
				KEY_WIDTH_ALPHA(1.5), KEY_HEIGHT_ALPHA,
				font_normal, EVE_OPT_FLAT, "Ctrl");
	}
}

static uint8_t draw_keys(int16_t x, int16_t y,
		uint16_t options, const char *display, const uint8_t *tag_array, uint8_t offset_map)
{
	uint32_t button_colour;
	int i;
	int count = strlen(display);
	char str[2] = {0,0};
	int16_t use_font;
	uint8_t cdisp;
	uint8_t cmap;

	for (i = 0; i < count; i++)
	{
		cdisp = (uint8_t)display[i];
		cmap = (uint8_t)tag_array[i];

		button_colour = (cmap == (uint8_t)options)?button_bg_highlight:button_bg;
		EVE_CMD_FGCOLOR(button_colour);

		EVE_TAG(cmap);

		use_font = font_normal;
		if (cdisp >= 0x80)
		{
			// Special encoding in the custom gfont file
			use_font = font_alt;
			switch (cdisp)
			{
			case (uint8_t)0xac: // not sign
																																										cdisp = '\x10';
			break;
			case (uint8_t)0xa3: // Pound sign
																																										cdisp = '\x11';
			break;
			case (uint8_t)0x80: // Euro sign
																																										cdisp = '\x12';
			break;
			case (uint8_t)0xa2: // Cent sign
																																										cdisp = '\x13';
			break;
			case (uint8_t)0xa5: // Yen sign
																																										cdisp = '\x14';
			break;
			case (uint8_t)0xa7: // section sign
																																										cdisp = '\x20';
			break;
			case (uint8_t)0xdc: // Capital U umlaut
																																										cdisp = '\x21';
			break;
			case (uint8_t)0xfc: // Lowercase u umlaut
																																										cdisp = '\x22';
			break;
			case (uint8_t)0xd6: // Capital O umlaut
																																										cdisp = '\x23';
			break;
			case (uint8_t)0xf6: // Lowercase o umlaut
																																										cdisp = '\x24';
			break;
			case (uint8_t)0xc4: // Capital A umlaut
																																										cdisp = '\x25';
			break;
			case (uint8_t)0xe4: // Lowercase a umlaut
																																										cdisp = '\x26';
			break;
			case (uint8_t)0xd0: // Degree symbol
																																										cdisp = '\x27';
			break;
			case (uint8_t)'`': // Back tick, grave accent 0x60
																																										cdisp = '\x28';
			break;
			case (uint8_t)0xb4: // Front tick, acute accent
																																										cdisp = '\x29';
			break;
			case (uint8_t)0xdf: // eszett
																																										cdisp = '\x2a';
			break;
			default:
				cdisp = '\x0f';
			}
		}

		str[0] = (char)cdisp;
		EVE_CMD_BUTTON(
				x + KEY_COL_ALPHA(i), y,
				KEY_WIDTH_ALPHA(1) , KEY_HEIGHT_ALPHA,
				use_font, EVE_OPT_FLAT, str);
		offset_map++;
	}

	return offset_map;
}

static void draw_fixed_keys_uk_de(struct key_scan *pScan)
{
	uint32_t button_colour;

	button_colour = (pScan->KeyTag == TAG_ENTER)?button_bg_highlight:button_bg_alt;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_ENTER); // Enter
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(13.75), KEY_ROW_ALPHA(2),
			KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA * 2 + KEY_SPACER_ALPHA,
			font_normal, EVE_OPT_FLAT,"Enter");

	button_colour = (pScan->ShiftL)?button_bg_highlight:button_bg_alt;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_SHIFTL); // Shift Left
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(0), KEY_ROW_ALPHA(4),
			KEY_WIDTH_ALPHA(1.25), KEY_HEIGHT_ALPHA,
			font_normal, EVE_OPT_FLAT,"Shift");
}

static void draw_de_keyboard(struct key_report *pReport, struct key_scan *pScan)
{
	uint8_t offset = 1;

	draw_fixed_keys_uk_de(pScan);

	if (pScan->ShiftL || pScan->ShiftR)
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				pScan->KeyTag, "\xd0!\"\xa7$%&/()=?`", keymap_num_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
				pScan->KeyTag, "QWERTZUIOP\xdc*", keymap_top_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
				pScan->KeyTag, "ASDFGHJKL\xd6\xc4'", keymap_mid_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
				pScan->KeyTag, ">YXCVBNM;:_", keymap_bot_row_non_us, offset);
	}
	else
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				pScan->KeyTag, "^1234567890ߴ\xdf`",keymap_num_row_non_us, offset);

		if (pReport->Caps)
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					pScan->KeyTag, "QWERTZUIOP\xdc+", keymap_top_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					pScan->KeyTag, "ASDFGHJKL\xd6\xc4#", keymap_mid_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
					pScan->KeyTag, "<YXCVBNM,.-", keymap_bot_row_non_us, offset);
		}
		else
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					pScan->KeyTag, "qwertzuiop\xfc+", keymap_top_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					pScan->KeyTag, "asdfghjkl\xf6\xe4#", keymap_mid_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
					pScan->KeyTag, "<yxcvbnm,.-", keymap_bot_row_non_us, offset);
		}
	}
	draw_function_keys(pScan);
	draw_keyboard_fixed_keys(pReport, pScan);
}

static void draw_uk_keyboard(struct key_report *pReport, struct key_scan *pScan)
{
	uint8_t offset = 1;

	draw_fixed_keys_uk_de(pScan);

	if (pScan->ShiftL || pScan->ShiftR)
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				pScan->KeyTag, "\xac!\"\xa3$%^&*()_+", keymap_num_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
				pScan->KeyTag, "QWERTYUIOP{}", keymap_top_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
				pScan->KeyTag, "ASDFGHJKL:@~", keymap_mid_row_non_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
				pScan->KeyTag, "|ZXCVBNM<>?", keymap_bot_row_non_us, offset);
	}
	else
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				pScan->KeyTag, "`1234567890-=", keymap_num_row_non_us, offset);

		if (pReport->Caps)
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					pScan->KeyTag, "QWERTYUIOP[]", keymap_top_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					pScan->KeyTag, "ASDFGHJKL;'#", keymap_mid_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
					pScan->KeyTag, "\\ZXCVBNM,./", keymap_bot_row_non_us, offset);
		}
		else
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					pScan->KeyTag, "qwertyuiop[]", keymap_top_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					pScan->KeyTag, "asdfghjkl;'#", keymap_mid_row_non_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.25),  KEY_ROW_ALPHA(4),
					pScan->KeyTag, "\\zxcvbnm,./", keymap_bot_row_non_us, offset);
		}
	}
	draw_function_keys(pScan);
	draw_keyboard_fixed_keys(pReport, pScan);
}

static void draw_us_keyboard(struct key_report *pReport, struct key_scan *pScan)
{
	uint32_t button_colour;
	uint8_t offset = 1;

	button_colour = (pScan->KeyTag == TAG_ENTER)?button_bg_highlight:button_bg_alt;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_ENTER); // Enter
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(12.75), KEY_ROW_ALPHA(3),
			KEY_WIDTH_ALPHA(2.25), KEY_HEIGHT_ALPHA,
			font_normal, EVE_OPT_FLAT,"Enter");

	button_colour = (pScan->ShiftL)?button_bg_highlight:button_bg_alt;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_SHIFTL); // Shift Left
	EVE_CMD_BUTTON(
			KEY_COL_ALPHA(0), KEY_ROW_ALPHA(4),
			KEY_WIDTH_ALPHA(2.25), KEY_HEIGHT_ALPHA,
			font_normal, EVE_OPT_FLAT,"Shift");

	if (pScan->ShiftL || pScan->ShiftR)
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				pScan->KeyTag, "~!@#$%^&*()_+", keymap_num_row_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
				pScan->KeyTag, "QWERTYUIOP{}|", keymap_top_row_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
				pScan->KeyTag, "ASDFGHJKL:\"", keymap_mid_row_us, offset);

		offset = draw_keys(KEY_COL_ALPHA(2.25),  KEY_ROW_ALPHA(4),
				pScan->KeyTag, "ZXCVBNM<>?", keymap_bot_row_us, offset);
	}
	else
	{
		offset = draw_keys(KEY_COL_ALPHA(0),  KEY_ROW_ALPHA(1),
				pScan->KeyTag, "`1234567890-=", keymap_num_row_us, offset);

		if (pReport->Caps)
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					pScan->KeyTag, "QWERTYUIOP[]\\", keymap_top_row_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					pScan->KeyTag, "ASDFGHJKL;\'", keymap_mid_row_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(2.25),  KEY_ROW_ALPHA(4),
					pScan->KeyTag, "ZXCVBNM,./", keymap_bot_row_us, offset);
		}
		else
		{
			offset = draw_keys(KEY_COL_ALPHA(1.5),  KEY_ROW_ALPHA(2),
					pScan->KeyTag, "qwertyuiop[]\\", keymap_top_row_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(1.75),  KEY_ROW_ALPHA(3),
					pScan->KeyTag, "asdfghjkl;'", keymap_mid_row_us, offset);

			offset = draw_keys(KEY_COL_ALPHA(2.25),  KEY_ROW_ALPHA(4),
					pScan->KeyTag, "zxcvbnm,./", keymap_bot_row_us, offset);
		}
	}
	draw_function_keys(pScan);
	draw_keyboard_fixed_keys(pReport, pScan);
}

void eve_ui_keyboard_draw(struct key_report *pReport, struct key_scan *pScan)
{
	EVE_COLOR(button_fg);

	// Colour for Special Function Keys
	EVE_CMD_FGCOLOR(button_fg_alt);
	EVE_CMD_BGCOLOR(button_bg_alt);

	if (key_state.Screen == KEYBOARD_SCREEN_SETTINGS)
	{
		draw_layout_selectors(&key_state);
	}
	else if (key_state.Screen == KEYBOARD_SCREEN_ALPHANUMERIC)
	{
		if (key_state.Layout == KEYBOARD_LAYOUT_PC_UK_ALPHA)
		{
			draw_uk_keyboard(pReport, pScan);
		}
		else if (key_state.Layout == KEYBOARD_LAYOUT_PC_US_ALPHA)
		{
			draw_us_keyboard(pReport, pScan);
		}
		else if (key_state.Layout == KEYBOARD_LAYOUT_PC_DE_ALPHA)
		{
			draw_de_keyboard(pReport, pScan);
		}
	}
#ifdef KEYBOARD_USE_MEDIA_SCREEN
	else if (key_state.Screen == KEYBOARD_SCREEN_MEDIA)
	{
		draw_media(pReport, pScan);
	}
#endif // KEYBOARD_USE_MEDIA_SCREEN
	else if (key_state.Screen == KEYBOARD_SCREEN_KEYPAD)
	{
		draw_keypad(pReport, pScan);
	}

	if (key_state.Screen != KEYBOARD_SCREEN_SETTINGS)
	{
		if (key_state.Components & KEYBOARD_COMPONENTS_LEDS)
		{
			draw_leds(pReport);
		}
	}
}


void eve_ui_keyboard_set_layout(uint8_t layout)
{
	key_state.Layout = layout;
}

uint8_t eve_ui_keyboard_get_layout(void)
{
	return key_state.Layout;
}

void eve_ui_keyboard_set_components(uint32_t components)
{
	key_state.Components = components;
}

uint32_t eve_ui_keyboard_get_components(void)
{
	return key_state.Components;
}

void eve_ui_keyboard_set_screen(uint8_t screen)
{
	static uint8_t push_screen = KEYBOARD_SCREEN_SETTINGS;

	if ((key_state.Screen == KEYBOARD_SCREEN_SETTINGS)
			&& (screen == KEYBOARD_SCREEN_SETTINGS))
	{
		key_state.Screen = push_screen;
	}
	else if (screen == KEYBOARD_SCREEN_SETTINGS)
	{
		push_screen = key_state.Screen;
		key_state.Screen = screen;
	}
	else
	{
		key_state.Screen = screen;
	}
}

uint8_t eve_ui_keyboard_get_screen(void)
{
	return key_state.Screen;
}

uint8_t eve_ui_keyboard_loop(struct key_report *pReport, struct key_scan *pScan)
{
	static uint8_t key_pressed = KEY_PRESS_NONE;
	uint8_t ret = -1;
	uint8_t tag_code = 0;
	uint8_t key_detect = 0;
	uint8_t key_change = 0;
	uint8_t led_change = 0;

	pScan->KeyTag = 0;
	key_detect = eve_ui_read_tag(&tag_code);

	if (key_detect)
	{
		// A key is currently pressed. If it was not pressed the last time
		// we came past then signal a new keypress.
		if (key_pressed == KEY_PRESS_NONE)
		{
			eve_ui_play_sound(0x51, 100);

			if (tag_code != 0)
			{
				key_pressed = KEY_PRESS_SCAN;
				key_change = 1;

				// Check for modifiers.
				// Modifier keys are 'sticky' for resistive touchscreens as only one
				// touch can be registered at a time.
				// Turn scan keys into modifiers if appropriate.
				switch(tag_code)
				{
				case TAG_SHIFTL:
					pScan->ShiftL ^= 1;       // toggle the shift button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case TAG_SHIFTR:
					pScan->ShiftR ^= 1;       // toggle the shift button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case TAG_CTRLL:
					pScan->CtrlL ^= 1;       // toggle the Ctrl button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case TAG_CTRLR:
					pScan->CtrlR ^= 1;       // toggle the Ctrl button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case TAG_ALT:
					pScan->Alt ^= 1;        // toggle the Alt button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case TAG_ALTGR:
					pScan->AltGr ^= 1;      // toggle the AltGr button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case TAG_WINL:
					pScan->WinL ^= 1;        // toggle the Windows button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case TAG_WINR:
					pScan->WinR ^= 1;        // toggle the Windows button on when the key detect
					key_pressed = KEY_PRESS_MODIFIER;
					break;

				case TAG_CAPS_LOCK:
					led_change = 1;
					if (pReport->Caps == 0)
					{
						pReport->Caps = 1;
					}
					else
					{
						pReport->Caps = 0;
					}
					break;

				case TAG_SCROLL_LOCK:
					led_change = 1;
					if (pReport->Scroll == 0)
					{
						pReport->Scroll = 1;
					}
					else
					{
						pReport->Scroll = 0;
					}
					break;

				case TAG_NUMBER_LOCK:
					led_change = 1;
					if (pReport->Numeric == 0)
					{
						pReport->Numeric = 1;
					}
					else
					{
						pReport->Numeric = 0;
					}
					break;
				}
				if (key_pressed == KEY_PRESS_SCAN)
				{
					pScan->KeyTag = tag_code;
				}
			}
		}
	}
	else
	{
		// No key is currently pressed. If one was pressed the last time we
		// came past then signal a key off event.
		if (key_pressed == KEY_PRESS_SCAN)
		{
			key_change = 1;
		}
		key_pressed = KEY_PRESS_NONE;
	}

	if ((key_change) || (led_change))
	{
		ret = 0;
	}

	return ret;
}

void eve_ui_keyboard_font(uint8_t font, uint8_t altfont)
{
	font_normal = font;
	font_alt = altfont;
}

void eve_ui_keyboard_screen(uint32_t bg)
{
	screen_bg = bg;
}

void eve_ui_keyboard_leds(uint32_t on, uint32_t off)
{
	led_on = on;
	led_off = off;
}

void eve_ui_keyboard_buttons(uint32_t fg, uint32_t fg_alt, uint32_t fg_highlight,
		uint32_t bg, uint32_t bg_alt, uint32_t bg_highlight)
{
	button_fg = fg;
	button_fg_alt = fg_alt;
	button_fg_highlight = fg_highlight;
	button_bg = bg;
	button_bg_alt = bg_alt;
	button_bg_highlight = bg_highlight;
}

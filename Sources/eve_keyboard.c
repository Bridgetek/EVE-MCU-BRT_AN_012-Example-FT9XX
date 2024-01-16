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

#include "eve_ui.h"
#include "eve_ram_g.h"
#include "keyboard.h"
#include "eve_keyboard.h"
#include "eve_ui_keyboard.h"

#define PACK __attribute__ ((__packed__))

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/**
 @brief Dimensions of custom images.
 */
//@{
uint16_t img_bridgetek_logo_width;
uint16_t img_bridgetek_logo_height;
uint16_t img_settings_width;
uint16_t img_settings_height;
uint16_t img_cancel_width;
uint16_t img_cancel_height;
uint16_t img_tick_width;
uint16_t img_tick_height;
uint16_t img_refresh_width;
uint16_t img_refresh_height;
uint16_t img_keypad_width;
uint16_t img_keypad_height;
uint16_t img_keyboard_width;
uint16_t img_keyboard_height;
uint16_t img_media_width;
uint16_t img_media_height;
uint16_t img_special_width;
uint16_t img_special_height;
//@}

static struct key_scan key_scan;
static struct key_report key_report;
static uint8_t key_mode = KEYBOARD_DISPLAY_KEYBOARD;

static uint16_t keyboard_header_options = 0;

static uint16_t keyboard_font_width = 0;
static uint16_t keyboard_font_height = 0;

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

static void eve_header_bar(uint32_t options);

/* FUNCTIONS ***********************************************************************/

// Extern links to fonts which are used here.
extern const uint8_t EVE_UI_FLASH font_Montserrat_Bold_ttf_15_L4[];
extern const uint32_t font_Montserrat_Bold_ttf_15_L4_size;
extern const uint8_t EVE_UI_FLASH font_arial_ttf_15_L4[];
extern const uint32_t font_arial_ttf_15_L4_size;
extern const uint8_t EVE_UI_FLASH img_bridgetek_logo_jpg[], img_end_bridgetek_logo_jpg[];
extern const uint8_t EVE_UI_FLASH img_cancel_jpg[], img_end_cancel_jpg[];
extern const uint8_t EVE_UI_FLASH img_keyboard_jpg[], img_end_keyboard_jpg[];
extern const uint8_t EVE_UI_FLASH img_keypad_jpg[], img_end_keypad_jpg[];
extern const uint8_t EVE_UI_FLASH img_media_jpg[], img_end_media_jpg[];
extern const uint8_t EVE_UI_FLASH img_refresh_jpg[], img_end_refresh_jpg[];
extern const uint8_t EVE_UI_FLASH img_settings_jpg[], img_end_settings_jpg[];
extern const uint8_t EVE_UI_FLASH img_tick_jpg[], img_end_tick_jpg[];
extern const uint8_t EVE_UI_FLASH img_special_jpg[], img_end_special_jpg[];

void eve_keyboard_splash(char *toast, uint32_t options)
{
	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_CLEAR_COLOR_RGB(0, 0, 0);
	EVE_CLEAR(1,1,1);
	//EVE_CLEAR_TAG(TAG_NO_ACTION);
	EVE_COLOR_RGB(255, 255, 255);
	eve_header_bar(EVE_HEADER_LOGO);
	EVE_CMD_TEXT(EVE_DISP_WIDTH/2, EVE_DISP_HEIGHT/2,
			FONT_HEADER, EVE_OPT_CENTERX | EVE_OPT_CENTERY, toast);
	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();
}

static void eve_header_bar(uint32_t options)
{
	uint32_t x = EVE_SPACER;

	// No tint on bitmaps drawn.
	EVE_CMD_FGCOLOR(0xffffff);

	if (options & OPTIONS_HEADER_LOGO)
	{
		EVE_TAG(TAG_LOGO);
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		EVE_VERTEX_TRANSLATE_X(((EVE_DISP_WIDTH/2)-((uint32_t)img_bridgetek_logo_width/2)) * 16);
		EVE_VERTEX2II(0, 0, BITMAP_BRIDGETEK_LOGO, 0);
	}
	EVE_VERTEX_TRANSLATE_Y(EVE_SPACER * 16);
	if (options & OPTIONS_HEADER_SETTINGS_BUTTON)
	{
		EVE_TAG(TAG_SETTINGS);
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_SETTINGS, 0);
		x += (img_settings_width + EVE_SPACER);
	}
	if (options & OPTIONS_HEADER_REFRESH_BUTTON)
	{
		EVE_TAG(TAG_REFRESH);
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_REFRESH, 0);
		x += (img_refresh_width + EVE_SPACER);
	}
	if (options & OPTIONS_HEADER_CANCEL_BUTTON)
	{
		EVE_TAG(TAG_CANCEL);
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_CANCEL, 0);
		x += (img_cancel_width + EVE_SPACER);
	}
	if (options & OPTIONS_HEADER_TICK_BUTTON)
	{
		EVE_TAG(TAG_TICK);
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_TICK, 0);
		x += (img_tick_width + EVE_SPACER);
	}

	x = EVE_DISP_WIDTH - EVE_SPACER;
	if (options & OPTIONS_HEADER_KEYPAD_BUTTON)
	{
		EVE_TAG(TAG_KEYPAD);
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		x -= (img_keypad_width + EVE_SPACER);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_KEYPAD, 0);
	}
	if (options & OPTIONS_HEADER_KEYBOARD_BUTTON)
	{
		EVE_TAG(TAG_KEYBOARD);
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		x -= (img_keyboard_width + EVE_SPACER);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_KEYBOARD, 0);
	}
	if (options & OPTIONS_HEADER_MEDIA_BUTTON)
	{
		EVE_TAG(TAG_MEDIA);
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		x -= (img_media_width + EVE_SPACER);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_MEDIA, 0);
	}
	if (options & OPTIONS_HEADER_CUSTOM_BUTTON)
	{
		EVE_TAG(TAG_CUSTOM);
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		x -= (img_special_width + EVE_SPACER);
		EVE_VERTEX_TRANSLATE_X(x * 16);
		EVE_VERTEX2II(0, 0, BITMAP_CUSTOM, 0);
	}

	// Restore offset.
	EVE_VERTEX_TRANSLATE_X(0);
	EVE_VERTEX_TRANSLATE_Y(0);
}

void eve_splash(char *toast, uint32_t options)
{
	uint16_t width = 0, height = keyboard_font_height * 2;
	int c;

	for (c = 0; toast[c]; c++)
	{
		width += keyboard_font_width;
	    if (toast[c] == '\n')
	    {
	        height += keyboard_font_height;
	    }
	}

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_CLEAR_COLOR(KEY_COLOUR_BG_SCREEN);
	EVE_CLEAR(1,1,1);
	EVE_CLEAR_TAG(TAG_NO_ACTION);
	eve_header_bar(options);

	EVE_TAG(TAG_NO_ACTION);
	EVE_BEGIN(EVE_BEGIN_RECTS);
	EVE_COLOR(KEY_COLOUR_BG_BUTTONS);
	EVE_VERTEX2F((EVE_DISP_WIDTH - width) * 8, (EVE_DISP_HEIGHT - height) * 8);
	EVE_VERTEX2F((EVE_DISP_WIDTH + width) * 8, (EVE_DISP_HEIGHT + height) * 8);
	EVE_COLOR(KEY_COLOUR_FG_BUTTONS);
	EVE_CMD_TEXT(EVE_DISP_WIDTH/2, EVE_DISP_HEIGHT/2,
			FONT_HEADER, EVE_OPT_CENTERX|EVE_OPT_CENTERY, toast);
	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();
}

#ifdef KEYBOARD_USE_CUSTOM_SCREEN
void eve_custom_draw(struct key_report *pReport, struct key_scan *pScan)
{
	uint32_t button_colour;

	// Custom display goes here.
	button_colour = (pScan->KeyTag == TAG_CUSTOM)?KEY_COLOUR_BG_BUTTONS_HIGHLIGHT:KEY_COLOUR_BG_BUTTONS;
	EVE_CMD_FGCOLOR(button_colour);
	EVE_TAG(TAG_CUSTOM); // Volume mute
	EVE_CMD_BUTTON(
			EVE_DISP_WIDTH/2 - 100, EVE_DISP_HEIGHT/2 - 40,
			200, 80,
			KEYBOARD_FONT, EVE_OPT_FLAT, "Back to Keyboard");
}
#endif // KEYBOARD_USE_CUSTOM_SCREEN

void eve_keyboard_start(void)
{
	init_ram_g();

	eve_ui_load_font(font_Montserrat_Bold_ttf_15_L4, font_Montserrat_Bold_ttf_15_L4_size, KEYBOARD_FONT);
	eve_ui_load_font(font_arial_ttf_15_L4, font_arial_ttf_15_L4_size, KEYBOARD_FONT_ALT);

	eve_ui_font_size(KEYBOARD_FONT, &keyboard_font_width, &keyboard_font_height);

	eve_ui_jpg_image_size(img_bridgetek_logo_jpg, &img_bridgetek_logo_width, &img_bridgetek_logo_height);
	eve_ui_load_jpg(img_bridgetek_logo_jpg, img_end_bridgetek_logo_jpg - img_bridgetek_logo_jpg, BITMAP_BRIDGETEK_LOGO);
	eve_ui_jpg_image_size(img_cancel_jpg, &img_cancel_width, &img_cancel_height);
	eve_ui_load_jpg(img_cancel_jpg, img_end_cancel_jpg - img_cancel_jpg, BITMAP_CANCEL);
	eve_ui_jpg_image_size(img_settings_jpg, &img_settings_width, &img_settings_height);
	eve_ui_load_jpg(img_settings_jpg, img_end_settings_jpg - img_settings_jpg, BITMAP_SETTINGS);
	eve_ui_jpg_image_size(img_tick_jpg, &img_tick_width, &img_tick_height);
	eve_ui_load_jpg(img_tick_jpg, img_end_tick_jpg - img_tick_jpg, BITMAP_TICK);
	eve_ui_jpg_image_size(img_refresh_jpg, &img_refresh_width, &img_refresh_height);
	eve_ui_load_jpg(img_refresh_jpg, img_end_refresh_jpg - img_refresh_jpg, BITMAP_REFRESH);
	eve_ui_jpg_image_size(img_keypad_jpg, &img_keypad_width, &img_keypad_height);
	eve_ui_load_jpg(img_keypad_jpg, img_end_keypad_jpg - img_keypad_jpg, BITMAP_KEYPAD);
	eve_ui_jpg_image_size(img_keyboard_jpg, &img_keyboard_width, &img_keyboard_height);
	eve_ui_load_jpg(img_keyboard_jpg, img_end_keyboard_jpg - img_keyboard_jpg, BITMAP_KEYBOARD);
	eve_ui_jpg_image_size(img_media_jpg, &img_media_width, &img_media_height);
	eve_ui_load_jpg(img_media_jpg, img_end_media_jpg - img_media_jpg, BITMAP_MEDIA);
	eve_ui_jpg_image_size(img_special_jpg, &img_special_width, &img_special_height);
	eve_ui_load_jpg(img_special_jpg, img_end_special_jpg - img_special_jpg, BITMAP_CUSTOM);

	memset(&key_scan, 0xff, sizeof(struct key_scan));
	memset(&key_report, 0xff, sizeof(struct key_report));

	eve_ui_keyboard_font(KEYBOARD_FONT, KEYBOARD_FONT_ALT);
	eve_ui_keyboard_screen(KEY_COLOUR_BG_SCREEN);
	eve_ui_keyboard_leds(KEY_COLOUR_FG_LEDS,
			KEY_COLOUR_BG_LEDS);
	eve_ui_keyboard_buttons(
			KEY_COLOUR_FG_BUTTONS,
			KEY_COLOUR_FG_BUTTONS_ALT,
			KEY_COLOUR_FG_BUTTONS_HIGHLIGHT,
			KEY_COLOUR_BG_BUTTONS,
			KEY_COLOUR_BG_BUTTONS_ALT,
			KEY_COLOUR_BG_BUTTONS_HIGHLIGHT
	);
	eve_ui_keyboard_set_screen(KEYBOARD_SCREEN_ALPHANUMERIC);
	eve_ui_keyboard_set_layout(KEYBOARD_LAYOUT_PC_UK_ALPHA);
	eve_ui_keyboard_set_components(KEYBOARD_COMPONENTS_FULL_KEYBOARD);
	keyboard_header_options = OPTIONS_HEADER_LOGO | OPTIONS_HEADER_SETTINGS_BUTTON
#ifdef KEYBOARD_USE_MEDIA_SCREEN
			| OPTIONS_HEADER_MEDIA_BUTTON
#endif
#ifdef KEYBOARD_USE_KEYPAD_SCREEN
			| OPTIONS_HEADER_KEYPAD_BUTTON
#endif
#ifdef KEYBOARD_USE_KEYBOARD_SCREEN
			| OPTIONS_HEADER_KEYBOARD_BUTTON
#endif
#ifdef KEYBOARD_USE_MEDIA_SCREEN
			| OPTIONS_HEADER_MEDIA_BUTTON
#endif
#ifdef KEYBOARD_USE_CUSTOM_SCREEN
			| OPTIONS_HEADER_CUSTOM_BUTTON
#endif
			;
}

int8_t eve_keyboard_loop(struct key_report *pReport, struct key_scan *pScan)
{
	uint8_t scan_change = 0;
	uint8_t led_change = 0;
	uint8_t screen_change = 0;

	if (pReport)
	{
		// Compare incoming settings with currently displayed settings.
		if ((pReport->Caps != key_report.Caps)
				|| (pReport->Scroll != key_report.Scroll)
				|| (pReport->Numeric != key_report.Numeric))
		{
			// Signal
			led_change = 1;
			key_report.Caps = pReport->Caps;
			key_report.Scroll = pReport->Scroll;
			key_report.Numeric = pReport->Numeric;
		}
	}

	if (pScan)
	{
		memcpy(&key_scan, pScan, sizeof(struct key_scan));
	}

#ifdef KEYBOARD_USE_CUSTOM_SCREEN
	if (key_mode == KEYBOARD_DISPLAY_CUSTOM)
	{
		// Custom keyboard operation:
		scan_change = eve_ui_keyboard_loop(&key_report, &key_scan);

		if (scan_change == 0)
		{
			switch (key_scan.KeyTag)
			{
			case TAG_CUSTOM:
				// Revert to normal keyboard
				key_mode = KEYBOARD_DISPLAY_KEYBOARD;
				screen_change = 1;
				break;
			}

			if (screen_change)
			{
				pScan->KeyTag = 0;
			}
		}
	}
	else
#endif // KEYBOARD_USE_CUSTOM_SCREEN
	{
		// key_mode == KEYBOARD_DISPLAY_KEYBOARD
		// Normal keyboard operation:
		scan_change = eve_ui_keyboard_loop(&key_report, &key_scan);

		if (scan_change == 0)
		{
			// Special function keys not directly related to the keyboard action.
			switch (key_scan.KeyTag)
			{
			case TAG_UK_ALPHA:
				eve_ui_keyboard_set_layout(KEYBOARD_LAYOUT_PC_UK_ALPHA);
				screen_change = 1;
				break;

			case TAG_US_ALPHA:
				eve_ui_keyboard_set_layout(KEYBOARD_LAYOUT_PC_US_ALPHA);
				screen_change = 1;
				break;

			case TAG_DE_ALPHA:
				eve_ui_keyboard_set_layout(KEYBOARD_LAYOUT_PC_DE_ALPHA);
				screen_change = 1;
				break;

			case TAG_KEYPAD:
				eve_ui_keyboard_set_screen(KEYBOARD_SCREEN_KEYPAD);
				screen_change = 1;
				break;

			case TAG_KEYBOARD:
				eve_ui_keyboard_set_screen(KEYBOARD_SCREEN_ALPHANUMERIC);
				screen_change = 1;
				break;

			case TAG_MEDIA:
				eve_ui_keyboard_set_screen(KEYBOARD_SCREEN_MEDIA);
				screen_change = 1;
				break;

			case TAG_CUSTOM:
#ifdef KEYBOARD_USE_CUSTOM_SCREEN
				key_mode = KEYBOARD_DISPLAY_CUSTOM;
				screen_change = 1;
#endif // KEYBOARD_USE_CUSTOM_SCREEN
				break;

			case TAG_SETTINGS:
			case TAG_TICK:
				eve_ui_keyboard_set_screen(KEYBOARD_SCREEN_SETTINGS);
				screen_change = 1;
				break;

			case TAG_LOGO:
				eve_ui_screenshot();
				break;
			case TAG_REFRESH:
				break;
			}

			if (screen_change)
			{
				pScan->KeyTag = 0;
			}
		}
	}

	if (pScan)
	{
		memcpy(pScan, &key_scan, sizeof(struct key_scan));
	}
	if (pReport)
	{
		memcpy(pReport, &key_report, sizeof(struct key_report));
	}

	if ((scan_change == 0) || (led_change) || (screen_change))
	{
		if ((pScan) && (pReport))
		{
			uint8_t keyboard_screen = eve_ui_keyboard_get_screen();
			// Display List start
			EVE_LIB_BeginCoProList();
			EVE_CMD_DLSTART();

			EVE_CLEAR_COLOR(KEY_COLOUR_BG_SCREEN);
			EVE_CLEAR(1, 1, 1);

#ifdef KEYBOARD_USE_CUSTOM_SCREEN
			if (key_mode == KEYBOARD_DISPLAY_CUSTOM)
			{
				// CODE NEEDED HERE TO DO SOMETHING
				eve_header_bar(OPTIONS_HEADER_CUSTOM_BUTTON);

				eve_custom_draw(pReport, pScan);
			}
			else
#endif // KEYBOARD_USE_CUSTOM_SCREEN
			{
				// if (key_mode == KEYBOARD_DISPLAY_KEYBOARD)
				if (keyboard_screen != KEYBOARD_SCREEN_SETTINGS)
				{
					// Draw header bar without unnecessary buttons
					if (keyboard_screen == KEYBOARD_SCREEN_ALPHANUMERIC)
					{
						eve_header_bar(keyboard_header_options & (~(OPTIONS_HEADER_KEYBOARD_BUTTON)));
					}
					else if (keyboard_screen == KEYBOARD_SCREEN_KEYPAD)
					{
						eve_header_bar(keyboard_header_options & (~(OPTIONS_HEADER_KEYPAD_BUTTON)));
					}
					else if (keyboard_screen == KEYBOARD_SCREEN_MEDIA)
					{
						eve_header_bar(keyboard_header_options & (~(OPTIONS_HEADER_MEDIA_BUTTON)));
					}
				}
				else // (keyboard_screen == KEYBOARD_SCREEN_SETTINGS)
				{
					eve_header_bar((keyboard_header_options & (~(OPTIONS_HEADER_KEYBOARD_BUTTON
							| OPTIONS_HEADER_KEYPAD_BUTTON
							| OPTIONS_HEADER_MEDIA_BUTTON
							| OPTIONS_HEADER_SETTINGS_BUTTON
							| OPTIONS_HEADER_CUSTOM_BUTTON))) | OPTIONS_HEADER_TICK_BUTTON);
				}

				eve_ui_keyboard_draw(pReport, pScan);
			}

			EVE_DISPLAY();
			EVE_CMD_SWAP();
			EVE_LIB_EndCoProList();
			EVE_LIB_AwaitCoProEmpty();
		}
	}

	if (scan_change == 0)
	{
		return 0;
	}

	return -1;
}

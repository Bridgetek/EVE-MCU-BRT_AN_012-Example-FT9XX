/**
    @file eve_ui_common.h
 **/
/*
 * ============================================================================
 * History
 * =======
 *
 * Copyright (C) Bridgetek Pte Ltd
 * ============================================================================
 *
 * This source code ("the Software") is provided by Bridgetek Pte Ltd
 *  ("Bridgetek") subject to the licence terms set out
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

#ifndef _EVE_KEYBOARD_H
#define _EVE_KEYBOARD_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 @brief Select supported keyboard/displays.
 @details The normal keyboard will be displayed. Custom keyboards are added in
 	 the eve_keyboard.c file where indicated by the macro.
 */
//@{
#define KEYBOARD_DISPLAY_KEYBOARD 0
#define KEYBOARD_DISPLAY_CUSTOM 1
//@}

/**
 @brief Select keyboard screens that are displayed.
 */
//@{
#define KEYBOARD_USE_KEYBOARD_SCREEN
#define KEYBOARD_USE_KEYPAD_SCREEN
#define KEYBOARD_USE_MEDIA_SCREEN
#define KEYBOARD_USE_CUSTOM_SCREEN
//@}

/**
 * @brief Theme colours.
 * @details The highlight colour is used for an active keypress on a button or
 * a selected option (e.g. caps lock).
 * Colours derived from BridgeTek Logo.
 */
//@{
#define KEY_COLOUR_BG_SCREEN 0xFFFFFF
#define KEY_COLOUR_BG_BUTTONS 0x0f75bc
#define KEY_COLOUR_BG_BUTTONS_ALT 0xed1c24
#define KEY_COLOUR_BG_BUTTONS_HIGHLIGHT 0x4d090c
#define KEY_COLOUR_FG_BUTTONS 0xFFFFFF
#define KEY_COLOUR_FG_BUTTONS_ALT 0xFFFFFF
#define KEY_COLOUR_FG_BUTTONS_HIGHLIGHT 0xFFFFFFFF
#define KEY_COLOUR_FG_LEDS KEY_COLOUR_FG_BUTTONS_ALT
#define KEY_COLOUR_BG_LEDS KEY_COLOUR_BG_BUTTONS_ALT
//@}

/**
 @brief Custom font and bitmap definitions.
 @details These utilise handles 0 to 14.
 */
//@{
#define KEYBOARD_FONT 0
#define KEYBOARD_FONT_ALT 1

#define BITMAP_BRIDGETEK_LOGO 2
#define BITMAP_SETTINGS 3
#define BITMAP_REFRESH 4
#define BITMAP_CANCEL 5
#define BITMAP_TICK 6
#define BITMAP_KEYPAD 7
#define BITMAP_KEYBOARD 8
#define BITMAP_MEDIA 9
#define BITMAP_CUSTOM 10
//@}

/** @name Special Key TAG Definitions
 * @details Special key definitions switching between screen layouts.
 */
//@{
#define TAG_DE_ALPHA 		(TAG_RESERVED_START + 0) // Switch to Alphanumeric Screen Germany/Austria
#define TAG_US_ALPHA 		(TAG_RESERVED_START + 1) // Switch to Alphanumeric Screen US
#define TAG_UK_ALPHA 		(TAG_RESERVED_START + 2) // Switch to Alphanumeric Screen UK
#define TAG_CUSTOM			(TAG_RESERVED_START + 3)
#define TAG_MEDIA			(TAG_RESERVED_START + 4)
#define TAG_REFRESH			(TAG_RESERVED_START + 5)
#define TAG_TICK			(TAG_RESERVED_START + 6)
#define TAG_KEYPAD			(TAG_RESERVED_START + 7)
#define TAG_KEYBOARD		(TAG_RESERVED_START + 8)
#define TAG_SETTINGS		(TAG_RESERVED_START + 9) // Click on Settings
#define TAG_LOGO			(TAG_RESERVED_START + 10) // Click on Bridgetek logo
#define TAG_CANCEL			(TAG_RESERVED_START + 10) // Cancel
//@}

/**
 * @brief Keyboard screen components to show on display.
 */
//@{
#define EVE_HEADER_LOGO				(1 << 0) // Show logo
#define EVE_HEADER_SETTINGS_BUTTON	(1 << 1) // Show settings button
#define EVE_HEADER_CANCEL_BUTTON	(1 << 2) // Show cancel button
#define EVE_HEADER_REFRESH_BUTTON	(1 << 3) // Show refresh button
#define EVE_HEADER_SAVE_BUTTON		(1 << 4) // Show save button
#define EVE_HEADER_KEYPAD_BUTTON	(1 << 5) // Show keypad button (switch to keypad)
#define EVE_HEADER_KEYBOARD_BUTTON	(1 << 6) // Show keyboard button (switch to keyboard)
#define EVE_HEADER_EXTRA_BUTTON		(1 << 7) // Show media button (switch to media controls)
#define EVE_HEADER_SPECIAL_BUTTON	(1 << 8) // Show media button (switch to media controls)
//@}

/** @brief Spacer for gaps between keys.
 */
#define EVE_SPACER (EVE_DISP_WIDTH / 120)

/** @name Key TAG Definitions
 * @details Key definitions for TAGs for touchscreen.
 * These are unique tags for each virtual button or key used on
 * the touchscreen. Keys which send data to the host are mapped to
 * scancodes.
 */
//@{
enum {
	TAG_NULL,
	TAG_A, TAG_B, TAG_C, TAG_D, TAG_E, TAG_F, TAG_G, TAG_H, TAG_I,
	TAG_J, TAG_K, TAG_L, TAG_M, TAG_N, TAG_O, TAG_P, TAG_Q, TAG_R,
	TAG_S, TAG_T, TAG_U, TAG_V, TAG_W, TAG_X, TAG_Y, TAG_Z,
	TAG_1, TAG_2, TAG_3, TAG_4, TAG_5, TAG_6, TAG_7, TAG_8, TAG_9,	TAG_0,
	TAG_ENTER, TAG_ESCAPE, TAG_BACKSPACE, TAG_TAB, TAG_SPACE,
	TAG_MINUS, TAG_EQUALS, TAG_SQB_OPEN, TAG_SQB_CLS,
	TAG_BACKSLASH, TAG_HASH, TAG_SEMICOLON, TAG_SQUOTE,
	TAG_TILDE, TAG_COMMA, TAG_DOT, TAG_SLASH,
	TAG_CAPS_LOCK,
	TAG_F1, TAG_F2, TAG_F3, TAG_F4, TAG_F5, TAG_F6, TAG_F7, TAG_F8,
	TAG_F9, TAG_F10, TAG_F11, TAG_F12,
	TAG_PRINT_SCREEN, TAG_SCROLL_LOCK,
	TAG_PAUSE, TAG_INSERT,	TAG_HOME, TAG_PAGE_UP, TAG_DEL,	TAG_END,
	TAG_PAGE_DOWN, TAG_RIGHT_ARROW, TAG_LEFT_ARROW, TAG_DOWN_ARROW,
	TAG_UP_ARROW,
	TAG_NUMBER_LOCK,
	TAG_PAD_DIV, TAG_PAD_MUL, TAG_PAD_MINUS, TAG_PAD_PLUS, TAG_PAD_ENTER,
	TAG_PAD_1, TAG_PAD_2, TAG_PAD_3, TAG_PAD_4, TAG_PAD_5, TAG_PAD_6,
	TAG_PAD_7, TAG_PAD_8, TAG_PAD_9, TAG_PAD_0, TAG_PAD_DOT,
	TAG_APP,
	TAG_CTRLL, TAG_SHIFTL, TAG_ALT,	TAG_WINL, TAG_CTRLR, TAG_SHIFTR,
	TAG_ALTGR, TAG_WINR,
	TAG_SC_POWER, TAG_SC_SLEEP, TAG_SC_WAKEUP,
	TAG_CC_PLAY, TAG_CC_PAUSE, TAG_CC_RECORD, TAG_CC_FFWD, TAG_CC_RWD,
	TAG_CC_NEXT, TAG_CC_PREV, TAG_CC_STOP,
	TAG_CC_MUTE, TAG_CC_VOL_UP, TAG_CC_VOL_DOWN,
	TAG_CC_CUT, TAG_CC_COPY, TAG_CC_PASTE, TAG_CC_UNDO, TAG_CC_REDO, TAG_CC_FIND,
};


/**
 * @brief Keyboard screen components to show on display.
 */
//@{
#define OPTIONS_HEADER_LOGO				(1 << 0) // Show logo
#define OPTIONS_HEADER_SETTINGS_BUTTON	(1 << 1) // Show settings button
#define OPTIONS_HEADER_CANCEL_BUTTON	(1 << 2) // Show cancel button
#define OPTIONS_HEADER_REFRESH_BUTTON	(1 << 3) // Show refresh button
#define OPTIONS_HEADER_TICK_BUTTON		(1 << 4) // Show tick button
#define OPTIONS_HEADER_KEYPAD_BUTTON	(1 << 5) // Show keypad button (switch to keypad)
#define OPTIONS_HEADER_KEYBOARD_BUTTON	(1 << 6) // Show keyboard button (switch to keyboard)
#define OPTIONS_HEADER_MEDIA_BUTTON		(1 << 7) // Show media button (switch to media controls)
#define OPTIONS_HEADER_CUSTOM_BUTTON	(1 << 8) // Show custom display button (switch to custom display)
//@}

struct key_report
{
	uint8_t Caps :1;
	uint8_t Numeric : 1;
	uint8_t Scroll : 1;
};

struct key_scan
{
	uint8_t ShiftL :1;
	uint8_t ShiftR :1;
	uint8_t CtrlL :1;
	uint8_t CtrlR :1;
	uint8_t Alt :1;
	uint8_t AltGr :1;
	uint8_t WinL :1;
	uint8_t WinR :1;
	uint8_t KeyTag;
};

void eve_keyboard_splash(char *toast, uint32_t options);
void eve_keyboard_start(void);
int8_t eve_keyboard_loop(struct key_report *report, struct key_scan *scan);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _EVE_KEYBOARD_H */

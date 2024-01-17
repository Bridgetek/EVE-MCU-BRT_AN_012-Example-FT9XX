/*
 * eve_extra.h
 *
 *  Created on: 20 Nov 2017
 *      Author: Gordon.McNab
 */

#ifndef SOURCES_EVE_UI_KEYBOAD_H_
#define SOURCES_EVE_UI_KEYBOAD_H_

/**
 * @brief Keyboard colours.
 * @details The highlight colour is used for an active keypress on a button or
 * a selected option (e.g. caps lock). The alpha
 */
//@{
#ifndef KEY_COLOUR_BG_SCREEN
#define KEY_COLOUR_BG_SCREEN 0xFFFFFF
#endif
#ifndef KEY_COLOUR_BG_BUTTONS
#define KEY_COLOUR_BG_BUTTONS 0xFC6719
#endif
#ifndef KEY_COLOUR_BG_BUTTONS_ALT
#define KEY_COLOUR_BG_BUTTONS_ALT 0xBD4B12
#endif
#ifndef KEY_COLOUR_BG_BUTTONS_HIGHLIGHT
#define KEY_COLOUR_BG_BUTTONS_HIGHLIGHT 0x01B2CC
#endif
#ifndef KEY_COLOUR_FG_BUTTONS
#define KEY_COLOUR_FG_BUTTONS 0xFFFFFF
#endif
#ifndef KEY_COLOUR_FG_BUTTONS_ALT
#define KEY_COLOUR_FG_BUTTONS_ALT 0xFFFFFF
#endif
#ifndef KEY_COLOUR_FG_BUTTONS_HIGHLIGHT
#define KEY_COLOUR_FG_BUTTONS_HIGHLIGHT 0xFFFFFFFF
#endif
#ifndef KEY_COLOUR_FG_LEDS
#define KEY_COLOUR_FG_LEDS 0xffff00
#endif
#ifndef KEY_COLOUR_BG_LEDS
#define KEY_COLOUR_BG_LEDS 0x202000
#endif
//@}

/**
 * @brief Keyboard layout to show on keyboard/keypad section of display.
 */
//@{
#define KEYBOARD_LAYOUT_PC_US_ALPHA 1
#define KEYBOARD_LAYOUT_PC_UK_ALPHA 2
#define KEYBOARD_LAYOUT_PC_DE_ALPHA 3
//@}

/**
 * @brief Keyboard screen to show on keyboard/keypad section of display.
 */
//@{
#define KEYBOARD_SCREEN_SETTINGS		0
#define KEYBOARD_SCREEN_ALPHANUMERIC	1
#define KEYBOARD_SCREEN_KEYPAD   		2
#define KEYBOARD_SCREEN_MEDIA			3
//@}

/**
 * @brief Keyboard screen components to show on display.
 */
//@{
#define KEYBOARD_COMPONENTS_ALPHANUMERIC	(1 << 0) // Show keyboard/keypad
#define KEYBOARD_COMPONENTS_FUNCTION   		(1 << 1) // F1 to F12
#define KEYBOARD_COMPONENTS_MODIFIERS		(1 << 2) // Ctrl, Alt keys etc
#define KEYBOARD_COMPONENTS_LEDS			(1 << 3) // Scroll, Num and Caps Lock LEDs
#define KEYBOARD_COMPONENTS_TOAST			(1 << 4) // Toast (announcement area)
#define KEYBOARD_COMPONENTS_EDIT			(1 << 5) // Edit area
#define KEYBOARD_COMPONENTS_ESCAPE			(1 << 6) // Escape key
#define KEYBOARD_COMPONENTS_TAB				(1 << 7) // Tab key
#define KEYBOARD_COMPONENTS_KEYPAD_DOT		(1 << 11) // Keypad .
#define KEYBOARD_COMPONENTS_KEYPAD_CONTROL	(1 << 13) // Keypad control items (Ins, Home, arrows etc)
#define KEYBOARD_COMPONENTS_KEYPAD_ARITH	(1 << 14) // Keypad arithmetic operators
#define KEYBOARD_COMPONENTS_FULL_KEYBOARD 	(KEYBOARD_COMPONENTS_ALPHANUMERIC \
		| KEYBOARD_COMPONENTS_FUNCTION \
		| KEYBOARD_COMPONENTS_MODIFIERS \
		| KEYBOARD_COMPONENTS_LEDS \
		| KEYBOARD_COMPONENTS_ESCAPE \
		| KEYBOARD_COMPONENTS_TAB \
		| KEYBOARD_COMPONENTS_KEYPAD_DOT \
		| KEYBOARD_COMPONENTS_KEYPAD_CONTROL \
		| KEYBOARD_COMPONENTS_KEYPAD_ARITH)
#define KEYBOARD_COMPONENTS_SIMPLE_KEYBOARD 	(KEYBOARD_COMPONENTS_ALPHANUMERIC \
		| KEYBOARD_COMPONENTS_LEDS \
		| KEYBOARD_COMPONENTS_ESCAPE)
//@}

/**
 * @brief Simple state values for detecting a keypress and acting on it.
 * @details A modifier key will only affect the next scan code sent in this
 * example. However, a scan key will send a report to the host.
 * The modifier keys (shift, alt, ctrl, Gui (Windows Key) behave like this
 * because the resistive touchscreen displays support one touch only so
 * holding shift and another key cannot be detected.
 */
//@{
#define KEY_PRESS_NONE 0
#define KEY_PRESS_MODIFIER 1
#define KEY_PRESS_SCAN 2
//@}

/** @name Display section definitions.
 * @brief Macros to simplify selecting locations on the display for buttons,
 * text and indicators.
 * @details The display is split into 2 areas: at the top is a status area with
 * LED indicators for Caps, Num and Scroll Lock and mode selection buttons;
 * the lower portion is the keyboard buttons.
 */
//@{
/**
 * @brief Key dimensions for LEDs and layout switches.
 * @details The top 1/4 of the display is reserved for persistent objects.
 * These are the LED status indicators and buttons to switch the layout
 * of the keyboard.
 * There are 2 rows of 10 positions in the matrix. Both start counting at
 * zero.
 */
//@{
#define KEY_SPACER_STATUS (EVE_DISP_WIDTH / 120)

#define KEY_ROWS_STATUS 2
#define KEY_COLS_STATUS 10

#define KEY_WIDTH_STATUS(a) (((EVE_DISP_WIDTH * ((a) * 8)) / (KEY_COLS_STATUS * 8)) - KEY_SPACER_STATUS)
#define KEY_HEIGHT_STATUS (((EVE_DISP_HEIGHT / 4) / KEY_ROWS_STATUS) - KEY_SPACER_STATUS)

#define KEY_ROW_STATUS(a) (0 + ((a) * (KEY_HEIGHT_STATUS + KEY_SPACER_STATUS)))
#define KEY_COL_STATUS(a) ((a) * (EVE_DISP_WIDTH / KEY_COLS_STATUS))
//@}

/**
 * @brief Key dimensions for alphanumeric keys.
 * @details This defines a block of 15 columns and 6 rows for the alphanumeric
 * keys. Some keys are positioned a fraction offset to simulate a real
 * keyboard.
 * The top of this region is defined by: KEY_ROW_STATUS(KEY_ROWS_STATUS)
 */
//@{
#define KEY_SPACER_ALPHA (EVE_DISP_WIDTH / 120)

#define KEY_ROWS_ALPHA 6
#define KEY_COLS_ALPHA 15

#define KEY_WIDTH_ALPHA(a) (((EVE_DISP_WIDTH * ((a) * 8)) / (KEY_COLS_ALPHA * 8)) - KEY_SPACER_ALPHA)
#define KEY_HEIGHT_ALPHA (((EVE_DISP_HEIGHT - KEY_ROW_STATUS(KEY_ROWS_STATUS)) / KEY_ROWS_ALPHA) - KEY_SPACER_ALPHA)

#define KEY_ROW_ALPHA(a) (KEY_ROW_STATUS(KEY_ROWS_STATUS) + ((a) * (KEY_HEIGHT_ALPHA + KEY_SPACER_ALPHA)))
#define KEY_COL_ALPHA(a) ((a) * (EVE_DISP_WIDTH / KEY_COLS_ALPHA))
//@}

/**
 * @brief Key dimensions for keypad keys.
 * @details This defines a block of 12 columns and 5 rows for the numeric and
 * control keys.
 * The top of this region is defined by: KEY_ROW_STATUS(KEY_ROWS_STATUS)
 */
//@{
#define KEY_SPACER_KEYPAD (EVE_DISP_WIDTH / 120)

#define KEY_ROWS_KEYPAD 6
#define KEY_COLS_KEYPAD 12

#define KEY_WIDTH_KEYPAD(a) (((EVE_DISP_WIDTH * ((a) * 8)) / (KEY_COLS_KEYPAD * 8)) - KEY_SPACER_KEYPAD)
#define KEY_HEIGHT_KEYPAD(a) ((((EVE_DISP_HEIGHT - KEY_ROW_STATUS(KEY_ROWS_STATUS)) * (a)) / KEY_ROWS_KEYPAD) - KEY_SPACER_KEYPAD)

#define KEY_ROW_KEYPAD(a) (KEY_ROW_STATUS(KEY_ROWS_STATUS) + ((a) * (KEY_HEIGHT_KEYPAD(1) + KEY_SPACER_KEYPAD)))
#define KEY_COL_KEYPAD(a) ((a) * (EVE_DISP_WIDTH / KEY_COLS_KEYPAD))
//@}
//@}

void eve_ui_keyboard_draw(struct key_report *pReport, struct key_scan *pScan);
uint8_t eve_ui_keyboard_loop(struct key_report *pReport, struct key_scan *pScan);
void eve_ui_keyboard_set_layout(uint8_t layout);
uint8_t eve_ui_keyboard_get_layout(void);
void eve_ui_keyboard_set_screen(uint8_t screen);
uint8_t eve_ui_keyboard_get_screen(void);
void eve_ui_keyboard_set_components(uint32_t components);
uint32_t eve_ui_keyboard_get_components(void);
void eve_ui_keyboard_font(uint8_t font, uint8_t altfont);
void eve_ui_keyboard_screen(uint32_t bg);
void eve_ui_keyboard_leds(uint32_t on, uint32_t off);
void eve_ui_keyboard_buttons(uint32_t fg, uint32_t fg_alt, uint32_t fg_highlight,
		uint32_t bg, uint32_t bg_alt, uint32_t bg_highlight);

#endif /* SOURCES_EVE_UI_KEYBOAD_H_ */

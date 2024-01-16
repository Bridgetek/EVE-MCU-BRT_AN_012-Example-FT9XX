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

#ifndef _EVE_UI_H
#define _EVE_UI_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Implement flash address space for FT9xx. */
#if defined(__FT900__) && !defined(__CDT_PARSER__)
#define EVE_UI_FLASH __flash__
#else
#define EVE_UI_FLASH
#endif // __FT900__ && !__CDT_PARSER__

/**
 * @brief Theme colours.
 * @details The highlight colour is used for an active keypress on a button or
 * a selected option (e.g. caps lock). The alpha
 */
//@{
#ifndef EVE_COLOUR_HIGHLIGHT
#define EVE_COLOUR_HIGHLIGHT 0x808080
#endif //EVE_COLOUR_HIGHLIGHT
#ifndef EVE_COLOUR_BG_1
#define EVE_COLOUR_BG_1 0x404040
#endif //EVE_COLOUR_BG_1
#ifndef EVE_COLOUR_BG_2
#define EVE_COLOUR_BG_2 0x202020
#endif //EVE_COLOUR_BG_2
#ifndef EVE_COLOUR_FG_1
#define EVE_COLOUR_FG_1 0xffffff
#endif //EVE_COLOUR_FG_1
//@}

#ifndef EVE_UI_SPACER
#define EVE_UI_SPACER (EVE_DISP_WIDTH / 120)
#endif //EVE_UI_SPACER

/**
 @brief Custom font and bitmap definitions.
 @details These utilise handles 0 to 14.
 */
//@{
#define FONT_HEADER 28

/** @name Key TAG Definitions
 * @details Common key definitions.
 */
//@{
#define TAG_NO_ACTION		255
#define TAG_RESERVED_START 	240
//@}

/**
 @brief Structure to hold touchscreen calibration settings.
 @details This is used to store the touchscreen calibration settings persistently
 in Flash and identify if the calibration needs to be re-performed.
 */
struct touchscreen_calibration {
	uint32_t key; // VALID_KEY_TOUCHSCREEN
	uint32_t transform[6];
};

/* Prototypes from eve_ui_main.c */
void eve_ui_calibrate();
void eve_ui_play_sound(uint8_t sound, uint8_t volume);
uint8_t eve_ui_read_tag(uint8_t *key);

void eve_ui_setup();
void eve_ui_wait(void);
void eve_ui_screenshot(void);

/* Platform specific functions. */
int8_t eve_ui_arch_flash_calib_init(void);
int8_t eve_ui_arch_flash_calib_write(struct touchscreen_calibration *calib);
int8_t eve_ui_arch_flash_calib_read(struct touchscreen_calibration *calib);
void eve_ui_arch_write_ram_from_pm(const uint8_t EVE_UI_FLASH *ImgData, uint32_t length, uint32_t dest);
void eve_ui_memcpy_pm(void *dst, const EVE_UI_FLASH void* src, size_t s);
void eve_ui_arch_sleepms(uint32_t);

/* Prototypes from eve_ui_load_images.c */
uint32_t eve_ui_jpg_image_size(const uint8_t EVE_UI_FLASH *image_data, uint16_t *width, uint16_t *height);
uint32_t eve_ui_load_jpg(const uint8_t EVE_UI_FLASH *image_data, uint32_t image_size, uint8_t image_handle);
uint32_t eve_ui_load_argb2(const uint8_t EVE_UI_FLASH *image_data, uint32_t image_size, uint8_t image_handle,
		uint16_t img_width, uint16_t img_height);
uint32_t eve_ui_load_argb1555(const uint8_t EVE_UI_FLASH *image_data, uint32_t image_size, uint8_t image_handle,
		uint16_t img_width, uint16_t img_height);

/* Prototypes from eve_ui_load_font.c */
uint8_t eve_ui_font_header(uint8_t font_handle, EVE_GPU_FONT_HEADER *font_hdr);
uint8_t eve_ui_font_size(uint8_t font_handle, uint16_t *width, uint16_t *height);
uint8_t eve_ui_font_char_width(uint8_t font_handle, char ch);
uint8_t eve_ui_font_string_width(uint8_t font_handle, const char *str);
uint32_t eve_ui_load_font(const uint8_t EVE_UI_FLASH *font_data, uint32_t font_size, uint8_t font_handle);
uint32_t eve_ui_load_font2(uint8_t first, const uint8_t EVE_UI_FLASH *font_data, uint32_t font_size, uint8_t font_handle);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* _EVE_UI_H */

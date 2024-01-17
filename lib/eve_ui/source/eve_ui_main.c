/**
  @file eve_ui_main.c
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
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "EVE_config.h"
#include "EVE.h"
#include "HAL.h"

#ifdef __CDT_PARSER__
#define __flash__ // to avoid eclipse syntax error
#endif

#include "eve_ui.h"
#include "eve_ram_g.h"

/* CONSTANTS ***********************************************************************/

/**
 @brief Allow click on the BridgeTek Logo to make a screenshot.
 Set to P6 for a binary portable pixmap format (PPM).
 Set to P3 for an ASCII portable pixmap format (PPM).
 PPM format has 0-255 for each channel R, G, B.
 */
//*{
#define PPM_P6 6
#define PPM_P3 3
#define ENABLE_SCREENSHOT PPM_P6
//*}

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

/* FUNCTIONS ***********************************************************************/

void eve_ui_setup()
{
	EVE_Init();

	eve_ui_calibrate();
}

void eve_ui_wait(void)
{
	uint8_t key_code = 0;
	uint8_t key_detect = 0;

	key_detect = eve_ui_read_tag(&key_code);
	if (key_detect)
	{
	}
	eve_ui_arch_sleepms(100);
}

void eve_ui_calibrate()
{
	struct touchscreen_calibration calib;

	eve_ui_arch_flash_calib_init();

	if (eve_ui_arch_flash_calib_read(&calib) != 0)
	{
		EVE_LIB_BeginCoProList();
		EVE_CMD_DLSTART();
		EVE_CLEAR_COLOR_RGB(0, 0, 0);
		EVE_CLEAR(1,1,1);
		EVE_COLOR_RGB(255, 255, 255);
		EVE_CMD_TEXT(EVE_DISP_WIDTH/2, EVE_DISP_HEIGHT/2,
				FONT_HEADER, EVE_OPT_CENTERX | EVE_OPT_CENTERY,
				"Please tap on the dots");
		EVE_CMD_CALIBRATE(0);
		EVE_LIB_EndCoProList();
		EVE_LIB_AwaitCoProEmpty();

		calib.transform[0] = HAL_MemRead32(EVE_REG_TOUCH_TRANSFORM_A);
		calib.transform[1] = HAL_MemRead32(EVE_REG_TOUCH_TRANSFORM_B);
		calib.transform[2] = HAL_MemRead32(EVE_REG_TOUCH_TRANSFORM_C);
		calib.transform[3] = HAL_MemRead32(EVE_REG_TOUCH_TRANSFORM_D);
		calib.transform[4] = HAL_MemRead32(EVE_REG_TOUCH_TRANSFORM_E);
		calib.transform[5] = HAL_MemRead32(EVE_REG_TOUCH_TRANSFORM_F);
		eve_ui_arch_flash_calib_write(&calib);
	}
	HAL_MemWrite32(EVE_REG_TOUCH_TRANSFORM_A, calib.transform[0]);
	HAL_MemWrite32(EVE_REG_TOUCH_TRANSFORM_B, calib.transform[1]);
	HAL_MemWrite32(EVE_REG_TOUCH_TRANSFORM_C, calib.transform[2]);
	HAL_MemWrite32(EVE_REG_TOUCH_TRANSFORM_D, calib.transform[3]);
	HAL_MemWrite32(EVE_REG_TOUCH_TRANSFORM_E, calib.transform[4]);
	HAL_MemWrite32(EVE_REG_TOUCH_TRANSFORM_F, calib.transform[5]);
}

void eve_ui_screenshot()
{
#ifdef ENABLE_SCREENSHOT
	uint32_t line_address;
	uint32_t line_count = 0;
	uint32_t pixel_counter = 0;
	uint32_t c_data = 0;
	uint8_t c_red = 0;
	uint8_t c_green = 0;
	uint8_t c_blue = 0;

	line_address = malloc_ram_g(EVE_DISP_WIDTH * sizeof(uint32_t));

	printf("Screenshot...\n");

	// Use this marker to identify the start of the image.
	printf("ARGB start\n");

    // Notes on SnapShot2 (see Programmers Guide)
    //   0x20 means ARGB8 format
    //   Pointer specifies destination of snapshot data in RAM_G
    //   X value: 0 means the snapshot begins at X = 0 (start of row)
    //   Y Value: line_count specifies Y as current row number
    //   Width: Width of Snapshot is 2 * Screen Width (the 2 * is required if ARGB8)
    //   Height: Height is always 1 row as we are taking the snapshot line-by-line

	// Send PPM header depending on the setting for ENABLE_SCREENSHOT
	printf("P%d %d %d 255 ", ENABLE_SCREENSHOT, EVE_DISP_WIDTH, EVE_DISP_HEIGHT);

	while(line_count < EVE_DISP_HEIGHT)
	{
  		EVE_LIB_BeginCoProList();
		EVE_CMD_SNAPSHOT2(0x20, line_address, 0, line_count, (EVE_DISP_WIDTH * 2), 1); 	// 0x20 = ARGB8
		EVE_LIB_EndCoProList();
		EVE_LIB_AwaitCoProEmpty();
		eve_ui_arch_sleepms(20); // ensure co-pro has finished taking snapshot

  		pixel_counter = 0;
		while(pixel_counter < (EVE_DISP_WIDTH * sizeof(uint32_t)))
		{
			c_data = HAL_MemRead32(line_address + pixel_counter);

			c_red = (uint8_t) (c_data >> 16);
			c_green = (uint8_t) (c_data >> 8);
			c_blue = (uint8_t) ((c_data >> 0));
#if ENABLE_SCREENSHOT == PPM_P6
			printf("%c%c%c", c_red, c_green, c_blue);
#elif ENABLE_SCREENSHOT == PPM_P3
			printf("%d %d %d ", c_red, c_green, c_blue);
#endif // ENABLE_SCREENSHOT

			pixel_counter += sizeof(uint32_t); // increment address

#if ENABLE_SCREENSHOT == PPM_P3
			if ((pixel_counter % (5 * sizeof(uint32_t))) == 0)
			{
				printf("\n");
			}
#endif // ENABLE_SCREENSHOT
		}
		line_count ++;
	}

	printf("\nARGB end\n"); // Marker to identify the end of the image.

	eve_ui_arch_sleepms(500);
	free_ram_g(line_address);

#endif // ENABLE_SCREENSHOT
}

void eve_ui_play_sound(uint8_t sound, uint8_t volume)
{
	HAL_MemWrite8(EVE_REG_VOL_SOUND, volume);
	HAL_MemWrite8(EVE_REG_SOUND, sound);
	HAL_MemWrite8(EVE_REG_PLAY, 1);
}

uint8_t eve_ui_read_tag(uint8_t *key)
{
	uint8_t Read_tag;
	uint8_t key_detect = 0;

	Read_tag = HAL_MemRead8(EVE_REG_TOUCH_TAG);

	if (!(HAL_MemRead16(EVE_REG_TOUCH_RAW_XY) & 0x8000))
	{
		key_detect = 1;
		*key = Read_tag;
	}

	return key_detect;
}



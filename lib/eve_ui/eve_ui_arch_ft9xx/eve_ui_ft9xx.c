/**
  @file eve_ui_ft9xx.c
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

#include <ft900.h>
#include <ft900_dlog.h>

#include "EVE_config.h"
#include "EVE.h"
#include "HAL.h"

#include "eve_ui.h"

/* CONSTANTS ***********************************************************************/

/**
 @brief Key for identifying if touchscreen calibration values are programmed into
 datalogger memory in the Flash.
 */
#define VALID_KEY_TOUCHSCREEN 0xd72f91a3

/* GLOBAL VARIABLES ****************************************************************/

/* LOCAL VARIABLES *****************************************************************/

/**
 @brief Defined area in flash to store EVE calibration data.
 @details This 256 byte array is initialised to all 1s. It will be used to store the
 	 touch screen calibration data. There is a key placed at the start of the array
 	 when the calibration is programmed, this indicates that the data is valid.
 	 Only if this array is in place will calibration data be programmed.
 	 The size is 256 bytes and it is aligned on a 256 byte boundary as the minimum
 	 programmable Flash sector is 256 bytes.
 */
__flash__ uint8_t __attribute__((aligned(256))) dlog_pm[256] = {[0 ... 255] = 0xff };

/* MACROS **************************************************************************/

/* LOCAL FUNCTIONS / INLINES *******************************************************/

/**
 * @brief Functions used to store calibration data in flash.
 */
//@{
int8_t eve_ui_arch_flash_calib_init(void)
{
	const __flash__ struct touchscreen_calibration *pcalibpm = (const __flash__ struct touchscreen_calibration *)dlog_pm;

	/* Check that there is an area set aside in the data section for calibration data. */
	if ((pcalibpm->key != VALID_KEY_TOUCHSCREEN) && (pcalibpm->key != 0xFFFFFFFF))
	{
		if (((uint32_t)pcalibpm & 255) != 0)
		{
			/* An aligned 256 byte Flash sector not has been correctly setup for calibration data. */
			return -1;
		}
	}

	return 0;
}

int8_t eve_ui_arch_flash_calib_write(struct touchscreen_calibration *calib)
{
	uint8_t	dlog_flash[260] __attribute__((aligned(4)));
	struct touchscreen_calibration *pcalibflash = (struct touchscreen_calibration *)dlog_flash;

	/* Read calibration data from Flash to a properly aligned array. */
	CRITICAL_SECTION_BEGIN
	memcpy_flash2dat((void *)dlog_flash, (uint32_t)dlog_pm, 256);
	CRITICAL_SECTION_END

	/* Check that the Flash blank, so it is possible to write to this sector in Flash. */
	if (pcalibflash->key == 0xFFFFFFFF)
	{
		/* Copy calibration data into a properly aligned array. */
		calib->key = VALID_KEY_TOUCHSCREEN;
		memset(dlog_flash, 0xff, sizeof(dlog_flash));
		memcpy(dlog_flash, calib, sizeof(struct touchscreen_calibration));

		CRITICAL_SECTION_BEGIN
		/* This are must be set to 0xff for Flash programming to work. */
		memcpy_dat2flash ((uint32_t)dlog_pm, dlog_flash, 256);
		CRITICAL_SECTION_END;

		return 0;
	}

	return -1;
}

int8_t eve_ui_arch_flash_calib_read(struct touchscreen_calibration *calib)
{
	uint8_t dlog_flash[256] __attribute__((aligned(4)));
	struct touchscreen_calibration *pcalibflash = (struct touchscreen_calibration *)dlog_flash;

	/* Read calibration data from Flash to a properly aligned array. */
	CRITICAL_SECTION_BEGIN
	memcpy_flash2dat((void *)dlog_flash, (uint32_t)dlog_pm, 256);
	CRITICAL_SECTION_END

	/* Flash blank, program memory blank: flash calibration data blank. */
	if (pcalibflash->key != VALID_KEY_TOUCHSCREEN)
	{
		return -2;
	}

	/* Calibration data is valid. */
	memcpy(calib, dlog_flash, sizeof(struct touchscreen_calibration));

	return 0;
}
//@}

void eve_ui_arch_write_cmd_from_pm(const uint8_t __flash__ *ImgData, uint32_t length)
{
	uint32_t offset = 0;
	uint8_t ramData[512];
	uint32_t left;

	while (offset < length)
	{
		memcpy_pm2dat(ramData, (const __flash__ void *)ImgData, 512);

		if (length - offset < 512)
		{
			left = length - offset;
		}
		else
		{
			left = 512;
		}
		EVE_LIB_WriteDataToCMD(ramData, left);
		offset += left;
		ImgData += left;
	};
}

void eve_ui_arch_write_ram_from_pm(const uint8_t __flash__ *ImgData, uint32_t length, uint32_t dest)
{
	uint32_t offset = 0;
	uint8_t ramData[512];
	uint32_t left;

	while (offset < length)
	{
		memcpy_pm2dat(ramData, (const __flash__ void *)ImgData, 512);

		if (length - offset < 512)
		{
			left = length - offset;
		}
		else
		{
			left = 512;
		}
		EVE_LIB_WriteDataToRAMG(ramData, left, dest);
		offset += left;
		ImgData += left;
		dest += left;
	};
}

void eve_ui_memcpy_pm(void *dst, const __flash__ void * src, size_t s)
{
	memcpy_pm2dat(dst, src, s);
}

void eve_ui_arch_sleepms(uint32_t delay)
{
	delayms(delay);
}

/* FUNCTIONS ***********************************************************************/

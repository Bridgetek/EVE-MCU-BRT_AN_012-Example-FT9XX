/**
 @file arial_L4_15.c
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
#include <stddef.h>
#include <machine/endian.h>

#include "EVE_config.h"
#include "EVE.h"

#ifdef __CDT_PARSER__
#define __flash__ // to avoid eclipse syntax error
#endif

#include "eve_ui.h"
#include "eve_ram_g.h"

uint32_t eve_ui_jpg_image_size(const uint8_t EVE_UI_FLASH *image_data, uint16_t *width, uint16_t *height)
{
	uint16_t img_width = 0;
	uint16_t img_height = 0;
	uint8_t EVE_UI_FLASH *pCh = (uint8_t EVE_UI_FLASH *)image_data;

	// Read in raw JPEG encoded image data to find width and height of image.
	while (1)
	{
		if (*pCh == 0xff)
		{
			pCh++;
			if (*pCh == 0xc0)
			{
				// Found SOF0
				pCh++;pCh++;pCh++;pCh++;
				img_height = *pCh++;
				img_height <<= 8;
				img_height |= (*pCh++);
				img_width = *pCh++;
				img_width <<= 8;
				img_width |= (*pCh++);
				break;
			}
		}
		pCh++;
	}

#if BYTE_ORDER == BIG_ENDIAN
	*width = (img_width >> 8) | (img_width << 8);
	*height = (img_height >> 8) | (img_height << 8);
#else
	*width = img_width;
	*height = img_height;
#endif

	return img_width * 2 * img_height;
}

uint32_t eve_ui_load_argb2(const uint8_t EVE_UI_FLASH *image_data, uint32_t image_size, uint8_t image_handle,
		uint16_t img_width, uint16_t img_height)
{
	uint32_t img_offset;

	img_offset = malloc_ram_g(image_size);
	if (img_offset)
	{
		eve_ui_arch_write_ram_from_pm(image_data, image_size, img_offset);

		EVE_LIB_BeginCoProList();
		EVE_CMD_DLSTART();
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		EVE_BITMAP_HANDLE(image_handle);
		EVE_BITMAP_SOURCE(img_offset);
		EVE_BITMAP_LAYOUT(EVE_FORMAT_ARGB2, img_width, img_height);
		EVE_BITMAP_LAYOUT_H((img_width) >> 10, img_height >> 9);
		EVE_BITMAP_SIZE(EVE_FILTER_NEAREST, EVE_WRAP_BORDER, EVE_WRAP_BORDER,
				img_width, img_height);
		EVE_BITMAP_SIZE_H(img_width >> 9, img_height >> 9);
		EVE_END();

		EVE_DISPLAY();
		EVE_CMD_SWAP();
		EVE_LIB_EndCoProList();
		EVE_LIB_AwaitCoProEmpty();
	}

	return img_offset;
}

uint32_t eve_ui_load_argb1555(const uint8_t EVE_UI_FLASH *image_data, uint32_t image_size, uint8_t image_handle,
		uint16_t img_width, uint16_t img_height)
{
	uint32_t img_offset;

	img_offset = malloc_ram_g(image_size);
	if (img_offset)
	{
		eve_ui_arch_write_ram_from_pm(image_data, image_size, img_offset);

		EVE_LIB_BeginCoProList();
		EVE_CMD_DLSTART();
		EVE_BEGIN(EVE_BEGIN_BITMAPS);
		EVE_BITMAP_HANDLE(image_handle);
		EVE_BITMAP_SOURCE(img_offset);
		EVE_BITMAP_LAYOUT(EVE_FORMAT_ARGB1555, img_width * 2, img_height);
		EVE_BITMAP_LAYOUT_H((img_width * 2) >> 10, img_height >> 9);
		EVE_BITMAP_SIZE(EVE_FILTER_NEAREST, EVE_WRAP_BORDER, EVE_WRAP_BORDER,
				img_width, img_height);
		EVE_BITMAP_SIZE_H(img_width >> 9, img_height >> 9);
		EVE_END();

		EVE_DISPLAY();
		EVE_CMD_SWAP();
		EVE_LIB_EndCoProList();
		EVE_LIB_AwaitCoProEmpty();
	}

	return img_offset;
}

uint32_t eve_ui_load_jpg(const uint8_t EVE_UI_FLASH *image_data, uint32_t image_size, uint8_t image_handle)
{
	uint32_t img_offset;
	uint32_t img_expanded_size;
	uint16_t img_width = 0;
	uint16_t img_height = 0;
	uint8_t EVE_UI_FLASH *pData = (uint8_t EVE_UI_FLASH *)image_data;
	uint8_t ramData[256];

	img_expanded_size = eve_ui_jpg_image_size(image_data, &img_width, &img_height);

	img_offset = malloc_ram_g(img_expanded_size);

	if (img_offset)
	{
		EVE_LIB_BeginCoProList();
		EVE_CMD_LOADIMAGE(img_offset, 0);
		// Send raw JPEG encoded image data to co-processor. It will be decoded
		// as the data is received.
		while (image_size)
		{
			uint32_t length = image_size;
			if (length >= 256)
			{
				length = 256;
			}
			eve_ui_memcpy_pm(ramData, (const EVE_UI_FLASH void *)pData, length);
			EVE_LIB_WriteDataToCMD(ramData, length);
			pData += length;
			image_size -= length;
		}
		EVE_LIB_EndCoProList();
		EVE_LIB_AwaitCoProEmpty();
	}

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();

	EVE_BEGIN(EVE_BEGIN_BITMAPS);
	EVE_BITMAP_HANDLE(image_handle);
	EVE_BITMAP_SOURCE(img_offset);
	EVE_BITMAP_LAYOUT(EVE_FORMAT_RGB565, img_width * 2, img_height);
	EVE_BITMAP_LAYOUT_H((img_width * 2) >> 10, img_height >> 9);
	EVE_BITMAP_SIZE(EVE_FILTER_NEAREST, EVE_WRAP_BORDER, EVE_WRAP_BORDER,
			img_width, img_height);
	EVE_BITMAP_SIZE_H(img_width >> 9, img_height >> 9);
	EVE_END();

	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();

	return img_offset;
}

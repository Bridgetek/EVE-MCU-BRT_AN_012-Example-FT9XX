/*
 * eve_ram_g.c
 *
 *  Created on: 20 Nov 2017
 *      Author: Gordon.McNab
 */

#include <stdint.h>
#include <stddef.h>

#include "EVE_config.h"
#include "EVE.h"
#include "HAL.h"

#include "eve_ram_g.h"

void init_ram_g()
{
	// Write size of free area to first dword of RAM_G.
	// This is the entirety of RAM_G (EVE_RAM_G_SIZE).
	uint32_t ptr = EVE_RAM_G_SIZE - sizeof(uint32_t);

	EVE_LIB_WriteDataToRAMG((uint8_t *)&ptr, sizeof(uint32_t), 0);

	DEBUG_RAMG_MALLOC("RAM_G memory init %x\r\n", ptr);
}

uint32_t malloc_ram_g(size_t size)
{
	uint32_t ptr = 0;
	uint32_t blocklen;
	uint32_t nextblock = 0;
	uint32_t currblock = 0;
	uint32_t marker;

	DEBUG_RAMG_MALLOC("RAM_G memory malloc bytes %d\r\n", size);
	do
	{
		EVE_LIB_ReadDataFromRAMG((uint8_t *)&marker, sizeof(uint32_t), currblock);
		DEBUG_RAMG_MALLOC("RAM_G memory malloc 0x%x marker 0x%x\r\n", currblock, marker);

		// Work out next block start.
		blocklen = marker & (EVE_RAM_G_SIZE - 1);
		// Next block marker is at current address + sizeof marker + length of block.
		nextblock = currblock + (sizeof(uint32_t) + blocklen);

		// High bit set. Memory block is used. Skip to next block.
		if (!(marker >> 31))
		{
			DEBUG_RAMG_MALLOC("RAM_G memory malloc found %d bytes\r\n", blocklen);
			// High bit clear. Can use or subdivide this block.
			if (blocklen >= size)
			{
				ptr = currblock + sizeof(uint32_t);

				// Create new marker for this block. High bit set.
				marker = size | (1<<31);
				EVE_LIB_WriteDataToRAMG((uint8_t *)&marker, sizeof(uint32_t), currblock);
				DEBUG_RAMG_MALLOC("RAM_G memory malloc reserved 0x%x at 0x%x\r\n", marker, currblock);
				currblock += (sizeof(uint32_t) + size);

				if (currblock != nextblock)
				{
					// Incomplete block used.
					marker = nextblock - currblock - sizeof(uint32_t);
					EVE_LIB_WriteDataToRAMG((uint8_t *)&marker, sizeof(uint32_t), currblock);
					DEBUG_RAMG_MALLOC("RAM_G memory malloc allowed 0x%x at 0x%x\r\n", marker, currblock);
				}

				DEBUG_RAMG_MALLOC("RAM_G memory malloc assigned 0x%x\r\n", ptr);
				break;
			}
		}

		currblock = nextblock;
	} while (nextblock < EVE_RAM_G_SIZE);

	return ptr;
}

void free_ram_g(uint32_t addr)
{
	uint32_t blocklen;
	uint32_t nextblock = 0;
	uint32_t currblock = 0;
	uint32_t marker;

	DEBUG_RAMG_MALLOC("RAM_G memory free 0x%x\r\n", addr);
	do
	{
		EVE_LIB_ReadDataFromRAMG((uint8_t *)&marker, sizeof(uint32_t), currblock);
		DEBUG_RAMG_MALLOC("RAM_G memory free 0x%x marker 0x%x\r\n", currblock, marker);

		// Work out next block start.
		blocklen = marker & (EVE_RAM_G_SIZE - 1);
		// Next block marker is at current address + sizeof marker + length of block.
		nextblock = currblock + (sizeof(uint32_t) + blocklen);

		// High bit set. Memory block is used. Skip to next block.
		if (addr == currblock + sizeof(uint32_t))
		{
			DEBUG_RAMG_MALLOC("RAM_G memory free found 0x%x of length %d\r\n", addr, blocklen);

			// Remove high bit from marker.
			marker = blocklen;
			EVE_LIB_WriteDataToRAMG((uint8_t *)&marker, sizeof(uint32_t), currblock);
			DEBUG_RAMG_MALLOC("RAM_G memory freed 0x%x at 0x%x\r\n", marker, currblock);
			break;
		}

		currblock = nextblock;
	} while (nextblock < EVE_RAM_G_SIZE);

	DEBUG_RAMG_MALLOC("RAM_G memory coalesce\r\n");
	uint32_t freeblock = 0;
	nextblock = 0;
	currblock = 0;
	do
	{
		EVE_LIB_ReadDataFromRAMG((uint8_t *)&marker, sizeof(uint32_t), currblock);
		DEBUG_RAMG_MALLOC("RAM_G memory coalesce 0x%x marker 0x%x\r\n", currblock, marker);

		// Work out next block start.
		blocklen = marker & (EVE_RAM_G_SIZE - 1);
		// Next block marker is at current address + sizeof marker + length of block.
		nextblock = currblock + (sizeof(uint32_t) + blocklen);

		if (!(marker >> 31))
		{
			// If this block is free and last block is free then coalesce them.
			// We can end up coalescing several blocks.
			marker = nextblock - freeblock - sizeof(uint32_t);
			DEBUG_RAMG_MALLOC("RAM_G memory coalesce %d byte block\r\n", marker);
			EVE_LIB_WriteDataToRAMG((uint8_t *)&marker, sizeof(uint32_t), freeblock);
			DEBUG_RAMG_MALLOC("RAM_G memory coalesce 0x%x at 0x%x\r\n", marker, freeblock);
		}
		else
		{
			freeblock = nextblock;
		}

		currblock = nextblock;
	} while (nextblock < EVE_RAM_G_SIZE);
}


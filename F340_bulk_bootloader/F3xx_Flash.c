//-----------------------------------------------------------------------------
// F3xx_Flash.c
//-----------------------------------------------------------------------------
// Copyright 2012 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Flash utilities for writing/erasing flash pages
//
//
// How To Test:    See Readme.txt
//
//
// Target:         C8051F320/1,
//                 C8051F326/7,
//                 C8051F34x,
//                 C8051F38x
//
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Command Line:   See Readme.txt
// Project Name:   USB0_Bulk
//
// Release 2.0
//    -Rewrite (CM)
//    -02 NOV 2012
//

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "c8051f3xx.h"
#include "F3xx_Flash.h"

//-----------------------------------------------------------------------------
// Static Global Variables
//-----------------------------------------------------------------------------

// Buffer used to create POLY CRC32 TABLE
SEGMENT_VARIABLE (poly_table[0x100], U32, SEG_XDATA);

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// GetFlashCRC32
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U16 address : the address of the flash page to calculate CRC32
//                2) U16 size : the number of bytes 
//
// Calculate CRC32 the specified address.
//
//-----------------------------------------------------------------------------
U32 GetFlashCRC32(U16 address, U16 data_size)
{
    VARIABLE_SEGMENT_POINTER (pAddr, U8, SEG_CODE) = (U8 SEG_CODE *) address;
    U16 i;
    U32 crc32;
 
    crc32 = CRC32_INIT;
    for (i = 0; i < data_size; i++)
    {
		crc32 = (crc32 >> 8) ^ poly_table[((U8) crc32 & 0xFF) ^ pAddr[i]];
    }
    return ~crc32;
}

//-----------------------------------------------------------------------------
// CreateCRC32Table
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Create CRC32 POLY table.
//
//-----------------------------------------------------------------------------
void CreateCRC32Table(void)
{
	U8 j;
    U16 i;
    U32 poly_element;
	for (i = 0; i < 0x100; i++)
	{
		poly_element = i;
		for (j = 0; j < 0x08; j++)
		{	
			if (poly_element & 0x01)
			{
				poly_element = (poly_element >> 1) ^ POLY_SIGN;
			}
			else 
			{
				poly_element >>= 1;
			}
		}
		poly_table[(U8) i] = poly_element;
	}
}

//-----------------------------------------------------------------------------
// EraseFlashPage
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U16 pageAddress : the address of the flash page to erase
//
// Erases the specified flash page
//
//-----------------------------------------------------------------------------
void EraseFlashPage(U16 pageAddress)
{
   U8 EA_Save = EA;
   VARIABLE_SEGMENT_POINTER (pAddr, U8, SEG_XDATA) = (U8 SEG_XDATA *) pageAddress;

   // Disable interrupts
   EA = 0;

   // Write flash key codes
   FLKEY = 0xA5;
   FLKEY = 0xF1;

   // Enable program erase
   PSCTL = 0x03;

   // Erase page by writing to a byte within the page
   *pAddr = 0x00;

   // Disable program erase
   PSCTL = 0x00;

   // Restore interrupts
   EA = EA_Save;
}

//-----------------------------------------------------------------------------
// WriteFlashPage
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U16 address : the address of the flash page to write
//                2) U8 buffer[] : a buffer to write to the flash page starting
//                                 at the specified page address
//                3) U16 size : the number of bytes in the buffer to write
//
// Write the specified number of bytes in the buffer to the specified address.
//
//-----------------------------------------------------------------------------
void WriteFlashPage(U16 address, U8* buffer, U16 data_size)
{
   U8 EA_Save = EA;
   VARIABLE_SEGMENT_POINTER (pAddr, U8, SEG_XDATA) = (U8 SEG_XDATA *) address;
   U16 i;

   // Disable interrupts
   EA = 0;

   // Enable program writes
   PSCTL = 0x01;

#if FLASH_GROUP_WRITE_EN
   // Enable two-byte flash writes
   PFE0CN |= 0x01;
#endif // FLASH_GROUP_WRITE_EN

   for (i = 0; i < data_size; i++)
   {
      // Write flash key codes
      FLKEY = 0xA5;
      FLKEY = 0xF1;

      // Write a single byte to the page
      pAddr[i] = buffer[i];
   }

   // Disable program writes
   PSCTL = 0x00;

   // Restore interrupts
   EA = EA_Save;
}

//-----------------------------------------------------------------------------
// ReadFlashPage
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U16 address : the address of the flash page to read
//                2) U8 buffer[] : a buffer to read from the flash page starting
//                                 at the specified page address
//                3) U16 size : the number of bytes to read into the buffer
//
// Read the specified number of bytes from flash and store in the buffer.
//
//-----------------------------------------------------------------------------
void ReadFlashPage(U16 address, U8* buffer, U16 data_size)
{
   VARIABLE_SEGMENT_POINTER (pAddr, U8, SEG_CODE) = (U8 SEG_CODE *) address;
   U16 i;

   for (i = 0; i < data_size; i++)
   {
      buffer[i] = pAddr[i];
   }
}

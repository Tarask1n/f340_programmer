//-----------------------------------------------------------------------------
// F3xx_Flash.h
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

#ifndef _F3XX_FLASH_H_
#define _F3XX_FLASH_H_

#define CRC32_INIT  0xFFFFFFFF
//#define POLY_SIGN   0x04C11DB7
#define POLY_SIGN   0xEDB88320      // Standard polynom

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

U32 GetFlashCRC32(U16 address, U16 data_size);
void CreateCRC32Table(void);
void EraseFlashPage(U16 pageAddress);
void WriteFlashPage(U16 address, U8* buffer, U16 data_size);
void ReadFlashPage(U16 address, U8* buffer, U16 data_size);

#endif // _F3XX_FLASH_H_

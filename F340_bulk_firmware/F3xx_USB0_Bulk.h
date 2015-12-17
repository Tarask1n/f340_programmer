//-----------------------------------------------------------------------------
// F3xx_USB0_Bulk.c
//-----------------------------------------------------------------------------
// Copyright 2008-2012 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Device specific initialization routines.
//
//
// How To Test:    See Readme.txt
//
//
// Target:         C8051F38x
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Command Line:   See Readme.txt
// Project Name:   USB0_Bulk
//
// Release 2.0
//    -Rewrite (CM)
//    -02 NOV 2012
//
// Release 1.1
//    -All changes by BW
//    -1 SEP 2010
//    -Updated USB0_Init() to write 0x89 to CLKREC instead of 0x80
//
// Release 1.0
//    -Initial Revision (PD)
//    -04 JUN 2008
//

//#define SYSCLK             24000000    // SYSCLK frequency in Hz
#define SYSCLK             48000000    // SYSCLK frequency in Hz

#ifndef _USB_BULK_H_
#define _USB_BULK_H_

//-----------------------------------------------------------------------------
// External Global Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void System_Init (void);
void USB0_Init (void);
void USB0_Suspend (void);
//void Delay (void);
void SetLed (U8 on);
U32 Get_U32 (U8* Src);
U16 Get_U16 (U8* Src);
void MemCopy (U8* Src, U8* Dest, U16 Length);
void TIMER2_Init (void);
void Delay_us (U8 time_us);
void Delay_ms (U8 time_ms);
void PowerOn50V (void);
void PowerOn33V (void);
void PowerOff (void);

#endif // _USB_BULK_H_

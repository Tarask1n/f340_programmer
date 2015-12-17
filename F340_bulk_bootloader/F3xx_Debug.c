//-----------------------------------------------------------------------------
// F3xx_Debug.c
//-----------------------------------------------------------------------------
// Copyright 2015 Kin
// Program Description:
//
// Store Debug Messages in Main Flash
//

//-----------------------------------------------------------------------------
// Definitions - Debug Messages 
//-----------------------------------------------------------------------------
// Debug Message format - D0123456789ABCXXXX
//     D             - signature
//     0123456789ABC - 12 bytes of message
//     XXXX          - Debug value

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "c8051f3xx.h"
#include "F3xx_Debug.h"

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
// Buffer used for debug message
SEGMENT_VARIABLE (Debug_Buffer[16], U8, SEG_XDATA);

// Current Debug Address offset
U16 DebugAddress;

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

void Debug_Init (void)
{
    VARIABLE_SEGMENT_POINTER (dAddr, U8, SEG_CODE) = (U8 SEG_CODE *) FLASH_DEBUG_START;

    DebugAddress = 0;

    while (dAddr[DebugAddress] == DEBUG_TRACE_MESSAGE[0])
    {
        DebugAddress += 16;
    }
}

void Store_Debug_Message (U8* DebugMessage, U32 DebugData)
{
    U8 EA_Save;
    VARIABLE_SEGMENT_POINTER (dAddr, U8, SEG_XDATA) = (U8 SEG_XDATA *) DebugAddress + FLASH_DEBUG_START;
    U8 i;

    for (i = 0; i < 16; i++)
    {
        Debug_Buffer[i] = 0x00;
    }

    Debug_Buffer[0] = DEBUG_TRACE_MESSAGE[0];
//    Debug_Buffer[1] = DEBUG_TRACE_MESSAGE[1];
    i = 0;
    while ((DebugMessage[i] != 0x00) && (i < 11))
    {
        Debug_Buffer[i + 1] = DebugMessage[i];
         i++;
    }
    Debug_Buffer[12] = ((U8[]) &DebugData)[3];
    Debug_Buffer[13] = ((U8[]) &DebugData)[2];
    Debug_Buffer[14] = ((U8[]) &DebugData)[1];
    Debug_Buffer[15] = ((U8[]) &DebugData)[0];
  
    // Disable interrupts
    EA_Save = EA;
    EA = 0;

    // Enable program writes
    PSCTL = 0x01;

#if FLASH_GROUP_WRITE_EN
    // Enable two-byte flash writes
    PFE0CN |= 0x01;
#endif // FLASH_GROUP_WRITE_EN

    for (i = 0; i < 16; i++)
    {
       // Write flash key codes
       FLKEY = 0xA5;
       FLKEY = 0xF1;

       // Write a single byte to the page
       dAddr[i] = Debug_Buffer[i];
   }

   // Disable program writes
   PSCTL = 0x00;

   // Next Debug Address and Number
   DebugAddress += 16;
   
   // Restore interrupts
   EA = EA_Save;
}
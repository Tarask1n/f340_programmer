//-----------------------------------------------------------------------------
// F3xx_Debug.h
//-----------------------------------------------------------------------------
// Copyright 2015 Kin
// Program Description:
//
// Store Debug Messages in Main Flash
//

//-----------------------------------------------------------------------------
// Definitions - Debug Messages 
//-----------------------------------------------------------------------------
// Debug Message format - TRXX0123456789ABC
//     TR            - signature
//     XX            - Subsequence number
//     0123456789ABC - 12 bytes of message
#define DEBUG_TRACE_MESSAGE     "D"
#define FLASH_DEBUG_START       0x7000
//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void Debug_Init (void);
void Store_Debug_Message (U8* DebugMessage, U32 DebugData);

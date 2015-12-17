//-----------------------------------------------------------------------------
// F3xx_USB0_Vendor_Requests.h
//-----------------------------------------------------------------------------
// Copyright 2012 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Handle vendor specific control requests.
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

#ifndef _USB_VENDOR_REQ_H_
#define _USB_VENDOR_REQ_H_

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

#define REQ_RST_STATE      0x01        // Reset state machine
#define VALUE_DEFAULT      0x0000      // Leave flash interface unchanged
#define VALUE_UNLOCK       0x0001      // Unlock flash interface

// Vendor Specific: Additional Requests
#define  GET_NAME                0x54  // Code for Get Name
#define  GET_VERSION             0x56  // Code for Get Version
#define  GET_SERIAL              0x53  // Code for Get Serial
#define  GET_REV                 0x45  // Code for Get Revision
// #define  SELF_CHECK              0x43  // Code for Process self check
// #define  GET_WRITE_STATE         0x41  // Code for Get Write State
//#define  SELF_RST_BL              0x42  // Code for Restart    'LB' - for start in bootloader mode
#define  SELF_RESTART            0x52  // Code for Restart

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void Handle_Vendor (void);
void Reset_State (void);

#endif // _USB_VENDOR_REQ_H_

//-----------------------------------------------------------------------------
// Additional Function Prototypes
//-----------------------------------------------------------------------------
void Get_Name (void);
void Get_Version (void);
void Get_Serial (void);
void Get_Rev (void);
// void Self_Check (void);
// void Get_Write_State (void);
void Self_Restart (void);


//-----------------------------------------------------------------------------
// F3xx_USB0_Vendor_Requests.c
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

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------

#include "c8051f3xx.h"
#include "F3xx_USB0_Register.h"
#include "F3xx_USB0_Descriptor.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_USB0_Vendor_Requests.h"
#include "F3xx_USB0_Main.h"
#include "F3xx_USB0_Bulk.h"

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

// taraskin Note: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define STRING_NAME_LEN sizeof("CORIGHT BOOTLOADER")
static SEGMENT_VARIABLE(STRING_NAME[STRING_NAME_LEN], const U8, SEG_CODE) = "CORIGHT BOOTLOADER";

#define STRING_VERSION_LEN sizeof("1.0.0.1")
static SEGMENT_VARIABLE(STRING_VERSION[STRING_VERSION_LEN], const U8, SEG_CODE) = "1.0.0.1";

VARIABLE_SEGMENT_POINTER (pARRAY_SERIAL, U8, SEG_CODE) = (U8 SEG_CODE *) 0x0100;
VARIABLE_SEGMENT_POINTER (pARRAY_REV, U8, SEG_CODE) = (U8 SEG_CODE *) 0x0110;
//static SEGMENT_VARIABLE(ARRAY_SERIAL[4], const U8, SEG_CODE) = {0xFF, 0xFF, 0xFF, 0xFF};
//static SEGMENT_VARIABLE(ARRAY_REV[2], const U8, SEG_CODE) = {0xFF, 0xFF};
//static SEGMENT_VARIABLE(ARRAY_10BYTES[10], const U8, SEG_CODE) = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// static SEGMENT_VARIABLE(ARRAY_WRITE_OK[3], const U8, SEG_CODE) = {0x41, 0x4f, 0x4b};
//-----------------------------------------------------------------------------
// Handle_Vendor
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Handle vendor-specific control requests
// - Decode Incoming Setup requests
// - Load data packets on fifo while in transmit mode
//
//-----------------------------------------------------------------------------
void Handle_Vendor ()
{
   switch (Setup.bRequest)
   {
      case REQ_RST_STATE:
         Reset_State ();
         break;
// taraskin
// Additional bRequest states
      case GET_NAME:
         Get_Name ();
         break;
      case GET_VERSION:
         Get_Version ();
         break;
      case GET_SERIAL:
         Get_Serial ();
         break;
      case GET_REV:
         Get_Rev ();
         break;
//       case SELF_CHECK:
//          Self_Check ();
//          break;
//       case GET_WRITE_STATE:
//          Get_Write_State ();
//          break;
      case SELF_RESTART:
         Self_Restart ();
         break;  
// End of additional states
      default:
         Force_Stall ();      // Send stall to host if invalid request
         break;
   }
}

//-----------------------------------------------------------------------------
// Reset_State
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Reset device state machine to idle. Optionally unlock the flash write/erase
// interface by resetting the device.
//
//-----------------------------------------------------------------------------
void Reset_State ()
{
   U8 reset = 0;

   // Send procedural stall if device isn't configured
   // Or request is made to host(remote wakeup not supported)
   // Or request is made to interface
   // Or msbs of value or index set to non-zero value
   // Or data length set to non-zero.
   if ((USB0_State != DEV_CONFIGURED) ||
       (Setup.bmRequestType != (DIR_OUT | REQ_TYPE_VENDOR | RECP_DEVICE)) ||
       (Setup.wIndex.U16 != 0) ||
       (Setup.wLength.U16 != 0) ||
       (Setup.wValue.U16 > 1))
   {
      Force_Stall ();
   }
   else
   {
      // Signal the foreground to transition to the IDLE state
      AsyncResetState = 1;

      // Flash write/erase interface is locked
      if (Setup.wValue.U16 == VALUE_UNLOCK && FLKEY == 0x03)
      {
         reset = 1;
      }
   }

   // Reset Index to 0
   POLL_WRITE_BYTE (INDEX, 0);

   if (EP_Status[0] != EP_STALL)
   {
      // Set Serviced Out packet ready and
      // data end to indicate transaction
      // is over
      POLL_WRITE_BYTE (E0CSR, (rbSOPRDY | rbDATAEND));
   }

   if (reset)
   {
      // Wait to allow the control transfer
      // to finish before resetting the device
      Delay ();

      // Perform software reset
      RSTSRC |= 0x10;
   }
}

//-----------------------------------------------------------------------------
// Self_Restart
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Reset device by request from host.
//
void Self_Restart (void)
{
      // Wait to allow the control transfer
      // to finish before resetting the device
      Delay ();

      // Perform software reset
      RSTSRC |= 0x10;
}

//-----------------------------------------------------------------------------
// Get_Name
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Additional request that should not change for custom HID designs.
//
// This routine returns a packet to the host which contain a string name of device.
//
// ----------------------------------------------------------------------------
void Get_Name (void)
{
   // Return string
   DataPtr = (U8*)&STRING_NAME;
   DataSize = STRING_NAME_LEN;

   if (EP_Status[0] != EP_STALL)
   {
      // Set serviced Setup Packet, Endpoint 0 in transmit mode, and
      // reset DataSent counter
      POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
      EP_Status[0] = EP_TX;
      DataSent = 0;
   }
}

void Get_Version (void)
{
   // Return string
   DataPtr = (U8*)&STRING_VERSION;
   DataSize = STRING_VERSION_LEN;

   if (EP_Status[0] != EP_STALL)
   {
      // Set serviced Setup Packet, Endpoint 0 in transmit mode, and
      // reset DataSent counter
      POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
      EP_Status[0] = EP_TX;
      DataSent = 0;
   }
}

void Get_Serial (void)
{
   // Return serial number
   DataPtr = pARRAY_SERIAL;
   DataSize = 4;

   if (EP_Status[0] != EP_STALL)
   {
      // Set serviced Setup Packet, Endpoint 0 in transmit mode, and
      // reset DataSent counter
      POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
      EP_Status[0] = EP_TX;
      DataSent = 0;
   }
}

void Get_Rev (void)
{
   // Return attributes
   DataPtr = pARRAY_REV;
   DataSize = 2;

   if (EP_Status[0] != EP_STALL)
   {
      // Set serviced Setup Packet, Endpoint 0 in transmit mode, and
      // reset DataSent counter
      POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
      EP_Status[0] = EP_TX;
      DataSent = 0;
   }
}

// void Self_Check (void)
// {
//    // Return attributes
//    DataPtr = (U8*)&ARRAY_10BYTES;
//    DataSize = sizeof(ARRAY_10BYTES);

//    if (EP_Status[0] != EP_STALL)
//    {
//       // Set serviced Setup Packet, Endpoint 0 in transmit mode, and
//       // reset DataSent counter
//       POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
//       EP_Status[0] = EP_TX;
//       DataSent = 0;
//    }
// }

// void Get_Write_State (void)
// {
//    // Return attributes
//    DataPtr = (U8*)&ARRAY_WRITE_OK;
//    DataSize = sizeof(ARRAY_WRITE_OK);

//    if (EP_Status[0] != EP_STALL)
//    {
//       // Set serviced Setup Packet, Endpoint 0 in transmit mode, and
//       // reset DataSent counter
//       POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
//       EP_Status[0] = EP_TX;
//       DataSent = 0;
//    }
// }
//-----------------------------------------------------------------------------
// F3xx_USB_Main.c
//-----------------------------------------------------------------------------
// Copyright 2008-2012 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// USB bulk file transfer example
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
// Release 1.1
//    -Changed "Usb_Init" to "USB_Init" (TP)
//    -07 OCT 2010
//
// Release 1.0
//    -Initial Revision (PD)
//    -04 JUN 2008
//

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------

#include "c8051f3xx.h"
#include "F3xx_USB0_Register.h"
#include "F3xx_USB0_Descriptor.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_USB0_Main.h"
#include "F3xx_USB0_Bulk.h"
#include "F3xx_Flash.h"
#include "F3xx_Debug.h"
#include "F3xx_XTEA.h"

//-----------------------------------------------------------------------------
// Firmware Call Prototypes
//-----------------------------------------------------------------------------
void (*Application)(void) = (void SEG_CODE *) 0x2800;
//INTERRUPT_PROTO (Usb_ISR, INTERRUPT_USB0);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
// Buffer used to transmit a USB packet to the host
// In_Packet (Foreground) => USB IN FIFO (ISR) => Host
SEGMENT_VARIABLE (In_Packet[IN_EP1_PACKET_SIZE], U8, SEG_XDATA);

// Buffer used to receive a USB packet from the host
// Host => USB OUT FIFO (ISR) => Out_Packet (Foreground)
//SEGMENT_VARIABLE (Out_Packet[OUT_EP1_PACKET_SIZE], U8, SEG_XDATA);
SEGMENT_VARIABLE (Out_Packet[OUT_EP2_PACKET_SIZE], U8, SEG_XDATA);

// Buffer used to store a temporary page data
SEGMENT_VARIABLE (Buffer_Sector[BUFFER_SECTOR_SIZE], U8, SEG_XDATA);

// Buffer used to store a temporary flash CRC data
SEGMENT_VARIABLE (Temp_Buffer[TEMP_BUFFER_SIZE], U8, SEG_XDATA);

// Buffer used to store a FW request to boot sign
VARIABLE_SEGMENT_POINTER (pBoot_By_FW_Request, U16, SEG_XDATA) = (U16 SEG_XDATA *) 0x0FFE;

// Pointer to activate read-flash protection 
VARIABLE_SEGMENT_POINTER (pRead_Flash_Protection, U8, SEG_XDATA) = (U8 SEG_XDATA *) 0xFBFF;

// State of the In_Packet buffer used to transmit a USB packet to the host:
//
// In_Packet_Ready = 0:
//    - In_Packet is empty
//    - Load In_Packet with a USB packet
//    - Set In_Packet_Ready to 1
// In_Packet_Ready = 1:
//    - In_Packet contains a USB packet to send to the host
//    - Wait until In_Packet_Ready = 0, indicating transmission of a packet
//      (to USB OUT FIFO)
U8 In_Packet_Ready = 0;

// State of the Out_Packet buffer used to receive a USB packet from the host:
//
// Out_Packet_Ready = 0:
//    - Out_Packet is empty
//    - Wait until Out_Packet_Ready = 1, indicating reception of a packet
// Out_Packet_Ready = 1:
//    - Out_Packet contains a USB packet received from the host
//    - Unload Out_Packet
//    - Set Out_Packet_Ready = 0
U8 Out_Packet_Ready = 0;

// ISR can set this flag to reset the state machine to idle
U8 AsyncResetState = 0;

SBIT (Boot_Pin, SFR_P0, 1);     // Boot_Pin = '0' means manually load boot

//-----------------------------------------------------------------------------
// Static Global Variables - State Machine
//-----------------------------------------------------------------------------

// State machine state
static U8 State = ST_IDLE;

// State variables for Read Page
static U16 TxBlock;    // Current block to send to host
static U16 MaxBlock;   // Max block to send to host
// static U8 TxPage;     // Current flash page to send to host
// static U8 TxValid;    // Current flash page to send to host is valid

// State variables for Write Page
static U16 RxBlock;    // Current block to receive from host
// static U8 RxPage;     // Current flash page to receive from host
// static U8 RxValid;    // Current flash page to receive from host is valid

// 
static U16 Active_Command;      // Active command from host
static U16 Buffer_Length;       // Length
static U16 Begin_Flash_Address; // Begin flash address
static U16 End_Flash_Address;   // End flash address
static U16 Length_Flash;        // Length flash 
static U16 Current_Flash_Address;

static U32 Received_Flash_CRC;

//-----------------------------------------------------------------------------
// Static Function Prototypes - State Machine
//-----------------------------------------------------------------------------

static void StateIdle (void);
// static void StateSetFlashKey (void);
// static void StateTxPageInfo (void);
static void StateReadPage (void);
static void StateTxBlock (void);
static void StateWritePage (void);
static void StateRxBlock (void);
// static void StateTxSuccess (void);
// static void StateTxInvalid (void);
static void StateMachine (void);

//-----------------------------------------------------------------------------
// Static Functions - State Machine
//-----------------------------------------------------------------------------
// Get U32 Big Endian from &U32 Little Endian 
U32 Get_U32 (U8* Src)
{
    UU32 Dest;
    Dest.U8[3] = Src[0];
    Dest.U8[2] = Src[1];
    Dest.U8[1] = Src[2];
    Dest.U8[0] = Src[3];
    return Dest.U32;
}

// Put &U32 Little Endian from U32 Big Endian 
void Put_U32 (U8* Dest, U32 Src)
{
    UU32 Data;
    Data.U32 = Src;
    Dest[3] = Data.U8[0];
    Dest[2] = Data.U8[1];
    Dest[1] = Data.U8[2];
    Dest[0] = Data.U8[3];

//     Dest[3] = ((U8[]) &Src)[0];
//     Dest[2] = ((U8[]) &Src)[1];
//     Dest[1] = ((U8[]) &Src)[2];
//     Dest[0] = ((U8[]) &Src)[3];   
}

// Get U16 Big Endian from &U16 Little Endian 
U16 Get_U16 (U8* Src)
{
    UU16 Dest;
    Dest.U8[1] = Src[0];
    Dest.U8[0] = Src[1];
    return Dest.U16;
}

// Copy Length bytes in XDATA
void MemCopy (U8* Src, U8* Dest, U16 Length)
{
    U16 i;
    for ( i = 0; i < Length; i++)
    {
        Dest[i] = Src[i];
    }
}

// Ready to receive a command from the host
static void StateIdle (void)
{
    // Recieved an OUT packet
    if (Out_Packet_Ready)
    {
        // Get and decode the command and parameters
        Active_Command = Get_U16(Out_Packet);
        switch (Active_Command)
        {
//          case CMD_SET_FLASH_KEY:
//             State = ST_SET_FLASH_KEY;
//             break;

//          case CMD_GET_PAGE_INFO:
//             // Done processing Out_Packet
//             Out_Packet_Ready = 0;
//             State = ST_TX_PAGE_INFO;
//             break;

            case CMD_READ_PAGE:
                Begin_Flash_Address = Get_U16(&Out_Packet[8]);
                End_Flash_Address = Get_U16(&Out_Packet[4]);
                Length_Flash = End_Flash_Address - Begin_Flash_Address;
                
                State = ST_READ_PAGE;
                break;

            case CMD_WRITE_PAGE:
                Begin_Flash_Address = Get_U16(&Out_Packet[8]);
                End_Flash_Address = Get_U16(&Out_Packet[4]);
                Length_Flash = End_Flash_Address - Begin_Flash_Address;

                State = ST_WRITE_PAGE;
                break;

            case CMD_CALC_FLASH_CRC:
                Received_Flash_CRC = Get_U32(&Out_Packet[2]);
                Received_Flash_CRC ^=  0x61726154; // 'Tara' -> 'araT'
                State = ST_CALC_FLASH_CRC;
                break;

            default:
            // Done processing Out_Packet
//                Out_Packet_Ready = 0;
                break;
        }
        // Packet released
        Out_Packet_Ready = 0;
    }
}

// // Receive flash key from host
// static void StateSetFlashKey (void)
// {
//    // Set the flash key
//    SetFlashKey (&Out_Packet[1]);

//    // Done processing Out_Packet
//    Out_Packet_Ready = 0;

//    State = ST_TX_SUCCESS;
// }

// // Send number of flash pages and flash page size to host
// static void StateTxPageInfo (void)
// {
//    // If able to send an IN packet
//    if (!In_Packet_Ready)
//    {
//       In_Packet[0] = RSP_SUCCESS;
//       In_Packet[1] = FLASH_NUM_PAGES;
//       In_Packet[2] = LOBYTE(FLASH_PAGE_SIZE);
//       In_Packet[3] = HIBYTE(FLASH_PAGE_SIZE);

//       // Ready to send IN packet
//       In_Packet_Ready = 1;

//       State = ST_IDLE;
//    }
// }

// Send flash page to host in blocks (command stage)
static void StateReadPage (void)
{
    if (!In_Packet_Ready)
    {
        U8 i;
        // Turn on LED1 while reading page
        SetLed (1);
        
        // Prepare empty 0x00 packet for answer on ReadDev command
        for (i = 0; i < IN_EP1_PACKET_SIZE; i++)
        {
            In_Packet[i] = 0x00;
        }
        // Ready to send IN packet
        In_Packet_Ready = 1;

        // Initialize Crypt
        InitCrypt ();

        // Reset the block counter
        //
        // There are 8, 64-byte blocks in a 512-byte flash page
        Current_Flash_Address = Begin_Flash_Address;
        MaxBlock = (Length_Flash + (IN_EP1_PACKET_SIZE - 1)) / IN_EP1_PACKET_SIZE;
        TxBlock = 0;

        State = ST_TX_BLOCK;
    }
}

// Send flash page to host in blocks (data/response stage)
static void StateTxBlock (void)
{
    // Response stage:
    // Finished sending last block to host
    if (TxBlock == MaxBlock)
    {
        // Turn off LED1 after finished reading page
        SetLed (0);

        State = ST_IDLE;
    }
    // Data stage:
    // If able to send an IN packet
    else if (!In_Packet_Ready)
    {
        if ((TxBlock & 7) == 0)
        {
            ReadFlashPage (Current_Flash_Address, Buffer_Sector, BUFFER_SECTOR_SIZE);
            EnCrypt (Buffer_Sector);
        }
        // Copy packet to sector buffer
        MemCopy (&Buffer_Sector[(TxBlock & 7) * IN_EP1_PACKET_SIZE], In_Packet, IN_EP1_PACKET_SIZE);

        // Ready to send IN packet
        In_Packet_Ready = 1;
        
        Current_Flash_Address += IN_EP1_PACKET_SIZE;
        TxBlock++;
   }
}

// Receive flash page from host in blocks and write to flash (command stage)
static void StateWritePage (void)
{
    if (!In_Packet_Ready)
    {
        U8 i;
        // Turn on LED1 while reading page
        SetLed (1);

        // Prepare empty 0x00 packet for answer on ReadDev command
        for (i = 0; i < IN_EP1_PACKET_SIZE; i++)
        {
            In_Packet[i] = 0x00;
        }
        // Ready to send IN packet
        In_Packet_Ready = 1;

        // Clear CRC32 and flash length data
        // Erase 0xF600 page
        EraseFlashPage(0xF600);
        // Write to Flash Length_Flash
        Temp_Buffer[0] = ((U8[]) &Length_Flash)[1];
        Temp_Buffer[1] = ((U8[]) &Length_Flash)[0];
        WriteFlashPage(0xF6FA, Temp_Buffer, 2);

        // Initialize Crypto
        InitCrypt ();

        // Reset the block counter
        // There are 8, 64-byte blocks in a 512-byte flash page
        Current_Flash_Address = Begin_Flash_Address;
//        Current_Flash_Address = 0x2800;
//        End_Flash_Address = Current_Flash_Address + Length_Flash;
        MaxBlock = (Length_Flash + (IN_EP1_PACKET_SIZE - 1)) / IN_EP1_PACKET_SIZE;
        RxBlock = 0;

#if (DEBUG_MODE == 1)
        Store_Debug_Message ("End flash", End_Flash_Address);
        Store_Debug_Message ("Max block", MaxBlock);
#endif
        
        State = ST_RX_BLOCK;
    }
}

// Receive flash page from host in blocks and write to flash (response/data stage)
static void StateRxBlock (void)
{
    // Response stage:
    // Finished receiving last block 
    if (RxBlock == MaxBlock)
    {
        // Turn off LED1 after finished writing page
        SetLed (0);

        State = ST_IDLE;
    }
    // Data stage:
    // If received an OUT packet
    else if (Out_Packet_Ready)
    {
        // Copy packet to sector buffer
        MemCopy (Out_Packet, &Buffer_Sector[(RxBlock & 7) * OUT_EP2_PACKET_SIZE], OUT_EP2_PACKET_SIZE);
        
        if ((RxBlock & 7) == 0)
//        if (!(Current_Flash_Address & 0x1FF))
        {
            EraseFlashPage (Current_Flash_Address);
        }
            
        // Finished processing OUT packet
        Out_Packet_Ready = 0;

        RxBlock++;

        // If we receive sector then write to flash
        if ((RxBlock & 7) == 0)
        {
            DeCrypt (Buffer_Sector);
            WriteFlashPage (Current_Flash_Address & 0xFE00, Buffer_Sector, BUFFER_SECTOR_SIZE);
        }

        Current_Flash_Address += OUT_EP2_PACKET_SIZE;
    }
}

// Receive flash page from host in blocks and write to flash (command stage)
static void StateCalcFlashCrc (void)
{
    if (!In_Packet_Ready)
    {
        U8 i, ResponseState;
        // Check calculated CRC32 with received
        ReadFlashPage(0xF6FA, Temp_Buffer, 2);
        Begin_Flash_Address = BEGIN_FW_ADDR;
        Length_Flash = Get_U16(Temp_Buffer);
//         if (Begin_Flash_Address != 0x2800)
//         {
//             Begin_Flash_Address = 0x2800;
//         }
        if (Length_Flash > 0xCE00)
        {
            Length_Flash = 0xCE00;
        }
#if (DEBUG_MODE == 1)
        Store_Debug_Message ("ReceiveCRC", Received_Flash_CRC);
#endif
        if (Received_Flash_CRC == GetFlashCRC32(Begin_Flash_Address, Length_Flash))
        {
            Temp_Buffer[0] = ((U8[]) &Received_Flash_CRC)[3];
            Temp_Buffer[1] = ((U8[]) &Received_Flash_CRC)[2];
            Temp_Buffer[2] = ((U8[]) &Received_Flash_CRC)[1];
            Temp_Buffer[3] = ((U8[]) &Received_Flash_CRC)[0];

            WriteFlashPage(0xF6FC, Temp_Buffer, 4);
            ResponseState = 0x00;
        }
        else
        {
            ResponseState = 0xFF;
        }

        // Prepare empty 0x00 packet for answer on ReadDev command
        for (i = 0; i < IN_EP1_PACKET_SIZE; i++)
        {
            In_Packet[i] = ResponseState;
        }
        // Ready to send IN packet
        In_Packet_Ready = 1;
        
        State = ST_IDLE;
    }
}

// // Send a response (success)
// static void StateTxSuccess (void)
// {
//    // If able to send an IN packet
//    if (!In_Packet_Ready)
//    {
//       In_Packet[0] = RSP_SUCCESS;

//       // Ready to send IN packet
//       In_Packet_Ready = 1;

//       State = ST_IDLE;
//    }
// }

// // Send a response (invalid)
// static void StateTxInvalid (void)
// {
//    // If able to send an IN packet
//    if (!In_Packet_Ready)
//    {
//       In_Packet[0] = RSP_INVALID;

//       // Ready to send IN packet
//       In_Packet_Ready = 1;

//       State = ST_IDLE;
//    }
// }

// Manage state transitions
static void StateMachine (void)
{
   U8 EA_Save;

   // Received a reset state control request
   // from the host. Transition to the idle
   // state after completing the current state
   // function
   if (AsyncResetState)
   {
      // Disable interrupts
      EA_Save = EA;
      EA = 0;

// taraskin
      // Set index to endpoint 1 registers
//      POLL_WRITE_BYTE(INDEX, 1);

      // Flush IN/OUT FIFOs
//      POLL_WRITE_BYTE(EINCSR1, rbInFLUSH);
//      POLL_WRITE_BYTE(EOUTCSR1, rbOutFLUSH);

// taraskin
      // Set index to endpoint 1 registers
      POLL_WRITE_BYTE(INDEX, 1);
      // Flush IN FIFOs
      POLL_WRITE_BYTE(EINCSR1, rbInFLUSH);
      POLL_WRITE_BYTE(EOUTCSR1, rbOutFLUSH);
      // Set index to endpoint 2 registers
      POLL_WRITE_BYTE(INDEX, 2);
      // Flush OUT FIFOs
      POLL_WRITE_BYTE(EINCSR1, rbInFLUSH);
      POLL_WRITE_BYTE(EOUTCSR1, rbOutFLUSH);

      // Flush software In_Packet/Out_Packet buffers
      In_Packet_Ready = 0;
      Out_Packet_Ready = 0;
// taraskin

      // Reset state to idle
      State = ST_IDLE;

      // Turn off LED
      SetLed (0);

      // Finished resetting state
      AsyncResetState = 0;

      // Restore interrupts
      EA = EA_Save;
   }

   switch (State)
   {
      case ST_IDLE:
         StateIdle ();
         break;

//       case ST_SET_FLASH_KEY:
//          StateSetFlashKey ();
//          break;

//       case ST_TX_PAGE_INFO:
//          StateTxPageInfo ();
//          break;

      case ST_READ_PAGE:
         StateReadPage ();
         break;

      case ST_TX_BLOCK:
         StateTxBlock ();
         break;

      case ST_WRITE_PAGE:
         StateWritePage ();
         break;

      case ST_RX_BLOCK:
         StateRxBlock ();
         break;

      case ST_CALC_FLASH_CRC:
         StateCalcFlashCrc ();
         break;
      
//       case ST_TX_SUCCESS:
//          StateTxSuccess ();
//          break;

//       case ST_TX_INVALID:
//          StateTxInvalid ();
//          break;
   }
}

U8 CheckFirmwareCRC()
{
    U32 Stored_CRC32, Calc_CRC32;
    // Check calculated CRC32 with previously stored
    Begin_Flash_Address = BEGIN_FW_ADDR;

    ReadFlashPage(0xF6FA, Temp_Buffer, 6);
    Length_Flash = Get_U16(Temp_Buffer);
    Stored_CRC32 = Get_U32(&Temp_Buffer[2]);

    #if (DEBUG_MODE == 1)
    Store_Debug_Message ("Stored CRC", Stored_CRC32);
#endif
//     if (Begin_Flash_Address != 0x2800)
//     {
//         Begin_Flash_Address = 0x2800;
//     }
    if (Length_Flash > 0xCE00)
    {
        Length_Flash = 0xCE00;
    }
    Calc_CRC32 = GetFlashCRC32(Begin_Flash_Address, Length_Flash);
#if (DEBUG_MODE == 1)
        Store_Debug_Message ("Calc CRC", Calc_CRC32);
#endif
    if (Stored_CRC32 == Calc_CRC32)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//-----------------------------------------------------------------------------
// Main Routine
//-----------------------------------------------------------------------------

void main (void)
{
    U8 EA_Save, Count = 0;

    System_Init ();                     // Initialize Sysclk, Port IO
    CreateCRC32Table();                 // Initialize CRC32 poly table
    
#if (DEBUG_MODE == 1)
    Debug_Init ();                      // Initialize Debug 
    Store_Debug_Message ("Start Boot", 2015);
#endif
    EA = 1;                             // Enable global interrupts

    // Turn off LED
    SetLed (0);
    
    // Check for read-protection status
#if (FLASH_PROTECTION == 1)
    if (*pRead_Flash_Protection != 0x82)      // 0xBF - until 0x8000 address, 0x82 - until 0xFA00 address
    {
        // Disable interrupts
        EA = 0;

        // Enable program writes
        PSCTL = 0x01;

//     #if FLASH_GROUP_WRITE_EN
//         // Enable two-byte flash writes
//         PFE0CN |= 0x01;
//     #endif // FLASH_GROUP_WRITE_EN

        FLKEY = 0xA5;
        FLKEY = 0xF1;

        // Write a single byte to the protection register
        *pRead_Flash_Protection = 0x82;       // 0xBF - until 0x8000 address, 0x82 - until 0xFA00 address

        // Disable program writes
        PSCTL = 0x00;

        // Restore interrupts
        EA = EA_Save;
    }
#endif    

    // Read 'LB' sign to force load bootloader
    Length_Flash = *pBoot_By_FW_Request;
#if (DEBUG_MODE == 1)
    Store_Debug_Message ("FW boot val", Length_Flash);
#endif
    if (Length_Flash == 0x424C)   // 'LB' - Force start bootloader from firmware
    {
        *pBoot_By_FW_Request = 0xFFFF;
    }
    // Check firmware CRC, if OK run Application
    else if (CheckFirmwareCRC ()) //  && Boot_Pin)
    {
#if (DEBUG_MODE == 1)
        Store_Debug_Message ("Launch Main", 0);
#endif
        Application ();
    }

#if (DEBUG_MODE == 1)
        Store_Debug_Message ("Launch Boot", 0);
#endif

    USB0_Init ();                       // Initialize USB0

    while (1)
    {
        if (Count++ == 4)
        {
            StateMachine ();
            Count = 0;
        }

        EA_Save = EA;
        EA = 0;
//       Receive_Packet_Foreground ();
        Usb_ISR ();
        EA = EA_Save;
    }
}

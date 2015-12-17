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
//#include "F3xx_Flash.h"
#include "F3xx_SPI0.h"
#include "F3xx_Debug.h"

//-----------------------------------------------------------------------------
// Interrupt Prototypes
//-----------------------------------------------------------------------------

INTERRUPT_PROTO (Usb_ISR, INTERRUPT_USB0);
INTERRUPT_PROTO (Spi_ISR, INTERRUPT_SPI0);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
// Buffer used to transmit a USB packet to the host
// In_Packet (Foreground) => USB IN FIFO (ISR) => Host
//SEGMENT_VARIABLE (In_Packet[IN_EP1_PACKET_SIZE], U8, SEG_XDATA);
//SEGMENT_VARIABLE (In_Packets[BUFFER_PACKET_NUM * 4][IN_EP1_PACKET_SIZE], U8, SEG_XDATA);

// Buffer used to receive a USB packet from the host
// Host => USB OUT FIFO (ISR) => Out_Packet (Foreground)
//SEGMENT_VARIABLE (Out_Packet[OUT_EP1_PACKET_SIZE], U8, SEG_XDATA);
//SEGMENT_VARIABLE (Out_Packet[OUT_EP2_PACKET_SIZE], U8, SEG_XDATA);
//SEGMENT_VARIABLE (Out_Packets[BUFFER_PACKET_NUM][OUT_EP2_PACKET_SIZE], U8, SEG_XDATA);

// Buffer used to store a temporary page data
SEGMENT_VARIABLE (InOut_Packets[BUFFER_PACKET_NUM][BUFFER_PACKET_SIZE], U8, SEG_XDATA);

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
U8 In_Packet_Num = BUFFER_PACKET_NUM - 1;
// U8 In_Packets_State[2] = {EMPTY, EMPTY};

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
U8 Out_Packet_Num = 0;
U8 Out_Packets_State[2] = {EMPTY, EMPTY};

// ISR can set this flag to reset the state machine to idle
U8 AsyncResetState = 0;

U32 DeviceStartAddr;    // Device Flash Start Address
U32 DeviceEndAddr;      // Device Flash Length (current operation)
U32 DeviceCurSector;    // Device Flash Current Sector
U32 DeviceMaxSector;    // Device Flash Max Sector

U8 StatusSPIWriteOk = 1;

//-----------------------------------------------------------------------------
// Static Global Variables - State Machine
//-----------------------------------------------------------------------------

// State machine state
static U8 State = ST_IDLE;

// // State variables for Read Page
// static U8 TxBlock;    // Current block to send to host
// static U8 TxPage;     // Current flash page to send to host
// static U8 TxValid;    // Current flash page to send to host is valid

// // State variables for Write Page
// static U8 RxBlock;    // Current block to receive from host
// static U8 RxPage;     // Current flash page to receive from host
// static U8 RxValid;    // Current flash page to receive from host is valid

//-----------------------------------------------------------------------------
// Static Function Prototypes - State Machine
//-----------------------------------------------------------------------------

static void StateIdle (void);
// static void StateSetFlashKey (void);
// static void StateTxPageInfo (void);
// static void StateReadPage (void);
// static void StateTxBlock (void);
// static void StateWritePage (void);
// static void StateRxBlock (void);
// static void StateTxSuccess (void);
// static void StateTxInvalid (void);
static void StateMachine (void);

static void StateSPI0GetDevId (void);
static void StateSPI0ReadDev (void);
static void StateSPI0WriteDev (void);
static void StateSPI0EraseDev (void);


//-----------------------------------------------------------------------------
// Static Functions - State Machine
//-----------------------------------------------------------------------------

// Ready to receive a command from the host
static void StateIdle (void)
{
    // Recieved an OUT packet
    if (Out_Packet_Ready)
    {
      // Decode the command
        switch (Get_U16(&InOut_Packets[Out_Packet_Num][0]))
        {
//             case CMD_SET_FLASH_KEY:
//                 State = ST_SET_FLASH_KEY;
//                 break;

//             case CMD_GET_PAGE_INFO:
//             // Done processing Out_Packet
//                 Out_Packet_Ready = 0;
//                 State = ST_TX_PAGE_INFO;
//                 break;

//             case CMD_READ_PAGE:
//                 State = ST_READ_PAGE;
//                 break;

//             case CMD_WRITE_PAGE:
//                 State = ST_WRITE_PAGE;
//                 break;
// tarask1n
// USB SPI Command IDs				 
            case CMD_SPI0_GET_DEV_ID:
                State = ST_SPI0_GET_DEV_ID;
                break;

            case CMD_SPI0_READ_DEV:
                State = ST_SPI0_READ_DEV;
                break;
                 
            case CMD_SPI0_WRITE_DEV:
                State = ST_SPI0_WRITE_DEV;
                break;

            case CMD_SPI0_ERASE_DEV:
                State = ST_SPI0_ERASE_DEV;
                break;

            default:
            // Done processing Out_Packet
                Out_Packet_Ready = 0;
//                State = ST_TX_INVALID;
                break;
         }
    }
}

// Receive flash key from host
// static void StateSetFlashKey (void)
// {
//    // Set the flash key
//    SetFlashKey (&Out_Packet[1]);

//    // Done processing Out_Packet
//    Out_Packet_Ready = 0;

//    State = ST_TX_SUCCESS;
// }

// Send number of flash pages and flash page size to host
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
// static void StateReadPage (void)
// {
//    // Turn on LED1 while reading page
//    SetLed (1);

//    // Store the page number
//    TxPage = Out_Packet[1];

//    // Done processing Out_Packet
//    Out_Packet_Ready = 0;

//    // Reset the block counter
//    //
//    // There are 8, 64-byte blocks in a 512-byte
//    // flash page
//    TxBlock = 0;

//    // Check if the requested page number is valid.
//    //
//    // If the page is invalid, we'll send dummy blocks
//    // back and return an error in the response packet.
//    if (TxPage < FLASH_NUM_PAGES)
//       TxValid = 1;
//    else
//       TxValid = 0;

//    State = ST_TX_BLOCK;
// }

// // Send flash page to host in blocks (data/response stage)
// static void StateTxBlock (void)
// {
//    // Response stage:
//    // Finished sending last block to host
//    if (TxBlock == 8)
//    {
//       // Turn off LED1 after finished reading page
//       SetLed (0);

//       // Send appropriate response
//       if (TxValid)
//          State = ST_TX_SUCCESS;
//       else
//          State = ST_TX_INVALID;
//    }
//    // Data stage:
//    // If able to send an IN packet
//    else if (!In_Packet_Ready)
//    {
//       // Calculate the flash address based on current page and block number
//       U16 address = FLASH_START + (FLASH_PAGE_SIZE * TxPage) + (IN_EP1_PACKET_SIZE * TxBlock);

//       // Only read from flash if the flash page is valid.
//       // Otherwise, send whatever dummy data is left in In_Packet
//       if (TxValid)
//       {
//          ReadFlashPage (address, In_Packet, IN_EP1_PACKET_SIZE);
//       }

//       // Ready to send IN packet
//       In_Packet_Ready = 1;

//       TxBlock++;
//    }
// }

// Receive flash page from host in blocks and write to flash (command stage)
// static void StateWritePage (void)
// {
//    // Turn on LED2 while writing page
//    SetLed (1);

//    // Store the page number
//    RxPage = Out_Packet[1];

//    // Done processing Out_Packet
//    Out_Packet_Ready = 0;

//    // Reset the block counter
//    //
//    // There are 8, 64-byte blocks in a 512-byte
//    // flash page
//    RxBlock = 0;

//    // Check if the requested page number is valid.
//    //
//    // If the page is invalid, we'll send dummy blocks
//    // back and return an error in the response packet.
//    if (RxPage < FLASH_NUM_PAGES)
//    {
//       // Flash page is valid
//       RxValid = 1;

//       // Erase the flash page
//       EraseFlashPage (FLASH_START + (RxPage * FLASH_PAGE_SIZE));
//    }
//    else
//    {
//       // Flash page is invalid
//       RxValid = 0;
//    }

//    State = ST_RX_BLOCK;
// }

// Receive flash page from host in blocks and write to flash (response/data stage)
// static void StateRxBlock (void)
// {
//    // Response stage:
//    // Finished receiving last block 
//    if (RxBlock == 8)
//    {
//       // Turn off LED1 after finished writing page
//       SetLed (0);

//       if (RxValid)
//          State = ST_TX_SUCCESS;
//       else
//          State = ST_TX_INVALID;
//    }
//    // Data stage:
//    // If received an OUT packet
//    else if (Out_Packet_Ready)
//    {
// //      U16 address = FLASH_START + (FLASH_PAGE_SIZE * RxPage) + (OUT_EP1_PACKET_SIZE * RxBlock);
//       U16 address = FLASH_START + (FLASH_PAGE_SIZE * RxPage) + (OUT_EP2_PACKET_SIZE * RxBlock);

//       if (RxValid)
//       {
// //         WriteFlashPage (address, Out_Packet, OUT_EP1_PACKET_SIZE);
//          WriteFlashPage (address, Out_Packet, OUT_EP2_PACKET_SIZE);
//       }

//       // Finished processing OUT packet
//       Out_Packet_Ready = 0;

//       RxBlock++;
//    }
// }

// Send a response (success)
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

// Send a response (invalid)
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

// Send flash Device ID to host in following format
// 1st byte - Manufacturer ID
// 2nd byte - Device ID (Memory Type)
// 3rd byte - Device ID (Capacity)
static void StateSPI0GetDevId (void)
{
#if (DEBUG_MODE == 1)
    U32 TempLong;
#endif

    if (!In_Packet_Ready)
    {
        // Turn on LED1 while accessing the device
        SetLed (1);
        // Activate SPI0 Power
        SetSPI0PowerOn ();
        
        // Done processing Out_Packet
        Out_Packet_Ready = 0;
        
        GetSPI0DevId (&InOut_Packets[In_Packet_Num][0]);

        // Debug messages
#if (DEBUG_MODE == 1)
        TempLong = Get_U32(&InOut_Packets[In_Packet_Num][0]);
        Store_Debug_Message ("DeviceID", TempLong);
#endif

        // Ready to send IN packet
        In_Packet_Ready = 1;

        // Turn off LED1
        SetLed (0);
        // Deactivate SPI0 Power
        SetSPI0PowerOff ();

        State = ST_IDLE;
    }
}

// Erase whole device (command stage)
static void StateSPI0EraseDev (void)
{
    if (!In_Packet_Ready)
    {
        U8 i;
        // Turn on LED1 while reading whole device
        SetLed (1);
        // Activate SPI0 Power
        SetSPI0PowerOn ();

        // Store the required bytes number
        DeviceStartAddr = Get_U32(&InOut_Packets[Out_Packet_Num][8]);
        DeviceEndAddr = Get_U32(&InOut_Packets[Out_Packet_Num][4]);
        // Copy Spi Commands Set
        MemCopy (&InOut_Packets[Out_Packet_Num][19], (U8 *) &Cmd_Spi, sizeof (Cmd_Spi));

//         // Done processing Out_Packet
//         Out_Packet_Ready = 0;

        // Prepare empty 0x00 packet for answer on ReadDev command
        for (i = 0; i < IN_EP1_PACKET_SIZE; i++)
        {
            InOut_Packets[In_Packet_Num][i] = 0x00;
        }
        // Ready to send IN packet
        In_Packet_Ready = 1;

//         while (1)
//         {
//             Delay_ms (1000);
//             SetLed (0);
//             Delay_ms (1000);
//             SetLed (1);
//         }
        
        // Erase whole device
        EraseSPI0Device ();
            
        // Debug messages
#if (DEBUG_MODE == 1)        
        Store_Debug_Message ("ErCapByte", DeviceEndAddr);
#endif
        
        // There are Device Capacity calculated in 64-bytes sectors
        DeviceCurSector = DeviceStartAddr / IN_EP1_PACKET_SIZE;
        DeviceMaxSector = (DeviceEndAddr / IN_EP1_PACKET_SIZE) >> 2;

        // Done processing Out_Packet
        Out_Packet_Ready = 0;

        State = ST_SPI0_ER_SECTORS;
    }
}

// Receive device flash from host in sectors (data/response stage)
static void StateSPI0EraseSectors (void)
{
    U8 i, j; 
    if (DeviceCurSector < DeviceMaxSector)
    {
        if (Out_Packet_Ready)
        {
            if (InOut_Packets[Out_Packet_Num][0] == 0x20)
            {
                // Temp variable j
                j = GetSPI0StatusBusy ();
                // Prepare empty 0x00 packet for answer on ErasePoll command
                for (i = 0; i < IN_EP1_PACKET_SIZE; i++)
                {
                    InOut_Packets[In_Packet_Num][i] = j; //0x00;
                }

    //            MemCopy ((U8 *) &Cmd_Spi, &InOut_Packets[In_Packet_Num][0], sizeof (Cmd_Spi));
                
                if (j) // || (DeviceCurSector < 400))
                {
                    // Waiting status
                    InOut_Packets[In_Packet_Num][0] = 0x02;
                }
                else
                {
                    // Success status
                    InOut_Packets[In_Packet_Num][0] = 0x01;
                    // Debug messages
    #if (DEBUG_MODE == 1)                
                    Store_Debug_Message ("OkErSector", DeviceCurSector);
    #endif                

                    // Turn off LED1 after error erasing device
                    SetLed (0);
                    // Deactivate SPI0 Power
                    SetSPI0PowerOff ();

                    State = ST_IDLE;           
                }
                // Ready to send IN packet
                In_Packet_Ready = 1;
            }
            // Ready to receive OUT packet
            Out_Packet_Ready = 0;
            DeviceCurSector++;
        }
    }
    else
    {
        if (!In_Packet_Ready)
        {
            for (i = 0; i < IN_EP1_PACKET_SIZE; i++)
            {
                InOut_Packets[In_Packet_Num][i] = 0x00;
            }

            // Put Error state sign
            InOut_Packets[In_Packet_Num][0] = 0x00;

            // Ready to send IN packet
            In_Packet_Ready = 1;

    #if (DEBUG_MODE == 1)                
            Store_Debug_Message ("ErrErSector", DeviceCurSector);
    #endif                

            // Ready to receive OUT packet
            Out_Packet_Ready = 0;
            // Turn off LED1 after error erasing device
            SetLed (0);
            // Deactivate SPI0 Power
            SetSPI0PowerOff ();

            State = ST_IDLE;
        }
    }
}

// Send whole device to host in Sectors (command stage)
static void StateSPI0ReadDev (void)
{
    if (!In_Packet_Ready)
    {
        U8 i;
        // Turn on LED1 while reading whole device
        SetLed (1);
        // Activate SPI0 Power
        SetSPI0PowerOn ();

        // Store the required bytes number
        DeviceStartAddr = Get_U32(&InOut_Packets[Out_Packet_Num][8]);
        DeviceEndAddr = Get_U32(&InOut_Packets[Out_Packet_Num][4]);
        // If flash capacity more than 16Mb
        if (DeviceEndAddr > 0x1000000)
        {
            Spi_4xAddress = 1;
        }
        else
        {
            Spi_4xAddress = 0;
        }
        // Copy Spi Commands Set
        MemCopy (&InOut_Packets[Out_Packet_Num][19], (U8 *) &Cmd_Spi, sizeof (Cmd_Spi));
        // Done processing Out_Packet
        Out_Packet_Ready = 0;

        // Pointer to the last packet in Buffer
        In_Packet_Num = BUFFER_PACKET_NUM - 1;
        Out_Packet_Num = 0x00;
        
        // Prepare empty 0x00 packet for answer on ReadDev command
        for (i = 0; i < IN_EP1_PACKET_SIZE; i++)
        {
            InOut_Packets[In_Packet_Num][i] = 0x00;
        }
        // Ready to send IN packet
        In_Packet_Ready = 1;

        // Debug messages
#if (DEBUG_MODE == 1)        
        Store_Debug_Message ("RdCapByte", DeviceEndAddr);
#endif        

        // Reset the block counter
        // There are Device Capacity calculated in 64-bytes sectors
        DeviceCurSector = DeviceStartAddr / IN_EP1_PACKET_SIZE;
        DeviceMaxSector = DeviceEndAddr / IN_EP1_PACKET_SIZE;

        // Clear SPI0 Buffer Packets Num
        Spi_Buffer_Packets_Num = 0;
        
        State = ST_SPI0_TX_SECTORS;
    }
}

// WORKING TWO BUFFERS UP TO 256 BYTES - TWO ISR READS
// Send device flash to host in sectors (data/response stage)
// static void StateSPI0TxSectors (void)
// {
//     if (!In_Packet_Ready)
//     {
//         if (DeviceCurSector < DeviceMaxSector)
//         {
//             // If this is last packet in half of buffer
//             if (((In_Packet_Num) & 3) == 3)
//             {
//                 // If current half of buffer is under read
//                 if ((In_Packets_State[(~In_Packet_Num & 4) >> 2] == EMPTY) && (Spi_Isr_Mode == SPI_ISR_IDLE))
//                 {
// //                    if (In_Packets_State[(In_Packet_Num & 4) >> 2] == RDFD) 
// //                    {    
// //                            In_Packets_State[(In_Packet_Num & 4) >> 2] = EMPTY;
// //                    }

//                     // Calculate the flash address based on current device's sector
//                     U32 address = DeviceCurSector * IN_EP1_PACKET_SIZE;

//                     In_Packets_State[(In_Packet_Num & 4) >> 2] = EMPTY;

//                     // Buffer captured for read from device
//                     In_Packets_State[(~In_Packet_Num & 4) >> 2] = RDFD;
//                     
//                     // Read from flash current device's sector
//                     ReadSPI0Sector (address, &In_Packets[~In_Packet_Num & 4][0], 4 * IN_EP1_PACKET_SIZE);
// //                    while (Spi_Buffer_Packets_Num != 0);
//                 }
//             }
//             if (Spi_Buffer_Packets_Num > 0)
//             {
//                 // Decrement number of packets which are ready to transmit
//                 Spi_Buffer_Packets_Num--;
//                 DeviceCurSector++;
//                 In_Packet_Num++;
//                 In_Packet_Num &= (BUFFER_PACKET_NUM - 1);
//                 // Ready to send IN packet
//                 In_Packet_Ready = 1;
//             }
//         }
//         else
//         {
//             // Debug messages
// #if (DEBUG_MODE == 1)            
//             Store_Debug_Message ("EndTxSector", DeviceCurSector);
// #endif          
//             // Pointer to the first IN buffer
//             In_Packet_Num = 0;
//             In_Packets_State[0] = EMPTY;
//             In_Packets_State[1] = EMPTY;

//             // Turn off LED1 after finished reading device
//             SetLed (0);
//             // Deactivate SPI0 Power
//             SetSPI0PowerOff ();

//             State = ST_IDLE;
//         }
//     }
// }

// Send device flash to host in sectors (data/response stage)
static void StateSPI0TxSectors (void)
{
    if (!In_Packet_Ready)
    {
        if (DeviceCurSector < DeviceMaxSector)
        {
            if (((In_Packet_Num & (BUFFER_PACKET_NUM - 1)) == (BUFFER_PACKET_NUM - 1)) && (Spi_Isr_Mode == SPI_ISR_IDLE))
            {
                // Calculate the flash address based on current device's sector
                U32 address = DeviceCurSector * IN_EP1_PACKET_SIZE;

                // Read from flash current device's sector
                ReadSPI0Sector (address, &InOut_Packets[0][0], BUFFER_PACKET_NUM * BUFFER_PACKET_SIZE);
                // Wait until at least 1 packets read
//                while (Spi_Buffer_Packets_Num == 0);
            }
            
            // Copy from sector buffer to packet 
//            MemCopy (&Buffer_Sector[DeviceCurSector & 7][0], In_Packet, IN_EP1_PACKET_SIZE);
            
            if (Spi_Buffer_Packets_Num > 0)
            {
                // Decrement number of packets which are ready to transmit
                Spi_Buffer_Packets_Num--;
                DeviceCurSector++;
                In_Packet_Num++;
                In_Packet_Num &= (BUFFER_PACKET_NUM - 1);
                // Ready to send IN packet
                In_Packet_Ready = 1;
            }
        }
        else
        {
            // Debug messages
#if (DEBUG_MODE == 1)            
            Store_Debug_Message ("EndTxSector", DeviceCurSector);
#endif          
            // Pointer to the first IN buffer
            In_Packet_Num = BUFFER_PACKET_NUM - 1;
            Out_Packet_Num = 0x00;

            // Turn off LED1 after finished reading device
            SetLed (0);
            // Deactivate SPI0 Power
            SetSPI0PowerOff ();

            State = ST_IDLE;
        }
    }
}

// Receive whole device from host in Sectors (command stage)
static void StateSPI0WriteDev (void)
{
    if (!In_Packet_Ready)
    {
        U8 i;
        // Turn on LED1 while writting whole device
        SetLed (1);
        // Activate SPI0 Power
        SetSPI0PowerOn ();

        // Store the required bytes number
        DeviceStartAddr = Get_U32(&InOut_Packets[Out_Packet_Num][8]);
        DeviceEndAddr = Get_U32(&InOut_Packets[Out_Packet_Num][4]);
        // If flash capacity more than 16Mb
        if (DeviceEndAddr > 0x1000000)
        {
            Spi_4xAddress = 1;
        }
        else
        {
            Spi_4xAddress = 0;
        }       
        // Copy Spi Commands Set
        MemCopy (&InOut_Packets[Out_Packet_Num][19], (U8 *) &Cmd_Spi, sizeof (Cmd_Spi));
        // Done processing Out_Packet
        Out_Packet_Ready = 0;

        // Pointer to the first packet in Buffer
        In_Packet_Num = BUFFER_PACKET_NUM - 1;
        Out_Packet_Num = 0x00;

        // Prepare empty 0x00 packet for answer on WriteDev command
        for (i = 0; i < IN_EP1_PACKET_SIZE; i++)
        {
            InOut_Packets[In_Packet_Num][i] = 0x00;
        }
        // Ready to send IN packet
        In_Packet_Ready = 1;

        // Debug messages
#if (DEBUG_MODE == 1)        
        Store_Debug_Message ("WrCapByte", DeviceEndAddr);
#endif        

        // There are Device Capacity calculated in 64-bytes sectors
        DeviceCurSector = DeviceStartAddr / OUT_EP2_PACKET_SIZE;
        DeviceMaxSector = DeviceEndAddr / OUT_EP2_PACKET_SIZE;

        // Clear SPI0 Buffer Packets Num
        Spi_Buffer_Packets_Num = 0;
        // Set WriteOk status to OK state
        StatusSPIWriteOk = 1;

        State = ST_SPI0_RX_SECTORS;
    }
}

// Receive device flash from host in sectors (data/response stage)
static void StateSPI0RxSectors (void)
{
    if (DeviceCurSector < DeviceMaxSector)
    {
        if (Out_Packet_Ready)
        {
//            if (Out_Packets_State[(Out_Packet_Num & 4) >> 2] == EMPTY)
//            {
                if (((Out_Packet_Num + 1) & 3) == 0)
                {
                    if (Out_Packets_State[(~Out_Packet_Num & 4) >> 2] == WRTD) 
                    {    
                        if (Spi_Isr_Mode == SPI_ISR_IDLE)
                        {
                            Out_Packets_State[(~Out_Packet_Num & 4) >> 2] = EMPTY;
                        }
                    }
                    else
                    {
                        // Calculate the flash address based on current device's sector
                        U32 address = (DeviceCurSector - 3) * OUT_EP2_PACKET_SIZE;
                        // Buffer captured to write to device
                        Out_Packets_State[(Out_Packet_Num & 4) >> 2] = WRTD;

                        if (StatusSPIWriteOk)
                        {
                            // Write to flash current device's sector
                            if (!WriteSPI0Sector (address, &InOut_Packets[Out_Packet_Num & 4][0], 4 * OUT_EP2_PACKET_SIZE))
                            {
                                // Error write current sector
                                StatusSPIWriteOk = 0;

                                // Debug messages
                    #if (DEBUG_MODE == 1)                
                                Store_Debug_Message ("ErrWrSector", DeviceCurSector);
                    #endif                
                            }
                        }
                        DeviceCurSector++;
                        Out_Packet_Num++;
                        Out_Packet_Num &= 7;
                        
                        // Ready to receive OUT packet
                        Out_Packet_Ready = 0;
                    }
                } 
                else
                {
                    DeviceCurSector++;
                    Out_Packet_Num++;
                    Out_Packet_Num &= 7;
                    
                    // Ready to receive OUT packet
                    Out_Packet_Ready = 0;
                }
//            }
        }
    }
    else
    {
        if (WaitSPI0ChipError ())
        {
            // Error write current sector
            StatusSPIWriteOk = 0;

            // Debug messages
#if (DEBUG_MODE == 1)                
            Store_Debug_Message ("ErrWrSector", DeviceCurSector);
#endif                
        }

    // Debug messages
#if (DEBUG_MODE == 1)        
        Store_Debug_Message ("EndRxSector", DeviceCurSector);
#endif        

        // Pointer to the first OUT buffer
        Out_Packet_Num = 0x00;
        In_Packet_Num = BUFFER_PACKET_NUM - 1;
        Out_Packets_State[0] = EMPTY;
        Out_Packets_State[1] = EMPTY;
        // Turn off LED1 after finished writting device
        SetLed (0);
        // Deactivate SPI0 Power
        SetSPI0PowerOff ();

        State = ST_IDLE;
    }
}


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
// tarask1n
      case ST_SPI0_RX_SECTORS:
         StateSPI0RxSectors ();
         EA_Save = EA;
         EA = 0;
//       Send_Packet_Foreground ();
         Receive_Packet_Foreground ();
         EA = EA_Save;
         break;			

      case ST_SPI0_TX_SECTORS:
         StateSPI0TxSectors ();
         break;			

      case ST_SPI0_ER_SECTORS:
         StateSPI0EraseSectors ();
         break;			

      case ST_SPI0_GET_DEV_ID:
         StateSPI0GetDevId ();
         break;			

      case ST_SPI0_READ_DEV:
         StateSPI0ReadDev ();
         break;			

      case ST_SPI0_WRITE_DEV:
         StateSPI0WriteDev ();
         break;			

      case ST_SPI0_ERASE_DEV:
         StateSPI0EraseDev ();
         break;			
// tarask1n

      case ST_IDLE:
         StateIdle ();
         break;

//       case ST_SET_FLASH_KEY:
//          StateSetFlashKey ();
//          break;

//       case ST_TX_PAGE_INFO:
//          StateTxPageInfo ();
//          break;

//       case ST_READ_PAGE:
//          StateReadPage ();
//          break;

//       case ST_TX_BLOCK:
//          StateTxBlock ();
//          break;

//       case ST_WRITE_PAGE:
//          StateWritePage ();
//          break;

//       case ST_RX_BLOCK:
//          StateRxBlock ();
//          break;

//       case ST_TX_SUCCESS:
//          StateTxSuccess ();
//          break;

//       case ST_TX_INVALID:
//          StateTxInvalid ();
//          break;
   }
}

//-----------------------------------------------------------------------------
// Main Routine
//-----------------------------------------------------------------------------

void main (void)
{
//    U8 EA_Save;

#if (DEBUG_MODE == 1)    
    Debug_Init ();                      // Initialize Debug  
    Store_Debug_Message ("Start Main", 2015);
#endif    

    System_Init ();                     // Initialize Sysclk, Port IO
    USB0_Init ();                       // Initialize USB0

    EA = 1;                             // Enable global interrupts

    // Turn off LED
    SetLed (0);

    while (1)
    {
       StateMachine ();

//       EA_Save = EA;
//       EA = 0;
//       Send_Packet_Foreground ();
//       Receive_Packet_Foreground ();
//       EA = EA_Save;
    }
}

//-----------------------------------------------------------------------------
// F3xx_I2C0.c
//-----------------------------------------------------------------------------
//
// Program Description:
//
// I2C0 utilities for writing/erasing I2C0 pages
//
// Target:         C8051F320/1,
//                 C8051F326/7,
//                 C8051F34x,
//                 C8051F38x
//
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
// Project Name:   USB0_Bulk
//
// Release 0.1
//    -14 JUN 2015
//

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "c8051f3xx.h"
#include "F3XX_USB0_Bulk.h"
#include "F3xx_I2C0.h"

//-----------------------------------------------------------------------------
// Static Global Variables
//-----------------------------------------------------------------------------

//I2c_Commands_Set Cmd_I2c;
// SEGMENT_VARIABLE (Cmd_I2c, I2c_Commands_Set, SEG_XDATA);

U8  I2c_Isr_Mode = I2C_ISR_IDLE;
U8* I2c_Buffer;
U16 I2c_Data_Size;
U8  I2c_Buffer_Packets_Num = 0;
// static U8 I2C0Voltage = SET_33V;
U8  I2c_4xAddress = 0;              // For flashes more then 16Mb capacity must be equal to 1, for all others 0

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

SBIT (I2C0_SCK, SFR_P0, 0);		// I2C0 SCK
SBIT (I2C0_MISO, SFR_P0, 1);	// I2C0 SO
SBIT (I2C0_MOSI, SFR_P0, 2); 	// I2C0 SI
SBIT (I2C0_HOLD, SFR_P0, 3);	// I2C0 #HOLD
SBIT (I2C0_WP, SFR_P0, 4);		// I2C0 #WP
SBIT (I2C0_CE, SFR_P0, 5);		// I2C0 #CE

SBIT (I2C1_SCK, SFR_P2, 0);		// I2C1 SCK
SBIT (I2C1_MISO, SFR_P2, 1);    // I2C1 SO
SBIT (I2C1_MOSI, SFR_P2, 2); 	// I2C1 SI
SBIT (I2C1_HOLD, SFR_P2, 3);	// I2C1 #HOLD
SBIT (I2C1_WP, SFR_P2, 4);		// I2C1 #WP
SBIT (I2C1_CE, SFR_P2, 5);		// I2C1 #CE

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SetI2C0ChipEnable
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U8 enable : State of I2C0 CE pin  -  1 means CE is active
void SetI2C0ChipEnable(U8 enable)
{
    if (enable)
    {
     I2C0_CE = 0;       // Enable Chip select
//     Delay_us(10);
    }
    else
    {
     I2C0_CE = 1;       // Disable Chip select
    }
}


// //-----------------------------------------------------------------------------
// // I2C_ISR
// //-----------------------------------------------------------------------------
// //
// // Handles all error checks and single-byte writes.
// //
// INTERRUPT (I2c_ISR, INTERRUPT_I2C0)
// {
//     if (WCOL == 1)
//     {
//         // Write collision occurred
//         WCOL = 0;                        // Clear the write collision flag

//     //      Error_Flag = 1;
//     }
//     else if (I2c_Data_Size > 0)
//     {
//         // When the Master enters the ISR, the I2CF flag should be set from
//         // sending the Command byte.  This ISR handles the remaining steps of the
//         // I2C transfer process.
// //        VARIABLE_SEGMENT_POINTER (pAddr, U8, SEG_XDATA) = (U8 SEG_XDATA *) I2c_Buffer_Offset;
//         switch (I2c_Isr_Mode)
//         {
//             case I2C_ISR_READ:
// //                *pAddr = I2C0DAT;
//                 I2c_Buffer[0] = I2C0DAT;
//                 I2C0DAT = 0xFF;        // Send a dummy byte so the Slave can
//                                        // send the data
//                 I2c_Buffer++;
//                 I2c_Data_Size--;
//                 if ((I2c_Data_Size & (0x40 - 1)) == 0)
//                 {
//                     I2c_Buffer_Packets_Num++;
//                 }
//                 break;

//             case I2C_ISR_WRITE:
// //                I2C0DAT = pAddr[0];
//                 I2C0DAT = I2c_Buffer[0];
//                 I2c_Buffer++;
//                 I2c_Data_Size--;
//                 if ((I2c_Data_Size & 0x3F) == 0)
//                 {
//                     I2c_Buffer_Packets_Num++;
//                 }
//                 break;
//         }
//     }
//     else
//     {      
//         I2c_Isr_Mode = I2C_ISR_IDLE;
//         SetI2C0ChipEnable(0);
//     }
//     I2CF = 0;                        // Clear the I2CF flag
// }

//-----------------------------------------------------------------------------
// TransferI2C0Data
//-----------------------------------------------------------------------------
//
// Return Value : 1 byte - received data
// Parameters   : 
//                1) 1 byte - data to transmit
//                   
U8 TransferI2C0Data (U8 WriteData)
{
// //    U8 ReadData;
// //    while (I2C0CFG & mI2C_BSY);     // Wait until the I2C is free, in case it's already busy
//     I2C0DAT = WriteData;
//     while (!I2CF);                    // Wait until the I2C is free, in case it's already busy
// //    ReadData = I2C0DAT;
//     I2CF = 0;
//     return I2C0DAT;                   // Return Read I2C Data
}

//-----------------------------------------------------------------------------
// WriteI2C0Data
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) 1 byte - data to transmit
//                   
void WriteI2C0Data (U8 WriteData)
{
// //    while (I2C0CFG & mI2C_BSY);         // Wait until the I2C is free, in case it's already busy
//    I2C0DAT = WriteData;
}

//-----------------------------------------------------------------------------
// SetI2C0PowerOn
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U8 voltage : Apply to the I2C0 voltage
//                   SET_50V or SET_33V
void SetI2C0PowerOn(void)
{
// //     switch (I2C0Voltage)
// //     {
// //         case SET_50V:
// //         // Apply 5.0V to I2C0
// //      
// //             break;
// //         case SET_33V:
// //         // Apply 3.3V to I2C0

// //             break;
// //         default:
// //         // Error
// //             break;
// //     }
// //    P0MDOUT   = mI2C0_SCK | mI2C0_HOLD | mI2C0_WP | mI2C0_CE;     // Make SCK, HOLD, WP and CE push-pull
// //    P0MDOUT   = 0x00;                                               // P0 push-pull off
//     P0SKIP    = ~(mI2C0_SCK | mI2C0_MISO | mI2C0_MOSI);             // Skip all except mI2C0_SCK, mI2C0_MISO and mI2C0_MOSI
// //    P0        = 0xFF;
// //    P1MDOUT   = mVDD_50V | mVDD_33V | mVSS | mVSS_93;               // Make VDD_50V, VDD_33V, VSS, VSS_93 push-pull
// //    P1SKIP    = 0xFF;                                               // Skip all P1
// //    P1        = 0xFF;
// //    P2MDOUT   = 0x00;                                               // P2 push-pull off 
//     P2SKIP    = 0xFF;                                               // Skip all P2
// //    P2        = 0xFF;

// //    I2CF = 0;
// //    I2C0CFG   = 0x70;                   // Enable the I2C as a Master at mode 3
//                                          // CKPHA = '1', CKPOL = '1'
//     I2C0CFG   = 0x40;                   // Enable the I2C as a Master at mode 0
//                                         // CKPHA = '0', CKPOL = '0'
//     I2C0CN    = 0x03;                   // 3-wire Single Master, I2C enabled
// // I2C clock frequency equation from the datasheet
// //    I2C0CKR   = (SYSCLK/(2*I2C_CLOCK))-1;
//     I2C0CKR   = 5;      //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//     EI2C0     = 0;                      // Disable I2C interrupts
// //    EI2C0     = 1;                      // Enable I2C interrupts


//     XBR0      = 0x02;                   // Enable the I2C on the XBAR
//     XBR1      = 0x40;                   // Enable the XBAR and weak pull-ups
// //    XBR2      = 0x00;                   // Disabled after reset

//     // Power On
//     VSS_93 = 0;     // deactivate
//     VSS = 1;        // activate
//     VDD_50V = 1;    // deactivate
//     VDD_33V = 0;    // activate
//     I2C0_WP = 1;
//     I2C0_HOLD = 1;
// //    I2C0_CE = 0;    // Enable Chip select
//     Delay_ms (1);
}
//-----------------------------------------------------------------------------
// SetI2C0PowerOff
//-----------------------------------------------------------------------------
//
// Return Value : None
void SetI2C0PowerOff(void)
{
// // I2C0 Off
// //    P1MDOUT   = mLED1;                   // P1.4 is push-pull
// //    P1SKIP    = mLED1;                   // P1.4 skipped
// //    XBR0      = 0x00;                   // Disable the I2C on the XBAR
// //    XBR1      = 0x40;                   // Enable the crossbar   
// //    I2C0_CE = 1;    // Disable Chip select

//     P0        = 0xFF;
// //    P2        = 0xFF;

//     // Power On
//     VSS_93 = 0;     // deactivate
//     VSS = 0;        // deactivate
//     VDD_50V = 1;    // deactivate
//     VDD_33V = 1;    // deactivate  

//     XBR0      = 0x00;                   // Disable the I2C on the XBAR
// //    XBR1      = 0x40;                   // Enable the XBAR and weak pull-ups

}

//-----------------------------------------------------------------------------
// EraseI2C0Device
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U16 pageAddress : the address of the I2C0 page to erase
//
// Erases the specified I2C0 page
//
//-----------------------------------------------------------------------------
void EraseI2C0Device(void)
{
//     // Disable I2C0 interrupts
//     EI2C0 = 0;

//     // Select I2C0 Chip Enable
// 	SetI2C0ChipEnable (1);

//     // Send I2C command Write Enable
// 	TransferI2C0Data (Cmd_I2c.Write_Enable);

//     // Select I2C0 Chip Disable
// 	SetI2C0ChipEnable (0);

//     Delay_us (10);
//     
//     // Select I2C0 Chip Enable
// 	SetI2C0ChipEnable (1);

//     // Send I2C command Bulk Erase
// 	TransferI2C0Data (Cmd_I2c.Bulk_Erase);

//     // Select I2C0 Chip Disable
// 	SetI2C0ChipEnable (0);
}

//-----------------------------------------------------------------------------
// WriteI2C0Sector
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U16 address : the address of the I2C0 page to write
//                2) U8 buffer[] : a buffer to write to the I2C0 page starting
//                                 at the specified page address
//                3) U16 size : the number of bytes in the buffer to write
//
// Write the specified number of bytes in the buffer to the specified address.
//
//-----------------------------------------------------------------------------
U8 WriteI2C0Sector(U32 address, U8* buffer, U16 data_size)
{
// //    U16 i;

//     // Wait while Isr is processing now
//     while (I2c_Isr_Mode != I2C_ISR_IDLE);

//     // Disable I2C0 interrupts
//     EI2C0 = 0;

//     // Waiting for finishing write operation
//     while (GetI2C0StatusBusy ()); 

//     // Assign mode and data
//     I2c_Isr_Mode = I2C_ISR_WRITE;
//     I2c_Buffer = buffer;
//     I2c_Data_Size = data_size;

//     // Select I2C0 Chip Enable
// 	SetI2C0ChipEnable (1);

//     // Send I2C command Write Enable
// 	TransferI2C0Data (Cmd_I2c.Write_Enable);

//     // Select I2C0 Chip Disable
// 	SetI2C0ChipEnable (0);

//     Delay_us (10);

//     // Select I2C0 Chip Enable
// 	SetI2C0ChipEnable (1);

//     // If flash capacity more than 16Mb
//     if (I2c_4xAddress)
//     {
//         // Send I2C command WRITE and 4 bytes address
//         TransferI2C0Data (Cmd_I2c.Write | 0x10);
//         TransferI2C0Data (((U8[]) &address)[0]);
//     }
//     else
//     {
//         // Send I2C command WRITE and 3 bytes address
//         TransferI2C0Data (Cmd_I2c.Write);
//     }    
//     TransferI2C0Data (((U8[]) &address)[1]);
//     TransferI2C0Data (((U8[]) &address)[2]);

//     // Enable interrupts
//     EI2C0 = 1;
//     
//     // Write last byte of address and activate I2C ISR
//     WriteI2C0Data (((U8[]) &address)[3]);

// //  TransferI2C0Data ((address >> 16) & 0xFF);
// // 	TransferI2C0Data ((address >> 8) & 0xFF);
// // 	TransferI2C0Data (address & 0xFF);
//     // Write to I2C SIZE bytes from the buffer
// //     for (i = 0; i < data_size; i++)
// //     {
// //         TransferI2C0Data(buffer[i]);
// //     }

//     // Select I2C0 Chip Disable
// //    SetI2C0ChipEnable (0);

// //    Delay_us (10);

//     return 1;
}

//-----------------------------------------------------------------------------
// ReadI2C0Sector
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U16 address : the address of the I2C0 page to read
//                2) U8 buffer[] : a buffer to read from the I2C0 page starting
//                                 at the specified page address
//                3) U16 size : the number of bytes to read into the buffer
//
// Read the specified number of bytes from I2C0 and store in the buffer.
//
//-----------------------------------------------------------------------------
void ReadI2C0Sector (U32 address, U8* buffer, U16 data_size)
{
// //    U8 EA_Save = EA;
// //    U16 i;

//     // Wait while Isr is processing now
//     while (I2c_Isr_Mode != I2C_ISR_IDLE);

//     // Disable I2C0 interrupts
//     EI2C0 = 0;
//     // Assign mode and data
//     I2c_Isr_Mode = I2C_ISR_READ;
//     I2c_Buffer = buffer;
//     I2c_Data_Size = data_size;

//     // Select I2C0 Chip Enable
// 	SetI2C0ChipEnable (1);

//     // If flash capacity more than 16Mb
//     if (I2c_4xAddress)
//     {
//         // Send I2C command READ and 4 bytes address
//         TransferI2C0Data (Cmd_I2c.Read | 0x10);
//         TransferI2C0Data (((U8[]) &address)[0]);
//     }   
//     else
//     {
//         // Send I2C command READ and 3 bytes address
//         TransferI2C0Data (Cmd_I2c.Read);
//     }
//     TransferI2C0Data (((U8[]) &address)[1]);
//     TransferI2C0Data (((U8[]) &address)[2]);
//     TransferI2C0Data (((U8[]) &address)[3]);

//     // Enable interrupts
//     EI2C0 = 1;
//     
//     // Write dummy data to start read by ISR
//     WriteI2C0Data (0xFF);

// //  TransferI2C0Data ((address >> 16) & 0xFF);
// // 	TransferI2C0Data ((address >> 8) & 0xFF);
// // 	TransferI2C0Data (address & 0xFF);
//     // Read from I2C SIZE bytes and store it in the buffer
// //     for (i = 0; i < size; i++)
// //     {
// //         buffer[i] = TransferI2C0Data (0);
// //     }

// //     // Select I2C0 Chip Disable
// //     SetI2C0ChipEnable (0);
}

//-----------------------------------------------------------------------------
// GetI2C0StatusBusy
//-----------------------------------------------------------------------------
//
// Return Value : U8  BUSY bit state
// Parameters   : None
//
// Read the STATUS register S0 from I2C0 and return the BUSY bit. 
//
//-----------------------------------------------------------------------------
U8 GetI2C0StatusBusy (void)
{
    U8 Status;
    // Select I2C0 Chip Enable
	SetI2C0ChipEnable (1);

//     // Send I2C command READ and 3 bytes address
// 	TransferI2C0Data (Cmd_I2c.Read_Status_Register);

//     // Read from I2C STATUS register
//     Status = (TransferI2C0Data (0) & 0x01);

    // Select I2C0 Chip Disable
    SetI2C0ChipEnable (0);
    
    return Status;
}

//-----------------------------------------------------------------------------
// F3xx_SPI0.c
//-----------------------------------------------------------------------------
//
// Program Description:
//
// SPI utilities for writing/erasing SPI0 pages
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
#include "F3xx_SPI0.h"

//-----------------------------------------------------------------------------
// Static Global Variables
//-----------------------------------------------------------------------------

//Spi_Commands_Set Cmd_Spi;
SEGMENT_VARIABLE (Cmd_Spi, Spi_Commands_Set, SEG_XDATA);

U8  Spi_Isr_Mode = SPI_ISR_IDLE;
U8* Spi_Buffer;
U16 Spi_Data_Size;
U8  Spi_Buffer_Packets_Num = 0;
// static U8 SPI0Voltage = SET_33V;
U8  Spi_4xAddress = 0;              // For flashes more then 16Mb capacity must be equal to 1, for all others 0

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

SBIT (SPI0_SCK, SFR_P0, 0);		// SPI0 SCK
SBIT (SPI0_MISO, SFR_P0, 1);	// SPI0 SO
SBIT (SPI0_MOSI, SFR_P0, 2); 	// SPI0 SI
SBIT (SPI0_HOLD, SFR_P0, 3);	// SPI0 #HOLD
SBIT (SPI0_WP, SFR_P0, 4);		// SPI0 #WP
SBIT (SPI0_CE, SFR_P0, 5);		// SPI0 #CE

SBIT (SPI1_SCK, SFR_P2, 0);		// SPI1 SCK
SBIT (SPI1_MISO, SFR_P2, 1);    // SPI1 SO
SBIT (SPI1_MOSI, SFR_P2, 2); 	// SPI1 SI
SBIT (SPI1_HOLD, SFR_P2, 3);	// SPI1 #HOLD
SBIT (SPI1_WP, SFR_P2, 4);		// SPI1 #WP
SBIT (SPI1_CE, SFR_P2, 5);		// SPI1 #CE

// SBIT (VDD_50V, SFR_P1, 0);		// VDD 5.0 V    0 - activate
// SBIT (VDD_33V, SFR_P1, 1);		// VDD 3.3 V    0 - activate
// SBIT (VSS, SFR_P1, 2);	    	// VSS for 24/25 series  1 - activate
// SBIT (VSS_93, SFR_P1, 3);		// VSS for 93 series     1 - activate

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SetSPI0ChipEnable
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U8 enable : State of SPI0 CE pin  -  1 means CE is active
void SetSPI0ChipEnable(U8 enable)
{
    if (enable)
    {
        SPI0_CE = 0;       // Enable Chip select
//         Delay_us(10);
    }
    else
    {
        SPI0_CE = 1;       // Disable Chip select
    }
}


//-----------------------------------------------------------------------------
// SPI_ISR
//-----------------------------------------------------------------------------
//
// Handles all error checks and single-byte writes.
//
INTERRUPT (Spi_ISR, INTERRUPT_SPI0)
{
    if (WCOL == 1)
    {
        // Write collision occurred
        WCOL = 0;                        // Clear the write collision flag

    //      Error_Flag = 1;
    }
    else if (Spi_Data_Size > 0)
    {
        // When the Master enters the ISR, the SPIF flag should be set from
        // sending the Command byte.  This ISR handles the remaining steps of the
        // SPI transfer process.
//        VARIABLE_SEGMENT_POINTER (pAddr, U8, SEG_XDATA) = (U8 SEG_XDATA *) Spi_Buffer_Offset;
        switch (Spi_Isr_Mode)
        {
            case SPI_ISR_READ:
//                *pAddr = SPI0DAT;
                Spi_Buffer[0] = SPI0DAT;
                SPI0DAT = 0xFF;        // Send a dummy byte so the Slave can
                                       // send the data
                Spi_Buffer++;
                Spi_Data_Size--;
                if ((Spi_Data_Size & (0x40 - 1)) == 0)
                {
                    Spi_Buffer_Packets_Num++;
                }
                break;

            case SPI_ISR_WRITE:
//                SPI0DAT = pAddr[0];
                SPI0DAT = Spi_Buffer[0];
                Spi_Buffer++;
                Spi_Data_Size--;
                if ((Spi_Data_Size & 0x3F) == 0)
                {
                    Spi_Buffer_Packets_Num++;
                }
                break;
        }
    }
    else
    {      
        if (Spi_Isr_Mode != SPI_ISR_IDLE)
        {
            Spi_Isr_Mode = SPI_ISR_IDLE;
            SetSPI0ChipEnable(0);
        }
    }
    SPIF = 0;                        // Clear the SPIF flag
}

//-----------------------------------------------------------------------------
// TransferSPI0Data
//-----------------------------------------------------------------------------
//
// Return Value : 1 byte - received data
// Parameters   : 
//                1) 1 byte - data to transmit
//                   
U8 TransferSPI0Data (U8 WriteData)
{
//    U8 ReadData;
//    SPIF = 0;
//    while (SPI0CFG & mSPI_BSY);     // Wait until the SPI is free, in case it's already busy
    SPI0DAT = WriteData;
    while (!SPIF);                    // Wait until the SPI is free, in case it's already busy
//    ReadData = SPI0DAT;
    SPIF = 0;
    return SPI0DAT;                   // Return Read SPI Data
}

//-----------------------------------------------------------------------------
// WriteSPI0Data
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) 1 byte - data to transmit
//                   
void WriteSPI0Data (U8 WriteData)
{
//    while (SPI0CFG & mSPI_BSY);         // Wait until the SPI is free, in case it's already busy
    SPI0DAT = WriteData;
}

//-----------------------------------------------------------------------------
// SetSPI0PowerOn
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U8 voltage : Apply to the SPI0 voltage
//                   SET_50V or SET_33V
void SetSPI0PowerOn(void)
{
    XBR1      &= ~0x40;                   // Disable the XBAR and weak pull-ups
//     switch (SPI0Voltage)
//     {
//         case SET_50V:
//         // Apply 5.0V to SPI0
//      
//             break;
//         case SET_33V:
//         // Apply 3.3V to SPI0

//             break;
//         default:
//         // Error
//             break;
//     }
//    P0MDOUT   = mSPI0_SCK | mSPI0_HOLD | mSPI0_WP | mSPI0_CE;     // Make SCK, HOLD, WP and CE push-pull
//    P0MDOUT   = 0x00;                                               // P0 push-pull off
    P0SKIP    = ~(mSPI0_SCK | mSPI0_MISO | mSPI0_MOSI);             // Skip all except mSPI0_SCK, mSPI0_MISO and mSPI0_MOSI
//    P0        = 0xFF;
//    P1MDOUT   = mVDD_50V | mVDD_33V | mVSS | mVSS_93;               // Make VDD_50V, VDD_33V, VSS, VSS_93 push-pull
//    P1SKIP    = 0xFF;                                               // Skip all P1
//    P1        = 0xFF;
//    P2MDOUT   = 0x00;                                               // P2 push-pull off 
    P2SKIP    = 0xFF;                                               // Skip all P2
//    P2        = 0xFF;

//    SPI0CFG   = 0x70;                   // Enable the SPI as a Master at mode 3
                                         // CKPHA = '1', CKPOL = '1'
    SPI0CFG   = 0x40;                   // Enable the SPI as a Master at mode 0
                                        // CKPHA = '0', CKPOL = '0'
//    SPI0CN    = 0x03;                   // 3-wire Single Master, SPI enabled
    SPI0CN    = 0x01;                   // 3-wire Single Master, SPI enabled (disassembled)
// SPI clock frequency equation from the datasheet
    SPI0CKR   = (SYSCLK/(2*SPI_CLOCK))-1;
//    SPI0CKR   = 5;      //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    ESPI0     = 0;                      // Disable SPI interrupts
//    ESPI0     = 1;                      // Enable SPI interrupts
    SPIF = 0;


    XBR0      = 0x02;                   // Enable the SPI on the XBAR
    XBR1      |= 0x40;                   // Enable the XBAR and weak pull-ups
//    XBR2      = 0x00;                   // Disabled after reset

    // Power On
    PowerOn33V ();
    SPI0_WP = 1;
    SPI0_HOLD = 1;
//    SPI0_CE = 0;    // Enable Chip select
    Delay_ms (20);
}
//-----------------------------------------------------------------------------
// SetSPI0PowerOff
//-----------------------------------------------------------------------------
//
// Return Value : None
void SetSPI0PowerOff(void)
{
    XBR1      &= ~0x40;                   // Disable the XBAR and weak pull-ups

// SPI0 Off
//    P1MDOUT   = mLED1;                   // P1.4 is push-pull
//    P1SKIP    = mLED1;                   // P1.4 skipped
//    XBR0      = 0x00;                   // Disable the SPI on the XBAR
//    XBR1      = 0x40;                   // Enable the crossbar   
//    SPI0_CE = 1;    // Disable Chip select

    P0        = 0xFF;
//    P2        = 0xFF;

    // Power Off
    PowerOff ();
    XBR0      = 0x00;                   // Disable the SPI on the XBAR
    XBR1      |= 0x40;                   // Enable the XBAR and weak pull-ups

}

//-----------------------------------------------------------------------------
// EraseSPI0Device
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U16 pageAddress : the address of the SPI0 page to erase
//
// Erases the specified SPI0 page
//
//-----------------------------------------------------------------------------
void EraseSPI0Device(void)
{
    // Disable SPI0 interrupts
    ESPI0 = 0;
    Spi_Isr_Mode = SPI_ISR_IDLE;

//     // Select SPI0 Chip Enable
// 	SetSPI0ChipEnable (1);

//     // Send SPI command Write Enable
// 	TransferSPI0Data (Cmd_Spi.Write_Enable);

//     // Select SPI0 Chip Disable
// 	SetSPI0ChipEnable (0);

//     Delay_us (10);

//     // Select SPI0 Chip Enable
// 	SetSPI0ChipEnable (1);

//     // Send SPI command Write 
// 	TransferSPI0Data (Cmd_Spi.Write_Status_Register);
// 	TransferSPI0Data (0);
// 	TransferSPI0Data (0);

//     // Select SPI0 Chip Disable
// 	SetSPI0ChipEnable (0);

//     Delay_us (10);
    
    
    // Select SPI0 Chip Enable
	SetSPI0ChipEnable (1);

    // Send SPI command Write Enable
	TransferSPI0Data (Cmd_Spi.Write_Enable);

    // Select SPI0 Chip Disable
	SetSPI0ChipEnable (0);

    Delay_us (10);
    
    // Select SPI0 Chip Enable
	SetSPI0ChipEnable (1);

    // Send SPI command Bulk Erase
	TransferSPI0Data (Cmd_Spi.Bulk_Erase);

    // Select SPI0 Chip Disable
	SetSPI0ChipEnable (0);
}

//-----------------------------------------------------------------------------
// WaitSPI0ChipError
//-----------------------------------------------------------------------------
//
// Return Value : U8 1 = Wait fault, 0 = ChipReady
// Parameters   :
//
// Write the specified number of bytes in the buffer to the specified address.
//
//-----------------------------------------------------------------------------
U8 WaitSPI0ChipError (void)
{
    U16 wdt_write;
    wdt_write = WDT_INIT;

    // Wait while Isr is processing now
    while (Spi_Isr_Mode != SPI_ISR_IDLE);

    // Disable SPI0 interrupts
    ESPI0 = 0;

    // Waiting for finishing write operation
    while (GetSPI0StatusBusy ())
    {
        if (wdt_write-- == 0)
        {
            return 1;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
// WriteSPI0Sector
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U16 address : the address of the SPI0 page to write
//                2) U8 buffer[] : a buffer to write to the SPI0 page starting
//                                 at the specified page address
//                3) U16 size : the number of bytes in the buffer to write
//
// Write the specified number of bytes in the buffer to the specified address.
//
//-----------------------------------------------------------------------------
U8 WriteSPI0Sector(U32 address, U8* buffer, U16 data_size)
{
    // Check for Chip Error if Fault return 0 = Write Error
    if (WaitSPI0ChipError ())
    {
        return 0;
    }
    
    // Assign mode and data
    Spi_Isr_Mode = SPI_ISR_WRITE;
    Spi_Buffer = buffer;
    Spi_Data_Size = data_size;

//    Delay_us (10);
    
    // Select SPI0 Chip Enable
	SetSPI0ChipEnable (1);

    // Send SPI command Write Enable
	TransferSPI0Data (Cmd_Spi.Write_Enable);

    // Select SPI0 Chip Disable
	SetSPI0ChipEnable (0);

    Delay_us (10);

    // Select SPI0 Chip Enable
	SetSPI0ChipEnable (1);

    // If flash capacity more than 16Mb
    if (Spi_4xAddress)
    {
        // Send SPI command WRITE and 4 bytes address
        TransferSPI0Data (Cmd_Spi.Write | 0x10);
        TransferSPI0Data (((U8[]) &address)[0]);
    }
    else
    {
        // Send SPI command WRITE and 3 bytes address
        TransferSPI0Data (Cmd_Spi.Write);
    }    
    TransferSPI0Data (((U8[]) &address)[1]);
    TransferSPI0Data (((U8[]) &address)[2]);

    // Enable interrupts
    ESPI0 = 1;
    
    // Write last byte of address and activate SPI ISR
    WriteSPI0Data (((U8[]) &address)[3]);

//  TransferSPI0Data ((address >> 16) & 0xFF);
// 	TransferSPI0Data ((address >> 8) & 0xFF);
// 	TransferSPI0Data (address & 0xFF);
    // Write to SPI SIZE bytes from the buffer
//     for (i = 0; i < data_size; i++)
//     {
//         TransferSPI0Data(buffer[i]);
//     }

    // Select SPI0 Chip Disable
//    SetSPI0ChipEnable (0);

//    Delay_us (10);

    return 1;
}

//-----------------------------------------------------------------------------
// ReadSPI0Sector
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U16 address : the address of the SPI0 page to read
//                2) U8 buffer[] : a buffer to read from the SPI0 page starting
//                                 at the specified page address
//                3) U16 size : the number of bytes to read into the buffer
//
// Read the specified number of bytes from SPI0 and store in the buffer.
//
//-----------------------------------------------------------------------------
void ReadSPI0Sector (U32 address, U8* buffer, U16 data_size)
{
//    U8 EA_Save = EA;
//    U16 i;

    // Wait while Isr is processing now
    while (Spi_Isr_Mode != SPI_ISR_IDLE);

    // Disable SPI0 interrupts
    ESPI0 = 0;
    // Assign mode and data
    Spi_Isr_Mode = SPI_ISR_READ;
    Spi_Buffer = buffer;
    Spi_Data_Size = data_size;

    // Select SPI0 Chip Enable
	SetSPI0ChipEnable (1);

    // If flash capacity more than 16Mb
    if (Spi_4xAddress)
    {
        // Send SPI command READ and 4 bytes address
        TransferSPI0Data (Cmd_Spi.Read | 0x10);
        TransferSPI0Data (((U8[]) &address)[0]);
    }   
    else
    {
        // Send SPI command READ and 3 bytes address
        TransferSPI0Data (Cmd_Spi.Read);
    }
    TransferSPI0Data (((U8[]) &address)[1]);
    TransferSPI0Data (((U8[]) &address)[2]);
    TransferSPI0Data (((U8[]) &address)[3]);

    // Enable interrupts
    ESPI0 = 1;
    
    // Write dummy data to start read by ISR
    WriteSPI0Data (0xFF);

//  TransferSPI0Data ((address >> 16) & 0xFF);
// 	TransferSPI0Data ((address >> 8) & 0xFF);
// 	TransferSPI0Data (address & 0xFF);
    // Read from SPI SIZE bytes and store it in the buffer
//     for (i = 0; i < size; i++)
//     {
//         buffer[i] = TransferSPI0Data (0);
//     }

//     // Select SPI0 Chip Disable
//     SetSPI0ChipEnable (0);
}

//-----------------------------------------------------------------------------
// GetSPI0Status
//-----------------------------------------------------------------------------
//
// Return Value : U8  STATUS register
// Parameters   : None
//
// Read the STATUS register S0 from SPI0 and return it. 
//
//-----------------------------------------------------------------------------
U8 GetSPI0Status (void)
{
    U8 Stat;
    // Select SPI0 Chip Enable
	SetSPI0ChipEnable (1);

//    Delay_us (50);
    // Send SPI command READ status register
	TransferSPI0Data (Cmd_Spi.Read_Status_Register);

//    Delay_ms (1);
    // Read from SPI STATUS register
    Stat = TransferSPI0Data (0xFF); // & 0x01);
    Stat |= TransferSPI0Data (0xFF); // & 0x01);
//    Stat = (TransferSPI0Data (0x00)); // & 0x01);
    
    // Select SPI0 Chip Disable
    SetSPI0ChipEnable (0);

    return Stat;
}

//-----------------------------------------------------------------------------
// GetSPI0StatusBusy
//-----------------------------------------------------------------------------
//
// Return Value : U8  BUSY bit state
// Parameters   : None
//
// Read the STATUS register S0 from SPI0 and return the BUSY bit. 
//
//-----------------------------------------------------------------------------
U8 GetSPI0StatusBusy (void)
{
    return (GetSPI0Status()); // | GetSPI0Status());
}

//-----------------------------------------------------------------------------
// GetSPI0DevId
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//
// Get Device ID from SPI0 and store in the buffer.
//
//-----------------------------------------------------------------------------
void GetSPI0DevId(U8* buffer)
{
    U8 i;

    // Disable SPI0 interrupts
    ESPI0 = 0;

    // Select SPI0 Chip Enable
    SetSPI0ChipEnable (1);
    
    // Delay_us (100);

    // Send SPI command READ_ID, read 3 bytes answer and store it in the buffer
	TransferSPI0Data (CMD_SPI_RDID);
    for (i = 0; i < 3; i++)
    {
        buffer[i] = TransferSPI0Data (0);
	}

    // Select SPI0 Chip Disable
  	SetSPI0ChipEnable (0);

}

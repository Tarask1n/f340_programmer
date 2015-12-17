//-----------------------------------------------------------------------------
// F3xx_Debug.h
//-----------------------------------------------------------------------------
// Copyright 2015 Kin
// Program Description:
//
// Read, Write and Erase of I2C0
//

#define I2C0_CLOCK    50000      // Maximum I2C0 clock
#define I2C_ISR_IDLE        0xFF


//-----------------------------------------------------------------------------
// Definitions - Pinout
//-----------------------------------------------------------------------------

#define mI2C0_SCK    0x01
#define mI2C0_MISO   0x02
#define mI2C0_MOSI   0x04
#define mI2C0_HOLD   0x08
#define mI2C0_WP     0x10
#define mI2C0_CE     0x20

#define mVDD_50V     0x01
#define mVDD_33V     0x02
#define mVSS         0x04
#define mVSS_93      0x08
#define mLED1        0x10

#define mI2C1_SCK    0x01
#define mI2C1_MISO   0x02
#define mI2C1_MOSI   0x04
#define mI2C1_HOLD   0x08
#define mI2C1_WP     0x10
#define mI2C1_CE     0x20

//-----------------------------------------------------------------------------
// Definitions - I2C State Machine
//-----------------------------------------------------------------------------


// 03f0:0008:00000001:00000000:00000010:00000006:040302c7:05010000:00000000:48424400

// Spi Commands Set Type Definition
// typedef struct Spi_Commands_Set
// {
//     U8  Write_Enable;               // Write Enable command
//     U8  Write_Disable;              // Write Disable command
//     U8  Read;                       // Read command
//     U8  Write;                      // Write command
//     U8  Bulk_Erase;                 // Bulk Erase command
//     U8  Read_Status_Register;       // Read Status Register 1 command
//     U8  Write_Status_Register;      // Write Status Register 1 command
//     U8  Enter_4b_Mode;              // Enter 4 byte address mode command
//     U8  Exit_4b_Mode;               // Exit 4 byte address mode command
//     U8  Read_Extended_Address_Register;  // Read Extended Address Register command
//     U8  Write_Extended_Address_Register;  // Read Extended Address Register command
//     U8  Unknown_1;
//     U8  Unknown_2;
//     U8  Read_Security_Registers;    // Read Security Registers command
//     U8  Write_Security_Registers;   // Read Security Registers command
//     U8  Erase_Security_Registers;   // Read Security Registers command
// } Spi_Commands_Set;


//-----------------------------------------------------------------------------
// External Global Variables
//-----------------------------------------------------------------------------

// extern U8 Spi_Isr_Mode;
// extern U8 Spi_Buffer_Packets_Num;
// extern U8 Spi_4xAddress;

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void SetI2C0PowerOn(void);
void SetI2C0PowerOff(void);
void ReadI2C0Sector(U32 address, U8* buffer, U16 size);
U8 WriteI2C0Sector(U32 address, U8* buffer, U16 size);
U8 GetI2C0StatusBusy (void);
void EraseI2C0Device(void);

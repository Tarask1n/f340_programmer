//-----------------------------------------------------------------------------
// F3xx_Debug.h
//-----------------------------------------------------------------------------
// Copyright 2015 Kin
// Program Description:
//
// Get ID, Read, Write and Erase of SPI0
//

#define SPI_CLOCK     4000000      // Maximum SPI clock
#define WDT_INIT        1000
// #define SET_50V       0x50
// #define SET_33V       0x33

//-----------------------------------------------------------------------------
// Definitions - Pinout
//-----------------------------------------------------------------------------
#define mSPI_BSY     0x80             // SPI busy

#define mSPI0_SCK    0x01
#define mSPI0_MISO   0x02
#define mSPI0_MOSI   0x04
#define mSPI0_HOLD   0x08
#define mSPI0_WP     0x10
#define mSPI0_CE     0x20

#define mVDD_50V     0x01
#define mVDD_33V     0x02
#define mVSS         0x04
#define mVSS_93      0x08
#define mLED1        0x10

#define mSPI1_SCK    0x01
#define mSPI1_MISO   0x02
#define mSPI1_MOSI   0x04
#define mSPI1_HOLD   0x08
#define mSPI1_WP     0x10
#define mSPI1_CE     0x20

//-----------------------------------------------------------------------------
// Definitions - SPI State Machine
//-----------------------------------------------------------------------------
// SPI Command IDs
// #define CMD_SPI_WREN        0x06
// #define CMD_SPI_WRDI        0x04
// #define CMD_SPI_READ        0x03
// #define CMD_SPI_WRITE       0x02
// #define CMD_SPI_BE          0xC7
// #define CMD_SPI_STATUS_S0   0x05
#define CMD_SPI_RDID        0x9F

// 03f0:0008:00000001:00000000:00000010:00000006:040302c7:05010000:00000000:48424400

// Spi Commands Set Type Definition
typedef struct Spi_Commands_Set
{
    U8  Write_Enable;               // Write Enable command
    U8  Write_Disable;              // Write Disable command
    U8  Read;                       // Read command
    U8  Write;                      // Write command
    U8  Bulk_Erase;                 // Bulk Erase command
    U8  Read_Status_Register;       // Read Status Register 1 command
    U8  Write_Status_Register;      // Write Status Register 1 command
    U8  Enter_4b_Mode;              // Enter 4 byte address mode command
    U8  Exit_4b_Mode;               // Exit 4 byte address mode command
    U8  Read_Extended_Address_Register;  // Read Extended Address Register command
    U8  Write_Extended_Address_Register;  // Read Extended Address Register command
    U8  Unknown_1;
    U8  Unknown_2;
    U8  Read_Security_Registers;    // Read Security Registers command
    U8  Write_Security_Registers;   // Read Security Registers command
    U8  Erase_Security_Registers;   // Read Security Registers command
} Spi_Commands_Set;

#define SPI_ISR_READ        0x20
#define SPI_ISR_WRITE       0x21
#define SPI_ISR_IDLE        0x22

//-----------------------------------------------------------------------------
// External Global Variables
//-----------------------------------------------------------------------------

extern SEGMENT_VARIABLE (Cmd_Spi, Spi_Commands_Set, SEG_XDATA);
extern U8 Spi_Isr_Mode;
extern U8 Spi_Buffer_Packets_Num;
extern U8 Spi_4xAddress;

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void SetSPI0PowerOn(void);
void SetSPI0PowerOff(void);
void GetSPI0DevId(U8* buffer);
void ReadSPI0Sector(U32 address, U8* buffer, U16 size);
U8 WriteSPI0Sector(U32 address, U8* buffer, U16 size);
U8 GetSPI0StatusBusy (void);
void EraseSPI0Device(void);
U8 WaitSPI0ChipError (void);

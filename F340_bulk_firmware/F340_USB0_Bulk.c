//-----------------------------------------------------------------------------
// F340_USB0_Bulk.c
//-----------------------------------------------------------------------------
// Copyright 2008-2012 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Device specific initialization routines.
//
//
// How To Test:    See Readme.txt
//
//
// Target:         C8051F34x
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
//    -All changes by BW
//    -1 SEP 2010
//    -Updated USB0_Init() to write 0x89 to CLKREC instead of 0x80
//
// Release 1.0
//    -Initial Revision (PD)
//    -04 JUN 2008
//

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------

#include "c8051f3xx.h"
#include "F3xx_USB0_Descriptor.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_USB0_Register.h"
#include "F3xx_USB0_Bulk.h"
#include "F3xx_USB0_Main.h"
#include "F3xx_SPI0.h"
#include "F3xx_Debug.h"

#if TARGET_MCU != MCU_F340
#error Invalid TARGET_MCU definition
#endif // TARGET_MCU != MCU_F340

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

//#define SYSCLK             24000000    // SYSCLK frequency in Hz

// USB clock selections (SFR CLKSEL)
#define USB_4X_CLOCK       0x00        // Select 4x clock multiplier, for USB
#define USB_INT_OSC_DIV_2  0x10        // Full Speed
#define USB_EXT_OSC        0x20
#define USB_EXT_OSC_DIV_2  0x30
#define USB_EXT_OSC_DIV_3  0x40
#define USB_EXT_OSC_DIV_4  0x50

// System clock selections (SFR CLKSEL)
#define SYS_INT_OSC        0x00        // Select to use internal oscillator
#define SYS_EXT_OSC        0x01        // Select to use an external oscillator
#define SYS_4X_DIV_2       0x02
#define SYS_4X             0x03

// System voltage regulator
#define mREGDIS            0x80        // 1 = Disable output 3.3 Voltage regulator 

//-----------------------------------------------------------------------------
// Static Function Prototypes
//-----------------------------------------------------------------------------

static void Sysclk_Init (void);
static void Port_Init (void);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

SBIT (Led1, SFR_P1, 4);         // LED1 = '0' means ON

SBIT (VDD_50V, SFR_P1, 0);		// VDD 5.0 V    0 - activate
SBIT (VDD_33V, SFR_P1, 1);		// VDD 3.3 V    0 - activate
SBIT (VSS, SFR_P1, 2);	    	// VSS for 24/25 series  1 - activate
SBIT (VSS_93, SFR_P1, 3);		// VSS for 93 series     1 - activate

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------
// Power On 5.0 V
void PowerOn50V (void)
{
    VSS_93 = 0;     // deactivate
    VSS = 1;        // activate
    VDD_33V = 1;    // deactivate
    VDD_50V = 0;    // activate
}

// Power On 3.3 V
void PowerOn33V (void)
{
    VSS_93 = 0;     // deactivate
    VSS = 1;        // activate
    VDD_50V = 1;    // deactivate
    VDD_33V = 0;    // activate
}

// Power Off
void PowerOff (void)
{
    VSS_93 = 0;     // deactivate
    VSS = 0;        // deactivate
    VDD_50V = 1;    // deactivate
    VDD_33V = 1;    // deactivate  
}

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

//-----------------------------------------------------------------------------
// TIMER2_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Initializes Timer2 to be clocked by SYSCLK for use as a delay timer.
//
//-----------------------------------------------------------------------------
void TIMER2_Init (void)
{
   CKCON    |= 0x10;
}

//-----------------------------------------------------------------------------
// Delay_us
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : 1. time_us - time delay in microseconds
//                   range: 1 to 255
//
// Creates a delay for the specified time (in microseconds) using TIMER2. The 
// time tolerance is approximately +/-50 ns (1/SYSCLK + function call time).
//
//-----------------------------------------------------------------------------
void Delay_us (U8 time_us)
{
   TR2   = 0;                          // Stop timer
   TF2H  = 0;                          // Clear timer overflow flag
   TMR2  = -(((U16)(SYSCLK/1000000) * (U16)(time_us)) * 2);
   TR2   = 1;                          // Start timer
   while (!TF2H);                      // Wait till timer overflow occurs
   TR2   = 0;                          // Stop timer
}

//-----------------------------------------------------------------------------
// Delay_ms
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : 1. time_ms - time delay in milliseconds
//                   range: 1 to 255
//
// Creates a delay for the specified time (in milliseconds) using TIMER2. The 
// time tolerance is approximately +/-50 ns (1/SYSCLK + function call time).
//
//-----------------------------------------------------------------------------
void Delay_ms (U8 time_ms)
{
   U8 i;

   while(time_ms--)
      for(i = 0; i < 10; i++)           // 10 * 100 microsecond delay
         Delay_us (100);
}

//-----------------------------------------------------------------------------
// System_Init
//-----------------------------------------------------------------------------
// This function is declared in the header file F3xx_HIDtoUART.h and is
// called in the main(void) function.  It calls initialization routines
// local to this file.
//
//-----------------------------------------------------------------------------
void System_Init (void)
{
#if (DEBUG_MODE == 1)        
    Store_Debug_Message ("Sys Init", 0);
#endif        
    PCA0MD &= ~0x40;                    // Disable watchdog timer
#if (DEBUG_MODE == 1)        
    Store_Debug_Message ("WDT Dis Ok", 1);
#endif        
// !!!! HANGS IF ENABLED !!!!
//   Sysclk_Init ();                     // Initialize oscillator
#if (DEBUG_MODE == 1)        
    Store_Debug_Message ("SYSCLK Off", 2);
#endif        
   Port_Init ();                       // Initialize crossbar and GPIO
#if (DEBUG_MODE == 1)        
    Store_Debug_Message ("PortInit Ok", 3);
#endif        
   TIMER2_Init ();                     // Initialize TIMER2
#if (DEBUG_MODE == 1)        
    Store_Debug_Message ("T2 init Ok", 4);
#endif        
    
   VDM0CN = 0x80;                      // Enable VDD Monitor
//   Delay ();                           // Wait for VDD Monitor to stabilize
   Delay_us (80);
   RSTSRC = 0x02;                      // Enable VDD Monitor as a reset source
// taraskin
//   REG0CN &= ~mREGDIS;                  // Enable 3.3 Voltage output
#if (DEBUG_MODE == 1)        
    Store_Debug_Message ("VDDMon Ok", 5);
#endif        

}

//-----------------------------------------------------------------------------
// USB0_Init
//-----------------------------------------------------------------------------
// USB Initialization performs the following:
// - Initialize USB0
// - Enable USB0 interrupts
// - Enable USB0 transceiver
// - Enable USB0 with suspend detection
//
//-----------------------------------------------------------------------------
void USB0_Init (void)
{
   POLL_WRITE_BYTE (POWER,  0x08);     // Force Asynchronous USB Reset
   POLL_WRITE_BYTE (IN1IE,  0x03);     // Enable Endpoint 0-1 in interrupts
//   POLL_WRITE_BYTE (OUT1IE, 0x02);     // Enable Endpoint 1 out interrupts
   POLL_WRITE_BYTE (OUT1IE, 0x04);     // Enable Endpoint 2 out interrupts
   POLL_WRITE_BYTE (CMIE,   0x0F);     // Enable SOF, Reset, Resume, and
                                       // Suspend interrupts

   USB0XCN = 0xE0;                     // Enable transceiver; select full speed
   POLL_WRITE_BYTE (CLKREC, 0x89);     // Enable clock recovery, single-step
                                       // mode disabled

   EIE1 |= 0x02;                       // Enable USB0 Interrupts

                                       // Enable USB0 by clearing the USB
   POLL_WRITE_BYTE (POWER, 0x01);      // Inhibit Bit and enable suspend
                                       // detection
}

//-----------------------------------------------------------------------------
// USB0_Suspend
//-----------------------------------------------------------------------------
// Enter suspend mode after suspend signalling is present on the bus
//
void USB0_Suspend(void)
{
   // Power Down

   EA = 0;                    // Disable global interrupts
   XBR1 &= ~0x40;             // Disengage the crossbar

   USB0XCN &= ~0x40;          // Suspend USB Transceiver
   CLKSEL = 0x10;             // USB Clock now uses int osc / 2
   CLKMUL = 0x00;             // Disable Clock Multiplier
   REG0CN |= 0x10;            // Place voltage regulator in low power mode
   OSCICN |= 0x20;            // Force internal oscillator suspend

   NOP ();

   // Power Up
   // Code execution begins here when resume signaling is received.

   REG0CN &= ~0x10;           // Place voltage regulator in full power mode
   Sysclk_Init ();
   USB0XCN |= 0x40;           // Enable USB Transceiver

// tarask1n     
   CLKSEL = SYS_4X;             // Return maximal clock
    
   XBR1 |= 0x40;              // Engage the crossbar
   EA = 1;
}

//-----------------------------------------------------------------------------
// Delay
//-----------------------------------------------------------------------------
// Used for a small pause, approximately 80 us in Full Speed,
// and 1 ms when clock is configured for Low Speed
//
//-----------------------------------------------------------------------------
// void Delay (void)
// {
//    S16 x;

//    for (x = 0; x < 500; x)
//    {
//       x++;
//    }
// }

//-----------------------------------------------------------------------------
// SetLed
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) U8 on: 0 means off, 1 means on
//
// Set the state of the LED
//
//-----------------------------------------------------------------------------
void SetLed (U8 on)
{
    if (on)
     Led1 = 0;
    else
     Led1 = 1;
}

//-----------------------------------------------------------------------------
// Static Global Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sysclk_Init
//-----------------------------------------------------------------------------
// This function initializes the system clock and the USB clock.
// Full-speed System Clock: 24 MHz  USB Clock: 48 MHz
//
//-----------------------------------------------------------------------------
static void Sysclk_Init (void)
{
   OSCICN |= 0x03;                     // Configure internal oscillator for
                                       // its maximum frequency and enable
                                       // missing clock detector

   CLKMUL  = 0x00;                     // Select internal oscillator as
                                       // input to clock multiplier

   CLKMUL |= 0x80;                     // Enable clock multiplier
//   Delay();                            // Delay for clock multiplier to begin
   Delay_us (80);
   CLKMUL |= 0xC0;                     // Initialize the clock multiplier
//   Delay();                            // Delay for clock multiplier to begin
   Delay_us (80);

   while(!(CLKMUL & 0x20));            // Wait for multiplier to lock
//   CLKSEL  = 0x02;                     // Set sys clock to clkmul / 2
// Full-speed System Clock: 48 MHz  USB Clock: 48 MHz
   CLKSEL  = SYS_4X;                     // Set sys clock to clkmul  
}

//-----------------------------------------------------------------------------
// Port_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This function configures the crossbar and GPIO ports.
//
// P1.4   digital   push-pull    LED1
//
//-----------------------------------------------------------------------------
static void Port_Init (void)
{
//    SetSPI0PowerOn ();
    P0MDOUT   = 0x00;                                               // P0 push-pull off
    P0        = 0xFF;
    P1MDOUT   = mVDD_50V | mVDD_33V | mVSS | mVSS_93;               // Make VDD_50V, VDD_33V, VSS, VSS_93 push-pull
    P1SKIP    = 0xFF;                                               // Skip all P1
    P1        = 0xFF;
    P2MDOUT   = 0x00;                                               // P2 push-pull off 
    P2        = 0xFF;
    PowerOff ();
//   P1MDOUT   = 0x10; // P1.4 is push-pull
//   P1SKIP    = 0x10; // P1.4 skipped
//    XBR1       = 0x40; // Enable the crossbar
}

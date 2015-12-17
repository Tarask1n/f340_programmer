//-----------------------------------------------------------------------------
// F3xx_SPI0.c
//-----------------------------------------------------------------------------
//
// Program Description:
//
// Realization of XTEA crypto algo
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
#include "F3xx_XTEA.h"
#include "F3xx_USB0_Main.h"

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
// Buffer used to store a temporary page data
SEGMENT_VARIABLE (Key[4], U32, SEG_XDATA);

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// KeyTransform
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : 
//                None
//                   
void KeyTransform(void)
{
}

//-----------------------------------------------------------------------------
// InitCrypt
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : 
//                None
//                   
void InitCrypt(void)
{
    Key[0] = 0x1A6B6450;
    Key[1] = 0x69D74842;
    Key[2] = 0x2BC79044;
    Key[3] = 0x4EF310BC;
}

//-----------------------------------------------------------------------------
// BlockEnCrypt
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) pointer to data to encrypt
//                   
void BlockEnCrypt(U8* d0, U8* d1)
{
    U32 v0, v1, sum;
    U8 i_block;
    sum = 0;
//     v0 = ((U32[]) d0)[0];
//     v1 = ((U32[]) d0)[1];
    v0 = Get_U32(d0);
    v1 = Get_U32(d1);
    for (i_block = 0; i_block < 32; i_block++)
    {    
        v0 = v0 + (((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + Key[sum & 3]));
        sum = sum + DELTA;
        v1 = v1 + (((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + Key[sum >> 11 & 3]));
    }
//     *d0 = v0;
//     *d1 = v1;
    Put_U32(d0, v0);
    Put_U32(d1, v1);
}

//-----------------------------------------------------------------------------
// BlockDeCrypt
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) pointer to data to decrypt
//                   
void BlockDeCrypt(U8* d0, U8* d1)
{ 
    U32 v0, v1, sum;
    U8 i_block;
    sum = DELTA * 32;
    v0 = Get_U32(d0);
    v1 = Get_U32(d1);
    for (i_block = 0; i_block < 32; i_block++)
    {
        v1 = v1 - (((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + Key[sum >> 11 & 3]));
        sum = sum - DELTA;
        v0 = v0 - (((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + Key[sum & 3]));
    }
    Put_U32(d0, v0);
    Put_U32(d1, v1);
}

//-----------------------------------------------------------------------------
// EnCrypt
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) pointer to data to encrypt
//                   
void EnCrypt(U8* CryptoData)
{
    U8 i_sector;
    for (i_sector = 0; i_sector < (512/8); i_sector++)
    {
        BlockEnCrypt(&CryptoData[i_sector*8], &CryptoData[i_sector*8 + 4]);
        KeyTransform ();
    }
}

//-----------------------------------------------------------------------------
// DeCrypt
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) pointer to data to decrypt
//                   
void DeCrypt(U8* CryptoData)
{
    U8 i_sector;
    for (i_sector = 0; i_sector < (512/8); i_sector++)
    {
        BlockDeCrypt(&CryptoData[i_sector*8], &CryptoData[i_sector*8 + 4]);
        KeyTransform ();
    }
}


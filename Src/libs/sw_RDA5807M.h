/*
 * TEA5767.h
 *
 *  Created on: Jul 28, 2021
 *      Author: szymon.wroblewski
 */

#ifndef LIBS_SW_RDA5807M_H_
#define LIBS_SW_RDA5807M_H_



//////////////////////// RDS FM RECEIVER RDA5807.H //////////////////////
//
/////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------
// Global Functions
//--------------------------------------------------------------

//--------------------------------------------------------------
// Definitions
//--------------------------------------------------------------

/* I2C Address */     // Device #1  @20    Device #2  @22    Device #3  @C0
#define RDASequential     0x20  // Write address 20 sequen
#define RDARandom         0x22  // Read address 22
/* Initialization Options */
#define StartingFreq    87.00
#define EndingFreq      108.00
#define DefaultFreq     103.00  //105.30//105.30//103.30//87.50
#define InitialVolume        6
/* RDA5807M Function Code  */

/* RDA5807M Registers 0x02H (16Bits) */
#define RDA_DHIZ        0x8000
#define RDA_MUTE        0x8000
#define RDA_MONO_ON     0x2000
#define RDA_MONO_OFF    0xDFFF
#define RDA_BASS_ON     0x1000
#define RDA_BASS_OFF    0xEFFF
#define RDA_RCLK_MODE   0x0800
#define RDA_RCLK_DIRECT 0x0400
#define RDA_SEEK_UP     0x0300
#define RDA_SEEK_DOWN   0x0100
#define RDA_SEEK_STOP   0xFCFF
#define RDA_SEEK_WRAP   0x0080
#define RDA_SEEK_NOWRAP 0xFF7F
#define RDA_CLK_0327    0x0000
#define RDA_CLK_1200    0x0010
#define RDA_CLK_2400    0x0050
#define RDA_CLK_1300    0x0020
#define RDA_CLK_2600    0x0060
#define RDA_CLK_1920    0x0030
#define RDA_CLK_3840    0x0070
#define RDA_RDS_ON      0x0008
#define RDA_RDS_OFF     0xFFF7
#define RDA_NEW         0x0004
#define RDA_RESET       0x0002
#define RDA_POWER       0x0001
/* Register 0x03H (16Bits) */
#define RDA_TUNE_ON     0x0010
#define RDA_TUNE_OFF    0xFFEF
//--------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------
// It was not possible to lighten much the size of the variables
// because the code required them, otherwise I would have been forced
// to remove functions from the RDS


void RDA5807M_WriteAll(void);
void RDA5807M_WriteReg(int address,int data_);
void RDA5807M_Initialize(void);
void RDA5807M_Reset(void);
void RDA5807M_PowerOn(void);
void RDA5807M_PowerOff(void);
void RDA5807M_RDS_Init(void);
void RDA5807M_RDS_(void);
void RDA5807M_Mute(void);
void RDA5807M_Mono(void);
void RDA5807M_Softmute(void);
void RDA5807M_SoftBlend(void);
void RDA5807M_BassBoost(void);
void RDA5807M_AFC(void);
void RDA5807M_Volume(uint8_t vol);
void RDA5807M_Volume_bis(bool dir);
void RDA5807M_SeekUp(void);
void RDA5807M_SeekDown(void);
void RDA5807M_Frequency(float Freq);
void RDA5807M_Channel(void);


// Initialise internal variables before starting or after a change to another channel.
//void RDA5807M_RDSinit();

// Pass all available RDS data through this function.
void RDA5807M_ProcessData();




#endif /* LIBS_SW_RDA5807M_H_ */

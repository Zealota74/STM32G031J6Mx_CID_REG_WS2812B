/*
 * TEA5767.c
 *
 *  Created on: Jul 28, 2021
 *      Author: szymon.wroblewski
 */
#include <string.h>

#include "sw_mcu_conf.h"
#include "SW_BOARD/gpio.h"
#include "SW_TIMERS/sw_soft_timers.h"
#include "sw_i2c_simple.h"


#include "sw_RDA5807M.h"

#define TEA5767_ADDR		0x00  // TO nie tak!!!!


uint8_t 		nochannel, volume, signal, fmready, fmstation;
uint32_t 		rdsblockerror, rdssynchro, block1, block2, block3, block4; //was int
signed short	rdsready;

//rds = EX int -- power, bass, mute, tune, afc, softblend = EX unsiged short
bool	power, bass, mute, softmute, mono, afc, tuneok, softblend, rds;
bool	stereo; // stereo = ex short;

char    StationName[10];    // Station Name. 8 characters
float   freq;
char    RDSText[66];        // RDS test message 64 characters 64byte reserved
char    CTtime[12];         // CT time string formatted as 'CT hh:mm'
int     minutes;            // CT minutes transmitted on the minute
                            // RDS return cocal time in minutes
               // Tuned frequency

//--------------------------------------------------------------

const 	uint32_t WriteRegDef[6] = {0xC004, 0x0000, 0x0100, 0x84D4, 0x4000, 0x0000}; // initial data

		uint32_t WriteReg[6],	RDA5807M_ReadReg[7],	RDA5807M_RDS[32];

// ----- actual RDS values
int textAB, lasttextAB, lastTextIDX;
int rdsGroupType, rdsTP, rdsPTY;

uint8_t index_new, regind;
uint8_t x,l;
uint8_t buf[13];

int mins;           // RDS time in minutes
int lastmins;
uint8_t i, n; //
int idx;            // index_new of rdsText
int offset;         // RDS time offset and sign
char c1, c2;
char RDSTxt[66];
char RDStxt1[66];
char RDStxt2[66];
char PSName[10];    // including trailing '\00' character.
char PSName1[10];
char PSName2[10];
char rcv[12];



void Wire_beginTransmission( uint8_t freq ) {

}
void Wire_endTransmission(void) {}
void Wire_write(uint8_t freq) {}


//void TEA5767_setFrequency(float frequency) {
//	uint32_t frequencyB = 4 * (frequency * 1000000 + 225000) / 32768;
//
//	uint8_t frequencyH = frequencyB >> 8;
//	uint8_t frequencyL = frequencyB & 0XFF;
//
//	Wire_beginTransmission( TEA5767_ADDR );
//
//	Wire_write(frequencyL);
//	Wire_write(0xB0);
//	Wire_write(0x10);
//	Wire_write(0x00);
//	Wire_endTransmission();
//	delay_ms(100);
//}

// initial filling of the internal registers of the FM receiver

void RDA5807M_WriteAll() {
	regind = 0;
	// fill in to buf[] high uint8_t odd index_new
	for (index_new = 0; index_new < 12; index_new += 2) {
		buf[index_new] = (uint8_t) (WriteReg[regind] >> 8);
		regind++;
	}

	regind = 0;
	// fill in to buf[] low uint8_t  pair index_new
	for (index_new = 1; index_new < 12; index_new += 2) {
		buf[index_new] = (uint8_t) WriteReg[regind] & 0x00FF;
		regind++;
	}

//	I2C1_Start();
//	I2C1_Is_Idle();
//	I2C1_Wr(RDASequential);
//	I2C1_Is_Idle();
//	for (i = 0; i < 12; i += 2) {
//		I2C1_Wr(buf[i]);
//		I2C1_Is_Idle();
//		I2C1_Wr(buf[i + 1]);
//		I2C1_Is_Idle();
//	}
//	I2C1_Stop();
//	I2C1_Is_Idle();
}

void RDA5807M_Init() {
	for (index_new = 0; index_new < 6; index_new++) {
		WriteReg[index_new] = WriteRegDef[index_new];
	}
	RDA5807M_WriteAll();
}

void RDA5807M_PowerOn() {
	WriteReg[1] = WriteReg[1] | RDA_TUNE_ON;
	WriteReg[0] = WriteReg[0] | RDA_POWER;
	RDA5807M_WriteAll();
	power = 1;
	WriteReg[1] = WriteReg[1] & 0xFFEF; //Disable tune after PowerOn operation
}

void RDA5807M_PowerOff() {
	WriteReg[0] = WriteReg[0] ^ RDA_POWER;
	RDA5807M_WriteAll();
	power = 0;
}

void RDA5807M_Volume(uint8_t vol) {
	if (vol > 15) {
		vol = 15;
	}
	if (vol < 0) {
		vol = 0;
	}
	WriteReg[3] = (WriteReg[3] & 0xFFF0) | vol; // Set New Volume
	volume = vol;
	RDA5807M_WriteAll();
}

// for Decrease or increase volume is enought call function using direction
// like this:  RDA5807M_Volume_bis(true);  RDA5807M_Volume_bis(false);
void RDA5807M_Volume_bis(bool dir) {
	if (volume < 15 && dir == true) {
		volume++;
	}
	if (volume > 0 && dir == false) {
		volume--;
	}
	WriteReg[3] = (WriteReg[3] & 0xFFF0) | volume; // Set New Volume
	RDA5807M_WriteAll();
}

void RDA5807M_BassBoost() {
	if ((WriteReg[0] & 0x1000) == 0) {
		WriteReg[0] = WriteReg[0] | RDA_BASS_ON;
		bass = 1;
	} else {
		WriteReg[0] = WriteReg[0] & RDA_BASS_OFF;
		bass = 0;
	}
	RDA5807M_WriteAll();
}

void RDA5807M_Mono() {
	if ((WriteReg[0] & 0x2000) == 0) {
		WriteReg[0] = WriteReg[0] | RDA_MONO_ON;
		mono = 1;
	} else {
		WriteReg[0] = WriteReg[0] & RDA_MONO_OFF;
		mono = 0;
	}
	RDA5807M_WriteAll();
}

void RDA5807M_Mute() {
	if ((WriteReg[0] & 0x8000) == 0) {
		WriteReg[0] = WriteReg[0] | 0x8000;
		mute = 0;
	} else {
		WriteReg[0] = WriteReg[0] & 0x7FFF;
		mute = 1;
	}
	RDA5807M_WriteAll();
}

void RDA5807M_Softmute() {
	if ((WriteReg[2] & 0x0200) == 0) {
		WriteReg[2] = WriteReg[2] | 0x0200;
		softmute = 1;
	} else {
		WriteReg[2] = WriteReg[2] & 0xFDFF;
		softmute = 0;
	}
	RDA5807M_WriteAll();
}

void RDA5807M_SoftBlend() {
//	if ( (WriteReg[6] & 0x0001) == 0) {
//		WriteReg[6] = WriteReg[6] | 0x0001;
//		softblend = 1;
//	} else {
//		WriteReg[6] = WriteReg[6] & 0xFFFE;
//		softblend = 0;
//	}
	RDA5807M_WriteAll();
}

void RDA5807M_AFC() {
	if ((WriteReg[2] & 0x0100) == 0) {
		WriteReg[2] = WriteReg[2] | 0x0100;
		afc = 1;
	} else {
		WriteReg[2] = WriteReg[2] & 0xFEFE;
		afc = 0;
	}
	RDA5807M_WriteAll();
}

void RDA5807M_SeekUp() {
	//0000001100000000  (0x0300)
	WriteReg[0] = WriteReg[0] | RDA_SEEK_UP;   // Set Seek Up
	RDA5807M_WriteAll();
	//1111110011111111  (0xFCFF)
	WriteReg[0] = WriteReg[0] & RDA_SEEK_STOP; // Disable Seek
}

void RDA5807M_SeekDown() {
	// 0000000100000000(0x0100)
	WriteReg[0] = WriteReg[0] | RDA_SEEK_DOWN; // Set Seek Down
	RDA5807M_WriteAll();
	//1111110011111111  (0xFCFF)
	WriteReg[0] = WriteReg[0] & RDA_SEEK_STOP; // Disable Seek
}

//void RDA5807M_Frequency(float Freq){
void RDA5807M_Frequency(float Freq) {
	int Channel;
	Freq -= 0.3;  // minimal frequency adjust
	//Channel = ((Freq-StartingFreq)/0.1)+0.05;
	Channel = ((Freq - StartingFreq) / 0.1) + 0.05;
	//Channel = Channel & 0x03FF;
	Channel &= 0x03FF;
	WriteReg[1] = Channel * 64 + 0x10; // Channel + TUNE-Bit + Band=00(87-108) + Space=00(100kHz)
	RDA5807M_WriteAll();
	//WriteReg[1] = WriteReg[1]  RDA_TUNE_OFF;
	WriteReg[1] &= RDA_TUNE_OFF;
}

void RDA5807M_RDS_() {
	if ((WriteReg[0] & RDA_RDS_ON) == 0) {
		WriteReg[0] = WriteReg[0] | RDA_RDS_ON;
		rds = 1;
	} else {
		WriteReg[0] = WriteReg[0] & RDA_RDS_OFF;
		rds = 0;
	}
	RDA5807M_WriteAll();
}

void RDA5807M_Reset() {
	RDA5807M_Init();
	RDA5807M_PowerOn();
	RDA5807M_RDS_Init();
	RDA5807M_RDS_();
	RDA5807M_Volume(3);   //InitialVolume
	RDA5807M_Frequency(DefaultFreq + 0.3);    //DefaultFreq
}


void read_chip_ver(char chip_adr) {   //88FF
//	char buf_l, buf_h, vertxt[4];
//	int ver;
//	I2C1_Start();
//	I2C1_Wr(RDASequential);    // Address + W  // Address + Read 0x20 + 0x01
//	I2C1_Wr(0x00);
//	I2C1_Repeated_Start();
//	I2C1_Wr(RDASequential | 0x01);           // Address + Read
//	buf_h = I2C1_Rd(1u);
//	I2C1_Is_Idle();    // Read the data (acknowledge)
//	buf_l = I2C1_Rd(0u);
//	I2C1_Is_Idle();    // Read the data (NO acknowledge)
//	I2C1_Stop();
//
//	ver = buf_l | buf_h << 8;  // number version is present only first
//							   // higher part of uint8_t data.
//	// you can ignore the lower part of the second uint8_t received !!!
//	UART1_Write_Text("Version Chip:");
//	IntToHex(ver, vertxt);
//	UART1_Write_Text(vertxt);
//	UART1_Write_Text("\r\n");
}

void RDA5807M_Readregisters( char chip_adr, char start_adr, char stop_adr ) {

//	I2C1_Start();
//	I2C1_Wr( RDASequential | 0x01 );
//
//	for ( uint8_t i = 0; i < 11; i++) {
//		rcv[i] = I2C1_Rd(1u);
//		I2C1_Is_Idle();  				// Read the data (acknowledge)
//	}
//
//	rcv[11] = I2C1_Rd(0u);
//	I2C1_Is_Idle(); 					// Read the data (NO acknowledge)
//	I2C1_Stop();

	sw_i2c_read_block( chip_adr, RDASequential | 0x01, 11, (uint8_t *) rcv );

	for (i = 0; i < 6; i++) {
		RDA5807M_ReadReg[i] = ((rcv[i * 2] << 8) | rcv[(i * 2) + 1]);
	}

	block1 = RDA5807M_ReadReg[2];
	block2 = RDA5807M_ReadReg[3];
	block3 = RDA5807M_ReadReg[4];
	block4 = RDA5807M_ReadReg[5];

	rdsready 	= RDA5807M_ReadReg[0] & 0x8000; 	//if rdsready 	!= 0 rds data are ready
	tuneok 		= RDA5807M_ReadReg[0] & 0x4000;  	//if tuneok 	!= 0 seek/tune completed
	nochannel 	= RDA5807M_ReadReg[0] & 0x2000; 	//if nochannel 	!= 0 no channel found
	rdssynchro 	= RDA5807M_ReadReg[0] & 0x1000; 	//if rdssynchro = 1000 rds decoder syncrhonized
	stereo 		= RDA5807M_ReadReg[0] & 0x0400; 	//if stereo 	= 0 station is mono else stereo
	//0000001111111111  (0x03FF) +870;
	//freq = ((float)((RDA5807M_ReadReg[0] & 0x03FF) * 100) + 87000);    //return freq ex 102600KHz > 102.6MHz
	//freq = ( (RDA5807M_ReadReg[0] & 0x03FF)+870);
	freq 			= (((RDA5807M_ReadReg[0] & 0x03FF) + 870.001) / 10);
	signal 			= RDA5807M_ReadReg[1] >> 10;            //return signal strength rssi
	fmready 		= RDA5807M_ReadReg[1] & 0x0008;       //if fmready = 8 > fm is ready
	fmstation 		= RDA5807M_ReadReg[1] & 0x0100; //if fmstation = 100 fm station is true
	rdsblockerror 	= RDA5807M_ReadReg[1] & 0x000C;  //check for rds blocks errors
												   //00= 0 errors,01= 1~2 errors requiring correction
												   //10= 3~5 errors requiring correction
												   //11= 6+ errors or error in checkword, correction not possible.
}

void RDA5807M_RDS_Init() {
	strcpy(StationName, "        ");
	strcpy(PSName, "        ");
	strcpy(PSName1, "        ");
	strcpy(PSName2, "        ");
	memset(RDSText, '\0', sizeof(RDSText));
	memset(RDSTxt, '\0', sizeof(RDSTxt));
	lastTextIDX = 0;
	mins = 0;
//	sprintf(CTtime, "CT --:--");
}

void RDA5807M_ProcessData() {
	//uint8_t i,n;
	RDA5807M_Readregisters(RDASequential, 0x0A, 0x0F);
	if (rdssynchro != 0x1000) {  // reset all the RDS info. (4096 dec)
		RDA5807M_RDS_Init();
		return;
	}
	// analyzing Block 2
	rdsGroupType = 0x0A | ((block2 & 0xF000) >> 8) | ((block2 & 0x0800) >> 11);
	rdsTP = (block2 & 0x0400);
	rdsPTY = (block2 & 0x0400);

	switch (rdsGroupType) {
	case 0x0A:
	case 0x0B:
		// The data received is part of the Service Station Name
		idx = 2 * (block2 & 0x0003);
		// new data is 2 chars from block 4
		c1 = block4 >> 8;
		c2 = block4 & 0x00FF;
		// check that the data was received successfully twice
		// before sending the station name
		if ((PSName1[idx] == c1) && (PSName1[idx + 1] == c2)) {
			// retrieve the text a second time: store to _PSName2
			PSName2[idx] = c1;
			PSName2[idx + 1] = c2;
			PSName2[8] = '\0';
			if (strcmp(PSName1, PSName2) == 0) {
				// populate station name
				n = 0;
				for (i = 0; i < (8); i++) { // remove non-printable error ASCCi characters
					if (PSName2[i] > 31 && PSName2[i] < 127) {
						StationName[n] = PSName2[i];
						n++;
					}
				}
			}
		}
		if ((PSName1[idx] != c1) || (PSName1[idx + 1] != c2)) {
			PSName1[idx] = c1;
			PSName1[idx + 1] = c2;
			PSName1[8] = '\0';
		}
		break;
	case 0x2A:
		// RDS text
		textAB = (block2 & 0x0010);
		idx = 4 * (block2 & 0x000F);
		if (idx < lastTextIDX) {
			// The existing text might be complete because the index_new is starting at the beginning again.
			// Populate RDS text array.
			n = 0;
			for (i = 0; i < strlen(RDSTxt); i++) {
				if (RDSTxt[i] > 31 && RDSTxt[i] < 127) { // remove any non printable error charcters
					RDSText[n] = RDSTxt[i];
					n++;
				}
			}
		}
		lastTextIDX = idx;

		if (textAB != lasttextAB) {
			// when this bit is toggled the whole buffer should be cleared.
			lasttextAB = textAB;
			memset(RDSTxt, 0, sizeof(RDSTxt));
			memset(RDSText, '\0', sizeof(RDSText));
		}
		if (rdsblockerror < 4) {
			// new data is 2 chars from block 3
			RDSTxt[idx] = (block3 >> 8);
			idx++;
			RDSTxt[idx] = (block3 & 0x00FF);
			idx++;
			// new data is 2 chars from block 4
			RDSTxt[idx] = (block4 >> 8);
			idx++;
			RDSTxt[idx] = (block4 & 0x00FF);
			idx++;
		}
		break;
	case 0x4A:
		// Clock time and date
		if (rdsblockerror < 3) { // allow limited RDS data errors as we have no correctioin code
			offset = (block4) & 0x3F; // 6 bits
			mins = (block4 >> 6) & 0x3F; // 6 bits
			mins += 60 * (((block3 & 0x0001) << 4) | ((block4 >> 12) & 0x0F));
		}
		// adjust offset
		if (offset & RDARandom) {
			mins -= 30 * (offset & 0x1F);
		} else {
			mins += 30 * (offset & 0x1F);
		}
		if (mins == lastmins + 1) { // get CT time twice before populating time
			minutes = mins;
		}
		lastmins = mins;
		if (rdssynchro == 0x1000) {
			if (minutes > 0 && minutes < 1500) {
//				sprintf(CTtime, "CT %2d:%02d", (minutes / 60), (minutes % 60));
			}
		} else {
			minutes = 0;
//			sprintf(CTtime, "CT --:--");
		} // CT time formatted string
		break;
	case 0x6A:
		// IH
		break;
	case 0x8A:
		// TMC
		break;
	case 0xAA:
		// TMC
		break;
	case 0xCA:
		// TMC
		break;
	case 0xEA:
		// IH
		break;
	default:
		break;
	}
}

void getrds() {
	uint8_t n;
	if (strlen(RDSText) > 3) {
		memset(RDStxt1, '\0', sizeof(RDStxt1));
		memset(RDStxt2, '\0', sizeof(RDStxt2));

		// format into 2 lines of text seperated by first 'space' after 30 characters
		strcpy(RDStxt1, RDSText);
		n = strlen(RDStxt1);
		for (i = 0; i < (n); i++) {
			if (i > 30 && (RDStxt1[i] == ' ')) {
				RDStxt1[strlen(RDStxt1) - (n - i)] = '\0';
				l = strlen(RDStxt1);
				x = 1;
				break; // break if more than 30 characters with space
			}
			if (i > 39) {
				RDStxt1[strlen(RDStxt1) - (n - i)] = '\0';
				l = strlen(RDStxt1);
				x = 1;
				break; //break if more than 39 characters with no spaces
			}
		}
		if (x == 1) {
			strcpy(RDStxt2, RDSText + l);
			while (RDStxt2[0] == ' ') {
				strcpy(RDStxt2, (RDStxt2 + 1));
			}
			x = 0;
		}
	}
}

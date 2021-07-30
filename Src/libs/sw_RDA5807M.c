
#include <stddef.h>

#include "sw_mcu_conf.h"
#ifdef STM32G0
#define pgm_read_word	*
#define pgm_read_byte	*
#define PROGMEM
typedef uint16_t word;
typedef uint8_t byte;
#endif

#include "SW_BOARD/gpio.h"
#include "SW_BOARD/sw_led_blink_debug.h"
#include "SW_TIMERS/sw_soft_timers.h"

#include "sw_i2c_simple.h"


#include "sw_RDA5807M.h"



static INLINE byte highByte( word data ) {
	return (byte)(data >> 8);
}
static INLINE byte lowByte( word data ) {
	return (byte)data;
}

void Wire_beginTransmission( uint8_t freq ) {

}
void Wire_endTransmission( bool byte) {

}
void Wire_write(uint8_t freq) {}
byte Wire_read(void) { return 0; }
void Wire_begin(void) { }

byte Wire_requestFrom( byte addr, byte quantity, bool stop ) {
	return 0;
}

void setRegister( uint8_t reg, word data ) {
	sw_i2c_write_reg( RDA5807M_I2C_ADDR, reg, data );
}
word getRegister( uint8_t reg ) {
	uint8_t data;
	sw_i2c_read_reg( RDA5807M_I2C_ADDR, reg, &data );
	return data;
}
/*
* Description:
*   Read-before-write setter for single random access to registers.
* Parameters:
*   reg   - register to update, one of the RDA5807M_REG_* constants.
*   mask  - mask of the bits that are to be updated.
*   value - value to set the given register and bits to.
*/
void updateRegister( byte reg, word mask, word value) {
	setRegister( reg, getRegister(reg) & (~mask | value) );
};



void RDA5807M__begin(byte band) {
    Wire_begin();
    setRegister(RDA5807M_REG_CONFIG, RDA5807M_FLG_DHIZ | RDA5807M_FLG_DMUTE |
                RDA5807M_FLG_BASS | RDA5807M_FLG_SEEKUP | RDA5807M_FLG_RDS |
                RDA5807M_FLG_NEW | RDA5807M_FLG_ENABLE);
    updateRegister( RDA5807M_REG_TUNING, RDA5807M_BAND_MASK, band );
};

void RDA5807M__end(void) {
    setRegister(RDA5807M_REG_CONFIG, 0x00);
};

void RDA5807M__setRegister(byte reg, const word value) {
//    Wire_beginTransmission(RDA5807M_I2C_ADDR_RANDOM);
//    Wire_write(reg);
//    Wire_write( highByte(value));
//    Wire_write( lowByte(value));
//    Wire_endTransmission(true);
};

word RDA5807M__getRegister(byte reg) {
    word result = 0;

//    Wire_beginTransmission(RDA5807M_I2C_ADDR_RANDOM);
//    Wire_write(reg);
//    Wire_endTransmission(false);
//    Wire_requestFrom(RDA5807M_I2C_ADDR_RANDOM, (size_t)2, true);
//    //Don't let gcc play games on us, enforce order of execution.
//    result = (word)Wire_read() << 8;
//    result |= Wire_read();

    return result;
};

void RDA5807M__setRegisterBulk(byte count, const word regs[]) {
    Wire_beginTransmission(RDA5807M_I2C_ADDR_SEQRDA);

    for(byte i=0; i < count; i++) {
        Wire_write(highByte(regs[i]));
        Wire_write(lowByte(regs[i]));
    };

    Wire_endTransmission(true);
};

void RDA5807M__getRegisterBulk( byte count, word regs[] ) {
//    Wire_requestFrom(RDA5807M_I2C_ADDR_SEQRDA, (size_t)(count * 2), true);

    for(byte i=0; i < count; i++) {
        //Don't let gcc play games on us, enforce order of execution.
//        regs[count] = (word)Wire_read() << 8;
//        regs[count] |= Wire_read();
    };
};

void RDA5807M__setRegisterBulk1(const TRDA5807MRegisterFileWrite *regs) {
    const uint8_t * const ptr = (uint8_t *)regs;

    Wire_beginTransmission(RDA5807M_I2C_ADDR_SEQRDA);

    for(byte i=0; i < sizeof(TRDA5807MRegisterFileWrite); i++)
        Wire_write(ptr[i]);

    Wire_endTransmission(true);
};

void RDA5807M__getRegisterBulk1(TRDA5807MRegisterFileRead *regs) {
    uint8_t * const ptr = (uint8_t *)regs;

    Wire_requestFrom( RDA5807M_I2C_ADDR_SEQRDA, sizeof(TRDA5807MRegisterFileRead), true);

    for( byte i=0; i < sizeof(TRDA5807MRegisterFileRead); i++) {
        ptr[i] = Wire_read();
    }
};

bool RDA5807M__volumeUp(void) {
    const byte volume = getRegister(RDA5807M_REG_VOLUME) & RDA5807M_VOLUME_MASK;

    if (volume == RDA5807M_VOLUME_MASK)
        return false;
    else {
        updateRegister(RDA5807M_REG_VOLUME, RDA5807M_VOLUME_MASK, volume + 1);
        return true;
    };
};

bool RDA5807M__volumeDown(bool alsoMute) {
    const byte volume = getRegister(RDA5807M_REG_VOLUME) & RDA5807M_VOLUME_MASK;

    if (volume) {
        updateRegister(RDA5807M_REG_VOLUME, RDA5807M_VOLUME_MASK, volume - 1);
        if(!(volume - 1) && alsoMute)
            //If we are to trust the datasheet, this is superfluous as a volume
            //of zero triggers mute & HiZ on its own.
            mute();
        return true;
    } else
        return false;
};

void RDA5807M__seekUp(bool wrap) {
    updateRegister(RDA5807M_REG_CONFIG,
                   (RDA5807M_FLG_SEEKUP | RDA5807M_FLG_SEEK |
                    RDA5807M_FLG_SKMODE),
                   (RDA5807M_FLG_SEEKUP | RDA5807M_FLG_SEEK |
                    (wrap ? 0x00 : RDA5807M_FLG_SKMODE)));
};

void RDA5807M__seekDown(bool wrap) {
    updateRegister(RDA5807M_REG_CONFIG,
    				(RDA5807M_FLG_SEEKUP | RDA5807M_FLG_SEEK | RDA5807M_FLG_SKMODE),
					(0x00 | RDA5807M_FLG_SEEK | (wrap ? 0x00 : RDA5807M_FLG_SKMODE))
				  );
};

void RDA5807M__mute(void) {
    updateRegister(RDA5807M_REG_CONFIG, RDA5807M_FLG_DMUTE, 0x00);
};

void RDA5807M__unMute(bool minVolume) {
    if (minVolume) {
    	updateRegister(RDA5807M_REG_VOLUME, RDA5807M_VOLUME_MASK, 0x1);
    }
    updateRegister(RDA5807M_REG_CONFIG, RDA5807M_FLG_DMUTE, RDA5807M_FLG_DMUTE);
};

const word RDA5807M_BandLowerLimits[5] 	= { 8700, 7600, 7600, 6500, 5000 };
const word RDA5807M_BandHigherLimits[5] = { 10800, 9100, 10800, 7600, 6500 };
const byte RDA5807M_ChannelSpacings[4] 	= { 100, 200, 50, 25 };

word RDA5807M__getBandAndSpacing(void) {
    byte band = getRegister(RDA5807M_REG_TUNING) & (RDA5807M_BAND_MASK |
                                                    RDA5807M_SPACE_MASK);
    //Separate channel spacing
    const byte space = band & RDA5807M_SPACE_MASK;

    if ( (band & RDA5807M_BAND_MASK) == RDA5807M_BAND_EAST
      &&!(getRegister(RDA5807M_REG_BLEND) & RDA5807M_FLG_EASTBAND65M))
        //Lower band limit is 50MHz
        band = (band >> RDA5807M_BAND_SHIFT) + 1;
    else
        band >>= RDA5807M_BAND_SHIFT;

//    return word(space, band);
    return 0;
};

word RDA5807M__getFrequency(void) {
    const word spaceandband = getBandAndSpacing();

    return pgm_read_word(&RDA5807M_BandLowerLimits[lowByte(spaceandband)]) +
        (getRegister(RDA5807M_REG_STATUS) & RDA5807M_READCHAN_MASK) *
        pgm_read_byte( &RDA5807M_ChannelSpacings[highByte(spaceandband)] ) / 10;
};

bool RDA5807M__setFrequency( word frequency ) {
    const word spaceandband = getBandAndSpacing();
    const word origin = pgm_read_word( &RDA5807M_BandLowerLimits[lowByte(spaceandband)] );

    //Check that specified frequency falls within our current band limits
    if (frequency < origin ||
        frequency > pgm_read_word(
            &RDA5807M_BandHigherLimits[lowByte(spaceandband)]))
        return false;

    //Adjust start offset
    frequency -= origin;

    const byte spacing = pgm_read_byte(
        &RDA5807M_ChannelSpacings[highByte(spaceandband)]);

    //Check that the given frequency can be tuned given current channel spacing
    if (frequency * 10 % spacing)
        return false;

    //Attempt to tune to the requested frequency
    updateRegister( RDA5807M_REG_TUNING,
    				RDA5807M_CHAN_MASK | RDA5807M_FLG_TUNE,
                   ((frequency * 10 / spacing) << RDA5807M_CHAN_SHIFT) | RDA5807M_FLG_TUNE
				  );
    return true;
};

byte RDA5807M__getRSSI(void) {
    return ( getRegister(RDA5807M_REG_RSSI) & RDA5807M_RSSI_MASK) >> RDA5807M_RSSI_SHIFT;
};

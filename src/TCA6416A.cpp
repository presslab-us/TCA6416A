// TCA6416A module
// Copyright 2019 Håkon Nessjøen
#include <Arduino.h>
#include "TCA6416A.h"

// NB!: Only address 0 or 1
bool TCA6416A::begin(uint8_t addr_bit, TwoWire *theWire) {
	i2caddr = 0x20 | addr_bit;
	i2cwidth = 2;
	TW = theWire;
	TW->begin();

	/// Check if device is connected
	TW->beginTransmission((int)i2caddr);
	if (TW->endTransmission() != 0) {
		return false;
	}

	port_read();
	mode_read();
	pull_read();

	return true;
}

void TCA6416A::pin_write(uint8_t pinNum, uint8_t level) {
	uint16_t mask = 1 << pinNum;

	if (level == HIGH) {
		pinState |= mask;
	} else {
		pinState &= ~mask;
	}

	port_write(pinState);
}

void TCA6416A::pin_mode(uint8_t pinNum, int mode) {
	uint16_t mask = 1 << pinNum;

    switch(mode) {
		case INPUT:
		    pinModes |= mask;
            pinPullEn &= ~mask;
			break;
		case INPUT_PULLUP:
		    pinModes |= mask;
            pinPullEn |= mask;
			pinPullDir |= mask;
			break;
		case INPUT_PULLDOWN:
		    pinModes |= mask;
            pinPullEn |= mask;
			pinPullDir &= ~mask;
			break;
		default:
		    pinModes &= ~mask;
            pinPullEn &= ~mask;
			break;
	}
	mode_write(pinModes);
	pull_write(pinPullEn, pinPullDir);
}

bool TCA6416A::pin_read(uint8_t pinNum) {
	uint16_t mask = 1 << pinNum;

	return port_read() & mask;
}

void TCA6416A::port_write(uint16_t i2cportval) {
	TW->beginTransmission((int)i2caddr);
	TW->write(TCAREG_OUTPUT0);

	TW->write(i2cportval & 0x00FF);
	TW->write(i2cportval >> 8 );

	TW->endTransmission();

	pinState = i2cportval;
}

uint16_t TCA6416A::port_read() {
	uint16_t tempInput;
	TW->beginTransmission((int)i2caddr);
	TW->write(TCAREG_INPUT0);
	TW->endTransmission();

	TW->requestFrom((int)i2caddr, (int)i2cwidth);

	tempInput = TW->read();
	tempInput |= TW->read() << 8;

	return tempInput;
}

void TCA6416A::mode_write(uint16_t modes) {
	TW->beginTransmission((int)i2caddr);
	TW->write(TCAREG_CONFIG0);

	TW->write(modes & 0x00FF);
	TW->write(modes >> 8 );

	TW->endTransmission();

	pinModes = modes;
}

void TCA6416A::pull_write(uint16_t pullen, uint16_t pulldir) {
	TW->beginTransmission((int)i2caddr);
	TW->write(TCAREG_PULLEN0);

	TW->write(pullen & 0x00FF);
	TW->write(pullen >> 8 );

	TW->endTransmission();

	TW->beginTransmission((int)i2caddr);
	TW->write(TCAREG_PULLDIR0);

	TW->write(pulldir & 0x00FF);
	TW->write(pulldir >> 8 );

	TW->endTransmission();

	pinPullEn = pullen;
	pinPullDir = pulldir;
}

uint16_t TCA6416A::mode_read() {
	TW->beginTransmission((int)i2caddr);
	TW->write(TCAREG_CONFIG0);
	TW->endTransmission();

	TW->requestFrom((int)i2caddr, (int)i2cwidth);

	pinModes = TW->read();
	pinModes |= TW->read() << 8;

	return pinModes;
}

uint16_t TCA6416A::pull_read() {
	TW->beginTransmission((int)i2caddr);
	TW->write(TCAREG_PULLEN0);
	TW->endTransmission();

	TW->requestFrom((int)i2caddr, (int)i2cwidth);

	pinPullEn = TW->read();
	pinPullEn |= TW->read() << 8;

	TW->beginTransmission((int)i2caddr);
	TW->write(TCAREG_PULLDIR0);
	TW->endTransmission();

	TW->requestFrom((int)i2caddr, (int)i2cwidth);

	pinPullDir = TW->read();
	pinPullDir |= TW->read() << 8;

	return pinPullEn;
}

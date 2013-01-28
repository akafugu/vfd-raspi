/*
 * SPIDisplay: Arduino Library for Akafugu SPI serial displays
 * (C) 2011-13 Akafugu Corporation
 *
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 */

#include "SPIDisplay.h"

#if defined(ARDUINO) && ARDUINO < 100
#  define write send
#  define read  receive
#endif

SPIDisplay::SPIDisplay(int addr)
	: m_addr(addr)
	, m_dots(0)
	, m_apostrophes(0)
	, m_digits(4)
	, m_version(1)
{
}

void SPIDisplay::begin()
{
	m_digits = getDigits();
	m_version = getFirmwareRevision();
}

void SPIDisplay::set_number(uint16_t num)
{
	if (m_digits == 8) {
		m_data[7] = num % 10;
		num /= 10;
		m_data[6] = num % 10;
		num /= 10;
		m_data[5] = num % 10;
		num /= 10;
		m_data[4] = num % 10;
	}

	m_data[3] = num % 10;
	num /= 10;
	m_data[2] = num % 10;
	num /= 10;
	m_data[1] = num % 10;
	num /= 10;
	m_data[0] = num % 10;
}

void SPIDisplay::setBrightness(int brightness)
{
	SPI.transfer(0x80);
	delay(1);
	SPI.transfer(brightness);
	delay(1);
}

void SPIDisplay::clear()
{
	SPI.transfer(0x82);
	delay(1);
}

void SPIDisplay::setRotateMode()
{
	SPI.transfer(0x82);
	delay(1);
	SPI.transfer(0); // rotate mode
	delay(1);
}

void SPIDisplay::setScrollMode()
{
	SPI.transfer(0x82);
	delay(1);
	SPI.transfer(1); // scroll mode
	delay(1);
}

void SPIDisplay::setDot(int position, bool on)
{
	if (position < 0 || position > 7) return;
	
	if (position == 7 && on) m_dots |= 1;
	else if (position == 7 && !on) m_dots &= ~1;
	if (on) m_dots |= (1<<(position+1));
	else m_dots &= ~(1<<(position+1));
	
	SPI.transfer(0x85); //  set dots
	delay(1);
	SPI.transfer(m_dots);
	delay(1);
}

void SPIDisplay::setDots(bool dot0, bool dot1, bool dot2, bool dot3)
{
	m_dots = 0;
	if (dot0) m_dots |= 1<<1;
	if (dot1) m_dots |= 1<<2;
	if (dot2) m_dots |= 1<<3;
	if (dot3) m_dots |= 1<<4;

	SPI.transfer(0x85); //  set dots
	delay(1);
	SPI.transfer(m_dots);
	delay(1);
}

void SPIDisplay::setDots(bool dot0, bool dot1, bool dot2, bool dot3, bool dot4, bool dot5, bool dot6, bool dot7)
{
	m_dots = 0;
	if (dot0) m_dots |= 1<<1;
	if (dot1) m_dots |= 1<<2;
	if (dot2) m_dots |= 1<<3;
	if (dot3) m_dots |= 1<<4;
	if (dot4) m_dots |= 1<<5;
	if (dot5) m_dots |= 1<<6;
	if (dot6) m_dots |= 1<<7;
	if (dot7) m_dots |= 1<<0;

	SPI.transfer(0x85); //  set dots
	delay(1);
	SPI.transfer(m_dots);
	delay(1);
}

void SPIDisplay::setApostrophe(int position, bool on)
{
	if (m_version < 2) return; // not supported on revision 1 FW

	if (position < 0 || position > 7) return;
	
	if (position == 7 && on) m_apostrophes |= 1;
	else if (position == 7 && !on) m_apostrophes &= ~1;
	if (on) m_apostrophes |= (1<<(position+1));
	else m_apostrophes &= ~(1<<(position+1));

	SPI.transfer(0x93); // set apostrophes
	delay(1);
	SPI.transfer(m_apostrophes);
	delay(1);
}

void SPIDisplay::setApostrophes(bool a0, bool a1, bool a2, bool a3)
{
	if (m_version < 2) return; // not supported on revision 1 FW

	m_apostrophes = 0;
	if (a0) m_apostrophes |= 1<<1;
	if (a1) m_apostrophes |= 1<<2;
	if (a2) m_apostrophes |= 1<<3;
	if (a3) m_apostrophes |= 1<<4;

	SPI.transfer(0x93); // set apostrophes
	delay(1);
	SPI.transfer(m_apostrophes);
	delay(1);
}

void SPIDisplay::setApostrophes(bool a0, bool a1, bool a2, bool a3, bool a4, bool a5, bool a6, bool a7)
{
	m_apostrophes = 0;
	if (a0) m_apostrophes |= 1<<1;
	if (a1) m_apostrophes |= 1<<2;
	if (a2) m_apostrophes |= 1<<3;
	if (a3) m_apostrophes |= 1<<4;
	if (a4) m_apostrophes |= 1<<5;
	if (a5) m_apostrophes |= 1<<6;
	if (a6) m_apostrophes |= 1<<7;
	if (a7) m_apostrophes |= 1<<0;

	SPI.transfer(0x93); // set apostrophes
	delay(1);
	SPI.transfer(m_apostrophes);
	delay(1);
}


void SPIDisplay::setPosition(int position)
{
	setRotateMode();
	
	SPI.transfer(0x89); //  set position
	delay(1);
	SPI.transfer(position);
	delay(1);
}

void SPIDisplay::writeInt(int val)
{
	SPI.transfer(0x88);
	delay(1);
	SPI.transfer(val & 0xFF);
	delay(1);
	SPI.transfer(val >> 8);
	delay(1);
}

void SPIDisplay::writeChar(char val)
{
	SPI.transfer(val); //  set position
	delay(1);
}

void SPIDisplay::writeStr(char* val)
{
	clear();

	for (uint8_t i = 0; i < strlen(val); i++) {
		SPI.transfer(val[i]); //  set position
		delay(1);
		if (i == m_digits-1) break;
	}
}

void SPIDisplay::writeTemperature(int temp, char symbol)
{
	writeTemperature(temp, 0, symbol);
}

void SPIDisplay::writeTemperature(int temp_t, int temp_f, char symbol)
{
	setPosition(0);

	if (m_digits == 8) print(' ');

	if (temp_t >= 0) {
		if (m_digits == 8) print(' ');
		print2(temp_t);
	}
	else {
		print('-');
		print2(-temp_t);
	}

	if (m_digits == 8)
		print2(temp_f);
	else if (temp_t > 0) {
		if (temp_f > 10)
			print(temp_f/10);
		else
			print(temp_f);
	}

	print(symbol);

	if (m_digits == 8) {
		print(' ');
		setDot(2, false);
		setDot(3, true);
		setDot(4, false);
	}
	else if (temp_t > 0) {
		setDot(1, true);
	}
	else {
		setDot(1, false);
	}
}

void SPIDisplay::print2(int num)
{
  if (num < 10)
    print('0');
  print(num); 
}

void SPIDisplay::print2sp(int num)
{
  if (num < 10)
    print(' ');
  print(num); 
}

void SPIDisplay::writeTime(int hour, int min, int sec)
{
	setPosition(0);

	if (m_digits == 8) {
		print(' ');
		print2(hour);
		print2(min);
		print2(sec);
		print(' ');
		
		setDot(2, true);
		setDot(3, false);
		setDot(4, true);
	}
	else {
		print2(hour);
		print2(min);
	
		// second dot on/off
		setDot(1, sec % 2 == 0);
	}
}

void SPIDisplay::writeTime12h(int hour, int min, int sec)
{
	setPosition(0);
	if (hour == 0)  hour = 12;  // show 12 for midnight & noon

	if (m_digits == 8) {
		print(' ');
		print2sp(hour);  // use space for fill if <10
		print2(min);
		print2(sec);
		print(' ');
		
		setDot(2, true);
		setDot(3, false);
		setDot(4, true);
	}
	else {
		print2sp(hour);  // use space for fill if <10
		print2(min);
	
		// second dot on/off
		setDot(1, sec % 2 == 0);
	}
}

void SPIDisplay::writeSegments(int segments)
{
	SPI.transfer(0x84); //  receive segment data
	delay(1);
	SPI.transfer((uint8_t)segments);
	delay(1);
}

void SPIDisplay::writeSegments16(uint16_t segments)
{
	uint8_t a = segments & 0xFF;
	uint8_t b = (segments>>8) & 0xFF;

	SPI.transfer(0x94); //  receive segment data
	delay(1);
	SPI.transfer(a);
	delay(1);
	SPI.transfer(b);
	delay(1);
}

int SPIDisplay::getFirmwareRevision()
{
	uint8_t ret = 0;

	SPI.transfer(0x8a); // get firmware revision
	delay(1);
	ret = SPI.transfer(0xff);
	delay(1);

	return ret;
}

int SPIDisplay::getDigits()
{
	uint8_t ret = 0;

	SPI.transfer(0x8b); // get number of digits
	delay(1);
	ret = SPI.transfer(0xff);
	delay(1);

	return ret;
}

int SPIDisplay::getSegments()
{
	uint8_t ret = 0;

	SPI.transfer(0x8c); // get number of segments
	delay(1);
	ret = SPI.transfer(0xff);
	delay(1);

	return ret;
}

// SPIDisplay 8-digit LCD only
void SPIDisplay::setBeep(int val)
{
	if (m_version < 2) return; // not supported on revision 1 FW
	if (val < 0 || val > 2) return;

	SPI.transfer(0x91); //  receive segment data
	delay(1);
	SPI.transfer(val;
	delay(1);
}

// SPIDisplay 8-digit LCD only
void SPIDisplay::setBias(int val)
{
	if (m_version < 2) return; // not supported on revision 1 FW
	if (val != 2 && val != 3) return;

	SPI.transfer(0x92); //  receive segment data
	delay(1);
	SPI.transfer(val;
	delay(1);
}


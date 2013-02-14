/*
 * VFD Modular Clock
 * (C) 2011 Akafugu Corporation
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

#include <avr/io.h>

uint16_t calculate_segments_16(uint8_t character);

#define segA1 0b0000000000000001
#define segA2 0b0000000000000010
#define segB  0b0000000000000100
#define segC  0b0000000000001000
#define segD1 0b0000000000010000
#define segD2 0b0000000000100000
#define segE  0b0000000001000000
#define segF  0b0000000010000000
#define segG1 0b0000000100000000
#define segG2 0b0000001000000000
#define segH  0b0000010000000000
#define segI  0b0000100000000000
#define segJ  0b0001000000000000
#define segK  0b0010000000000000
#define segL  0b0100000000000000
#define segM  0b1000000000000000

uint16_t segments_16n[] = {
			segA1+segA2+segB+segC+segD1+segD2+segE+segF+segJ+segM,
			segB+segC+segJ,
			segA1+segA2+segB+segG1+segG2+segE+segD1+segD2,
			segA1+segA2+segB+segC+segD1+segD2+segG1+segG2,
			segB+segC+segG1+segG2+segF,
			segA1+segA2+segC+segD1+segD2+segF+segG1+segG2,
			segA1+segA2+segC+segD1+segD2+segE+segF+segG1+segG2,
			segA1+segA2+segB+segC+segF,
			segA1+segA2+segB+segC+segD1+segD2+segE+segF+segG1+segG2,
			segA1+segA2+segB+segC+segD1+segD2+segF+segG1+segG2,
			segA2+segB+segC+segD1+segL+segI+segE+segF,
			segB+segC+segE+segF,
			segA2+segB+segD1+segL+segG2+segE+segF,
			segA2+segB+segC+segD1+segG2+segE+segF,
			segB+segC+segI+segG2+segE+segF,
			segA2+segC+segD1+segI+segG2+segE+segF,
			segA2+segC+segD1+segL+segI+segG2+segE+segF,
			segA2+segB+segC+segI+segE+segF,
			segA2+segB+segC+segD1+segL+segI+segG2+segE+segF,
			segA2+segB+segC+segD1+segI+segG2+segE+segF,
			segA2+segB+segC+segD1+segL+segI+segH+segG1+segE+segD2
	};
uint16_t segments_16a[] = {
			segA1+segA2+segB+segC+segE+segF+segG1+segG2,
			segA1+segA2+segB+segC+segD1+segD2+segG2+segI+segL,
			segA1+segA2+segD1+segD2+segE+segF,
			segA1+segA2+segB+segC+segD1+segD2+segI+segL,
			segA1+segA2+segD1+segD2+segE+segF+segG1+segG2,
			segA1+segA2+segE+segF+segG1,
			segA1+segA2+segC+segD1+segD2+segE+segF+segG2,
			segB+segC+segE+segF+segG1+segG2,
			segA1+segA2+segI+segL+segD1+segD2,
			segB+segC+segD1+segD2+segE,
			segE+segF+segG1+segJ+segK,
			segD1+segD2+segE+segF,
			segB+segC+segE+segF+segH+segJ,
			segB+segC+segE+segF+segH+segK,
			segA1+segA2+segB+segC+segD1+segD2+segE+segF,
			segA1+segA2+segB+segE+segF+segG1+segG2,
			segA1+segA2+segB+segC+segD1+segD2+segE+segF+segK,
			segA1+segA2+segB+segE+segF+segG1+segG2+segK,
			segA1+segA2+segC+segD1+segD2+segF+segG1+segG2,
			segA1+segA2+segI+segL,
			segB+segC+segD1+segD2+segE+segF,
			segE+segF+segJ+segM,
			segB+segC+segE+segF+segK+segM,
			segH+segJ+segK+segM,
			segH+segJ+segL,
			segA1+segA2+segD1+segD2+segJ+segM
	};
	
uint16_t calculate_segments_16(uint8_t ch)
{
	uint16_t segs = 0;
	if ((ch >= 65) && (ch <= 90))
		segs = segments_16a[ch-65];  // A-Z
	else if ((ch >= 97) && (ch <= 122))
		segs = segments_16a[ch-97];  // a-z
	else if ((ch >= 48) && (ch <= 57))
		segs = segments_16n[ch-48];  // 0-9
	else if ((ch >= 0) && (ch <= 20))
		segs = segments_16n[ch];  // 0-20
	else if (ch == ' ')
		segs = 0;
	else if (ch == '-')
		segs = segG1+segG2;
	else if (ch == '+')
		segs = segG1+segG2+segI+segL;
	else if (ch == '<')
		segs = segJ+segK;
	else if (ch == '>')
		segs = segH+segM;
	else if (ch == '/')
		segs = segJ+segM;
	else
		segs = segH+segI+segJ+segK+segL+segM+segG1+segG2;  // asterisk
	return segs;
}

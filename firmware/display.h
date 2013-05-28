/*
 * VFD Modular Clock - Raspberry PI edition - SPI slave
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
 * 17Feb2013/wbp - wiring changes
 * MOVE:
 *  Strobe from PB2 to PD2 (doesn't need PWM)
 *  Blank from PB3 to PB2 (free up PB3 for speaker)
 * ADD:
 *  Piezo lines PB3 & PB4
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdbool.h>
#include <avr/io.h>

// DIGITS 1~14: HVOUT11~14
// SEGMENTS A~DP: HVOUT17~24

// HV5812 Data In (pin 27)
// HV518P Data In (pin 39)
#define DATA_BIT PB0
#define DATA_PORT PORTB
#define DATA_DDR DDRB
#define DATA_HIGH DATA_PORT |= _BV(DATA_BIT)
#define DATA_LOW DATA_PORT &= ~(_BV(DATA_BIT))

// HV5812 Clock (pin 15)
// HV518P Clock (pin 21)
#define CLOCK_BIT PB1
#define CLOCK_PORT PORTB
#define CLOCK_DDR DDRB
#define CLOCK_HIGH CLOCK_PORT |= _BV(CLOCK_BIT)
#define CLOCK_LOW CLOCK_PORT &= ~(_BV(CLOCK_BIT))

// HV5812 Latch / Strobe (pin 6)
// HV518P LE (pin ??)
#define STROBE_BIT PD2
#define STROBE_PORT PORTD
#define STROBE_DDR DDRD
#define STROBE_HIGH STROBE_PORT |= _BV(STROBE_BIT)
#define STROBE_LOW STROBE_PORT &= ~(_BV(STROBE_BIT))

//#define LATCH_ENABLE LATCH_LOW
//#define LATCH_DISABLE LATCH_HIGH

// HV5812 Blank (pin 14)
// HV518P Strobe (pin ??)
#define BLANK_BIT PB2
#define BLANK_PORT PORTB
#define BLANK_DDR DDRB
#define BLANK_HIGH BLANK_PORT |= _BV(BLANK_BIT)
#define BLANK_LOW BLANK_PORT &= ~(_BV(BLANK_BIT))

// HV5812 Piezo speaker (pins 15, 16)
#define PIEZO1_BIT PB3
#define PIEZO1_PORT PORTB
#define PIEZO1_DDR DDRB
#define PIEZO1_HIGH PIEZO1_PORT |= _BV(PIEZO1_BIT)
#define PIEZO1_LOW PIEZO1_PORT &= ~(_BV(PIEZO1_BIT))

#define PIEZO2_BIT PB4
#define PIEZO2_PORT PORTB
#define PIEZO2_DDR DDRB
#define PIEZO2_HIGH PIEZO2_PORT |= _BV(PIEZO2_BIT)
#define PIEZO2_LOW PIEZO2_PORT &= ~(_BV(PIEZO2_BIT))

// Shield signature
#define SIGNATURE_PORT  PORTD
#define SIGNATURE_DDR   DDRD
#define SIGNATURE_PIN   PIND
#define SIGNATURE_BIT_0 PD3
#define SIGNATURE_BIT_1 PD4
#define SIGNATURE_BIT_2 PD5

void display_init(uint8_t brightness);
int get_digits(void);
int get_shield(void);
void detect_shield(void);


// functions for showing settings
void show_setting_string(char* short_str, char* long_str, char* value, bool show_setting);
void show_setting_int(char* short_str, char* long_str, int value, bool show_setting);
void show_set_time(void);
void show_set_alarm(void);

void clear_screen(void);
void shift_in(char c);
void set_string(char* str);
void set_char_at(char c, uint8_t offset);
void set_number(uint16_t num);

void set_brightness(uint8_t brightness);

void set_blink(bool on);

enum shield_t {
	SHIELD_NONE = 0,
	SHIELD_IV6,
	SHIELD_IV17,
	SHIELD_IV18,
	SHIELD_IV22,
	SHIELD_IV17_6D,
};

#endif // DISPLAY_H_

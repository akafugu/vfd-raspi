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
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "display.h"

#define iv17_support
#define iv6_support
//#define iv22_support

void write_vfd_iv6(uint8_t digit, uint8_t segments);
void write_vfd_iv17(uint8_t digit, uint16_t segments);
void write_vfd_iv18(uint8_t digit, uint8_t segments);

void write_vfd_8bit(uint8_t data);
void clear_display(void);

// see font-16seg.c
uint16_t calculate_segments_16(uint8_t character);

// see font-7seg.c
uint8_t calculate_segments_7(uint8_t character);

enum shield_t shield = SHIELD_NONE;
uint8_t digits = 8;
uint8_t mpx_count = 8;
volatile char data[16]; // Digit data
volatile char segment_data[16]; // Segment data
uint8_t us_counter = 0; // microsecond counter
uint8_t multiplex_counter = 0;

// globals from main.c
extern uint8_t g_brightness;
extern uint8_t g_has_dots;
extern uint8_t g_iv18seg0;  // IV-18 segment 0 indicators

// variables for controlling display blink
uint8_t blink;
uint16_t blink_counter = 0;
uint8_t display_on = 1;

// dots [bit 0~5]
uint16_t dots = 0;

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

int get_digits(void)
{
	return digits;
}

int get_shield(void)
{
	return shield;
}

// detect which shield is connected
void detect_shield(void)
{
	// read shield bits
	uint8_t sig = 	
		(((SIGNATURE_PIN & _BV(SIGNATURE_BIT_0)) ? 0b1   : 0) |
		 ((SIGNATURE_PIN & _BV(SIGNATURE_BIT_1)) ? 0b10  : 0) |
		 ((SIGNATURE_PIN & _BV(SIGNATURE_BIT_2)) ? 0b100 : 0 ));
	// set common defaults
	mpx_count = 64;
	g_has_dots = true;
	switch (sig) {
#ifdef iv17_support
		case(1):  // IV-17 shield
			shield = SHIELD_IV17;
			digits = 4;
			mpx_count = 32;
			g_has_dots = false;
			break;
#endif
#ifdef iv6_support
		case(2):  // IV-6 shield
			shield = SHIELD_IV6;
			digits = 6;
			break;
#endif
#ifdef iv22_support
		case(6):  // IV-22 shield
			shield = SHIELD_IV22;
			digits = 4;
			break;
#endif
		case(7):  // IV-18 shield (note: same value as no shield - all bits on)
			shield = SHIELD_IV18;
			digits = 9;  // 9 including dot/dash
			mpx_count = 32;
			break;
		default:
			shield = SHIELD_NONE;
			break;
	}
}

void display_init(uint8_t brightness)
{
	// outputs
	DATA_DDR  |= _BV(DATA_BIT);
	CLOCK_DDR |= _BV(CLOCK_BIT);
	STROBE_DDR |= _BV(STROBE_BIT);
	BLANK_DDR |= _BV(BLANK_BIT);

	STROBE_LOW;
	BLANK_LOW;  // Unblank display

	clear_display();
	detect_shield();

// PD2 is Strobe
// PB2/OC0A is Blank (with 10k pullup)

// We use Timer0 for both the display multiplext interrupt
// and for PWM on OC0A for blanking

	// Inititalize Timer1 for multiplexing interrupt
//	TCCR1B = (1<<CS11); // Set Prescaler to clk/8 : 1 click = 1us. CS01=1 
//	TCCR1B = (1<<CS10); // Set Prescaler to clk/1 : 1 click = 1us. CS00=1 (8 bit timer)
//  TCCR1A |= _BV(WGM10); // Set TOP 0x00ff - makes Timer1 an 8 bit timer
//	TIMSK |= (1<<TOIE1); // Enable Overflow Interrupt
//	TCNT1 = 0;  // Initialize counter 

	// Inititalize Timer0 for PWM on PB2/OC0A (Blank)
	TCCR0B = (1<<CS00); // Set Prescaler to clk: 1 click = ???. CS00=1
  // fast PWM, 8 bit (TOP=0xff), clear OC0A (Blank pin) on match
  TCCR0A |= _BV(COM0A1) | _BV(WGM01) | _BV(WGM00);
	TIMSK |= (1<<TOIE0); // Enable Timer0 Overflow Interrupt for Strobe
	TCNT0 = 0; // Initialize counter
	
	set_brightness(10);
}

// brightness value: 0 (low) - 10 (high)
// Brightness is set by setting the PWM duty cycle for the blank
// pin of the VFD driver.
void set_brightness(uint8_t brightness) {
uint16_t brt;  // timer1/ocr1x using only 8 bits
	brt = brightness;
	if (brt > 10) brt = 10;
	brt = (10 - brt) * 25; // translate to PWM value
	OCR0A = brt;
}

void set_blink(bool on)
{
	blink = on;
	if (!blink) display_on = 1;
}

void display_multiplex(void)
{
	clear_display();
	if (display_on) {
		switch (shield) {
#ifdef iv6_support
			case SHIELD_IV6:
				write_vfd_iv6(multiplex_counter, calculate_segments_7(data[multiplex_counter]));
				break;
#endif
#ifdef iv17_support
			case SHIELD_IV17:
				write_vfd_iv17(multiplex_counter, calculate_segments_7(data[multiplex_counter]));
				break;
#endif
			case SHIELD_IV18:
				if (multiplex_counter == 8) 
					write_vfd_iv18(8, g_iv18seg0);
				else 
					write_vfd_iv18(multiplex_counter, calculate_segments_7(data[7-multiplex_counter]));
				break;
#ifdef iv22_support
			case SHIELD_IV22:
				display_multiplex_iv22();
				break;
#endif
			default:
				break;
		}
	}
	multiplex_counter++;
	if (multiplex_counter == digits) multiplex_counter = 0;
	STROBE_HIGH;  // Pulse Strobe to update Latch data
	STROBE_LOW;	
}

uint8_t interrupt_counter = 0;
uint16_t button_counter = 0;

// 1 click = 1us. Overflow every 255 us (adjust for 8 mhz clock???)
ISR(TIMER0_OVF_vect)
{
	// control blinking: on time is slightly longer than off time
	if (blink && display_on && ++blink_counter >= 0x900) {
		display_on = false;
		blink_counter = 0;
	}
	else if (blink && !display_on && ++blink_counter >= 0x750) {
		display_on = true;
		blink_counter = 0;
	}
	
	// display multiplex
	if (++interrupt_counter == mpx_count) {
		display_multiplex();
		interrupt_counter = 0;
	}
}

void set_string(char* str)
{
	if (!str) return;
	dots = 0;
	data[0] = data[1] = data[2] = data[3] = data[4] = data[5] = data[6] = data[7] = ' ';
	
	for (int i = 0; i <= digits-1; i++) {
		if (!*str) break;
		data[i] = *(str++);
	}
}

// Write 8 bits to HV5812 driver
void write_vfd_8bit(uint8_t data)
{
	// shift out 1 byte MSB first
	for (uint8_t i = 0; i < 8; i++)  {
		if (!!(data & (1 << (7 - i))))
			DATA_HIGH;
		else
			DATA_LOW;

		CLOCK_HIGH;
		CLOCK_LOW;
  }
}

#ifdef iv6_support
// Writes to the HV5812 driver for IV-6
// HV1~6:   Digit grids, 6 bits
// HV7~14:  VFD segments, 8 bits
// HV15~20: NC
void write_vfd_iv6(uint8_t digit, uint8_t segments)
{
	if (dots & (1<<digit))
		segments |= (1<<7); // DP is at bit 7
	
	uint32_t val = (1 << digit) | ((uint32_t)segments << 6);
	
//	write_vfd_8bit(0); // unused upper byte: for HV518P only
	write_vfd_8bit(val >> 16);
	write_vfd_8bit(val >> 8);
	write_vfd_8bit(val);
	
//	STROBE_HIGH;  // Strobe to update the latch
//	STROBE_LOW;
}
#endif

#ifdef iv17_support
// Writes to the HV5812 driver for IV-17
// HV1~4:  Digit grids, 4 bits
// HV 5~2: VFD segments, 16-bits
void write_vfd_iv17(uint8_t digit, uint16_t segments)
{
	uint32_t val = (1 << digit) | ((uint32_t)segments << 4);

//	write_vfd_8bit(0); // unused upper byte: for HV518P only
	write_vfd_8bit(val >> 16);
	write_vfd_8bit(val >> 8);
	write_vfd_8bit(val);

//	STROBE_HIGH;  // Strobe high
//	STROBE_LOW;  // Strobe low
}
#endif

// Writes to the HV5812 driver for IV-18
// HV1~10:  Digit grids, 10 bits
// HV11~18: VFD segments, 8 bits
// HV19~20: NC
void write_vfd_iv18(uint8_t digit, uint8_t segments)
{
	if (dots & (1<<digit))
		segments |= (1<<7); // DP is at bit 7
	
	uint32_t val = (1 << digit) | ((uint32_t)segments << 10);

//	write_vfd_8bit(0); // unused upper byte: for HV518P only
	write_vfd_8bit(val >> 16);
	write_vfd_8bit(val >> 8);
	write_vfd_8bit(val);
	
//	STROBE_HIGH;
//	STROBE_LOW;	
}


void clear_display(void)
{
//	write_vfd_8bit(0);
	write_vfd_8bit(0);
	write_vfd_8bit(0);
	write_vfd_8bit(0);

	STROBE_HIGH;
	STROBE_LOW;
}

void clear_screen(void)
{
	for (uint8_t i = 0; i < 16; i++)
		data[i] = ' ';
	g_iv18seg0 = 0;
}

void shift_in(char c)
{
	for (uint8_t i = 0; i < 7; i++) {
		data[i] = data[i+1];
		segment_data[i] = segment_data[i+1];
	}
	data[7] = c;
	segment_data[7] = 0;
}

void set_char_at(char c, uint8_t offset)
{
	data[offset] = c;
}

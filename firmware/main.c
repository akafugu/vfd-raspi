/*
 * VFD Modular Clock - Raspberry PI edition - SPI slave
 * (C) 2011-13 Akafugu Corporation
 * (C) 2013 William B Phelps
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

//#define DEMO

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include <stdbool.h>
#include <stdlib.h>

#include "usiSpiSlave.h"
#include "display.h"
#include "piezo.h"

#define SPIMODE 0	// Sample on leading _rising_ edge, setup on trailing _falling_ edge.
//#define SPIMODE 1	// Sample on leading _falling_ edge, setup on trailing _rising_ edge.

// brightness range 1-10
#ifndef DEFAULT_BRIGHTNESS
#define DEFAULT_BRIGHTNESS 10  // ???
#endif // DEFAULT_BRIGHTNESS
// volume lo/hi
#ifndef DEFAULT_VOLUME
#define DEFAULT_VOLUME 10 // HI
#endif // DEFAULT_VOLUME

uint8_t EEMEM b_brightness = DEFAULT_BRIGHTNESS;
uint8_t EEMEM b_volume = DEFAULT_VOLUME;

volatile uint8_t g_brightness = 10;
volatile uint8_t g_volume = 10;  // default loud
extern uint16_t dots;
extern uint16_t beep_counter;

uint8_t g_has_dots;
uint8_t g_iv18seg0;  // iv-18 segment 0 data

void init_EEPROM(void)
{
	eeprom_write_byte(&b_brightness, DEFAULT_BRIGHTNESS);
}

void init_SPI(void)
{
	cli();	// disable interrupts
	spiX_initslave(SPIMODE);
	sei(); // enable interrupts
}

/* ***
void show_address(uint8_t addr)
{
	uint8_t data[3];
	data[2] = addr % 10;
	addr /= 10;
	data[1] = addr % 10;
	addr /= 10;
	data[0] = addr % 10;
			
	set_char_at('A', 0);
	set_char_at('d', 1);
	set_char_at('d', 2);
	set_char_at('r', 3);
	set_char_at(' ', 4);
	set_char_at(data[0], 5);
	set_char_at(data[1], 6);
	set_char_at(data[2], 7);
}
*** */

// scroll mode
#define ROTATE 0 // use a rotating n byte buffer to store data
#define SCROLL 1  // scroll left each time a byte is received

uint8_t scroll_mode = ROTATE;
uint8_t counter = 0;

volatile unsigned char val = 0; // Dummy value to send for spi


uint8_t spi_xfer(uint8_t b)
{
	spiX_put(b);
	spiX_wait();
	return spiX_get();
}

void processSPI(void)
{
	uint8_t b, c, d;

	b = spi_xfer(0);
	
	switch (b) {
		case 0x80: // sync
			spi_xfer(0x80);  // send same code back
			break;
		case 0x81: // clear
			clear_screen();
			counter = 0;
			dots = 0;
			break;
		case 0x82: // get/set brightness
			c = spi_xfer(g_brightness);  // send old value back
			if (c > 10)
				c = 10;
			g_brightness = c;
			eeprom_write_byte(&b_brightness, c);
			set_brightness(c);
			break;
		case 0x83: // set scroll mode - 0 = ROTATE, 1 = SCROLL
			c = spi_xfer(0);
			if (c<2)
				scroll_mode = c;
			break;
#ifdef segment_data
		case 0x84: // receive segment data
			c = spi_xfer(0);

			/*
			if (scroll_mode == ROTATE) {
				set_segments_at(c, counter++);
				if (counter >= 4) counter = 0;
			}
			else {
				shift_in_segments(c);		
			}
			*/

			break;
#endif
		case 0x85: // set dots (the four bits of the second byte controls dots individually)
			dots = spi_xfer(0);
			break;
		case 0x86: // set IV-18 segment 0 indicators
			g_iv18seg0 = spi_xfer(0);
			break;

		case 0x89: // set position (only valid for ROTATE mode)
			counter = spi_xfer(0);
			break;
		case 0x8a: // get firmware revision
			spi_xfer(3);
			break;
		case 0x8b: // get number of digits
			spi_xfer(get_digits());  
			break;
		case 0x8c: // get display type
			spi_xfer(get_shield());  
			break;

		case 0x90: // get/set volume
			c = spi_xfer(g_volume);  // send old value back
			if (c>10) c = 10;
			g_volume = c;
			eeprom_write_byte(&b_volume, c);
			piezo_init();
			break;
		case 0x91: // beep tone/10, time/10
			c = spi_xfer(0);
			d = spi_xfer(0);
			beep(c*10, d*10);
			break;
		case 0x92: // get beep status - 1 if busy
			c = spi_xfer(beep_counter>0);
			break;
		case 0x93: // tick
			tick();
			break;
	
		default:
			if (b >= 0x80) break; // anything above 0x80 is considered a reserved command and is ignored

			if (scroll_mode == ROTATE) {
				set_char_at(b, counter++);
				if (counter >= 8) counter = 0;
			}
			else {
				shift_in(b);
			}
			break;
	}
}

/*
// Alarm switch changed interrupt
ISR( PCINT2_vect )
{
	if ( (SWITCH_PIN & _BV(SWITCH_BIT)) == 0)
		g_alarm_switch = false;
	else
		g_alarm_switch = true;
}
*/

void main(void) __attribute__ ((noreturn));

void main(void)
{
// We're running with an external 16 mhz resonator
// Set prescaler to divide by 2 for 8 mhz for 3.3v operation
//	CLKPR = 0b10000000 ; // PSE=1 to enable change
//  CLKPR = 0b00000001 ; // div by 2 to get 8 MHz Clock 	
	sei(); // enable interrupts

	g_brightness = eeprom_read_byte(&b_brightness);
	display_init(g_brightness);
	g_volume = eeprom_read_byte(&b_volume);

#ifdef DEMO
	set_char_at(' ', 0);
	// test: write alphabet
//	while (1) {
//	for (int j = 0; j < 5; j++) {
		for (int i = 'A'; i <= 'Z'+1; i++) {
//			set_char_at(i, 0);
			set_char_at(i+0, 1);
			set_char_at(i+1, 2);
			set_char_at(i+2, 3);
			set_char_at(i+3, 4);
			set_char_at(i+4, 5);
			set_char_at(i+5, 6);
			set_char_at(i+6, 7);
//			set_char_at(i+7, 8);
//			set_char_at(i+8, 9);
//			set_char_at(i+9, 10);
//			set_char_at(i+10, 11);
//			set_char_at(i+11, 12);
//			set_char_at(i+12, 13);
			_delay_ms(200);
		}
//	}
#endif // DEMO

	if (get_digits() == 9)
		set_string("vfdrpi10");
	else if (get_digits() == 6)
		set_string("vrpi10");
	else
		set_string("vr10");
	_delay_ms(1000);

	init_SPI();  // init SPI
	_delay_ms(200);

	piezo_init();
	beep(440, 100);
	_delay_ms(150);
	beep(1320, 100);
	_delay_ms(150);
	beep(440, 100);

	// clear display
	clear_screen();
		
	while (1) {
		processSPI();
	}
}

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

uint8_t EEMEM b_brightness = DEFAULT_BRIGHTNESS;

volatile uint8_t g_brightness = 10;
volatile int8_t g_volume = 1;  // default loud
extern uint16_t dots;

uint8_t g_has_dots;
uint8_t g_dot_flag;
uint8_t g_dash_flag;

void init_EEPROM(void)
{
	eeprom_write_byte(&b_brightness, DEFAULT_BRIGHTNESS);
}

void init_SPI(void)
{
	cli();	// disable interrupts
	spiX_initslave(SPIMODE);
	sei(); // enable interrupts
	/*
	// set up interrupt for alarm switch
	PCICR |= (1 << PCIE2);
	PCMSK2 |= (1 << PCINT18);
	*/
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
#define ROTATE 0 // use a rotating 4 byte buffer to store data
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
	uint8_t b, c;

	b = spi_xfer(0);
	
	switch (b) {
		case 0x80: // set brightness
			c = spi_xfer(0);
			set_brightness(c);
			eeprom_write_byte(&b_brightness, c);
			break;
		case 0x82: // clear
			clear_screen();
			counter = 0;
			dots = 0;
			break;
		case 0x83: // set scroll mode
			c = spi_xfer(0);
			if (c == 0)
				scroll_mode = ROTATE;
			else
				scroll_mode = SCROLL;
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
		case 0x86: // set dot indicator
			g_dot_flag = spi_xfer(0);
			break;
		case 0x87: // set dash indicator
			g_dash_flag = spi_xfer(0);
			break;
#ifdef set_number
		case 0x88: // display integer
			{
				uint8_t i1 = spi_xfer(0);
				uint8_t i2 = spi_xfer(0);
				uint16_t i = (i2 << 8) + i1;
				set_number(i);
			}
			break;
#endif
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
	CLKPR = 0b10000000 ; // PSE=1 to enable change
  CLKPR = 0b00000001 ; // div by 2 to get 8 MHz Clock 	
	sei(); // enable interrupts

//	set_brightness(eeprom_read_byte(&b_brightness));  /// redundant ???
	display_init(g_brightness);

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

	if (get_digits() == 8)
		set_string("vfdrpi10");
	else if (get_digits() == 6)
		set_string("vrpi10");
	else
		set_string("vr10");
	_delay_ms(1000);

	init_SPI();  // init SPI
	_delay_ms(200);

	piezo_init();
	beep(440, 1);
	beep(1320, 1);
	beep(440, 1);

	// clear display
	clear_screen();
		
	while (1) {
		processSPI();
	}
}

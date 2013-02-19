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

#include "piezo.h"
#include <util/delay.h>

extern uint8_t g_volume;

// piezo code from: https://github.com/adafruit/Ice-Tube-Clock
// WGM13 + WGM12 + WGM11 = Fast PWM with ICR1 as Top
void piezo_init(void) {
	PEZ_PORT &= ~_BV(PEZ1) & ~_BV(PEZ2);  // speaker off
	PEZ_DDR |= _BV(PEZ1) | _BV(PEZ2);
	TCCR1A = _BV(COM1B1) | _BV(COM1B0) | _BV(WGM11);  // set OC1B high on match, fast pwm
	TCCR1B = _BV(WGM13) | _BV(WGM12);
	if (g_volume) // high volume
		TCCR1A |= _BV(COM1A1);  // toggle both pins (clear OC1A on match)
  // start at 4khz:  250 * 8 multiplier * 4000 = 8mhz
  ICR1 = 250;
  OCR1B = OCR1A = ICR1 / 2;
}

void beep(uint16_t freq, uint16_t dur) {
  // set the PWM output to match the desired frequency
  ICR1 = (F_CPU/8)/freq;  // set Top
  // 50% duty cycle square wave
  OCR1A = OCR1B = ICR1/2;
  TCCR1B |= _BV(CS11); // connect clock/8 to turn speaker on
  // beep for the requested time
  _delay_ms(dur);
  TCCR1B &= ~_BV(CS11); // disconnect clock source to turn it off
  PEZ_PORT &= ~_BV(PEZ1) & ~_BV(PEZ2);
  // turn speaker off
  PEZ_PORT &= ~_BV(PEZ1) & ~_BV(PEZ2);
}

// This makes the speaker tick, it doesnt use PWM
// instead it just flicks the piezo
void tick(void) {
  TCCR1A = 0;  // turn pwm off 
  TCCR1B = 0;
  // Send a pulse thru both pins, alternating
  PEZ_PORT |= _BV(PEZ1);
  PEZ_PORT &= ~_BV(PEZ2);
  _delay_ms(5);
  PEZ_PORT |= _BV(PEZ2);
  PEZ_PORT &= ~_BV(PEZ1);
  _delay_ms(5);
	piezo_init();  // re-enable pwm
//  // turn them both off
//  PEZ_PORT &= ~_BV(PEZ1) & ~_BV(PEZ2);
//	// restore volume setting - 12oct12/wbp
//  TCCR1A = _BV(COM1B1) | _BV(COM1B0) | _BV(WGM11);
//  TCCR1B = _BV(WGM13) | _BV(WGM12);
//  if (g_volume) {  // 12oct12/wbp
//    TCCR1A |= _BV(COM1A1);
//  } 
}

void alarm(void)
{
	beep(500, 1);
}


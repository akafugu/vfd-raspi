// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* \li File:               spi_via_usi_driver.c
* \li Compiler:           IAR EWAAVR 3.10c
* \li Support mail:       avr@atmel.com
*
* \li Supported devices:  All devices with Universal Serial Interface (USI)
*                         capabilities can be used.
*                         The example is written for ATmega169.
*
* \li AppNote:            AVR319 - Using the USI module for SPI communication.
*
* \li Description:        Example on how to use the USI module for communicating
*                         with SPI compatible devices. The functions and variables
*                         prefixed 'spiX_' can be renamed to be able to use several
*                         spi drivers (using different interfaces) with similar names.
*                         Some basic SPI knowledge is assumed.
*
*                         $Revision: 1.4 $
*                         $Date: Monday, September 13, 2004 12:08:54 UTC $
****************************************************************************/

/*
 * USI SPI Slave
 * Based on code for Atmel appnote AVR319
 * ported to avr-gcc by Akafugu Corporation
 */

#include "usiSpiSlave.h"

/*! \brief  Data input register buffer.
 *
 *  Incoming bytes are stored in this byte until the next transfer is complete.
 *  This byte can be used the same way as the SPI data register in the native
 *  SPI module, which means that the byte must be read before the next transfer
 *  completes and overwrites the current value.
 */
unsigned char storedUSIDR;



/*! \brief  Driver status bit structure.
 *
 *  This struct contains status flags for the driver.
 *  The flags have the same meaning as the corresponding status flags
 *  for the native SPI module. The flags should not be changed by the user.
 *  The driver takes care of updating the flags when required.
 */
volatile struct usidriverStatus_t spiX_status; //!< The driver status bits.



/*! \brief  Timer/Counter 0 Compare Match Interrupt handler.
 *
 *  This interrupt handler is only enabled when transferring data
 *  in master mode. It toggles the USI clock pin, i.e. two interrupts
 *  results in one clock period on the clock pin and for the USI counter.
 */
 /*
ISR(SIG_TIMER0_COMPA)
{
	USICR |= (1<<USITC);	// Toggle clock output pin.
}
*/



/*! \brief  USI Timer Overflow Interrupt handler.
 *
 *  This handler disables the compare match interrupt if in master mode.
 *  When the USI counter overflows, a byte has been transferred, and we
 *  have to stop the timer tick.
 *  For all modes the USIDR contents are stored and flags are updated.
 */
ISR(USI_OVERFLOW_vect)
{
	/*
	// Master must now disable the compare match interrupt
	// to prevent more USI counter clocks.
	if( spiX_status.masterMode == 1 ) {
		TIMSK &= ~(1<<OCIE0A);
	}
	*/
	
	// Update flags and clear USI counter
	USISR = (1<<USIOIF);
	spiX_status.transferComplete = 1;

	// Copy USIDR to buffer to prevent overwrite on next transfer.
	storedUSIDR = USIDR;
}

/*
void spiX_initmaster( char spi_mode )
{
	// Configure port directions.
	USI_DIR_REG |= (1<<USI_DATAOUT_PIN) | (1<<USI_CLOCK_PIN); // Outputs.
	USI_DIR_REG &= ~(1<<USI_DATAIN_PIN);                      // Inputs.
	USI_OUT_REG |= (1<<USI_DATAIN_PIN);                       // Pull-ups.
	
	// Configure USI to 3-wire master mode with overflow interrupt.
	USICR = (1<<USIOIE) | (1<<USIWM0) |
	        (1<<USICS1) | (spi_mode<<USICS0) |
	        (1<<USICLK);

	// Enable 'Clear Timer on Compare match' and init prescaler.
	TCCR0A = (1<<WGM01) | TC0_PS_SETTING;
	
	// Init Output Compare Register.
	OCR0A = TC0_COMPARE_VALUE;
	
	// Init driver status register.
	spiX_status.masterMode       = 1;
	spiX_status.transferComplete = 0;
	spiX_status.writeCollision   = 0;
	
	storedUSIDR = 0;
}
*/

void spiX_initslave( char spi_mode )
{
	// Configure port directions.
	USI_DIR_REG |= (1<<USI_DATAOUT_PIN);                      // Outputs.
	USI_DIR_REG &= ~(1<<USI_DATAIN_PIN) | (1<<USI_CLOCK_PIN); // Inputs.
	USI_OUT_REG |= (1<<USI_DATAIN_PIN) | (1<<USI_CLOCK_PIN);  // Pull-ups.
	
	// Configure USI to 3-wire slave mode with overflow interrupt.
	USICR = (1<<USIOIE) | (1<<USIWM0) |
	        (1<<USICS1) | (spi_mode<<USICS0);
	
	// Init driver status register.
	spiX_status.masterMode       = 0;
	spiX_status.transferComplete = 0;
	spiX_status.writeCollision   = 0;
	
	storedUSIDR = 0;
}

char spiX_put( unsigned char val )
{
	// Check if transmission in progress,
	// i.e. USI counter unequal to zero.
	if( (USISR & 0x0F) != 0 ) {
		// Indicate write collision and return.
		spiX_status.writeCollision = 1;
		return 0;
	}
	
	// Reinit flags.
	spiX_status.transferComplete = 0;
	spiX_status.writeCollision = 0;

	// Put data in USI data register.
	USIDR = val;
	
	/*
	// Master should now enable compare match interrupts.
	if( spiX_status.masterMode == 1 ) {
		TIFR |= (1<<OCF0A);   // Clear compare match flag.
		TIMSK |= (1<<OCIE0A); // Enable compare match interrupt.
	}
	*/

	if( spiX_status.writeCollision == 0 ) return 1;
	return 0;
}

unsigned char spiX_get( void )
{
	return storedUSIDR;
}

void spiX_wait( void )
{
	do {} while( spiX_status.transferComplete == 0 );
}
 

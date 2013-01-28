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

#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* USI port and pin definitions.
 */
#define USI_OUT_REG	PORTB	//!< USI port output register.
#define USI_IN_REG	PINB	//!< USI port input register.
#define USI_DIR_REG	DDRB	//!< USI port direction register.
#define USI_CLOCK_PIN	PB7	//!< USI clock I/O pin.
#define USI_DATAIN_PIN	PB5	//!< USI data input pin.
#define USI_DATAOUT_PIN	PB6	//!< USI data output pin.

/*  Speed configuration:
 *  Bits per second = CPUSPEED / PRESCALER / (COMPAREVALUE+1) / 2.
 *  Maximum = CPUSPEED / 64.
 */
#define TC0_PRESCALER_VALUE 64	//!< Must be 1, 8, 64, 256 or 1024.
#define TC0_COMPARE_VALUE   4	//!< Must be 0 to 255. Minimum 31 with prescaler CLK/1.

#define SPI_MODE_RISING 0 //!< Sample data on _rising_ edge, setup on trailing _falling_ edge.
#define SPI_MODE_FALLING 1 //!< Sample data on _falling_ edge, setup on trailing _rising_ edge.

/*  Prescaler value converted to bit settings.
 */
#if TC0_PRESCALER_VALUE == 1
	#define TC0_PS_SETTING (1<<CS00)
#elif TC0_PRESCALER_VALUE == 8
	#define TC0_PS_SETTING (1<<CS01)
#elif TC0_PRESCALER_VALUE == 64
	#define TC0_PS_SETTING (1<<CS01)|(1<<CS00)
#elif TC0_PRESCALER_VALUE == 256
	#define TC0_PS_SETTING (1<<CS02)
#elif TC0_PRESCALER_VALUE == 1024
	#define TC0_PS_SETTING (1<<CS02)|(1<<CS00)
#else
	#error Invalid T/C0 prescaler setting.
#endif

//unsigned char storedUSIDR;

struct usidriverStatus_t {
	unsigned char masterMode : 1;       //!< True if in master mode.
	unsigned char transferComplete : 1; //!< True when transfer completed.
	unsigned char writeCollision : 1;   //!< True if put attempted during transfer.
};

/*! \brief  Initialize USI as SPI master.
 *
 *  This function sets up all pin directions and module configurations.
 *  Use this function initially or when changing from slave to master mode.
 *  Note that the stored USIDR value is cleared.
 *
 *  \param spi_mode  Required SPI mode, must be 0 or 1.
 */
//void spiX_initmaster( char spi_mode );

/*! \brief  Initialize USI as SPI slave.
 *
 *  This function sets up all pin directions and module configurations.
 *  Use this function initially or when changing from master to slave mode.
 *  Note that the stored USIDR value is cleared.
 *
 *  \param spi_mode  Required SPI mode, must be 0 or 1.
 */
void spiX_initslave( char spi_mode );


/*! \brief  Put one byte on bus.
 *
 *  Use this function like you would write to the SPDR register in the native SPI module.
 *  Calling this function in master mode starts a transfer, while in slave mode, a
 *  byte will be prepared for the next transfer initiated by the master device.
 *  If a transfer is in progress, this function will set the write collision flag
 *  and return without altering the data registers.
 *
 *  \returns  0 if a write collision occurred, 1 otherwise.
 */
char spiX_put( unsigned char val );

/*! \brief  Get one byte from bus.
 *
 *  This function only returns the previous stored USIDR value.
 *  The transfer complete flag is not checked. Use this function
 *  like you would read from the SPDR register in the native SPI module.
 */
unsigned char spiX_get( void );

/*! \brief  Wait for transfer to complete.
 *
 *  This function waits until the transfer complete flag is set.
 *  Use this function like you would wait for the native SPI interrupt flag.
 */
void spiX_wait( void );


/******************************************************************************
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    spi.c
 *
 * Description:
 *    Contains implementation of functions responsible for handling
 *    communication via SPI.
 *
 *****************************************************************************/


/************/
/* Includes */
/************/


#include "spi.h"
#include "integer.h"


/*************/
/* Variables */
/*************/


BYTE incoming;


/*****************************************************************************
 *
 * Description:
 *      Initializes SPI to work with SD cards
 *
 ****************************************************************************/
void initSpi(void){
	// setup GPIO
	SPI_IODIR |= (1 << SPI_SCK_PIN) | (1 << SPI_MOSI_PIN) | (1 << SPI_SS_PIN);
	SPI_IODIR &= ~(1 << SPI_MISO_PIN);

	// set Chip-Select high - unselect card
	UNSELECT_CARD();

	// reset Pin-Functions
	SPI_PINSEL &= ~( (3 << SPI_SCK_FUNCBIT) | (3 << SPI_MISO_FUNCBIT) |
		(3 << SPI_MOSI_FUNCBIT) | (3 << SPI_SS_FUNCBIT) );

	SPI_PINSEL |= ( (1 << SPI_SCK_FUNCBIT) | (1 << SPI_MISO_FUNCBIT) |
		(1 << SPI_MOSI_FUNCBIT) );

	// enable SPI-Master
	S0SPCR = (1 << MSTR) | (0 << CPOL);

	// low speed during init
	setSpiSpeed(254);

	CHAR i = 0;
	/* Send 20 spi commands with card not selected */
	for(i = 0; i < 21; i++) {
		spiSend(0xff);
	}

}


/*****************************************************************************
 *
 * Description:
 *      Sets SPI speed.
 *
 * Params:
 *      [in] speed - speed to set
 *
 ****************************************************************************/
void setSpiSpeed(BYTE speed){
	speed &= 0xFE;
	if (speed < SPI_PRESCALE_MIN) {
		speed = SPI_PRESCALE_MIN;
	}

	SPI_PRESCALE_REG = speed;
}


/*****************************************************************************
 *
 * Description:
 *      Sends data via SPI and returns response.
 *
 * Params:
 *      [in] toSend - data to be sent
 *
 * Returns:
 *      BYTE - data coming as response
 *
 ****************************************************************************/
BYTE spiSend(BYTE outgoing) {

	S0SPDR = outgoing;
	while( !(S0SPSR & (1 << SPIF)) ) ;
	incoming = S0SPDR;

	return incoming;
}

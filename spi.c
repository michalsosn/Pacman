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


/*************/
/* Functions */
/*************/


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
	S0SPCR = (1 << MSTR)|(0 << CPOL);

	// low speed during init
	setSpiSpeed(SPI_INITIAL_SPEED);

	tU8 i = 0;
	/* Send 20 spi commands with card not selected */
	for(i = 0; i < 21; i++)
		spiSend(0xff);

}

void setSpiSpeed(tU8 speed){
	speed &= 0xFE;
	if ( speed < SPI_PRESCALE_MIN  )
		speed = SPI_PRESCALE_MIN;

	SPI_PRESCALE_REG = speed;
}

tU8 spiSend(tU8 toSend){
    tU8 incoming = 0;

	S0SPDR = toSend;
	while( !(S0SPSR & (1 << SPIF)) ) ;

	incoming = S0SPDR;
	return incoming;
}

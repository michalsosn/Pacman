#include "spi.h"
#include "integer.h"

BYTE my_spiSend(BYTE outgoing);
//------------------------------------------------------------------------------
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
	S0SPCR = (1<<MSTR)|(0<<CPOL);

	// low speed during init
	setSpiSpeed(254); 

	CHAR i = 0;
	/* Send 20 spi commands with card not selected */
	for(i = 0; i < 21; i++)
		my_spiSend(0xff);

}

void setSpiSpeed(BYTE speed){
	speed &= 0xFE;
	if ( speed < SPI_PRESCALE_MIN  ) 
		speed = SPI_PRESCALE_MIN;
		
	SPI_PRESCALE_REG = speed;

}

BYTE my_spiSend(BYTE outgoing){
	BYTE incoming;

	S0SPDR = outgoing;
	while( !(S0SPSR & (1<<SPIF)) ) ;
	incoming = S0SPDR;

	return(incoming);
}

BYTE incoming;

BYTE spiSend(BYTE toSend){

	
	//moved, because that is faster ;)
	//SELECT_CARD();
	
	S0SPDR = toSend;
	while( !(S0SPSR & (1<<SPIF)) ) ;
	
	incoming = S0SPDR;
	
	//UNSELECT_CARD();
	
	return incoming;
}

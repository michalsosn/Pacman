/******************************************************************************
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    spi.h
 *
 * Description:
 *    Contains declarations of functions responsible for handling
 *    communication via SPI.
 *
 *****************************************************************************/


#ifndef _SPI_H
#define _SPI_H

/************/
/* Includes */
/************/


#include "pre_emptive_os/api/general.h"
#include "startup/lpc2xxx.h"


/***********/
/* Defines */
/***********/


// SP0SPCR  Bit-Definitions
#define CPHA    3
#define CPOL    4
#define MSTR    5

// SP0SPSR  Bit-Definitions
#define SPIF	7

#define SPI_IODIR      IODIR0
#define SPI_SCK_PIN    4   /* Clock*/
#define SPI_MISO_PIN   5   /* from Card*/
#define SPI_MOSI_PIN   6   /* to Card*/
#define SPI_SS_PIN	   13   /* Card-Select*/

#define SPI_PINSEL     PINSEL0
#define SPI_SCK_FUNCBIT   8
#define SPI_MISO_FUNCBIT  10
#define SPI_MOSI_FUNCBIT  12
#define SPI_SS_FUNCBIT    14

#define SPI_PRESCALE_REG  S0SPCCR
#define SPI_PRESCALE_MIN  64

#define SELECT_CARD()   IOCLR0 = (1 << SPI_SS_PIN)
#define UNSELECT_CARD()	IOSET0 = (1 << SPI_SS_PIN)

#define SPI_INITIAL_SPEED 254


/*************/
/* Functions */
/*************/

//inicjuj SPI
void initSpi(void);

//ustaw prêdkoœæ SPI
void setSpiSpeed(tU8 speed);

//wyœlij coœ na SPI
tU8 spiSend(tU8 toSend);


#endif //_SPI_H

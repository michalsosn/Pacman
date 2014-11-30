/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    lcd_hw.c
 *
 * Description:
 *    Implements hardware specific routines
 *
 *****************************************************************************/

/************/
/* Includes */
/************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include "lpc2xxx.h"
#include "lcd_hw.h"

/*************/
/* Functions */
/*************/

/*****************************************************************************
 *
 * Description:
 *    Send 9-bit data to LCD controller
 *
 ****************************************************************************/
void sendToLCD(tU8 firstBit, tU8 data) {

    // disable SPI
    IOCLR = LCD_CLK;
    PINSEL0 &= 0xffffc0ff;

    if (1 == firstBit) {
        IOSET = LCD_MOSI; // set MOSI
    } else {
        IOCLR = LCD_MOSI; // reset MOSI
    }

    // set clock high
    IOSET = LCD_CLK;

    // set clock low
    IOCLR = LCD_CLK;

    /*
     * Enable SPI again
     */
    // initialize SPI interface
    SPI_SPCCR = 0x08;
    SPI_SPCR = 0x20;

    // connect SPI bus to IO-pins
    PINSEL0 |= 0x00001500;

    // send byte
    SPI_SPDR = data;
    while (0 == (SPI_SPSR & 0x80));
}

/*****************************************************************************
 *
 * Description:
 *    Initialize the SPI interface for the LCD controller
 *
 ****************************************************************************/
void initSpiForLcd(void) {
    // make SPI slave chip select an output and set signal high
    IODIR |= (LCD_CS | LCD_CLK | LCD_MOSI);

    // deselect controller
    selectLCD(FALSE);

    // connect SPI bus to IO-pins
    PINSEL0 |= 0x00001500;

    // initialize SPI interface
    SPI_SPCCR = 0x08;
    SPI_SPCR = 0x20;
}

/*****************************************************************************
 *
 * Description:
 *    Select/deselect LCD controller (by controlling chip select signal)
 *
 ****************************************************************************/
void selectLCD(tBool select) {
    if (TRUE == select) {
        IOCLR = LCD_CS;
    } else {
        IOSET = LCD_CS;
    }
}

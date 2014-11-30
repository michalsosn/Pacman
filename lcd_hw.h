/******************************************************************************
 *
 * Copyright:
 *    (C) 2007 Embedded Artists AB
 *
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    lcd_hw.h
 *
 * Description:
 *    Expose hardware specific routines
 *
 *****************************************************************************/
#ifndef _PCA9532_H_
#define _PCA9532_H_

/************/
/* Includes */
/************/

#include "pre_emptive_os/api/general.h"
#include "startup/lpc2xxx.h"

/***********/
/* Defines */
/***********/

#define LCD_CS     0x00000080
#define LCD_CLK    0x00000010
#define LCD_MOSI   0x00000040

/*************/
/* Functions */
/*************/

void sendToLCD(tU8 firstBit, tU8 data);
void initSpiForLcd(void);
void selectLCD(tBool select);

#endif

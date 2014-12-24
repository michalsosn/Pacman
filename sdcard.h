/******************************************************************************
 *
 * File:
 *    sdcard.h
 *
 * Description:
 *    Reponsible for reading board from card.
 *
 *****************************************************************************/

#ifndef _SDCARD_H_
#define _SDCARD_H_

/************/
/* Includes */
/************/


#include "pacman.h"


/*************/
/* Functions */
/*************/

// Reads board from SD card.
tU8 readBoard(Field *board, tU8 boardHeight, tU8 boardWidth);

#endif

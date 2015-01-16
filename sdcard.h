#ifndef _SDCARD_H_
#define _SDCARD_H_

#include "pacman.h"

/*************/
/* Functions */
/*************/

// Reads board from SD card. 
tU8 readBoard(Field *board, tU8 boardHeight, tU8 boardWidth);

#endif

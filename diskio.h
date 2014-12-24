/******************************************************************************
 *
 * Copyright:
 *    Copyright (C) 2009, ChaN, all right reserved.
 *
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    diskio.h
 *
 * Description:
 *    Contains low-level I/O API for communicating with SD card
 *    required by PFF library.
 *
 *****************************************************************************/

#ifndef _DISKIO
#define _DISKIO

/************/
/* Includes */
/************/


#include "pre_emptive_os/api/general.h"


/*********/
/* Types */
/*********/


/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Function succeeded */
	RES_ERROR,		/* 1: Disk error */
	RES_NOTRDY,		/* 2: Not ready */
	RES_PARERR		/* 3: Invalid parameter */
} DRESULT;


/*************/
/* Functions */
/*************/

/* Initializes disk */
tU8 disk_initialize();

/* Reads piece of data from disk */
DRESULT disk_readp(tU8*, tU32, tU16, tU16);

#endif

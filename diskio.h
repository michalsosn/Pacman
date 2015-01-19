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
 *    Contains low-level I/O API required by PFF library.
 *
 *****************************************************************************/

#ifndef _DISKIO
#define _DISKIO

/************/
/* Includes */
/************/


#include "integer.h"


/************/
/* Defines  */
/************/


#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_NOREADY		0x03	/* card not ready */
#define STA_READY       0x00    /* card initialized and ready */

#define SECTOR_SIZE     512

/*********/
/* Types */
/*********/


/* Status of Disk Functions */
typedef BYTE	DSTATUS;


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
DSTATUS disk_initialize (void);

/* Reads piece of data from disk */
DRESULT disk_readp (BYTE*, DWORD, WORD, WORD);


#endif

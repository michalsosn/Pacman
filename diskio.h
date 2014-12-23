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


/***********/
/* Defines */
/***********/


#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_NOREADY		0x03	/* card not ready */

/* Card type flags (CardType) */
#define CT_MMC				0x01	/* MMC ver 3 */
#define CT_SD1				0x02	/* SD ver 1 */
#define CT_SD2				0x04	/* SD ver 2 */
#define CT_SDC				(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK			0x08	/* Block addressing */


/*********/
/* Types */
/*********/


/* Status of Disk Functions */
typedef tU8	DSTATUS;


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
DRESULT disk_readp (tU8*, tU32, tU16, tU16);

#endif

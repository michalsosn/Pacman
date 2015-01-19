/******************************************************************************
 *
 * Copyright:
 *    Copyright (C) 2009, ChaN, all right reserved.
 *
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    diskio.c
 *
 * Description:
 *    Contains implementations of low level I/O routines required by pff library.
 *
 *****************************************************************************/


/************/
/* Includes */
/************/


#include "diskio.h"
#include "spi.h"
#include "sd.h"
#include "startup/printf_P.h"


/***********/
/* Defines */
/***********/


#define START_BLOCK 0xfe


/*************/
/* Functions */
/*************/


/*****************************************************************************
 *
 * Description:
 *      Initializes SD card
 *
 * Returns:
 *      DSTATUS - STA_NOINIT, if initialization was not successful,
 *                STA_NOREADY, if drive is not ready
 *                STA_READY, if initialization was successful
 ****************************************************************************/
DSTATUS disk_initialize (){
	initSpi(); /*init at low speed */

	if (sdInit() < 0) {
		return STA_NOINIT;
	}

	if (sdState() < 0) {
		return STA_NOREADY;
	}

	setSpiSpeed(8);
	return STA_READY;
}


/*****************************************************************************
 *
 * Description:
 *      Reads part of sector with given number.
 *
 * Params:
 *      [out] dest - pointer to the buffer for read data
 *      [in] sector - number of sector to be read from
 *      [in] count - number of bytes to be read
 *
 * Returns:
 *      DRESULT - result of operation, RES_OK if it succeeded,
 *                RES_ERR in case of error
 ****************************************************************************/
DRESULT disk_readp (BYTE* dest,	DWORD sector, WORD sofs, WORD count) {
	DRESULT res;

	setSpiSpeed(8);

	BYTE cardresp = 0;
	BYTE firstblock = 0;
	BYTE c = 0;
	WORD fb_timeout = 0xffff;
	DWORD i = 0;

	DWORD place = SECTOR_SIZE * sector;
	sdCommand(CARD_CMD_READ, (WORD) (place >> 16), (WORD) place);

	cardresp = sdResp8b(); /* Card response */

	/* Wait for startblock */
	do {
		firstblock = sdResp8b();
	} while(CARD_BUSY == firstblock && fb_timeout--);

	if(cardresp != CARD_OK_RESP || firstblock != START_BLOCK) {
		sdResp8bError(firstblock);
		return RES_ERROR;
	}
	SELECT_CARD();
	for(i = 0; i < sofs; i++) {
		spiSend(0xff); /* Skip leading bytes */
	}

	for(i = 0; i < (SECTOR_SIZE - sofs ); i++){
		c = spiSend(0xff);

		if(i < count) {
			*dest = c;
			dest++;
		}
	}

	/* Checksum (2 byte) - ignore for now */
	spiSend(0xff);
	spiSend(0xff);
    UNSELECT_CARD();
	res = RES_OK;

	return res;
}


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
 *    Contains implementations of SD card I/O routines.
 *
 *****************************************************************************/


/************/
/* Includes */
/************/


#include "diskio.h"
#include "spi.h"
#include "startup/printf_P.h"


/***********/
/* Defines */
/***********/


#define	CMDREAD		17
#define	CMDWRITE	24
#define	CMDREADCSD	9


/*************/
/* Functions */
/*************/


static tS8 sdInit(void);
static tS8 sdState(void);
static void sdCommand(tU8 cmd, tU16 paramx, tU16 paramy);
static tU8 sdResp8b(void);
static void sdResp8bError(tU8 value);
static tU16 sdResp16b(void);

DSTATUS disk_initialize(void) {
	initSpi(); /*init at low speed */

	if(sdInit() < 0) {
		return STA_NOINIT;
	}

	if(sdState() < 0) {
		return STA_NOREADY;
	}

	setSpiSpeed(8);
	return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/
/* Pointer to the destination object */
/* Sector number (LBA) */
/* Offset in the sector */
/* tU8 count (bit15:destination) */
DRESULT disk_readp(tU8* dest, tU32 sector, tU16 sofs, tU16 count) {
	DRESULT res;
	//potrzebne by dzia³a³o z ekranem
	setSpiSpeed(8);

	tU8 cardresp = 0;
	tU8 firstblock = 0;
	tU8 c = 0;
	tU16 fb_timeout = 0xffff;
	tU32 i = 0;

	tU32 place = 512*(sector);

	sdCommand(CMDREAD, (tU16) (place >> 16), (tU16) place);

	cardresp = sdResp8b(); /* Card response */

	/* Wait for startblock */
	while (firstblock == 0xff && fb_timeout > 0) {
		firstblock = sdResp8b();
		fb_timeout--;
	}

	if(cardresp != 0x00 || firstblock != 0xfe){
		sdResp8bError(firstblock);
		return RES_ERROR;
	}
	SELECT_CARD();
	for (i = 0; i < sofs; i++) {
		spiSend(0xff); /* Skip leading bytes */
	}

	for (i = 0; i < (512 - sofs); i++) {
		c = spiSend(0xff);

		if(i < count) {
			*dest = c;
			dest++;
		}
	}

	/* Checksum (2 tU8) - ignore for now */
	spiSend(0xff);
	spiSend(0xff);
    UNSELECT_CARD();
	res = RES_OK;

	return res;
}

static tS8 sdInit(void){

	tU16 i;
	tU8 resp = 0;

	/* Try to send reset command up to 100 times */
	i = 100;
	while (resp != 1 && i > 0) {
		sdCommand(0, 0, 0);
		resp = sdResp8b();
		i--;
	}

	if(1 != resp){
		if(0xff == resp){
			return -1;
		}else{
			sdResp8bError(resp);
			return -2;
		}
	}

	/* Wait till card is ready initialising (returns 0 on CMD1) */
	/* Try up to 32000 times. */
	i = 32000;
	while (1 == resp && i > 0) {
		sdCommand(1, 0, 0);
		resp = sdResp8b();

		if (resp != 0) {
			sdResp8bError(resp);
		}

		i--;
	}

	if (resp != 0) {
		sdResp8bError(resp);
		return -3;
	}

	return 0;
}

static void sdCommand(tU8 cmd, tU16 paramx, tU16 paramy){
	SELECT_CARD();
	spiSend(0xff);

	spiSend(0x40 | cmd);
	spiSend((tU8) (paramx >> 8)); /* MSB of parameter x */
	spiSend((tU8) (paramx)); /* LSB of parameter x */
	spiSend((tU8) (paramy >> 8)); /* MSB of parameter y */
	spiSend((tU8) (paramy)); /* LSB of parameter y */

	spiSend(0x95); /* Checksum (should be only valid for first command (0) */

	spiSend(0xff); /* eat empty command - response */

	UNSELECT_CARD();

}

static tU8 sdResp8b(void) {
    tU8 i;
    tU8 resp8b;

	SELECT_CARD();
	/* Respone will come after 1 - 8 pings */
	for(i = 0; i < 8; i++){
		resp8b = spiSend(0xff);
		if(resp8b != 0xff) {
			return resp8b;
		}
	}
	UNSELECT_CARD();
	return resp8b;
}

static tU16 sdResp16b(void){
    tU16 resp16;

	SELECT_CARD();
	resp16 = (sdResp8b() << 8) & 0xff00;

	SELECT_CARD();
	resp16 |= spiSend(0xff);
	UNSELECT_CARD();
	return (resp16);
}

static void sdResp8bError(tU8 value){
	switch(value){
		case 0x40:
			printf("Argument out of bounds.\n");
			break;
		case 0x20:
			printf("Address out of bounds.\n");
			break;
		case 0x10:
			printf("Error during erase sequence.\n");
			break;
		case 0x08:
			printf("CRC failed.\n");
			break;
		case 0x04:
			printf("Illegal command.\n");
			break;
		case 0x02:
			printf("Erase reset (see SanDisk docs p5-13).\n");
			break;
		case 0x01:
			printf("Card is initialising.\n");
			break;
		default:
			printf("Unknown error 0x%x (see SanDisk docs p5-13).\n",value);
			break;
	}
}

static tS8 sdState(void){
	tU16 value;

	sdCommand(13, 0, 0);
	value = sdResp16b();

	switch(value) {
		case 0x000:
			return(1);
			break;
		case 0x0001:
			printf("Card is Locked.\n");
			break;
		case 0x0002:
			printf("WP Erase Skip, Lock/Unlock Cmd Failed.\n");
			break;
		case 0x0004:
			printf("General / Unknown error -- card broken?.\n");
			break;
		case 0x0008:
			printf("Internal card controller error.\n");
			break;
		case 0x0010:
			printf("Card internal ECC was applied, but failed to correct the data.\n");
			break;
		case 0x0020:
			printf("Write protect violation.\n");
			break;
		case 0x0040:
			printf("An invalid selection, sectors for erase.\n");
			break;
		case 0x0080:
			printf("Out of Range, CSD_Overwrite.\n");
			break;
		default:
			if(value > 0x00FF) {
				sdResp8bError((tU8) (value >> 8));
			} else {
				printf("Unknown error: 0x%x (see SanDisk docs).\n",value);
			}
			break;
	}
	return -1;
}

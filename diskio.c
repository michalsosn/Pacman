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

/* Card type flags (CardType) */
#define CT_MMC				0x01	/* MMC ver 3 */
#define CT_SD1				0x02	/* SD ver 1 */
#define CT_SD2				0x04	/* SD ver 2 */
#define CT_SDC				(CT_SD1|CT_SD2)	/* SD */
#define CT_BLOCK			0x08	/* Block addressing */


/*************/
/* Functions */
/*************/


static tU8 sdInit(void);
static tU8 sdState(void);
static void sdCommand(tU8 cmd, tU32 param);
static tU8 sdResp8b(void);
static void sdResp8bError(tU8 value);
static tU16 sdResp16b(void);


/*****************************************************************************
 *
 * Description:
 *      Initializes SD card
 *
 * Returns:
 *      tU8 - TRUE, if initialization succeeded, FALSE otherwise
 ****************************************************************************/
tU8 disk_initialize(void) {
	initSpi(); /*init at low speed */

	if(sdInit() == FALSE) {
		return FALSE;
	}

	if(sdState() == FALSE) {
		return FALSE;
	}

	setSpiSpeed(8);
	return TRUE;
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
DRESULT disk_readp(tU8* dest, tU32 sector, tU16 sofs, tU16 count) {
	DRESULT res;
	//potrzebne by dzia³a³o z ekranem
	//setSpiSpeed(8);

	tU8 cardresp = 0;
	tU8 firstblock = 0;
	tU8 c = 0;
	tU16 fb_timeout = 0xffff;
	tU32 i = 0;

	tU32 place = 512*(sector);

	sdCommand(CMDREAD, place);

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

/*****************************************************************************
 *
 * Description:
 *      Low level function responsible for initializing SD card.
 *
 * Returns:
 *      tU8 - TRUE if initialization was successful, FALSE otherwise
 *
 ****************************************************************************/
static tU8 sdInit() {

	tU16 i;
	tU8 resp = 0;

	/* Try to send reset command up to 100 times */
	i = 0;
	while (resp != 1 && i < 100) {
		sdCommand(0, 0);
		resp = sdResp8b();
		i++;
	}

	if(1 != resp){
		if(0xff == resp){
			return FALSE;
		} else {
			sdResp8bError(resp);
			return FALSE;
		}
	}

	/* Wait till card is ready initialising (returns 0 on CMD1) */
	/* Try up to 32000 times. */
	i = 0;
	while (1 == resp && i < 32000) {
		sdCommand(1, 0);
		resp = sdResp8b();

		if (resp != 0) {
			sdResp8bError(resp);
		}

		i++;
	}

	if (resp != 0) {
		sdResp8bError(resp);
		return FALSE;
	}

	return TRUE;
}

/*****************************************************************************
 *
 * Description:
 *      Sends specified command to SD card.
 *
 * Params:
 *      [in] cmd - command to be sent
 *      [in] param - command parameter
 *
 ****************************************************************************/
static void sdCommand(tU8 cmd, tU32 param){
	SELECT_CARD();
	spiSend(0xff);

	spiSend(0x40 | cmd);
	spiSend((tU8) (param >> 24)); /* MSB of first word of parameter */
	spiSend((tU8) (param >> 16)); /* LSB of first word of parameter */
	spiSend((tU8) (param >> 8)); /* MSB of second word of parameter */
	spiSend((tU8) (param)); /* LSB of second word of parameter */

	spiSend(0x95); /* Checksum (should be only valid for first command (0) */

	spiSend(0xff); /* eat empty command - response */

	UNSELECT_CARD();
}


/*****************************************************************************
 *
 * Description:
 *      Gets card's response for sent command. Its size is 1B.
 *
 * Returns:
 *      tU8 - card's response
 *
 ****************************************************************************/
static tU8 sdResp8b() {
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


/*****************************************************************************
 *
 * Description:
 *      Gets card's response for sent command. Its size is 2B.
 *
 * Returns:
 *      tU16 - card's response
 *
 ****************************************************************************/
static tU16 sdResp16b(){
    tU16 resp16;

	SELECT_CARD();
	resp16 = (sdResp8b() << 8) & 0xff00;

	SELECT_CARD();
	resp16 |= spiSend(0xff);
	UNSELECT_CARD();
	return (resp16);
}


/*****************************************************************************
 *
 * Description:
 *      Function used for debugging purposes. Prints information about error
 *      based on the error message received from card.
 *
 * Params:
 *      [in] value - error message received from card
 *
 ****************************************************************************/
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


/*****************************************************************************
 *
 * Description:
 *      Returns information about card state and
 *      prints information about errors if they occurred.
 *
 * Returns:
 *      tU8 - FALSE if some errors occurred, TRUE otherwise
 *
 ****************************************************************************/
static tU8 sdState(){
	tU16 value;

	sdCommand(13, 0);
	value = sdResp16b();

	switch(value) {
		case 0x000:
			return TRUE;
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
	return FALSE;
}

/******************************************************************************
 *
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    diskio.h
 *
 * Description:
 *    Contains implementation of SD card routines.
 *
 *****************************************************************************/


/************/
/* Includes */
/************/


#include "sd.h"
#include "spi.h"

#include "startup/printf_P.h"


/*************/
/* Variables */
/*************/


BYTE i;
BYTE resp8b;
WORD resp16;


/*************/
/* Functions */
/*************/


/*****************************************************************************
 *
 * Description:
 *      Low level function responsible for initializing SD card.
 *
 * Returns:
 *      CHAR - value greater than 0 if initialization was successful
 *             value lower than 0 in other case
 *
 ****************************************************************************/
CHAR sdInit() {
	SHORT i;
	BYTE resp;

	/* Try to send reset command up to 100 times */
	i = 100;
	do {
		sdCommand(CARD_CMD_RESET, 0, 0);
		resp = sdResp8b();
	} while(resp != CARD_RESET_RESP && i--);

	if(resp != CARD_RESET_RESP) {
		if(CARD_BUSY == resp) {
			return -1;
		} else {
			sdResp8bError(resp);
			return -2;
		}
	}

	/* Wait till card is ready initialising (returns 0 on CMD1) */
	/* Try up to 32000 times. */
	i = 32000;
	do {
		sdCommand(CARD_CMD_INIT, 0, 0);

		resp = sdResp8b();
		if (resp != CARD_OK_RESP) {
			sdResp8bError(resp);
		}
	} while(CARD_OK_RESP != resp && i--);

	if(resp != CARD_OK_RESP) {
		sdResp8bError(resp);
		return -3;
	}

	return 0;
}


/*****************************************************************************
 *
 * Description:
 *      Sends specified command to SD card.
 *
 * Params:
 *      [in] cmd - command to be sent
 *      [in] paramx - command parameter's older word
 *      [in] paramy - command parameter's younger word
 *
 ****************************************************************************/
void sdCommand(BYTE cmd, WORD paramx, WORD paramy) {
	SELECT_CARD();

	spiSend(0xff);

	spiSend(0x40 | cmd);
	spiSend((BYTE) (paramx >> 8)); /* MSB of parameter x */
	spiSend((BYTE) (paramx)); /* LSB of parameter x */
	spiSend((BYTE) (paramy >> 8)); /* MSB of parameter y */
	spiSend((BYTE) (paramy)); /* LSB of parameter y */

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
 *      BYTE - card's response
 *
 ****************************************************************************/
BYTE sdResp8b() {
	SELECT_CARD();
	/* Respone will come after 1 - 8 pings */
	for(i = 0; i < 8; i++) {
		resp8b = spiSend(0xff);
		if(resp8b != CARD_BUSY) {
			return(resp8b);
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
 *      WORD - card's response
 *
 ****************************************************************************/
WORD sdResp16b() {
	SELECT_CARD();
	resp16 = (sdResp8b() << 8) & 0xff00;

	SELECT_CARD();
	resp16 |= spiSend(0xff);
	UNSELECT_CARD();
	return resp16;
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
void sdResp8bError(BYTE value) {
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
 *      CHAR - value grater than 0 when everything is ok,
 *             value < 0 in case of some errors
 *
 ****************************************************************************/
CHAR sdState() {
	WORD value;

	sdCommand(CARD_CMD_STATUS, 0, 0);
	value = sdResp16b();

	switch(value)	{
		case 0x000:
			return 1;
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
				sdResp8bError((BYTE) (value >> 8));
			} else {
				printf("Unknown error: 0x%x (see SanDisk docs).\n", value);
			}
			break;
	}
	return -1;
}

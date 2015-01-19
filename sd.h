/******************************************************************************
 *
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    diskio.h
 *
 * Description:
 *    Contains API for communicating with SD card.
 *
 *****************************************************************************/


#ifndef _SD_H
#define _SD_H


/************/
/* Includes */
/************/


#include "integer.h"


/************/
/* Defines  */
/************/


#define CARD_CMD_RESET    0
#define CARD_CMD_INIT     1
#define CARD_CMD_STATUS   13
#define	CARD_CMD_READ	  17

#define CARD_BUSY         0xff
#define CARD_RESET_RESP   1
#define CARD_OK_RESP      0


/*************/
/* Functions */
/*************/


CHAR sdInit(void);

CHAR sdState(void);

void sdCommand(BYTE cmd, WORD paramx, WORD paramy);

BYTE sdResp8b(void);

void sdResp8bError(BYTE value);

WORD sdResp16b(void);


#endif //_SD_H

#include "sd.h"
#include "spi.h"

#include "startup/printf_P.h"


//komentarze po angielsku, takie zrobi³em na pocz¹tku nie bêde t³umaczyæ
CHAR sdInit(void){

	SHORT i;
	BYTE resp;

	/* Try to send reset command up to 100 times */
	i = 100;
	do{
		sdCommand(0, 0, 0);
		resp = sdResp8b();
	}
	while(resp != 1 && i--);

	if( resp != 1){
		if(resp == 0xff){
			return(-1);
		}else{
			sdResp8bError(resp);
			return(-2);
		}
	}

	/* Wait till card is ready initialising (returns 0 on CMD1) */
	/* Try up to 32000 times. */
	i = 32000;
	do{
		sdCommand(1, 0, 0);

		resp = sdResp8b();
		if(resp!=0)
			sdResp8bError(resp);
	}while(resp == 1 && i--);

	if( resp != 0){
		sdResp8bError(resp);
		return(-3);
	}

	return(0);

}

void sdCommand(BYTE cmd, WORD paramx, WORD paramy){
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

BYTE i;
BYTE resp8b;

BYTE sdResp8b(void) {

	SELECT_CARD();
	/* Respone will come after 1 - 8 pings */
	for(i=0;i<8;i++){
		resp8b = spiSend(0xff);
		if(resp8b != 0xff)
			return(resp8b);
	}
	UNSELECT_CARD();
	return(resp8b);
}

WORD resp16;
WORD sdResp16b(void){

	SELECT_CARD();
	resp16 = ( sdResp8b() << 8 ) & 0xff00;

	SELECT_CARD();
	resp16 |= spiSend(0xff);
	UNSELECT_CARD();
	return(resp16);
}

void sdResp8bError(BYTE value){
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

CHAR sdState(void){
	WORD value;

	sdCommand(13, 0, 0);
	value = sdResp16b();

	switch(value)	{
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
			if(value>0x00FF)
				sdResp8bError((BYTE) (value>>8));
			else
				printf("Unknown error: 0x%x (see SanDisk docs).\n",value);
			break;
	}
	return(-1);
}

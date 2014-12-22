#include "sd.h"
#include "spi.h"

#include "startup/printf_P.h"


//komentarze po angielsku, takie zrobi³em na pocz¹tku nie bêde t³umaczyæ
tS8 sdInit(void){

	tU16 i;
	tU8 resp;

	/* Try to send reset command up to 100 times */
	i = 100;
	do{
		sdCommand(0, 0, 0);
		resp = sdResp8b();
	}
	while(1 != resp && i--);

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
	do{
		sdCommand(1, 0, 0);

		resp = sdResp8b();
		if (0 != resp) {
			sdResp8bError(resp);
		}
	}while(1 == resp && i--);

	if (0 != resp) {
		sdResp8bError(resp);
		return -3;
	}

	return(0);

}

void sdCommand(tU8 cmd, tU16 paramx, tU16 paramy){
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

tU8 sdResp8b(void) {
    tU8 i;
    tU8 resp8b;

	SELECT_CARD();
	/* Respone will come after 1 - 8 pings */
	for(i=0; i<8; i++){
		resp8b = spiSend(0xff);
		if(resp8b != 0xff)
			return(resp8b);
	}
	UNSELECT_CARD();
	return(resp8b);
}

tU16 sdResp16b(void){
    tU16 resp16;

	SELECT_CARD();
	resp16 = ( sdResp8b() << 8 ) & 0xff00;

	SELECT_CARD();
	resp16 |= spiSend(0xff);
	UNSELECT_CARD();
	return (resp16);
}

void sdResp8bError(tU8 value){
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

tS8 sdState(void){
	tU16 value;

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
			if(value > 0x00FF)
				sdResp8bError((tU8) (value >> 8));
			else
				printf("Unknown error: 0x%x (see SanDisk docs).\n",value);
			break;
	}
	return -1;
}

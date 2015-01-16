/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for Petit FatFs (C)ChaN, 2009      */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "spi.h"
#include "sd.h"
#include "startup/printf_P.h"


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (void){

	//DSTATUS stat;

	initSpi(); /*init at low speed */

	
	if( sdInit() < 0 ) {
		return STA_NOINIT;
	}

	if( sdState() < 0 ){
		return STA_NOREADY;
	}

	setSpiSpeed(8);
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	BYTE* dest,			/* Pointer to the destination object */
	DWORD sector,		/* Sector number (LBA) */
	WORD sofs,			/* Offset in the sector */
	WORD count			/* Byte count (bit15:destination) */
)
{
	DRESULT res;

	//potrzebne by dzia³a³o z ekranem
	setSpiSpeed(8);


	BYTE cardresp = 0;
	BYTE firstblock = 0;
	BYTE c = 0;
	WORD fb_timeout=0xffff;
	DWORD i = 0;

	DWORD place = 512*(sector);
	sdCommand(CMDREAD, (WORD) (place >> 16), (WORD) place);

	cardresp = sdResp8b(); /* Card response */

	/* Wait for startblock */
	do
		firstblock = sdResp8b();
	while(firstblock == 0xff && fb_timeout--);

	//printf("firstblock: %x\n", firstblock);

	if(cardresp != 0x00 || firstblock != 0xfe){
		sdResp8bError(firstblock);
		return RES_ERROR;
	}
	SELECT_CARD();
	//printf("sector=%x, sofs=%x, count=%x\n", sector, sofs, count);
	for( i = 0; i < sofs; i++ )
		spiSend(0xff); /* Skip leading bytes */

	for(i=0;i < (512 - sofs );i++){
		c = spiSend(0xff);

		if( i < count )
			*dest++ = c;
	}


	/* Checksum (2 byte) - ignore for now */
	spiSend(0xff);
	spiSend(0xff);
UNSELECT_CARD();
	res = RES_OK;

	return res;
}


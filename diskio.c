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

tU8 cardresp;
tU8 firstblock;
tU8 c;
tU16 fb_timeout=0xffff;
tU32 i;
tU32 place;

DRESULT disk_readp (
	tU8* dest,			/* Pointer to the destination object */
	tU32 sector,		/* Sector number (LBA) */
	tU16 sofs,			/* Offset in the sector */
	tU16 count			/* tU8 count (bit15:destination) */
)
{
	DRESULT res;

	//potrzebne by dzia³a³o z ekranem
	setSpiSpeed(8);


	cardresp = 0;
	firstblock = 0;
	c = 0;
	fb_timeout=0xffff;
	i = 0;

	place = 512*(sector);
	sdCommand(CMDREAD, (tU16) (place >> 16), (tU16) place);

	cardresp = sdResp8b(); /* Card response */

	//printf("cmdread resp: %x\n", cardresp);

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

	for(i=0;i<(512 - sofs );i++){
		c = spiSend(0xff);

		if( i < count )
			*dest++ = c;
	}


	/* Checksum (2 tU8) - ignore for now */
	spiSend(0xff);
	spiSend(0xff);
UNSELECT_CARD();
	res = RES_OK;

	return res;
}

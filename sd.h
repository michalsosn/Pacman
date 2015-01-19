#ifndef _SD_H
#define _SD_H

#include "integer.h"

#define	CMDREAD		17
#define	CMDWRITE	24
#define	CMDREADCSD	9

//inicjuj czytnik
CHAR sdInit(void);
//sprawdü system plikÛw
CHAR sdState(void);
//pozwala na odczyt rejestru z karty i np pobranie wielkoúci karty
//z mojπ to i tak nie dzia≥a ;)
//CHAR sdgetDriveSize(void);
//wyúlij komende
void sdCommand(BYTE cmd, WORD paramx, WORD paramy);

//odpowiedü 8bitowa
BYTE sdResp8b(void);
//b≥πd jeúli nastπpi
void sdResp8bError(BYTE value);
//odpowiedü 16 bitowa
WORD sdResp16b(void);


#endif //_SD_H

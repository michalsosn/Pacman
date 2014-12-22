#ifndef _SD_H
#define _SD_H

#include "pre_emptive_os/api/general.h"

#define	CMDREAD		17
#define	CMDWRITE	24
#define	CMDREADCSD	9

//inicjuj czytnik
tS8 sdInit(void);
//sprawdü system plikÛw
tS8 sdState(void);
//pozwala na odczyt rejestru z karty i np pobranie wielkoúci karty
//z mojπ to i tak nie dzia≥a ;)
//CHAR sdgetDriveSize(void);
//wyúlij komende
void sdCommand(tU8 cmd, tU16 paramx, tU16 paramy);

//odpowiedü 8bitowa
tU8 sdResp8b(void);
//b≥πd jeúli nastπpi
void sdResp8bError(tU8 value);
//odpowiedü 16 bitowa
tU16 sdResp16b(void);


#endif //_SD_H

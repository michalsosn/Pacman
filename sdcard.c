/******************************************************************************
 *
 * File:
 *    sdcard.c
 *
 * Description:
 *    Contains definitions of functions responsible for
 *    initializing SD and reading pacman board
 *
 *****************************************************************************/


/************/
/* Includes */
/************/


#include "pff.h"
#include "startup/printf_P.h"
#include "sdcard.h"


/***********/
/* Defines */
/***********/

#define BOARD_BUFFER_SIZE 500


/*************/
/* Variables */
/*************/


tU8 boardBuffer[BOARD_BUFFER_SIZE];
tU32 result;
FATFS fatfs;


/*************/
/* Functions */
/*************/


/*****************************************************************************
 *
 * Description:
 *      Check if SD card is inserted and prepares it.
 *
 * Returns:
 *      tU8 - TRUE if card initialization succeeded
 *            FALSE if there is no card inserted or an I/O error occurred
 ****************************************************************************/
static tU8 findAndInitSD() {
	result = pf_mount(&fatfs);
	if (result) {
		if (FR_DISK_ERR == result || FR_NOT_READY == result) {
			printf("Blad interfejsu");
		} else if (FR_NO_FILESYSTEM == result) {
			printf("Nieprawidlowy system plikow lub jego brak na karcie pamieci");
		}

		return FALSE;
	}

	return TRUE;
}


/*****************************************************************************
 *
 * Description:
 *      Reads Pacman board from SD card
 *
 * Params:
 *      [out] board - pointer to the board object
 *      [in] boardHeight - board height
 *      [in] boardWidth - board width
 *
 * Returns:
 *      tU8 - TRUE if operation ended successfully, FALSE otherwise
 ****************************************************************************/
tU8 readBoard(Field *board, tU8 boardHeight, tU8 boardWidth) {

	tU8 initResult = findAndInitSD();
	if (initResult == FALSE) {
		return initResult;
	}

	printf("Proba otwarcia pliku z plansza.\n");
	result = pf_open("board.txt");
	if (result) {
		printf("Nie udalo sie otworzyc pliku.\n");
		if (FR_NO_FILE == result) {
			printf("Nie ma takiego pliku\n");
		} else if (FR_NOT_ENABLED == result) {
			printf("Problem z systemem plikow\n");
		}

		return FALSE;
	}

	printf("Proba odczytu z pliku z plansza.\n");
	tU16 bytesRead = 0;
	result = pf_read(boardBuffer, BOARD_BUFFER_SIZE, &bytesRead);
	printf("Odczyt zakonczony\n");
	if (result) {
		printf("Nie udalo sie odczytac wszystkich danych z pliku. Odczytano %d bajtow\n", bytesRead);
		return FALSE;
	} else {
		printf("Odczytano wszystkie dane z pliku.\n");
	}

	tU32 i, j = 0;
	for (i = 0; i < BOARD_BUFFER_SIZE && boardBuffer[i] != 0; ++i) {
		if (boardBuffer[i] == '\n' || boardBuffer[i] == '\r') {
			continue;
		}

		board[j] = boardBuffer[i] - '0';
		++j;
	}
	printf("Wprowadzono dane do tablicy.\n");

	return TRUE;
}


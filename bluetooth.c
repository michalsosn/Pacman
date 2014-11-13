#include "bluetooth.h"
#include "uart.h"
#include "startup/config.h"
#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include "alphalcd.h"
#include "startup/printf_P.h"

tU8 answerBuffer[MAX_ANSWER_BUFFER];

tBool waitForAnswer(char *expectedAnswer, tU8 answerLength, tU8 maxLength) {
	if (maxLength > MAX_ANSWER_BUFFER) {
		maxLength = MAX_ANSWER_BUFFER;
	}

	printf("\nOczekuje na odpowiedz: %s\n", expectedAnswer);
	printf("Przyjmuje maksymalnie %d znakow\n", maxLength);
	printf("Dane z UART1:\n");

	tU8 i;
	for (i = 0; i < maxLength; ++i) {
		answerBuffer[i] = uart1GetCh();
		printf("%c", answerBuffer[i]);
		if (i >= answerLength - 1) {
			tU8 k;
			for (k = 0; k < answerLength; ++k) {
				if (expectedAnswer[k] != answerBuffer[i - answerLength + k + 1]) {
					break;
				}
			}
			if (k == answerLength) {
				printf("\nOtrzymalem oczekiwana odpowiedz\n\n");
				return TRUE;
			}
		}
	}
	printf("\nNie otrzymalem oczekiwanej odpowiedzi\n\n");
	return FALSE;
}

void sendDataThroughBluetooth(char *text) {
	if (waitForAnswer("BTSTATE:4", 9, 250)) {
		printf("\nOtrzymalem BTSTATE:4\n");
		waitForAnswer("result", 6, 250);
		printf("\nOtrzymalem prosbe o wynik gry\n");
	} else {
		printf("\nNIE otrzymalem BTSTATE:4\n");
	}

	printf("\nProbuje wyslac: %s\n", text);
	uart1SendString(text);
	printf("\nTekst zostal przeslany\n");
}

void initBluetooth(void) {

	printf("\n --------------------------------------------------- \n");

    initUart1(B38400(CORE_FREQ / PBSD), UART_8N1, UART_FIFO_8);
    printf("\nZainicjalizowalem UART1\n");

	osSleep(5);

	// reset modem settings
	IODIR0 |= 0x00008000;
	IOSET0  = 0x00008000;
	IOCLR0  = 0x00008000;
	osSleep(2);
	IOSET0  = 0x00008000;

	// indicate Request To Send and Data Terminal Ready
	IODIR0 |= 0x00000400;  //P0.10-RTS output
	IOCLR0  = 0x00000400;
	IODIR0 |= 0x00002000;  //P0.13-DTR output
	IOSET0  = 0x00002000;
	printf("\nZrestartowalem ustawienia modemu\n");

	osSleep(25);

	uart1SendString("+++");
	printf("Wyslalem sekwencje +++\n");

	osSleep(20);
	
	waitForAnswer("OK", 2, 200);
	
	uart1SendString("\n+STAUTO=0\n");
	printf("\nWylaczylem automatyczne laczenie\n");

	waitForAnswer("STAUTO", 6, 240);
	waitForAnswer("OK", 2, 240);
	
	printf("\nUstawiam tryb Slave/Master\n");

    uart1SendString("\n+STWMOD=0\n");
	printf("\nWyslalem STWMOD\n");

	waitForAnswer("STWMOD", 6, 240);
	waitForAnswer("OK", 2, 240);
	
	printf("\nZmieniam nazwe\n");
    uart1SendString("\n+STNA=pacman\n");
    printf("\nWyslalem STNA\n");

    waitForAnswer("STNA", 4, 240);
    waitForAnswer("OK", 2, 240);
	
	osSleep(100);
	
	printf("Zezwalam na odpytywanie\n");
	uart1SendString("\n+INQ=1\n");
	printf("\nWyslalem INQ\n");

	waitForAnswer("OK", 2, 240);
	
	printf("\nZakonczylem inicjalizacje\n");
}

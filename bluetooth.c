#include "bluetooth.h"
#include "uart.h"
#include "startup/config.h"
#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include "alphalcd.h"
#include "startup/printf_P.h"

void initBluetoothMode2(void) {

	printf("Inicjalizuje Bluetooth\n");

    initUart1Mode2(B9600(CORE_FREQ / PBSD), UART_8N1, UART_FIFO_8);

    printf("Zainicjalizowano UART1\n");

	osSleep(50);

	// reset modem settings
	// IODIR0 |= 0x00008000;
	// IOSET0  = 0x00008000;
	// IOCLR0  = 0x00008000;
	// osSleep(2);
	// IOSET0  = 0x00008000;

	// indicate Request To Send and Data Terminal Ready
	// IODIR0 |= 0x00000400;  //P0.10-RTS output
	// IOCLR0  = 0x00000400;
	// IODIR0 |= 0x00002000;  //P0.13-DTR output
	// IOSET0  = 0x00002000;
	
	// osSleep(25);

	printf("Wysylam sekwencje +++\n");

	uart1SendString("+++");
	osSleep(5);

	tU8 currentChar;
	tU8 done = 0;
	
	char buffer[10];
	buffer[9] = 0;
	
	printf("Dane z UART1:\n");

	tU8 pos = 0;
	tBool ok = FALSE;
	while (ok == FALSE) {
		currentChar = uart1GetCh();
		printf("%c", currentChar);
		buffer[pos] = currentChar;

		messageOnAlpha(buffer, TRUE);
		osSleep(20);

		if (buffer[pos] == 'K') {
			tU8 temp = (pos == 0 ? 8 : pos - 1);
			if (buffer[temp] == 'O') {
				ok = TRUE;
			}
		}

		++pos;
		if (pos > 8) {
			pos = 0;
		}		
	}
	
	printf("Ustawiam tryb Slave\n");

    uart1SendString("\n+STWMOD=0\n");
	osSleep(100);

	printf("Zezwalam na odpytywanie\n");
	
    //uart1SendString("\n+STNA=pacman\n");
	//uart1SendString("\n+STBD=9600\n");
    //uart1SendString("\n+STPIN=1234\n");
	uart1SendString("\n+INQ=1\n");

	printf("Zakonczylem inicjalizacje\n");
}


void initBluetooth(void) {

	printf("\n --------------------------------------------------- \n\n");

	printf("Inicjalizuje Bluetooth\n");

    initUart1(B38400(CORE_FREQ / PBSD), UART_8N1, UART_FIFO_8);

    printf("Zainicjalizowano UART1\n");

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
	
	osSleep(25);

	printf("Wysylam sekwencje +++\n");

	uart1SendString("+++");
	osSleep(20);

	tU8 currentChar;
	tU8 done = 0;
	
	char buffer[10];
	buffer[9] = 0;
	
	printf("Dane z UART1:\n");

	tU8 pos = 0;
	tBool ok = FALSE;
	while (ok == FALSE) {
		currentChar = uart1GetCh();
		printf("%c", currentChar);
		buffer[pos] = currentChar;

		messageOnAlpha(buffer, TRUE);
		osSleep(20);

		if (buffer[pos] == 'K') {
			tU8 temp = (pos == 0 ? 8 : pos - 1);
			if (buffer[temp] == 'O') {
				ok = TRUE;
			}
		}

		++pos;
		if (pos > 8) {
			pos = 0;
		}		
	}
	
	printf("\nUstawiam tryb Slave/Master\n");

    uart1SendString("\n+STWMOD=0\n");
	osSleep(200);
	
	tU8 znak;
	tU8 current;
	for (znak = 0; znak < 60; ++znak) {
		current = uart1GetCh();
		printf("%c", current);
	}
	printf("\n");
	
	osSleep(200);
	
	printf("Zmieniam nazwe\n");
    uart1SendString("\n+STNA=pacman\n");
	for (znak = 0; znak < 60; ++znak) {
		current = uart1GetCh();
		printf("%c", current);
	}
	printf("\n");
	
	osSleep(200);
	
	uart1SendString("\n+STPIN=1234\n");
	osSleep(200);
	
	printf("Zezwalam na odpytywanie\n");
	//uart1SendString("\n+STBD=9600\n");
	uart1SendString("\n+INQ=1\n");
	for (znak = 0; znak < 60; ++znak) {
		current = uart1GetCh();
		printf("%c", current);
	}	
	printf("\n");
	
	osSleep(200);
	
	printf("Probuje sie polaczyc\n");
	//uart1SendString("\n+STBD=9600\n");
    //uart1SendString("\n+STPIN=1234\n");
    //uart1SendString("\n+CONN=50,FC,9F,5F,B8,3B\n");
	uart1SendString("\n+CONN=30,39,26,3B,B7,A3\n");
	for (znak = 0; znak < 60; ++znak) {
		current = uart1GetCh();
		printf("%c", current);
	}	
	printf("\n");
	
	printf("Zakonczylem inicjalizacje\n");
}

inline void sendDataThroughBluetooth() {
	//uart1SendString("Testowo przesyłam kilka linii danych/n")
	int i;
	for(i = 0; i < 20; ++i) {
		uart1SendString("Gratulacje\n");
		osSleep(50);
	}
}
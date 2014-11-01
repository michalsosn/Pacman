#include "bluetooth.h"
#include "uart.h"
#include "startup/config.h"
#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include "alphalcd.h"
#include "startup/printf_P.h"

void initBluetoothMode2(void) {

	printf("Inicjalizuje Bluetooth\n");

    initUart1Mode2(B38400(CORE_FREQ / PBSD), UART_8N1, UART_FIFO_8);

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

	printf("Inicjalizuje Bluetooth\n");

    initUart1(B38400(CORE_FREQ / PBSD), UART_8N1, UART_FIFO_8);

    printf("Zainicjalizowano UART1\n");

	osSleep(50);

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

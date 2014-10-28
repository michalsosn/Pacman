#include "bluetooth.h"
#include "uart.h"
#include "startup/config.h"
#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include "alphalcd.h"
#include "startup/printf_P.h"

void initBluetooth(void) {
    initUart1(B9600(CORE_FREQ / PBSD), UART_8N1, UART_FIFO_8);
	osSleep(50);
	
	uart1SendString("+++");
	osSleep(50);

	tU8 currentChar;
	tU8 done = 0;
	
	char buffer[2];
	buffer[1] = 0;
	
	printf("Dane z UART1\n");
	while (done < 2) {
		currentChar = uart1GetCh();
		printf("%c", currentChar);
		buffer[0] = currentChar;
		messageOnAlpha(buffer, TRUE);
		osSleep(100);
		
		if (currentChar == 'O') {
			done = 1;
		} else if (currentChar == 'K') {
			done = 2;
		}
	}
	
    uart1SendString("\r\n+STWMOD=0\r\n");
	osSleep(100);
	
    //uart1SendString("\r\n+STNA=pacman");
	//uart1SendString("\r\n+STBD=9600\r\n");
    //uart1SendString("\r\n+STPIN=1234\r\n");
	uart1SendString("\r\n+INQ=1\r\n");
}

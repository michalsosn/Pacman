#include "bluetooth.h"
#include "uart.h"

void initBluetooth(void) {
    initUart1(9600, UART_8N1, UART_FIFO_8);
    uart1SendString("\r\n+STWMOD=0");
    uart1SendString("\r\n\r\n+STNA=pacman");
    uart1SendString("\r\n+STPIN=1234\r\n");
}

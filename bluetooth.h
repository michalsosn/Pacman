/******************************************************************************
 *
 * File:
 *    bluetooth.h
 * 
 * Description:
 *    The library is responsible for communication over Bluetooth.
 * 
 *****************************************************************************/

#ifndef _bluetooth_h_
#define _bluetooth_h_

/***********/
/* Defines */
/***********/

#define MAX_ANSWER_BUFFER	250

/*************/
/* Functions */
/*************/

void initBluetooth(void);

void sendDataThroughBluetooth(char *text);

#endif

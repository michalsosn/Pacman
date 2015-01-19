/******************************************************************************
 *
 * File:
 *    pca9532.h
 * 
 * Description:
 *    The library is responsible for controlling LEDs on PCA9532.
 *
 *****************************************************************************/
#ifndef _PCA9532_H_
#define _PCA9532_H_

/************/
/* Includes */
/************/

#include <general.h>
#include <lpc2xxx.h>

/*************/
/* Functions */
/*************/

tBool pca9532Init(void);
void setPca9532Pin(tU8 pinNum, tU8 value);

#endif

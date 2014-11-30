/******************************************************************************
 *
 * File:
 *    eeprom.h
 * 
 * Description:
 *    The library is responsible for communication with PCA9532 module and 
 *    retrieving temperature from the sensor.
 *
 *****************************************************************************/

#ifndef _EEPROM_H
#define _EEPROM_H

/************/
/* Includes */
/************/

#include "i2c.h"

/*************/
/* Functions */
/*************/

tS8 lm75Read(tU8 address, tU8* pBuf, tU16 len);

tS8 pca9532(tU8* pBuf, tU16 len, tU8* pBuf2, tU16 len2);

#endif

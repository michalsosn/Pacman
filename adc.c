/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    adc.c
 * 
 * Description:
 *    The library is responsible for retrieving digital value of the temperature.
 * 
 *****************************************************************************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include "eeprom.h"
#include "startup/printf_P.h"
#include "startup/lpc2xxx.h"
#include "startup/consol.h"
#include "startup/config.h"

/*************/
/* Functions */
/*************/

/*****************************************************************************
 *
 * Description:
 *    Gets digital value of temperature from the sensor.
 *
 * Returns:
 *    tU16 - Temperature in degrees Celsius.
 *
 ****************************************************************************/
tU16 getTemperature(void) {
    tU8 data[3];
    tU16 temperature = 52;
    if (1 == lm75Read(0x90, &data[0], 3)) {
        temperature = (((tU16) data[0] << 8) + (tU16) data[1]) >> 7;
        printf("\nTemperature: %d.%d", temperature / 2, (temperature & 1) * 5);
    }
    return temperature / 2;
}

/******************************************************************************
 *
 * File:
 *    pca9532.c
 * 
 * Description:
 *    The library is responsible for controlling LEDs on PCA9532.
 *
 *****************************************************************************/

/************/
/* Includes */
/************/

#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <lpc2xxx.h>
#include <printf_P.h>
#include "i2c.h"
#include "eeprom.h"
#include "pca9532.h"

/*************/
/* Functions */
/*************/

/*****************************************************************************
 *
 * Description:
 *    Initialize the io-pins.
 * Returns:
 *    TRUE if connection was successful, FALSE otherwise
 *
 ****************************************************************************/
tBool pca9532Init(void) {
    tU8 initCommand[] = {0x12, 0x97, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00};
    if (I2C_CODE_OK == pca9532(initCommand, sizeof (initCommand), NULL, 0)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*****************************************************************************
 *
 * Description:
 *    Set value on selected pin
 *
 ****************************************************************************/
void setPca9532Pin(tU8 pinNum, tU8 value) {
    tU8 command[] = {0x00, 0x00};
    tU8 regValue;
    tU8 mask;

    if (pinNum < 4) {
        command[0] = 0x06;
    } else if (pinNum < 8) {
        command[0] = 0x07;
    } else if (pinNum < 12) {
        command[0] = 0x08;
    } else {
        command[0] = 0x09;
    }

    pca9532(command, 1, &regValue, 1);

    mask = (3 << 2 * (pinNum % 4));

    regValue &= ~mask;

    if (0 == value) {
        command[1] = 0x01;
    } else {
        command[1] = 0x00;
    }

    command[1] <<= 2 * (pinNum % 4);

    command[1] |= regValue;

    pca9532(command, sizeof (command), NULL, 0);
}

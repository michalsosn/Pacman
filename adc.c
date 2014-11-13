/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 *****************************************************************************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include "eeprom.h"
#include "startup/printf_P.h"
#include "startup/lpc2xxx.h"
#include "startup/consol.h"
#include "startup/config.h"

tU16 getTemperature(void) {
  tU8 data[3];
  tU16 temperature = 52;
  if (1 == lm75Read(0x90, &data[0], 3)) {
    temperature = (((tU16)data[0]<<8) + (tU16)data[1]) >> 7;
    printf("\nTemperature: %d.%d", temperature / 2, (temperature & 1) * 5);
  }
  return temperature / 2;
}

/******************************************************************************
 *
 * File:
 *    rgbled.c
 * 
 * Description:
 *    The library is responsible for setting RGB Led color.
 *
 *****************************************************************************/

#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <lpc2xxx.h>

/*************/
/* Functions */
/*************/

/*****************************************************************************
 *
 * Description:
 *    Sets color of RGB Led
 * Params:
 *    [in] r - red
 *    [in] g - green
 *    [in] b - blue
 *
 ****************************************************************************/
void setRGBLedColor(tU8 r, tU8 g, tU8 b) {
    PINSEL0 = (PINSEL0 & 0xfff03fff) | 0x000a8000; //Enable PWM2 on P0.7, PWM4 on P0.8, and PWM6 on P0.9

    PWM_PR = 0x00; // Prescale Register - PWM_TC will be incremented each tick
    PWM_MCR = 0x02; // Match Control Register - Reset on PWMMR0: the PWMTC will be reset if PWMMR0 matches it
    PWM_MR0 = 0x1000; // Period duration - 4096 tics
    PWM_MR2 = r * 4096 / 255; // Pulse duration (P0.7) red
    PWM_MR4 = b * 4096 / 255; // Pulse duration (P0.8) blue
    PWM_MR6 = g * 4096 / 255; // Pulse duration (P0.9) green
    PWM_LER = 0x55; // Latch Enable Register - Allows the last value written to the PWMMR0, PWMMR2, PWMMR4, PWMMR6 registers to be become effective when the timer is next reset by a PWM Match event.
    PWM_PCR = 0x5400; // Prescale Counter Register - PWMENA2, PWMENA4, PWMENA6 - Enable output PWM2, PWM4, PWM6, disable others, set single-edge controled mode
    PWM_TCR = 0x09; // Counter Enable and PWM Enable
}

/******************************************************************************
 *
 * File:
 *    music.c
 *
 * Description:
 *    Contains procedures for playing sounds using DAC and timer.
 *    The procedure only plays the pacman beginning sound defined in
 *    beginning_sound.c file, but it could be easily generalized to play any sound
 *    had the need arose.
 *
 *****************************************************************************/

/************/
/* Includes */
/************/

#include "pre_emptive_os/api/general.h"
#include "lpc2xxx.h"
#include "startup/config.h"

#include "music.h"
#include "music/beginning_sound.h"


/*************/
/* Functions */
/*************/

/*****************************************************************************
 *
 * Description:
 *    Writes into PINSEL1 register, settings pins 18-19 (the ones controlling DAC)
 *    enabling the second alternate option - P0.25 pin (AOUT)
 *
 ****************************************************************************/
void initDAC(void) {
	PINSEL1 &= ~0x000C0000;  		// Clear bits 18 and 19
	PINSEL1 |=  0x00080000;  		// Set them to 10 to activate AOUT
}


/*****************************************************************************
 *
 * Description:
 *    Writes to timer registers, reseting the counters and starting the timer.
 *    The timer is set to stop after the delay passed as the argument.
 *
 * Params:
 *      [in] delay - time in microseconds after which the timer will stop
 *
 ****************************************************************************/
void setTimer(int delay) {
  T1TCR = 0x02;          // sets the 2nd bit of TimerControlRegister to 1 resets the Timer and the Prescale Counter
  T1PR  = 0x00;          // writes 0 to the Prescale Register - we don't need scaling
  T1MR0 = (long) (delay * (long long) CORE_FREQ / 1000) / 1000;  // sets the Match Register to a value we expect after 90us
  T1IR  = 0xff;          // resets all bits in the Interrupt Register, probably unnecessary, but I didn't get to test it
  T1MCR = 0x04;          // sets the 3rd bit of MatchControlRegister - when the TC reaches the value in MR0, the timer
  	  	  	  	  	  	 // will stop and the 0 bit in TCR will be set to 0
  T1TCR = 0x01;          	// writes 1 to TCR[0] starting the timer
}

/*****************************************************************************
 *
 * Description:
 *    Busy-waits for the timer to stop.
 *
 ****************************************************************************/
void waitForTimer(void) {
	while (T1TCR & 0x01)		// The 1st bit of the TCR will be set to 0 when the timer reaches MR and stops.
		;
}


/*****************************************************************************
 *
 * Description:
 *    Sequentially writes all contents of pacmanBeginningSound array (see file beginning_sound.c
 *    in music directory) into DAC register. Uses timer to delay the writes to match
 *    the sound frequency.
 *
 ****************************************************************************/
void playBeginningSound(void) {
	int t;

	for (t = 0; t < BEGINNING_SOUND_LEN; ++t) {
		setTimer(BEGINNING_SOUND_DELAY);

		tS32 val;
		val = pacmanBeginningSound[t] - 128;  		// transform 8 bit values from the array
		val = val * 2;						  		// into 10 bits we must set in DAC register
		if (val > 127) {
			val = 127;
		}
		else if (val < -127) {
			val = -127;
		}

		DACR = ((val + 128) << 8)		// bits 6 - 15 (10) are for the value and determine the voltage on AOUT
			 | (1 << 16);				// the 16th bit is for BIAS, 1 means voltage needs more time to match
										// the value but the power consumption is lower

		waitForTimer();
	}
}

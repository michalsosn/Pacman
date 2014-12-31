/******************************************************************************
 *
 * File:
 *    music.h
 *
 * Description:
 *    Contains procedures for playing sounds using DAC and timer.
 *    The procedure only plays the pacman beginning sound defined in
 *    beginning_sound.c file, but it could be easily generalized to play any sound
 *    had the need arose.
 *
 *****************************************************************************/
#ifndef MUSIC_H_
#define MUSIC_H_

/*************/
/* Functions */
/*************/

// writes into PINSEL1 register, enabling P0.25 pin (AOUT)
void initDAC(void);

// writes all contents of pacmanBeginningSound array (see files in music directory) into
// DAC register, using timer to delay subsequent writes in order to match the sound frequency
void playBeginningSound(void);

#endif /* MUSIC_H_ */

/******************************************************************************
 *
 * File:
 *    beginning_sound.h
 *
 * Description:
 *    The file is a header file for an automatically generated file
 *    defining an array with bytes of the pacman intro sound,
 *    extracted from a wave file.
 *    Sounds in the program are all hard coded, distributed and loaded
 *    into memory with the binary itself.
 *
 *    The pacmanBeginningSound has the sample rate of 11025Hz, it's easy
 *    to calculate that to play it with such a frequency, we must wait for 1/f = ~90.7us
 *    before each sample. The 90us is a fine approximation.
 *
 *****************************************************************************/

#ifndef MUSIC_BEGINNING_SOUND_H_
#define MUSIC_BEGINNING_SOUND_H_

#define BEGINNING_SOUND_LEN 		46548
#define BEGINNING_SOUND_DELAY 		90

extern const char pacmanBeginningSound[46548];

#endif /* MUSIC_BEGINNING_SOUND_H_ */

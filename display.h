/******************************************************************************

 * File:
 *    display.h
 * 
 * Description:
 *    The library is responsible for displaying objects on LCD.
 *
 *****************************************************************************/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/***********/
/* Defines */
/***********/

// size of one side of the field in pixels
#define FIELD_SIZE			6

// colors of different objects on the board
#define COLOR_BACKGROUND	0x00
#define COLOR_WALL    		0x6d
#define COLOR_PACMAN  		0xfc
#define COLOR_GHOST   		0xe0
#define COLOR_POINT   		0x35
#define COLOR_BONUS   		0xb9
#define COLOR_DOORS		0xb1
#define COLOR_EYES		0xff
#define COLOR_EYES_BORDER       0xf6
#define COLOR_EATABLE_GHOST     0x5b

/*************/
/* Functions */
/*************/

// display game menu
void displayMenu(void);

// display given text on the screen
void displayText(char *text);

// display different types of fields
void displayEmptyField(tU8 x, tU8 y);
void displayWall(tU8 x, tU8 y);
void displayPoint(tU8 x, tU8 y);
void displayBonus(tU8 x, tU8 y);
void displayDoors(tU8 x, tU8 y);

// display different types of characters
void displayGhost(tU8 x, tU8 y);
void displayPacman(tU8 x, tU8 y);
void displayEatableGhost(tU8 x, tU8 y);
void displayEyes(tU8 x, tU8 y);

#endif

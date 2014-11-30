/******************************************************************************

 * File:
 *    display.c
 * 
 * Description:
 *    The library is responsible for displaying objects on LCD.
 *
 *****************************************************************************/

/************/
/* Includes */
/************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"

#include "display.h"
#include "lcd.h"


/***********/
/* Defines */
/***********/

// colors for menu and messages
#define COLOR_GLOBAL_BACKGROUND		0x00
#define COLOR_MENU_BACKGROUND		0x00
#define COLOR_MESSAGE_BACKGROUND	0x00
#define COLOR_TEXT					0xff

/*************/
/* Functions */
/*************/

/*****************************************************************************
 *
 * Description:
 *    Displays game menu. 
 *
 ****************************************************************************/
void displayMenu(void) {

    // sets background and text color
    lcdColor(COLOR_GLOBAL_BACKGROUND, COLOR_TEXT);
    // paints the screen with background color
    lcdClrscr();

    lcdRect(0, 20, 128, 88, COLOR_MENU_BACKGROUND);

    lcdGotoxy(10, 54);
    lcdPuts("Press joystick");
}

/*****************************************************************************
 *
 * Description:
 *    Displays text on the screen.
 * 
 * Params:
 *    [in] text - text to be displayed 
 *
 ****************************************************************************/
void displayText(char *text) {

    lcdRect(0, 60, 128, 30, COLOR_MESSAGE_BACKGROUND);

    lcdGotoxy(10, 70);
    lcdPuts(text);
}

/*****************************************************************************
 *
 * Description:
 *    The functions below display different types of game objects.
 * 
 * Params:
 *    [in] x - x coordinate of the left side of object
 *    [in] y - y coordinate of the upper side of object
 *
 ****************************************************************************/

void displayEmptyField(tU8 x, tU8 y) {
    lcdRect(x, y, FIELD_SIZE, FIELD_SIZE, COLOR_BACKGROUND);
}

void displayWall(tU8 x, tU8 y) {
    lcdRect(x, y, FIELD_SIZE, FIELD_SIZE, COLOR_WALL);
}

void displayPoint(tU8 x, tU8 y) {
    displayEmptyField(x, y);
    lcdRect(x + 2, y + 2, 2, 2, COLOR_POINT);
}

void displayBonus(tU8 x, tU8 y) {
    displayEmptyField(x, y);
    lcdRect(x + 1, y + 2, 4, 2, COLOR_BONUS);
    lcdRect(x + 2, y + 1, 2, 4, COLOR_BONUS);
}

void displayDoors(tU8 x, tU8 y) {
    lcdRect(x, y, FIELD_SIZE, FIELD_SIZE, COLOR_DOORS);
}

void displayGhost(tU8 x, tU8 y) {
    lcdRect(x + 2, y, 2, 1, COLOR_GHOST);
    lcdRect(x, y + 2, 1, 4, COLOR_GHOST);
    lcdRect(x + 5, y + 2, 1, 4, COLOR_GHOST);
    lcdRect(x + 1, y + 1, 4, 4, COLOR_GHOST);
    lcdRect(x + 1, y + 2, 1, 1, COLOR_EYES);
    lcdRect(x + 4, y + 2, 1, 1, COLOR_EYES);
}

void displayPacman(tU8 x, tU8 y) {
    lcdRect(x, y + 2, 6, 2, COLOR_PACMAN);
    lcdRect(x + 2, y, 2, 6, COLOR_PACMAN);
    lcdRect(x + 1, y + 1, 4, 4, COLOR_PACMAN);
}

void displayEatableGhost(tU8 x, tU8 y) {
    lcdRect(x + 2, y, 2, 1, COLOR_EATABLE_GHOST);
    lcdRect(x, y + 2, 1, 4, COLOR_EATABLE_GHOST);
    lcdRect(x + 5, y + 2, 1, 4, COLOR_EATABLE_GHOST);
    lcdRect(x + 1, y + 1, 4, 4, COLOR_EATABLE_GHOST);
    lcdRect(x + 1, y + 3, 1, 1, COLOR_EYES);
    lcdRect(x + 4, y + 3, 1, 1, COLOR_EYES);
}

void displayEyes(tU8 x, tU8 y) {
    lcdRect(x, y + 1, 4, 4, COLOR_EYES_BORDER);
    lcdRect(x + 3, y + 2, 4, 4, COLOR_EYES_BORDER);
    lcdRect(x + 1, y + 2, 1, 1, COLOR_EYES);
    lcdRect(x + 4, y + 3, 1, 1, COLOR_EYES);
}

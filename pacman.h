#ifndef _PACMAN_H_
#define _PACMAN_H_

/************/
/* Includes */
/************/

//#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"

/***********/
/* Defines */
/***********/
// game constants
#define NUMBER_OF_GHOSTS    4
#define BOARD_WIDTH         21
#define BOARD_HEIGHT        21
#define INIT_TIME_TO_EAT    31

/*********/
/* Types */
/*********/

typedef enum {LEFT, RIGHT, UP, DOWN} Direction;
typedef enum {EMPTY, WALL, POINT, BONUS, DOORS} Field;
typedef enum {PACMAN, GHOST, EATABLE_GHOST, EYES} CharacterType;

typedef struct {
    tU8 x;
    tU8 y;
} Coordinates;

typedef struct {
    CharacterType type;
    Coordinates from;
    Coordinates to;
} Move;

typedef struct character {
    CharacterType type;
    Coordinates position;
    Coordinates birthplace;
    Direction homeDirection;
    Direction currentDirection;
    Direction nextDirection;
    tU8 startTime;
    tU8 timeToStart;
    Direction (*defaultUpdateDirection)(struct character *c);
    Direction (*updateDirection)(struct character *c);
} Character;

/********************/
/* Extern variables */
/********************/

// Contains static elements of the board.
extern Field board[BOARD_HEIGHT][BOARD_WIDTH];

/*************/
/* Functions */
/*************/

// Initializes game
void initPacman(tU8 useDefaultBoard);

// Registers function for changing pacman direction
void setDirectionCallback(Direction (*updateDirection)(struct character *c));

// Lets you replace default movement rules of ghosts
void setGhostDirectionCallback(tU8 ghost, Direction (*updateDirection)(struct character *c));

// Sets handler for game lost event
void onGameLost(void (*handler)(tU8 level, tU8 score));

// Sets handler for life lost event
void onLifeLost(void (*handler)(tU8 lifes));

// Sets handler for score changed event
void onScoreChanged(void (*handler)(tU8 score));

// Sets handler for time to eat changed event
void onTimeToEatChanged(void (*handler)(tU8 remainingTime));

// Sets handler for level complete event
void onLevelCompleted(void (*handler)(tU8 level, tU8 score));

// Sets handler for ghost eaten event
void onGhostEaten(void (*handler)());

// Calculates next positions of all characters,
// Includes collision detection
// Returns array of moves to display
Move *makeMove();
#endif

/******************************************************************************
 *
 * File:
 *    pacman.h
 * 
 * Description:
 *    Responsible for game logic
 *
 *****************************************************************************/
#ifndef _PACMAN_H_
#define _PACMAN_H_

/************/
/* Includes */
/************/

#include "pre_emptive_os/api/general.h"

/***********/
/* Defines */
/***********/

// game constants
#define NUMBER_OF_GHOSTS     4
#define BOARD_WIDTH         21
#define BOARD_HEIGHT        21
#define INIT_TIME_TO_EAT    31
#define POINTS_FOR_EATING   10
#define POINTS_FOR_BONUS     5

// default initial values
#define INIT_LIVES           3
#define INIT_LEVEL           0
#define INIT_SCORE           0
#define INIT_SEED          128

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

extern Field board[BOARD_HEIGHT][BOARD_WIDTH];

/*************/
/* Functions */
/*************/

void initPacman(tU8 useDefaultBoard);
void setDirectionCallback(Direction (*updateDirection)(struct character *c));
void setGhostDirectionCallback(tU8 ghost, Direction (*updateDirection)(struct character *c));
void onGameLost(void (*handler)(tU8 level, tU8 score));
void onLifeLost(void (*handler)(tU8 lifes));
void onScoreChanged(void (*handler)(tU8 score));
void onTimeToEatChanged(void (*handler)(tU8 remainingTime));
void onLevelCompleted(void (*handler)(tU8 level, tU8 score));
void onGhostEaten(void (*handler)());
Move *makeMove();

#endif

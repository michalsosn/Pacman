/******************************************************************************
 *
 * File:
 *    pacman.c
 * 
 * Description:
 *    Responsible for game logic
 *
 *****************************************************************************/

/************/
/* Includes */
/************/

#include "pacman.h"
#include "startup/printf_P.h"

/*************/
/* Variables */
/*************/

// Characters
static Character pacman;
static Character ghosts[NUMBER_OF_GHOSTS];

// Contains static elements of the board.
Field defaultBoard[BOARD_HEIGHT][BOARD_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1},
    {1, 3, 1, 2, 2, 2, 2, 2, 1, 2, 1, 2, 1, 2, 2, 2, 2, 2, 1, 3, 1},
    {1, 2, 1, 2, 1, 2, 1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 2, 1, 2, 1},
    {1, 2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 2, 1},
    {1, 2, 1, 2, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 2, 1, 2, 1},
    {1, 2, 2, 2, 2, 2, 0, 0, 1, 1, 4, 1, 1, 0, 0, 2, 2, 2, 2, 2, 1},
    {1, 1, 1, 2, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 2, 1, 1, 1},
    {1, 2, 2, 2, 2, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 2, 2, 2, 2, 1},
    {1, 2, 1, 1, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 2, 1},
    {1, 2, 2, 1, 2, 2, 1, 1, 1, 1, 0, 1, 1, 1, 1, 2, 2, 1, 2, 2, 1},
    {1, 1, 2, 1, 1, 2, 2, 2, 2, 1, 0, 1, 2, 2, 2, 2, 1, 1, 2, 1, 1},
    {1, 2, 2, 1, 1, 1, 1, 1, 2, 1, 0, 1, 2, 1, 1, 1, 1, 1, 2, 2, 1},
    {1, 2, 1, 1, 2, 2, 2, 1, 2, 1, 0, 1, 2, 1, 2, 2, 2, 1, 1, 2, 1},
    {1, 3, 2, 2, 2, 1, 2, 2, 2, 2, 0, 2, 2, 2, 2, 1, 2, 2, 2, 3, 1},
    {1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

Field board[BOARD_HEIGHT][BOARD_WIDTH];

//game state indicators
static tU8 ghostEatingMode;
static tU8 moveToInitPositions;

static tU8 defaultBoardUsed;

//game counters
static tU8 level;
static tU8 lives;
static tU8 score;
static tU8 pointsToCompleteLevel;

//seed for random function
static int seed;

/************/
/* Handlers */
/************/

static void (*handleGameLost)(tU8 level, tU8 score);
static void (*handleLifeLost)(tU8 lives);
static void (*handleScoreChanged)(tU8 score);
static void (*handleLevelComplete)(tU8 level, tU8 score);
static void (*handleGhostEaten)(void);
static void (*handleTimeToEatChanged)(tU8 remainingTime);

/*************/
/* Functions */
/*************/

/*****************************************************************************
 *
 * Description:
 *    Generates seed for random() function
 *
 ****************************************************************************/
static void generateSeed() {
    seed = seed * (lives * score * pointsToCompleteLevel
                   + pacman.position.x * ghosts[0].position.x * ghosts[1].position.x * ghosts[2].position.x  * ghosts[3].position.x
                   + pacman.position.y * ghosts[0].position.y * ghosts[1].position.y * ghosts[2].position.y  * ghosts[3].position.y);
}

/*****************************************************************************
 *
 * Description:
 *    Generates pseudo random number
 *
 * Returns:
 *    tU16 - pseudo random number
 *
 ****************************************************************************/
static tU16 random() {
    seed = (seed * 1103515245 + 12345) % 2147483647;
    return seed;
}

/*****************************************************************************
 *
 * Description:
 *    Calculates position after move in given direction
 *
 * Params:
 *    [in] coords - current position
 *    [in] dir - direction
 *
 * Returns:
 *    Coordinates - calculated coordinates
 *
 ****************************************************************************/
inline static Coordinates calculateMove(Coordinates coords, Direction dir) {
    Coordinates newCoords = coords;

    switch (dir) {
        case LEFT: newCoords.x--; break;
        case RIGHT: newCoords.x++; break;
        case UP: newCoords.y--; break;
        case DOWN: newCoords.y++; break;
    }

    return newCoords;
}

/*****************************************************************************
 *
 * Description:
 *    Checks if given character can move to given position
 *
 * Params:
 *    [in] coords - destination coordinates
 *    [in] type - type of character
 *
 * Returns:
 *    tU8 - TRUE if given character can move to given position, FALSE if not
 *
 ****************************************************************************/
inline static tU8 canMove(Coordinates coords, CharacterType type) {
    if (WALL == board[coords.y][coords.x]) {
        return FALSE;
    }
    if (PACMAN == type && DOORS == board[coords.y][coords.x]) {
        return FALSE;
    }
    return TRUE;
}

/*****************************************************************************
 *
 * Description:
 *    Calculates direction after turning back
 *
 * Params:
 *    [in] dir - direction before turn
 *
 * Returns:
 *    Direction - direction after turn
 *
 ****************************************************************************/
inline static Direction turnBack(Direction dir) {
    switch (dir) {
        case LEFT: return RIGHT;
        case DOWN: return UP;
        case RIGHT: return LEFT;
        default: return DOWN;
    }
}

/*****************************************************************************
 *
 * Description:
 *    Changes position of given character according to its direction
 *
 * Params:
 *    [in] character - a moving character
 *
 * Returns:
 *    Move - structure describing character's move
 *
 ****************************************************************************/
static Move move(Character *character) {
    if (character->updateDirection)
        character->nextDirection = character->updateDirection(character);

    Move move;

    move.type = character->type;
    move.from = character->position;
    move.to = calculateMove(character->position, character->nextDirection);

    if (canMove(move.to, move.type)) {
        character->currentDirection = character->nextDirection;
    } else {
        move.to = calculateMove(character->position, character->currentDirection);
        if (!canMove(move.to, move.type)) {
            move.to = move.from;
        }
    }
    character->position = move.to;

    return move;
}

/*****************************************************************************
 *
 * Description:
 *    Default updating direction function for ghosts.
 *    Enables them to exit home.
 *
 * Params:
 *    [in] c - a moving character
 *
 * Returns:
 *    Direction - next character's direction
 *
 ****************************************************************************/
static Direction defaultExitHome(Character *c) {
    if (10 == c->position.x && 8 == c->position.y) {
        return UP;
    }
    if (10 == c->position.x && 7 == c->position.y) {
        return UP;
    }
    if (9 == c->position.x && 8 == c->position.y) {
        return RIGHT;
    }
    if (11 == c->position.x && 8 == c->position.y) {
        return LEFT;
    }
    if (10 == c->position.x && 6 == c->position.y) {
        c->updateDirection = c->defaultUpdateDirection;
    }
    return LEFT;
}

/*****************************************************************************
 *
 * Description:
 *    Default updating direction function for ghosts.
 *    Enables them to stay at home until startTime.
 *
 * Params:
 *    [in] c - a moving character
 *
 * Returns:
 *    Direction - next character's direction
 *
 ****************************************************************************/
static Direction defaultStayAtHome(Character *c) {
    if (!c->timeToStart) {
        generateSeed();
        c->updateDirection = defaultExitHome;
        return defaultExitHome(c);
    } else {
        c->timeToStart--;
        return c->homeDirection;
    }
}

/*****************************************************************************
 *
 * Description:
 *    Default updating direction function for ghosts.
 *    Enables them to go back home.
 *
 * Params:
 *    [in] c - a moving character
 *
 * Returns:
 *    Direction - next character's direction
 *
 ****************************************************************************/
static Direction defaultGoBackHome(Character *c) {
    if (c->position.x == c->birthplace.x && c->position.y == c->birthplace.y) {
        c->type = GHOST;
        c->updateDirection = defaultExitHome;
        return defaultExitHome(c);
    }
	
	if(!defaultBoardUsed) {
	    return c->defaultUpdateDirection(c);
	}

    if(!defaultBoardUsed) {
        return c->defaultUpdateDirection(c);
    }

    if (7 == c->position.y) {
        switch (c->position.x) {
            case 3:
            case 5:
                return RIGHT;
            case 7:
            case 13:
                return UP;
            case 15:
            case 17:
                return LEFT;
            case 10:
                return DOWN;
        }
    } else if (6 == c->position.y) {
        switch (c->position.x) {
            case 7:
                return RIGHT;
            case 13:
                return LEFT;
            case 10:
                return DOWN;
        }
    } else if (9 == c->position.y) {
        switch (c->position.x) {
            case 4:
                return RIGHT;
            case 16:
                return LEFT;
        }
    } else if (15 == c->position.y) {
        switch (c->position.x) {
            case 8:
            case 12:
                return UP;
        }
    } else if (8 == c->position.y && 10 == c->position.x) {
        Direction dir = 0;
        Coordinates coords = calculateMove(c->position, dir);
        while (coords.x != c->birthplace.x || coords.y != c->birthplace.y) {
            dir++;
            coords = calculateMove(c->position, dir);
        }
        return dir;
    }
    return c->defaultUpdateDirection(c);
}

/*****************************************************************************
 *
 * Description:
 *    Default updating direction function for ghosts.
 *    Enables them to move randomly, but without turning back.
 *
 * Params:
 *    [in] c - a moving character
 *
 * Returns:
 *    Direction - next character's direction
 *
 ****************************************************************************/
static Direction defaultGhostMovement(Character *c) {
    if (10 == c->position.x && 6 == c->position.y) {
        if(LEFT == c->currentDirection) {
            return LEFT;
		}
		return RIGHT;
    }
    Direction ranDir = random() % 4;
    Coordinates coords;
	tU8 canMoveBack = FALSE;
    do {
        ranDir = (ranDir + 1) % 4;
        if (c->currentDirection == turnBack(ranDir) && !canMoveBack) {
            ranDir++;
			canMoveBack = TRUE;
        }
        coords = calculateMove(c->position, ranDir);
    } while (!canMove(coords, c->type));
    return ranDir;
}

/*****************************************************************************
 *
 * Description:
 *    Calculates the number of points needed to complete the level
 *
 * Returns:
 *    int - number of points needed to complete the level
 *
 ****************************************************************************/
int calculatePointsToComplete() {
	int result = 0;
	int i, j;
	for (i = 0; i < BOARD_HEIGHT; ++i) {
		for (j = 0; j < BOARD_WIDTH; ++j) {
			if (board[i][j] == POINT) {
				++result;
			}
		}
	}
	return result;
}

/*****************************************************************************
 *
 * Description:
 *    Initializes board, characters and game counters with default values.
 *
 * Params:
 *    [in] useDefaultBoard - value that specifies if default board should be used
 *
 ****************************************************************************/
void initPacman(tU8 useDefaultBoard) {
	printf("InitPacman rozpoczete\n");
	defaultBoardUsed = useDefaultBoard;
	if (useDefaultBoard) {
		int row, column;
		for (row = 0; row < BOARD_HEIGHT; ++row) {
			for (column = 0; column < BOARD_WIDTH; ++column) {
				board[row][column] = defaultBoard[row][column];
			}
		}
	}
	printf("Plansza wczytana do odpowiedniej tablicy\n");

    ghostEatingMode = FALSE;
    moveToInitPositions = TRUE;
    level = INIT_LEVEL;
    lives = INIT_LIVES;
    score = INIT_SCORE;
    seed = INIT_SEED;
	
	pointsToCompleteLevel = calculatePointsToComplete();

    pacman.birthplace.x = 10;
    pacman.birthplace.y = 15;
    pacman.currentDirection = LEFT;
    pacman.nextDirection = LEFT;
    pacman.position.x = 10;
    pacman.position.y = 15;
    pacman.type = PACMAN;
	
	if (!pacman.updateDirection) {
		pacman.updateDirection = defaultGhostMovement;
	}

    ghosts[0].birthplace.x = 10;
    ghosts[0].birthplace.y = 6;
    ghosts[0].currentDirection = RIGHT;
    ghosts[0].nextDirection = LEFT;
    ghosts[0].homeDirection = LEFT;
    ghosts[0].startTime = 0;
    ghosts[0].timeToStart = 0;
    ghosts[0].position.x = 10;
    ghosts[0].position.y = 6;
    ghosts[0].type = GHOST;
    ghosts[0].updateDirection = defaultStayAtHome;
    ghosts[0].defaultUpdateDirection = defaultGhostMovement;

    ghosts[1].birthplace.x = 10;
    ghosts[1].birthplace.y = 8;
    ghosts[1].currentDirection = DOWN;
    ghosts[1].nextDirection = DOWN;
    ghosts[1].homeDirection = DOWN;
    ghosts[1].startTime = 8;
    ghosts[1].timeToStart = 8;
    ghosts[1].position.x = 10;
    ghosts[1].position.y = 8;
    ghosts[1].type = GHOST;
    ghosts[1].updateDirection = defaultStayAtHome;
    ghosts[1].defaultUpdateDirection = defaultGhostMovement;

    ghosts[2].birthplace.x = 9;
    ghosts[2].birthplace.y = 8;
    ghosts[2].currentDirection = LEFT;
    ghosts[2].nextDirection = LEFT;
    ghosts[2].homeDirection = LEFT;
    ghosts[2].startTime = 15;
    ghosts[2].timeToStart = 15;
    ghosts[2].position.x = 9;
    ghosts[2].position.y = 8;
    ghosts[2].type = GHOST;
    ghosts[2].updateDirection = defaultStayAtHome;
    ghosts[2].defaultUpdateDirection = defaultGhostMovement;

    ghosts[3].birthplace.x = 11;
    ghosts[3].birthplace.y = 8;
    ghosts[3].currentDirection = RIGHT;
    ghosts[3].nextDirection = RIGHT;
    ghosts[3].homeDirection = RIGHT;
    ghosts[3].startTime = 25;
    ghosts[3].timeToStart = 25;
    ghosts[3].position.x = 11;
    ghosts[3].position.y = 8;
    ghosts[3].type = GHOST;
    ghosts[3].updateDirection = defaultStayAtHome;
    ghosts[3].defaultUpdateDirection = defaultGhostMovement;

    if (handleLifeLost) {
		handleLifeLost(lives);
	}
}

/*****************************************************************************
 *
 * Description:
 *    Sets callback for changing pacman's direction
 *
 * Params:
 *    [in] updateDirection - pointer to function to be used as a callback
 *
 ****************************************************************************/
void setDirectionCallback(Direction (*updateDirection)(struct character *)) {
    pacman.updateDirection = updateDirection;
}

/*****************************************************************************
 *
 * Description:
 *    Sets callback for changing ghost's direction
 *
 * Params:
 *    [in] ghost - number indicating for which ghost callback will be used
 *    [in] updateDirection - pointer to function to be used as a callback
 *
 ****************************************************************************/
void setGhostDirectionCallback(tU8 ghost, Direction (*updateDirection)(struct character *)) {
    if (ghost < NUMBER_OF_GHOSTS) {
        ghosts[ghost].updateDirection = updateDirection;
    }
}

/*****************************************************************************
 *
 * Description:
 *    Sets callback to be called after losing game
 *
 * Params:
 *    [in] handler - pointer to function to be used as a callback
 *
 ****************************************************************************/
void onGameLost(void (*handler)(tU8, tU8)) {
    handleGameLost = handler;
}

/*****************************************************************************
 *
 * Description:
 *    Sets callback to be called after losing life
 *
 * Params:
 *    [in] handler - pointer to function to be used as a callback
 *
 ****************************************************************************/
void onLifeLost(void (*handler)(tU8)) {
    handleLifeLost = handler;
}

/*****************************************************************************
 *
 * Description:
 *    Sets callback to be called after score change
 *
 * Params:
 *    [in] handler - pointer to function to be used as a callback
 *
 ****************************************************************************/
void onScoreChanged(void (*handler)(tU8)) {
    handleScoreChanged = handler;
}

/*****************************************************************************
 *
 * Description:
 *    Sets callback to be called after completing level
 *
 * Params:
 *    [in] handler - pointer to function to be used as a callback
 *
 ****************************************************************************/
void onLevelCompleted(void (*handler)(tU8, tU8)) {
    handleLevelComplete = handler;
}

/*****************************************************************************
 *
 * Description:
 *    Sets callback to be called after eating a ghost
 *
 * Params:
 *    [in] handler - pointer to function to be used as a callback
 *
 ****************************************************************************/
void onGhostEaten(void (*handler)()) {
    handleGhostEaten = handler;
}

/*****************************************************************************
 *
 * Description:
 *    Sets callback to be called after change of remaining time in which
 *    pacman can eat ghosts
 *
 * Params:
 *    [in] handler - pointer to function to be used as a callback
 *
 ****************************************************************************/
void onTimeToEatChanged(void (*handler)(tU8)) {
    handleTimeToEatChanged = handler;
}

/*****************************************************************************
 *
 * Description:
 *    Makes one step of game loop. 
 *    Includes moving all characters according to their directions and updating
 *    directions for next move, detecting collisions, updating game counters.
 *    Calls callbacks when necessary.
 *
 * Returns:
 *    Move* - pointer to array of moves executed in current step of game loop.
 *            If current step aims to restore initial positions (first function
 *            call after initialization or losing life), from and to coordinates
 *            of each move are equal.
 *
 ****************************************************************************/
Move *makeMove() {
    tU8 i;
    static Move moves[1 + NUMBER_OF_GHOSTS];

    if (moveToInitPositions) {
        moves[0].from = pacman.birthplace;
        moves[0].to = pacman.birthplace;
        moves[0].type = pacman.type;
        pacman.position = pacman.birthplace;

        for (i = 0; i < NUMBER_OF_GHOSTS; ++i) {
            moves[i + 1].from = ghosts[i].birthplace;
            moves[i + 1].to = ghosts[i].birthplace;
            moves[i + 1].type = ghosts[i].type;
            ghosts[i].position = ghosts[i].birthplace;
            ghosts[i].updateDirection = defaultStayAtHome;
            ghosts[i].timeToStart = ghosts[i].startTime;
        }

        moveToInitPositions = FALSE;
        return moves;
    }

    moves[0] = move(&pacman);

    for (i = 0; i < NUMBER_OF_GHOSTS; ++i) {
        moves[i + 1] = move(&ghosts[i]);
        if (EYES != ghosts[i].type && moves[i + 1].to.x == moves[0].from.x && moves[i + 1].to.y == moves[0].from.y
                && moves[i + 1].from.x == moves[0].to.x && moves[i + 1].from.y == moves[0].to.y) {
            //collision PG version
            if (EATABLE_GHOST == ghosts[i].type) {
                ghosts[i].type = EYES;
                ghosts[i].updateDirection = defaultGoBackHome;
                ghosts[i].position = moves[i + 1].from;
                moves[i + 1].to = moves[i + 1].from;
                if (handleGhostEaten) {
                    handleGhostEaten();
                }
                score += POINTS_FOR_EATING;
                if (handleScoreChanged) {
                    handleScoreChanged(score);
                }
            } else {
                moveToInitPositions = TRUE;
                lives--;
                if (handleLifeLost) {
                    handleLifeLost(lives);
                }
                for (i = 0; i < NUMBER_OF_GHOSTS; ++i) {
                    ghosts[i].type = GHOST;
                }
                if (0 == lives && handleGameLost) {
                    handleGameLost(level, score);
                }
                pacman.position = moves[0].from;
                moves[0].to = moves[0].from;
                break;
            }
        } else if (EYES != ghosts[i].type && moves[i + 1].to.x == moves[0].to.x && moves[i + 1].to.y == moves[0].to.y) {
            //collision P_G version
            if (EATABLE_GHOST == ghosts[i].type) {
                ghosts[i].type = EYES;
                ghosts[i].updateDirection = defaultGoBackHome;
                if (handleGhostEaten) {
                    handleGhostEaten();
                }
                score += POINTS_FOR_EATING;
                if (handleScoreChanged) {
                    handleScoreChanged(score);
                }
            } else {
                moveToInitPositions = 1;
                lives--;
                if (handleLifeLost) {
                    handleLifeLost(lives);
                }
                for (i = 0; i < NUMBER_OF_GHOSTS; ++i) {
                    ghosts[i].type = GHOST;
                }
                if (lives == 0 && handleGameLost) {
                    handleGameLost(level, score);
                }
                break;
            }
        }
    }

    if (POINT == board[pacman.position.y][pacman.position.x]) {
        board[pacman.position.y][pacman.position.x] = EMPTY;
        score++;
        pointsToCompleteLevel--;
        if (handleScoreChanged) {
            handleScoreChanged(score);
        }
    } else if (BONUS == board[pacman.position.y][pacman.position.x]) {
        board[pacman.position.y][pacman.position.x] = EMPTY;
        score += POINTS_FOR_BONUS;
        ghostEatingMode = INIT_TIME_TO_EAT;
        for (i = 0; i < NUMBER_OF_GHOSTS; ++i) {
            if (GHOST == ghosts[i].type) {
                ghosts[i].type = EATABLE_GHOST;
            }
        }
        if (handleScoreChanged) {
            handleScoreChanged(score);
        }
    }

    if (ghostEatingMode) {
        ghostEatingMode--;
        if (handleTimeToEatChanged){
            handleTimeToEatChanged(ghostEatingMode);
        }
        if (!ghostEatingMode) {
            for (i = 0; i < NUMBER_OF_GHOSTS; ++i) {
                if (EATABLE_GHOST == ghosts[i].type) {
                    ghosts[i].type = GHOST;
                }
            }
        }
    }

    if (!pointsToCompleteLevel && handleLevelComplete) {
        handleLevelComplete(level, score);
    }

    return moves;
}

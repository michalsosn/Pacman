// game.c
// Reponsible for the interface between game logic and hardware.

/************/
/* Includes */
/************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"

#include "lcd.h"
#include "key.h"
#include "display.h"
#include "pacman.h"
#include "alphalcd.h"
#include "pca9532.h"

/***********/
/* Defines */
/***********/

// position of the top-left corner of the board
#define TOP_LEFT_X			1
#define TOP_LEFT_Y			1

// number of moving characters
#define CHARACTERS			NUMBER_OF_GHOSTS + 1

// game result
#define GAME_LOST			1
#define GAME_WON			2

/*************/
/* Variables */
/*************/

// Number of ticks to wait between moves in game.
static tU8 timeStep = 18;

// Registered joystick position.
tU8 pressedKey;

// Movement direction.
Direction direction;

// A flag indicating if the game is still active.
tU8 gameEnded;

// A flag indicating if the player lost a life;
tU8 lifeLost;

/*************/
/* Functions */
/*************/

// Changes direction of Pacman's moves.
Direction changeDirection(struct character *c) {
	if (pressedKey != KEY_NOTHING) {
		switch (pressedKey) {
			case KEY_UP:
				direction = UP;
				break;
			case KEY_RIGHT:
				direction = RIGHT;
				break;
			case KEY_DOWN:
				direction = DOWN;
				break;
			case KEY_LEFT:
				direction = LEFT;
				break;
		}
	}
	return direction;
}

// Called every time the player loses life.
void lifeLostEventHandler(tU8 lives) {
	if (lives < 3) {
		lifeLost = 1;
	}

	int i, j;
	for (i = 0; i < 3; ++i) {  	    // ##0##0## <- 8 diodes show 3 lives like that
		for (j = 0; j < 2; ++j) {   // each set of 2 diodes represents a life
			setPca9532Pin(i * 3 + j, i > 2 - lives ? 0 : 1);
		}
	}
}

void gameLostEventHandler(tU8 level, tU8 score) {
	gameEnded = GAME_LOST;
}

void levelCompletedEventHandler(tU8 level, tU8 score) {
	gameEnded = GAME_WON;
}

// Gets pixel x coordinate for given column.
tU8 getX(tU8 column) {
	return TOP_LEFT_X + column * FIELD_SIZE;
}

// Gets pixel y coordinate for given row.
tU8 getY(tU8 row) {
	return TOP_LEFT_Y + row * FIELD_SIZE;
}

// Displays the board on the screen.
void displayBoard(void) {
	int row, column;
	tU8 x, y;
	for (row = 0; row < BOARD_WIDTH; ++row) {
		for (column = 0; column < BOARD_HEIGHT; ++column) {
			x = getX(column);
			y = getY(row);
			switch (board[row][column]) {
				case EMPTY:
					displayEmptyField(x, y);
					break;
				case WALL:
					displayWall(x, y);
					break;
				case POINT:
					displayPoint(x, y);
					break;
				case BONUS:
					displayBonus(x, y);
					break;
				case DOORS:
					displayDoors(x, y);
					break;
			}
		}
	}
}

// Displays a character in a given step of movement.
void displayCharacter(Move move, tU8 animationStep) {
	tU8 x = getX(move.from.x) + animationStep * (move.to.x - move.from.x);
	tU8 y = getY(move.from.y) + animationStep * (move.to.y - move.from.y);
	switch (move.type) {
		case GHOST:
			displayGhost(x, y);
			break;
		case PACMAN:
			displayPacman(x, y);
			break;
		case EATABLE_GHOST:
			displayEatableGhost(x, y);
			break;
		case EYES:
			displayEyes(x, y);
			break;
	}
}

void initAlpha() {
	char message[] = "Current score:\n              0";
	messageOnAlpha(message, TRUE);
}

void displayScoreOnAlpha(tU8 score) {
	char message[] = "\n               ";
	int i = 16;

	while (score > 0) {
		message[i--] = '0' + (score % 10);
		score /= 10;
	}
	while (i > 0) {
		message[i--] = ' ';
	}

	messageOnAlpha(message, TRUE);
}

void displayTimeToEatOnI2C(tU8 remainingTime) {
	int diodesAlight = (remainingTime + 1) * 8 / INIT_TIME_TO_EAT;  // without +1 all diodes would be off when remainingTime = 1 which would
	int i;                                                           // mislead the player into thinking they're vulnerable again too soon
	for (i = 8 + diodesAlight; i < 16; ++i) {
		setPca9532Pin(i, 1);
	}
	for (i = 8; i < 8 + diodesAlight; ++i) {
		setPca9532Pin(i, 0);
	}
}

void startGame(void) {
	
	// mark the game as active
	gameEnded = 0;

	// default Pacman's direction at the beginning
	direction = LEFT;
	
	// sets the callback function responsible for changing Pacman's direction
	setDirectionCallback(changeDirection);

	// sets the handler for GameLost event
	onGameLost(gameLostEventHandler);

	// sets the handler for LifeLost event
	onLifeLost(lifeLostEventHandler);

	// sets the handler for LevelComplete event
	onLevelCompleted(levelCompletedEventHandler);

	// sets the handler for ScoreChanged event
	onScoreChanged(displayScoreOnAlpha);

	// sets the handler for TimeToEatChanged event
	onTimeToEatChanged(displayTimeToEatOnI2C);

	// initializes the game
	initPacman();

	initAlpha();
	
	lcdClrscr();
	displayText("Get ready");
	osSleep(150);

	// displays the initial board state
	displayBoard();

	// get the initial positions of characters
	Move *moves = makeMove();

	// display characters on their initial positions
	tU8 character;
	for (character = 0; character < CHARACTERS; ++character) {
		displayCharacter(moves[character], 0);
	}

	do {
		lifeLost = 0;
	
		// Display the most recent state of the board.
		displayBoard();

		// Let all characters make a move.
		moves = makeMove();

		// Display characters in movement.
		// Each move is split into steps to make it smoother.
		int animationStep;
		for (animationStep = 0; animationStep < FIELD_SIZE; ++animationStep) {
			int character;
			for (character = 0; character < CHARACTERS; ++character) {
				displayCharacter(moves[character], animationStep);
			}
			
			tU8 keyBuffer = checkKey();
			if (keyBuffer != KEY_NOTHING) {
				pressedKey = keyBuffer;
			}	
			
			osSleep(timeStep / FIELD_SIZE);
		}
		
		if (lifeLost == 1) {
			displayText("You died");
			osSleep(150);
		}
		
	} while (!gameEnded);
	
	if (gameEnded == GAME_LOST) {
		displayText("Game lost");
	} else {
		displayText("You won");
	}
	osSleep(150);
}

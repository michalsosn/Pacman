/***********************************************
 *          EMBEDDED SYSTEMS
 *
 * Project: Pacman
 * Device: LPC2148v3 + Expansion Board
 *
 * Group B09 (tuesday, 12:15)
 * Members:
 *    - Piotr Grzelak
 *    - Andrzej Kurczewski
 *    - Micha³ Soœnicki
 *    - Wojciech Sza³apski (team leader)
 *
 ***********************************************/

// main.c
// Reponsible for initialization and creating application processes.

/************/
/* Includes */
/************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"

#include "lcd.h"
#include "key.h"
#include "display.h"
#include "game.h"
#include "i2c.h"
#include "pca9532.h"
#include "bluetooth.h"
#include "startup/ea_init.h"

/***********/
/* Defines */
/***********/

// stack size for processes:
#define INIT_STACK_SIZE  	400
#define GAME_STACK_SIZE    2400

// LCD contrast value (this value should be from 0 to 127)
#define LCD_CONTRAST		 50

/*************/
/* Variables */
/*************/

// stacks for processes
static tU8 initStack[INIT_STACK_SIZE];
static tU8 gameStack[GAME_STACK_SIZE];

/*************/
/* Functions */
/*************/

static void initializationProcess(void *arg);
static void gameProcess(void *arg);

/*****************************************************************************
*
* Description:
*    The first function to be executed.
*    Creates the initialization process and then starts the OS.
*    Initialization process is executed with the highest priority to make sure no other processes start before it ends.
*
* Returns:
*    int - Program exit code
*
 ****************************************************************************/
int main(void) {

  tU8 initProcError;
  tU8 initProcPid;

  // osInit() is mandatory before any other call to OS
  osInit();
  osCreateProcess(initializationProcess, initStack, INIT_STACK_SIZE, &initProcPid, 1, NULL, &initProcError);
  osStartProcess(initProcPid, &initProcError);

  // starts the operating system
  // before calling this function at least one process must be created and started
  osStart();
  return 0;
}

/*****************************************************************************
 *
 * Description:
 *    Initialization process is responsible for creating application processes.
 *
 * Params:
 *    [in] arg - parameters passed to the function (not used)
 *
 ****************************************************************************/
static void initializationProcess(void *arg) {
	tU8 gameProcPid, gameProcError;

	// Initializes the consol for debugging and control messages
	eaInit();
	// Initializes I2C module by resetting it
	i2cInit();

	osCreateProcess(gameProcess, gameStack, GAME_STACK_SIZE, &gameProcPid, 2, NULL, &gameProcError);
  	osStartProcess(gameProcPid, &gameProcError);

	osDeleteProcess();
}

/*****************************************************************************
 *
 * Description:
 *    Process reponsible for starting and running the game.
 *
 * Params:
 *    [in] arg - parameters passed to the function (not used)
 *
 ****************************************************************************/
static void gameProcess(void *arg) {

	// Initializes LCD
	lcdInit();
	lcdContrast(LCD_CONTRAST);

	// Initializes joystick
	initKeyProc();

	// Initializes PCA9532 for diodes around the screen
	pca9532Init();

	displayMenu();

	while (1) {
		tU8 keyPressed = checkKey();

		if (keyPressed == KEY_CENTER) {
			startGame();
			break;
		}

		osSleep(20);
	}

	// Releases the process control block (PCB)
	osDeleteProcess();
}

/****************************************************************************
 *
 * Description:
 *    This function is called once for each timer tick interrupt in OS.
 *    It should be kept as short as possible because it runs in interrupt context.
 *
 * Params:
 *    [in] elapsedTime - Number of elapsed milliseconds since the last call of this function.
 *
 ****************************************************************************/
void appTick(tU32 elapsedTime) {
}

/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * Annotation:
 *    This library has been adjusted to the needs of 'Pacman Project'.
 *
 * File:
 *    key.c
 *
 * Description:
 *    Implements sampling and handling of joystick key.
 *
 *****************************************************************************/

/************/
/* Includes */
/************/

#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
#include "key.h"
#include "lpc2xxx.h"

/***********/
/* Defines */
/***********/

#define KEYPROC_STACK_SIZE 300

#define KEYPIN_CENTER 0x00000100
#define KEYPIN_UP     0x00000400
#define KEYPIN_DOWN   0x00001000
#define KEYPIN_LEFT   0x00000200 
#define KEYPIN_RIGHT  0x00000800

/*************/
/* Variables */
/*************/

static tU8 centerReleased = TRUE;
static tU8 keyUpReleased = TRUE;
static tU8 keyDownReleased = TRUE;
static tU8 keyLeftReleased = TRUE;
static tU8 keyRightReleased = TRUE;

static volatile tU8 activeKey = KEY_NOTHING;
static volatile tU8 activeKey2 = KEY_NOTHING;

static tU8 keyProcStack[KEYPROC_STACK_SIZE];
static tU8 keyProcPid;

/*************/
/* Functions */
/*************/

/*****************************************************************************
 *
 * Description:
 *    Get current state of joystick switch
 *
 ****************************************************************************/
tU8 getKeys(void) {
    tU8 readKeys = KEY_NOTHING;

    if ((IOPIN & KEYPIN_CENTER) == 0) readKeys |= KEY_CENTER;
    if ((IOPIN & KEYPIN_UP) == 0) readKeys |= KEY_UP;
    if ((IOPIN & KEYPIN_DOWN) == 0) readKeys |= KEY_DOWN;
    if ((IOPIN & KEYPIN_LEFT) == 0) readKeys |= KEY_LEFT;
    if ((IOPIN & KEYPIN_RIGHT) == 0) readKeys |= KEY_RIGHT;

    return readKeys;
}

/*****************************************************************************
 *
 * Description:
 *    Function to check if any key press has been detected
 *
 ****************************************************************************/
tU8 checkKey(void) {
    tU8 retVal = activeKey;
    activeKey = KEY_NOTHING;
    return retVal;
}

/*****************************************************************************
 *
 * Description:
 *    Function to check pressed key
 *
 ****************************************************************************/
void checkKeyStatus(tU8 key, tU8 *keyReleased) {
    if (*keyReleased == TRUE) {
        *keyReleased = FALSE;
        activeKey = key;
    }
}

/*****************************************************************************
 *
 * Description:
 *    Sample key states
 *
 ****************************************************************************/
void sampleKey(void) {

    tBool nothing = TRUE;
    tU8 readKeys;

    // get sample
    readKeys = getKeys();

    // check center key
    if (readKeys & KEY_CENTER) {
        nothing = FALSE;
        checkKeyStatus(KEY_CENTER, &centerReleased);
    } else {
        centerReleased = TRUE;
    }

    // check up key
    if (readKeys & KEY_UP) {
        nothing = FALSE;
        checkKeyStatus(KEY_UP, &keyUpReleased);
    } else {
        keyUpReleased = TRUE;
    }

    // check down key
    if (readKeys & KEY_DOWN) {
        nothing = FALSE;
        checkKeyStatus(KEY_DOWN, &keyDownReleased);
    } else {
        keyDownReleased = TRUE;
    }

    // check left key
    if (readKeys & KEY_LEFT) {
        nothing = FALSE;
        checkKeyStatus(KEY_LEFT, &keyLeftReleased);
    } else {
        keyLeftReleased = TRUE;
    }

    // check right key
    if (readKeys & KEY_RIGHT) {
        nothing = FALSE;
        checkKeyStatus(KEY_RIGHT, &keyRightReleased);
    } else {
        keyRightReleased = TRUE;
    }
}

/*****************************************************************************
 *
 * Description:
 *    A process entry function 
 *
 * Params:
 *    [in] arg - This parameter is not used in this application. 
 *
 ****************************************************************************/
static void procKey(void *arg) {

    // make all key signals as inputs
    IODIR &= ~(KEYPIN_CENTER | KEYPIN_UP | KEYPIN_DOWN | KEYPIN_LEFT | KEYPIN_RIGHT);

    //sample keys each 50 ms
    while (1) {
        sampleKey();
        osSleep(5);
    }
}

/*****************************************************************************
 *
 * Description:
 *    Creates and starts the key sampling process. 
 *
 ****************************************************************************/
void initKeyProc(void) {

    tU8 error;

    osCreateProcess(procKey, keyProcStack, KEYPROC_STACK_SIZE, &keyProcPid, 3, NULL, &error);
    osStartProcess(keyProcPid, &error);
}

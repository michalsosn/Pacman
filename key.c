/******************************************************************************
 *
 * Copyright:
 *    (C) 2006 Embedded Artists AB
 *
 * File:
 *    key.c
 *
 * Description:
 *    Implements sampling and handling of joystick key.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "pre_emptive_os/api/osapi.h"
#include "pre_emptive_os/api/general.h"
// #include <printf_P.h>
#include "key.h"
#include "lpc2xxx.h"


/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/
#define FIRST_REPEAT  4
#define SECOND_REPEAT 3

#define KEYPROC_STACK_SIZE 300

#define KEYPIN_CENTER 0x00000100
#define KEYPIN_UP     0x00000400
#define KEYPIN_DOWN   0x00001000
#define KEYPIN_LEFT   0x00000200 
#define KEYPIN_RIGHT  0x00000800

#if 0
      //check if P0.8 center-key is pressed
      if ((IOPIN & 0x00000100) == 0)

      //check if P0.9 left-key is pressed
      else if ((IOPIN & 0x00000200) == 0)

      //check if P0.10 up-key is pressed
      else if ((IOPIN & 0x00000400) == 0)

      //check if P0.11 right-key is pressed
      else if ((IOPIN & 0x00000800) == 0)

      //check if P0.12 down-key is pressed
      else if ((IOPIN & 0x00001000) == 0)
#endif

/*****************************************************************************
 * Local variables
 ****************************************************************************/
static tU8 centerReleased = TRUE;
static tU8 keyUpReleased = TRUE;
static tU8 keyDownReleased = TRUE;
static tU8 keyLeftReleased = TRUE;
static tU8 keyRightReleased = TRUE;
static tU8 centerKeyCnt;
static tU8 upKeyCnt;
static tU8 downKeyCnt;
static tU8 leftKeyCnt;
static tU8 rightKeyCnt;

static volatile tU8 activeKey = KEY_NOTHING;
static volatile tU8 activeKey2 = KEY_NOTHING;

static tU8 keyProcStack[KEYPROC_STACK_SIZE];
static tU8 keyProcPid;



/*****************************************************************************
 *
 * Description:
 *    Get current state of joystick switch
 *
 ****************************************************************************/
tU8
getKeys(void)
{
  tU8 readKeys = KEY_NOTHING;

  if ((IOPIN & KEYPIN_CENTER) == 0) readKeys |= KEY_CENTER;
  if ((IOPIN & KEYPIN_UP) == 0)     readKeys |= KEY_UP;
  if ((IOPIN & KEYPIN_DOWN) == 0)   readKeys |= KEY_DOWN;
  if ((IOPIN & KEYPIN_LEFT) == 0)   readKeys |= KEY_LEFT;
  if ((IOPIN & KEYPIN_RIGHT) == 0)  readKeys |= KEY_RIGHT;

  return readKeys;
}

/*****************************************************************************
 *
 * Description:
 *    Function to check if any key press has been detected
 *
 ****************************************************************************/
tU8
checkKey(void)
{
  tU8 retVal = activeKey;
  activeKey = KEY_NOTHING;
  return retVal;
}

/*****************************************************************************
 *
 * Description:
 *    Function to check current (instantaneous) key state
 *
 ****************************************************************************/
tU8
checkKey2(void)
{
  return activeKey2;
}

/*****************************************************************************
 *
 * Description:
 *    Sample key states
 *
 ****************************************************************************/
void
sampleKey(void)
{
  tBool nothing = TRUE;
  tU8   readKeys;
  
  //get sample
  readKeys = getKeys();
  

  //check center key
  if (readKeys & KEY_CENTER)
  {
    nothing = FALSE;
  	if (centerReleased == TRUE)
  	{
  		centerReleased = FALSE;
  		centerKeyCnt = 0;
  		activeKey = KEY_CENTER;
  		activeKey2 = KEY_CENTER;
  	}
  	else
  	{
  	  centerKeyCnt++;
  	  if (centerKeyCnt == FIRST_REPEAT)
  	  {
  		  activeKey = KEY_CENTER;
  		  activeKey2 = KEY_CENTER;
  	  }
  	  else if (centerKeyCnt >= FIRST_REPEAT + SECOND_REPEAT)
  	  {
  		  centerKeyCnt = FIRST_REPEAT;
  		  activeKey = KEY_CENTER;
  		  activeKey2 = KEY_CENTER;
  	  }
  	}
  }
  else
  	centerReleased = TRUE;

  //check up key
  if (readKeys & KEY_UP)
  {
    nothing = FALSE;
  	if (keyUpReleased == TRUE)
  	{
  		keyUpReleased = FALSE;
  		upKeyCnt = 0;
  		activeKey = KEY_UP;
  		activeKey2 = KEY_UP;
  	}
  	else
  	{
  	  upKeyCnt++;
  	  if (upKeyCnt == FIRST_REPEAT)
  	  {
  		  activeKey = KEY_UP;
  		  activeKey2 = KEY_UP;
  	  }
  	  else if (upKeyCnt >= FIRST_REPEAT + SECOND_REPEAT)
  	  {
  		  upKeyCnt = FIRST_REPEAT;
  		  activeKey = KEY_UP;
  		  activeKey2 = KEY_UP;
  	  }
  	}
  }
  else
  	keyUpReleased = TRUE;

  //check down key
  if (readKeys & KEY_DOWN)
  {
    nothing = FALSE;
  	if (keyDownReleased == TRUE)
  	{
  		keyDownReleased = FALSE;
  		downKeyCnt = 0;
  		activeKey = KEY_DOWN;
  		activeKey2 = KEY_DOWN;
  	}
  	else
  	{
  	  downKeyCnt++;
  	  if (downKeyCnt == FIRST_REPEAT)
  	  {
  		  activeKey = KEY_DOWN;
  		  activeKey2 = KEY_DOWN;
  	  }
  	  else if (downKeyCnt >= FIRST_REPEAT + SECOND_REPEAT)
  	  {
  		  downKeyCnt = FIRST_REPEAT;
  		  activeKey = KEY_DOWN;
  		  activeKey2 = KEY_DOWN;
  	  }
  	}
  }
  else
  	keyDownReleased = TRUE;

  //check left key
  if (readKeys & KEY_LEFT)
  {
    nothing = FALSE;
  	if (keyLeftReleased == TRUE)
  	{
  		keyLeftReleased = FALSE;
  		leftKeyCnt = 0;
  		activeKey = KEY_LEFT;
  		activeKey2 = KEY_LEFT;
  	}
  	else
  	{
  	  leftKeyCnt++;
  	  if (leftKeyCnt == FIRST_REPEAT)
  	  {
  		  activeKey = KEY_LEFT;
  		  activeKey2 = KEY_LEFT;
  	  }
  	  else if (leftKeyCnt >= FIRST_REPEAT + SECOND_REPEAT)
  	  {
  		  leftKeyCnt = FIRST_REPEAT;
  		  activeKey = KEY_LEFT;
  		  activeKey2 = KEY_LEFT;
  	  }
  	}
  }
  else
  	keyLeftReleased = TRUE;

  //check right key
  if (readKeys & KEY_RIGHT)
  {
    nothing = FALSE;
  	if (keyRightReleased == TRUE)
  	{
  		keyRightReleased = FALSE;
  		rightKeyCnt = 0;
  		activeKey = KEY_RIGHT;
  		activeKey2 = KEY_RIGHT;
  	}
  	else
  	{
  	  rightKeyCnt++;
  	  if (rightKeyCnt == FIRST_REPEAT)
  	  {
  		  activeKey = KEY_RIGHT;
  		  activeKey2 = KEY_RIGHT;
  	  }
  	  else if (rightKeyCnt >= FIRST_REPEAT + SECOND_REPEAT)
  	  {
  		  rightKeyCnt = FIRST_REPEAT;
  		  activeKey = KEY_RIGHT;
  		  activeKey2 = KEY_RIGHT;
  	  }
  	}
  }
  else
  	keyRightReleased = TRUE;
  
  if (nothing == TRUE)
    activeKey2 = KEY_NOTHING;
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
static void
procKey(void* arg)
{
  //make all key signals as inputs
  IODIR &= ~(KEYPIN_CENTER | KEYPIN_UP | KEYPIN_DOWN | KEYPIN_LEFT | KEYPIN_RIGHT);

  //sample keys each 50 ms, i.e., 20 times per second
  while(1)
  {
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
void
initKeyProc(void)
{
  tU8 error;

  osCreateProcess(procKey, keyProcStack, KEYPROC_STACK_SIZE, &keyProcPid, 3, NULL, &error);
  osStartProcess(keyProcPid, &error);
}


/******************************************************************************
 *
 * File:
 *    alphalcd.h
 * 
 * Description:
 *    The library is responsible for displaying messages on the alphanumeric display.
 * 
 *****************************************************************************/

/************/
/* Includes */
/************/

#include "../pre_emptive_os/api/osapi.h"
#include "../pre_emptive_os/api/general.h"
#include <printf_P.h>
#include <lpc2xxx.h>
#include <consol.h>

/***********/
/* Defines */
/***********/

#define LCD_DATA      0x00ff0000  // P1.16 - P1.23
#define LCD_E         0x02000000  // P1.25
#define LCD_RW        0x00400000  // P0.22
#define LCD_RS        0x01000000  // P1.24
#define LCD_BACKLIGHT 0x40000000  // P0.30

/*****************************************************************************
 *
 * Description:
 *    Initializes the display by setting GPIO state and direction.
 *
 ****************************************************************************/
static void initLCD(void) {
    IODIR1 |= (LCD_DATA | LCD_E | LCD_RS);
    IOCLR1 = (LCD_DATA | LCD_E | LCD_RS);

    IODIR0 |= LCD_RW;
    IOCLR0 = LCD_RW;

    IODIR0 |= LCD_BACKLIGHT;
    IOCLR0 = LCD_BACKLIGHT;
}

/*****************************************************************************
 *
 * Description:
 *    Generates 37us delay which is needed after calling writeLCD function.
 *
 ****************************************************************************/
static void delay37us(void) {
    volatile tU32 i;

    for (i = 0; i < 6 * 2500; i++) {
        asm volatile (" nop");
    }
}

/*****************************************************************************
 *
 * Description:
 *    xxx
 * 
 * Params:
 *    [in] reg - xxx
 *    [in] data - xxx
 *
 ****************************************************************************/
static void writeLCD(tU8 reg, tU8 data) {
    volatile tU8 i;

    if (0 == reg) {
        IOCLR1 = LCD_RS;
    } else {
        IOSET1 = LCD_RS;
    }

    IOCLR0 = LCD_RW;
    IOCLR1 = LCD_DATA;
    IOSET1 = ((tU32) data << 16) & LCD_DATA;

    IOSET1 = LCD_E;
    for (i = 0; i < 16; i++) {
        asm volatile (" nop");
    }
    //delay 15 ns x 16 = about 250 ns delay

    IOCLR1 = LCD_E;
    for (i = 0; i < 16; i++) {
        asm volatile (" nop");
    }
    //delay 15 ns x 16 = about 250 ns delay
}

/*****************************************************************************
 *
 * Description:
 *    Turns on or off the backlight of LCD display.
 * 
 * Params:
 *    [in] onOff - a flag indicating if the backlight should be turned on
 *
 ****************************************************************************/
static void lcdBacklight(tU8 onOff) {
    if (onOff == TRUE) {
        IOSET0 = LCD_BACKLIGHT;
    } else {
        IOCLR0 = LCD_BACKLIGHT;
    }
}

/*****************************************************************************
 *
 * Description:
 *    Displays text on the screen.
 * 
 * Params:
 *    [in] str - the text to be displayed
 *    [in] data - a flag indicating if the backlight should be turned on
 *
 ****************************************************************************/
void messageOnAlpha(char *str, tU8 keepBacklight) {
    char *ptr;
    static tU8 initialized;

    if (initialized == 0) {
        initLCD();
        initialized = 1;

        // display clear
        writeLCD(0, 0x01);
        osSleep(1);
    }

    lcdBacklight(TRUE);

    // function set
    writeLCD(0, 0x30);
    osSleep(1);
    writeLCD(0, 0x30);
    delay37us();
    writeLCD(0, 0x30);
    delay37us();

    // function set
    writeLCD(0, 0x38);
    delay37us();

    // display off
    writeLCD(0, 0x08);
    delay37us();

    // display control set
    writeLCD(0, 0x06);
    osSleep(1);

    // display control set
    writeLCD(0, 0x0c);
    delay37us();

    // cursor home
    writeLCD(0, 0x02);
    osSleep(1);

    for (ptr = str; *ptr; ptr++) {
        if ('\n' != *ptr) {
            writeLCD(1, *ptr);
        } else {
            writeLCD(0, 0x80 | 0x40);
        }
        delay37us();
    }

    lcdBacklight(keepBacklight);
}

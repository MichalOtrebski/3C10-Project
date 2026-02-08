/*
 * globals.h
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

#include "stdint.h"
#include "stddef.h"

#define LCD_WIDTH 	240
#define LCD_HEIGHT 	320

#define FB_WIDTH 	120
#define FB_HEIGHT 	160

#define RGB565_BLACK   0x0000
#define RGB565_WHITE   0xFFFF
#define RGB565_GRAY    0x8410
#define RGB565_BLUE    0x001F
#define RGB565_GREEN   0x07E0

typedef enum {
    STATE_MENU = 0,
    STATE_TETRIS,
    STATE_PONG,
    STATE_SETTINGS
} SystemState;

volatile SystemState g_state = STATE_MENU;

extern uint16_t framebuffer[FB_WIDTH * FB_HEIGHT];
extern uint16_t buf[LCD_WIDTH];

#endif /* INC_GLOBALS_H_ */

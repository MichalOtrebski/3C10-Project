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

#define LCD_WIDTH 	320
#define LCD_HEIGHT 	240

#define FB_WIDTH 	160
#define FB_HEIGHT 	120

extern uint16_t framebuffer[FB_WIDTH * FB_HEIGHT];
extern uint16_t buf[LCD_WIDTH];

#endif /* INC_GLOBALS_H_ */

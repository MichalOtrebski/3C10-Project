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

extern uint16_t framebuffer[FB_WIDTH * FB_HEIGHT];
extern uint16_t buf[LCD_WIDTH];

#endif /* INC_GLOBALS_H_ */

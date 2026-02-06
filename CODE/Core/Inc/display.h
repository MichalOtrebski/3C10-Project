/*
 * display.h
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include "stm32g4xx_hal.h"
#include "lcd_io.h"
#include "globals.h"

#include <stdint.h>
#include <stddef.h>

void LCD_DrawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_PresentFramebuffer(void);

#endif /* INC_DISPLAY_H_ */

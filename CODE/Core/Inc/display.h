/*
 * display.h
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include "globals.h"
#include <stdint.h>

void LCD_ClearFrame(void);
void LCD_DrawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
void render(void);
void render_dma(void);

void border(uint8_t t, uint8_t b, uint8_t l, uint8_t r, uint16_t color);

#endif /* INC_DISPLAY_H_ */

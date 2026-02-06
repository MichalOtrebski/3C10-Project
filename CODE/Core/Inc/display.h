/*
 * display.h
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include <stdint.h>

void LCD_ClearFrame();
void LCD_DrawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);
void render();

#endif /* INC_DISPLAY_H_ */

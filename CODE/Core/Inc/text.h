/*
 * text.h
 *
 *  Created on: 6 Feb 2026
 *      Author: motre
 */

#ifndef INC_TEXT_H_
#define INC_TEXT_H_

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint16_t w;
    uint16_t h;
    uint16_t *buf;   // RGB565, row-major
} fb565_t;

void LCD_DrawChar(int x, int y, char ch, uint16_t fg, uint16_t bg, int scale);

void LCD_DrawText(int x, int y, const char *s, uint16_t fg, uint16_t bg, int scale);

#endif /* INC_TEXT_H_ */

/*
 * text.c
 *
 *  Created on: 6 Feb 2026
 *      Author: motre
 */



#include "globals.h"
#include "text.h"
#include "font.h"


static inline void LCD_FB_PutPixel(int x, int y, uint16_t color)
{
    if ((unsigned)x >= FB_WIDTH || (unsigned)y >= FB_HEIGHT) return;
    framebuffer[y * FB_WIDTH + x] = color;
}

void LCD_DrawChar(int x, int y, char ch, uint16_t fg, uint16_t bg, int scale) {
    (void)bg;

    if (scale < 1) scale = 1;

    if ((unsigned char)ch < 32 || (unsigned char)ch > 127) ch = '?';
    const uint8_t *glyph = font5x7[(int)ch - 32];

    for (int col = 0; col < 5; col++)
    {
        uint8_t bits = glyph[col];

        for (int row = 0; row < 7; row++)
        {
            if ((bits & (1u << row)) == 0) continue;

            int px0 = x + col * scale;
            int py0 = y + row * scale;

            for (int dy = 0; dy < scale; dy++) {
                for (int dx = 0; dx < scale; dx++) {
                    LCD_FB_PutPixel(px0 + dx, py0 + dy, fg);
                }
            }
        }
    }
}

void LCD_DrawText(int x, int y, const char *text,
                  uint16_t fg, uint16_t bg, int scale)
{
    if (scale < 1) scale = 1;

    const int adv_x = 5 * scale + 1;  // 5 cols scaled + 1px spacing
    const int adv_y = 7 * scale;      // no extra row spacing

    int cx = x, cy = y;

    while (*text)
    {
        char c = *text++;

        if (c == '\n') { cx = x; cy += adv_y; continue; }
        if (c == '\r') continue;

        LCD_DrawChar(cx, cy, c, fg, bg, scale);
        cx += adv_x;

        // wrap inside framebuffer
        if (cx + adv_x > FB_WIDTH)
        {
            cx = x;
            cy += adv_y;
        }
        if (cy + adv_y > FB_HEIGHT)
            break;
    }
}

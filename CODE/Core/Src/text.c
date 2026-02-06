/*
 * text.c
 *
 *  Created on: 6 Feb 2026
 *      Author: motre
 */


#include "text.h"
#include "font.h"
#include "globals.h"

static inline void LCD_FB_PutPixel(int x, int y, uint16_t color)
{
    if ((unsigned)x >= FB_WIDTH || (unsigned)y >= FB_HEIGHT) return;
    framebuffer[y * FB_WIDTH + x] = color;
}

void LCD_DrawChar(int x, int y, char ch, uint16_t fg, uint16_t bg, int scale)
{
    if (scale < 1) scale = 1;

    if ((unsigned char)ch < 32 || (unsigned char)ch > 127) ch = '?';
    const uint8_t *glyph = font5x7[(int)ch - 32];

    const int glyph_w = 5 * scale;
    const int glyph_h = 7 * scale;
    const int spacing_x = 1;   // 1 px gap between chars in framebuffer pixels
    const int spacing_y = 0;   // IMPORTANT: no extra blank row
    const int cell_w = glyph_w + spacing_x;
    const int cell_h = glyph_h + spacing_y;

    // background fill
    for (int yy = 0; yy < cell_h; yy++)
        for (int xx = 0; xx < cell_w; xx++)
            LCD_FB_PutPixel(x + xx, y + yy, bg);

    // glyph draw
    for (int col = 0; col < 5; col++)
    {
        uint8_t bits = glyph[col];

        for (int row = 0; row < 7; row++)
        {
            if ((bits & (1u << row)) == 0) continue;

            int px0 = x + col * scale;
            int py0 = y + row * scale;

            for (int dy = 0; dy < scale; dy++)
                for (int dx = 0; dx < scale; dx++)
                    LCD_FB_PutPixel(px0 + dx, py0 + dy, fg);
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

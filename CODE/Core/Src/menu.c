/*
 * menu.c
 *
 *  Created on: 6 Feb 2026
 *      Author: motre
 */

#include "menu.h"
#include "main.h"

#include "text.h"
#include "buttons.h"
#include <string.h>
#include "colors.h"

static int clampi(int v, int lo, int hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

static void LCD_DrawMenuTitleFancy(int x, int y, const char *title, uint16_t normalFg, uint16_t bg)
{
    if (strcmp(title, "SNAKE") == 0)
    {
        // Blue S, rest green
        LCD_DrawText(x +  0, y, "S",     RGB565_CYAN,  bg, 2);
        LCD_DrawText(x + 12, y, "NAKE",  RGB565_GREEN, bg, 2);
        return;
    }

    if (strcmp(title, "TETRIS") == 0)
    {
        // Multicolour letters like Tetris blocks
        LCD_DrawText(x +  0, y, "T", RGB565_CYAN,    bg, 2);
        LCD_DrawText(x + 12, y, "E", RGB565_YELLOW,  bg, 2);
        LCD_DrawText(x + 24, y, "T", RGB565_MAGENTA, bg, 2);
        LCD_DrawText(x + 36, y, "R", RGB565_GREEN,   bg, 2);
        LCD_DrawText(x + 48, y, "I", RGB565_RED,     bg, 2);
        LCD_DrawText(x + 60, y, "S", RGB565_BLUE,    bg, 2);
        return;
    }

    // Default
    LCD_DrawText(x, y, title, normalFg, bg, 2);
}

void LCD_MenuDraw(void) {
    LCD_DrawRect(0, 0, FB_WIDTH, FB_HEIGHT, RGB565_BLACK);

    // Outer frame
    border(0, 0, 0, 0, 0xF81D);
    border(2, 2, 2, 2, RGB565_BLUE);

    // Header panel
    LCD_DrawRect(6, 6, FB_WIDTH - 12, 24, 0x2004);
    LCD_DrawRect(8, 8, FB_WIDTH - 16, 20, 0x4008);

    LCD_DrawText(17, 10, "GAME MAN", RGB565_BLACK, 0x4008, 2);
    LCD_DrawText(15, 8,  "GAME MAN", RGB565_WHITE, 0x4008, 2);

    const int topY = 38;
    const int rowH = 18;
    const int listX = 8;
    const int listW = FB_WIDTH - 16;
    const int visibleRows = (FB_HEIGHT - topY - 14) / rowH;

    // Keep selected item visible
    if (menu_selected < menu_scroll) menu_scroll = menu_selected;
    if (menu_selected >= menu_scroll + visibleRows) menu_scroll = menu_selected - visibleRows + 1;

    int maxScroll = (int)MENU_COUNT - visibleRows;
    if (maxScroll < 0) maxScroll = 0;
    menu_scroll = clampi(menu_scroll, 0, maxScroll);

    // List background panel
    LCD_DrawRect(listX, topY - 2, listW, visibleRows * rowH + 4, 0x1883);

    for (int i = 0; i < visibleRows; i++) {
        int idx = menu_scroll + i;
        if (idx >= (int)MENU_COUNT) break;

        int y = topY + i * rowH;
        int isSel = (idx == menu_selected);

        // Row background
        if (isSel) {
            // Selection box
            LCD_DrawRect(listX + 2, y, listW - 8, rowH - 2, RGB565_BLACK);
//            LCD_DrawRect(listX + 4, y + 2, listW - 12, rowH - 6, 0x039F);
        } else {
            LCD_DrawRect(listX + 2, y, listW - 8, rowH - 2, RGB565_BLACK);
        }

        // Small selector marker
        if (isSel) {
            LCD_DrawRect(listX + 4, y + 4, 4, rowH - 10, 0xFFE0);
        }

        uint16_t fg = isSel ? RGB565_WHITE : RGB565_GRAY;
        uint16_t bg = isSel ? RGB565_GRAY : RGB565_BLACK;

        LCD_DrawMenuTitleFancy(listX + 12, y + 2, menuItems[idx].title, fg, RGB565_BLACK);

        // Divider line for non-selected rows
        if (!isSel) {
//            LCD_DrawRect(listX + 4, y + rowH - 3, listW - 12, 1, 0x5aeb);
        }
    }

    // Scrollbar
    if (MENU_COUNT > (uint32_t)visibleRows) {
        int barX = FB_WIDTH - 6;
        int barY = topY;
        int barH = visibleRows * rowH - 2;

        LCD_DrawRect(barX, barY, 2, barH, 0x39E7);

        int thumbH = (barH * visibleRows) / MENU_COUNT;
        if (thumbH < 8) thumbH = 8;

        int thumbY = barY;
        if (maxScroll > 0) {
            thumbY = barY + ((barH - thumbH) * menu_scroll) / maxScroll;
        }

        LCD_DrawRect(barX - 1, thumbY, 4, thumbH, 0xF81D);
    }

    // Footer hint
//    LCD_DrawRect(6, FB_HEIGHT - 12, FB_WIDTH - 12, 10, 0x2004);
//    LCD_DrawText(10, FB_HEIGHT - 11, "UP/DOWN MOVE   A SELECT", RGB565_WHITE, 0x2004, 1);
}

uint8_t Menu_Update(uint16_t pressed, uint16_t held) {
    (void)held;

    const int topY = 38;
    const int rowH = 18;
    const int visibleRows = (FB_HEIGHT - topY - 14) / rowH;

    if (pressed & (1u << BTN_DOWN)) {
        menu_selected++;
    }

    if (pressed & (1u << BTN_UP)) {
        menu_selected--;
    }

    if (menu_selected < 0) {
        menu_selected = (int)MENU_COUNT - 1;
    }

    if (menu_selected >= (int)MENU_COUNT) {
        menu_selected = 0;
    }

    if (menu_selected < menu_scroll) {
        menu_scroll = menu_selected;
    }

    if (menu_selected >= menu_scroll + visibleRows) {
        menu_scroll = menu_selected - visibleRows + 1;
    }

    {
        const int maxScroll = (int)MENU_COUNT - visibleRows;
        menu_scroll = clampi(menu_scroll, 0, (maxScroll > 0) ? maxScroll : 0);
    }

    int chosen = 255;
    if (pressed & (1u << BTN_A)) {
        chosen = menu_selected;
    }

    LCD_MenuDraw();
    return (uint8_t)chosen;
}

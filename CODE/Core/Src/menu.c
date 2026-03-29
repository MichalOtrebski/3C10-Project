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
#include "menu_sfx.h"

int menu_selected = 0;
int menu_scroll = 0;

static int prev_menu_selected = -1;
static int prev_menu_scroll = -1;
static int menu_first_draw = 1;

static int clampi(int v, int lo, int hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

static void LCD_DrawMenuTitleFancy(int x, int y, const char *title, uint16_t normalFg, uint16_t bg)
{
    if (strcmp(title, "SNAKE") == 0)
    {
        LCD_DrawText(x +  0, y, "S",     RGB565_CYAN,  bg, 2);
        LCD_DrawText(x + 12, y, "NAKE",  RGB565_GREEN, bg, 2);
        return;
    }

    if (strcmp(title, "TETRIS") == 0)
    {
        LCD_DrawText(x +  0, y, "T", RGB565_CYAN,    bg, 2);
        LCD_DrawText(x + 12, y, "E", RGB565_YELLOW,  bg, 2);
        LCD_DrawText(x + 24, y, "T", RGB565_MAGENTA, bg, 2);
        LCD_DrawText(x + 36, y, "R", RGB565_GREEN,   bg, 2);
        LCD_DrawText(x + 48, y, "I", RGB565_RED,     bg, 2);
        LCD_DrawText(x + 60, y, "S", RGB565_BLUE,    bg, 2);
        return;
    }

    if (strcmp(title, "BREAKOUT") == 0)
    {
        LCD_DrawText(x, y, "BREAKOUT", RGB565_WHITE, bg, 2);
        return;
    }

    LCD_DrawText(x, y, title, normalFg, bg, 2);
}

static void Menu_DrawStatic(void)
{
    LCD_DrawRect(0, 0, FB_WIDTH, FB_HEIGHT, RGB565_BLACK);

    border(0, 0, 0, 0, 0xF81D);
    border(2, 2, 2, 2, RGB565_BLUE);

    LCD_DrawRect(6, 6, FB_WIDTH - 12, 24, 0x2004);
    LCD_DrawRect(8, 8, FB_WIDTH - 16, 20, 0x4008);

    LCD_DrawText(17, 12, "GAME BOX", RGB565_BLACK, 0x4008, 2);
    LCD_DrawText(16, 11, "GAME BOX", 0x3186, 0x4008, 2);
    LCD_DrawText(15, 10, "GAME BOX", RGB565_WHITE, 0x4008, 2);

    {
        const int topY = 38;
        const int rowH = 18;
        const int listX = 8;
        const int listW = FB_WIDTH - 16;
        const int visibleRows = (FB_HEIGHT - topY - 14) / rowH;

        LCD_DrawRect(listX, topY - 2, listW, visibleRows * rowH + 4, 0x1883);
    }
}

static void Menu_DrawRow(int idx, int screenRow)
{
    const int topY = 38;
    const int rowH = 18;
    const int listX = 8;
    const int listW = FB_WIDTH - 16;
    const int visibleRows = (FB_HEIGHT - topY - 14) / rowH;

    if (screenRow < 0 || screenRow >= visibleRows) return;

    {
        const int y = topY + screenRow * rowH;
        const int isSel = (idx == menu_selected);

        if (idx < 0 || idx >= (int)MENU_COUNT) {
            LCD_DrawRect(listX + 2, y, listW - 8, rowH - 2, 0x1883);
            return;
        }

        LCD_DrawRect(listX + 2, y, listW - 8, rowH - 2, RGB565_BLACK);

        if (isSel) {
            LCD_DrawRect(listX + 4, y + 4, 4, rowH - 10, 0xFFE0);
        } else {
            LCD_DrawRect(listX + 4, y + 4, 4, rowH - 10, RGB565_BLACK);
        }

        LCD_DrawMenuTitleFancy(listX + 12, y + 2,
                               menuItems[idx].title,
                               isSel ? RGB565_WHITE : RGB565_GRAY,
                               RGB565_BLACK);
    }
}

static void Menu_DrawVisibleRows(void)
{
    const int topY = 38;
    const int rowH = 18;
    const int visibleRows = (FB_HEIGHT - topY - 14) / rowH;

    for (int i = 0; i < visibleRows; i++) {
        Menu_DrawRow(menu_scroll + i, i);
    }
}

static void Menu_DrawScrollbar(void)
{
    const int topY = 38;
    const int rowH = 18;
    const int visibleRows = (FB_HEIGHT - topY - 14) / rowH;
    const int maxScroll = ((int)MENU_COUNT - visibleRows > 0) ? ((int)MENU_COUNT - visibleRows) : 0;

    int barX = FB_WIDTH - 6;
    int barY = topY;
    int barH = visibleRows * rowH - 2;

    LCD_DrawRect(barX - 1, barY, 4, barH, 0x1883);

    if (MENU_COUNT > (uint32_t)visibleRows) {
        int thumbH = (barH * visibleRows) / MENU_COUNT;
        int thumbY = barY;

        LCD_DrawRect(barX, barY, 2, barH, 0x39E7);

        if (thumbH < 8) thumbH = 8;

        if (maxScroll > 0) {
            thumbY = barY + ((barH - thumbH) * menu_scroll) / maxScroll;
        }

        LCD_DrawRect(barX - 1, thumbY, 4, thumbH, 0xF81D);
    }
}

void Menu_Invalidate(void)
{
    menu_first_draw = 1;
    prev_menu_selected = -1;
    prev_menu_scroll = -1;
}

void LCD_MenuDraw(void) {
    const int topY = 38;
    const int rowH = 18;
    const int visibleRows = (FB_HEIGHT - topY - 14) / rowH;

    if (menu_selected < menu_scroll) menu_scroll = menu_selected;
    if (menu_selected >= menu_scroll + visibleRows) menu_scroll = menu_selected - visibleRows + 1;

    {
        int maxScroll = (int)MENU_COUNT - visibleRows;
        if (maxScroll < 0) maxScroll = 0;
        menu_scroll = clampi(menu_scroll, 0, maxScroll);
    }

    if (menu_first_draw) {
        Menu_DrawStatic();
        Menu_DrawVisibleRows();
        Menu_DrawScrollbar();

        prev_menu_selected = menu_selected;
        prev_menu_scroll = menu_scroll;
        menu_first_draw = 0;
        return;
    }

    if (menu_scroll != prev_menu_scroll) {
        Menu_DrawVisibleRows();
        Menu_DrawScrollbar();
    } else if (menu_selected != prev_menu_selected) {
        int oldRow = prev_menu_selected - menu_scroll;
        int newRow = menu_selected - menu_scroll;

        if (oldRow >= 0 && oldRow < visibleRows) {
            Menu_DrawRow(prev_menu_selected, oldRow);
        }

        if (newRow >= 0 && newRow < visibleRows) {
            Menu_DrawRow(menu_selected, newRow);
        }
    }

    prev_menu_selected = menu_selected;
    prev_menu_scroll = menu_scroll;
}

uint8_t Menu_Update(uint16_t pressed, uint16_t held) {
    (void)held;

    {
        const int topY = 38;
        const int rowH = 18;
        const int visibleRows = (FB_HEIGHT - topY - 14) / rowH;

        if (pressed & (1u << BTN_DOWN)) {
            menu_selected++;
            MenuSFX_Move();
        }

        if (pressed & (1u << BTN_UP)) {
            menu_selected--;
            MenuSFX_Move();
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
    }

    {
        int chosen = 255;

        if (pressed & (1u << BTN_A)) {
            chosen = menu_selected;
            MenuSFX_Select();
        }

        LCD_MenuDraw();
        return (uint8_t)chosen;
    }
}

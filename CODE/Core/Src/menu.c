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

static int clampi(int v, int lo, int hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

void LCD_MenuDraw(void) {
	LCD_DrawRect(0, 0, FB_WIDTH, FB_HEIGHT, RGB565_BLACK);

    // Title
    LCD_DrawText(10, 8, "GAME MENU", RGB565_WHITE, RGB565_BLACK, 2);

    const int topY = 40;
    const int rowH = 16;        // pixels in framebuffer
    const int visibleRows = (FB_HEIGHT - topY) / rowH;

    // Clamp scroll so selected stays visible
    if (menu_selected < menu_scroll) menu_scroll = menu_selected;
    if (menu_selected >= menu_scroll + visibleRows) menu_scroll = menu_selected - visibleRows + 1;
    if (menu_scroll < 0) menu_scroll = 0;
    if (menu_scroll > (int)MENU_COUNT - visibleRows) menu_scroll = (int)MENU_COUNT - visibleRows;
    if (menu_scroll < 0) menu_scroll = 0;

    for (int i = 0; i < visibleRows; i++)
    {
        int idx = menu_scroll + i;
        if (idx >= (int)MENU_COUNT) break;

        int y = topY + i * rowH;

        const int isSel = (idx == menu_selected);

        // Highlight bar
        if (isSel)
        	LCD_DrawRect(6, y - 2, FB_WIDTH - 12, rowH, RGB565_BLUE);

        // Text: invert colors if selected
        uint16_t fg = isSel ? RGB565_WHITE : RGB565_GRAY;
        uint16_t bg = isSel ? RGB565_BLUE  : RGB565_BLACK;

        LCD_DrawText(14, y, menuItems[idx].title, fg, bg, 2);
    }

    // Hint line
//    LCD_DrawText(8, FB_HEIGHT - 14, "A=START  B=BACK", RGB565_GRAY, RGB565_BLACK, 1);
}

uint8_t Menu_Update(uint16_t pressed, uint16_t held) {

	(void)held;

	if (pressed & (1u << BTN_DOWN)) {
		menu_selected++;
	}

	if (pressed & (1u << BTN_UP)) {
		menu_selected--;
	}

	if (menu_selected < 0) {
		menu_selected = (uint8_t)MENU_COUNT - 1;
	}

	if (menu_selected >= (uint8_t)MENU_COUNT) {
		menu_selected = 0;
	}

	const uint8_t topY;
	const uint8_t rowH;
	const int visibleRows = (FB_HEIGHT - topY) / rowH;

	if (menu_selected < menu_scroll) {
		menu_scroll = menu_selected;
	}

	if (menu_selected >= menu_scroll + visibleRows) {
		menu_scroll = menu_selected - visibleRows + 1;
	}

	const int maxScroll = (int)MENU_COUNT - visibleRows;
	menu_scroll = clampi(menu_scroll, 0, (maxScroll > 0) ? maxScroll : 0);

	int chosen = -1;
	if (pressed & (1u << BTN_A))
		chosen = menu_selected;

	LCD_MenuDraw();
	return chosen;
}














/*
 * menu.h
 *
 *  Created on: 6 Feb 2026
 *      Author: motre
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

#include <stdint.h>

typedef struct {
	const char *title;
} MenuItem;

static const MenuItem menuItems[] = {
	{"TETRIS"},
	{"SNAKE"},
};

#define MENU_COUNT (sizeof(menuItems) / sizeof(menuItems[0]))

static uint8_t menu_selected = 0;
static int menu_scroll = 0;

void LCD_MenuDraw(void);

void Menu_Update(uint16_t pressed, uint16_t held);

#endif /* INC_MENU_H_ */

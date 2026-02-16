/*
 * loop.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "globals.h"

#include "main.h"
#include "gpio.h"
#include "loop.h"
#include "display.h"

#include "text.h"
#include "menu.h"
#include <stdbool.h>
#include "snake.h"

#include "buttons.h"
#include "audio.h"

bool once = true;
uint32_t last = 0;
uint8_t direction = 0;

void loop(void) {
    uint32_t now_ms = HAL_GetTick();

    LCD_ClearFrame();

    Buttons_BeginFrame();

    uint16_t pressed = Buttons_PressedSnapshot();
	uint16_t held    = Buttons_State();
	uint16_t held_ev = Buttons_HeldSnapshot();

	switch (g_state) {
	case STATE_MENU: {

		int chosen = Menu_Update(pressed, held);
		if (chosen >= 0) {
			if (chosen == 0) g_state = STATE_TETRIS;
			if (chosen == 1) g_state = STATE_SNAKE;
		}
	} break;

	case STATE_SNAKE:
		Snake_Update(pressed, held_ev);
		break;

	default:
		break;
	}

	render_dma();
}

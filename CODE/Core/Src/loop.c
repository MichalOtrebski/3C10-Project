/*
 * loop.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "main.h"
#include "gpio.h"
#include "loop.h"
#include "display.h"
#include "globals.h"
#include "text.h"
#include "menu.h"
#include <stdbool.h>
#include "snake.h"

bool once = true;
uint32_t last = 0;

void loop() {
	uint32_t t0 = DWT->CYCCNT;
//	static uint8_t x = 0;

	LCD_ClearFrame();

	border(1, 1, 1, 1, 0x2134);

//	LCD_DrawRect(x, 80, 5, 5, 0xf800);

//	LCD_DrawText(10, 10, "\"HELLO JOSH\"", 0x07E0, 0x9492, 1);
//	LCD_DrawText(10, 20, "- JOHN CAVAN", 0x07E0, 0x0000, 1);

//	x+= 5;
//	if (x + 5 >= FB_WIDTH - 2) x = 2;

    uint16_t pressed = Buttons_PressedEvents();
    uint16_t held    = Buttons_State();

	switch(g_state) {
	case STATE_MENU:
		Menu_update(pressed, held);
		break;

	case STATE_TETRIS:
		Tetris_Update(pressed, held);
		break;

	case STATE_PONG:
		Snake_Update(pressed, held);
		break;
	}


	LCD_MenuDraw();

//	if (once) {
//		Snake_Init();
//		once = false;
//	}
//
//    if(HAL_GetTick()-last>100) // speed
//    {
//        last=HAL_GetTick();
//        Snake_Tick();
//    }
//
//	Snake_Draw();




	render_dma();

	uint32_t t1 = DWT->CYCCNT;
	printf("loop=%.2f ms \r\n", (t1-t0) / (SystemCoreClock/1000.0f));
}

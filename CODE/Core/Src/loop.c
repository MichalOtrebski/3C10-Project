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

void loop() {

	static uint8_t x = 0;

	LCD_ClearFrame();

	LCD_DrawRect(5, 5, 50, 50, 0xF800);

	LCD_DrawRect(x, 80, 5, 5, 0xe201);


	x++;
	if (x == 119) x = 0;

	render();

}

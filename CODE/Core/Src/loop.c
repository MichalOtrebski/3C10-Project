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

void loop() {
	uint32_t t0 = DWT->CYCCNT;

	static uint8_t x = 0;

	LCD_ClearFrame();

//	LCD_DrawRect(5, 5, 50, 50, 0xF800);

	LCD_DrawRect(x, 80, 5, 5, 0xe201);

	LCD_DrawText(10, 10, "\"HELLO JOSH\"", 0x07E0, 0x0000, 1);
	LCD_DrawText(10, 20, "- JOHN CAVAN", 0x07E0, 0x0000, 1);

	uint32_t t1 = DWT->CYCCNT;


	x+= 5;
	if (x >= FB_WIDTH - 1) x = 0;

	render_dma();

	uint32_t t2 = DWT->CYCCNT;

	printf("cpu=%.2f ms, spi=%.2f ms\r\n",
	       (t1-t0) / (SystemCoreClock/1000.0f),
	       (t2-t1) / (SystemCoreClock/1000.0f));


}

/*
 * loop.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "main.h"
#include "gpio.h"
#include "loop.h"
#include "lcd_io.h"

static inline void LCD_FrameClear(void) {
	for (size_t i = 0; i < FB_WIDTH * FB_HEIGHT; i++) {
		framebuffer[i] = 0;
	}
}

void loop(void)
{

//	LCD_FrameClear();
//    static uint16_t line[320];
//
    // obvious colours in RGB565
//    static const uint16_t colors[] = {
//        0x0000, // black
//        0xFFFF, // white
//        0xF800, // red
//        0x07E0, // green
//        0x001F, // blue
//        0xFFE0, // yellow
//        0xF81F, // magenta
//        0x07FF  // cyan
//    };
//    static uint32_t ci = 0;
//
//    uint16_t color = colors[ci];

//    for (int x = 0; x < 320; x++) line[x] = color;
//
//    for (int y = 0; y < 240; y++)
//    {
//        uint32_t err = BSP_LCD_SetDisplayWindow(0, 0, y, 320, 1);
//        if (err) { printf("SetWindow err=%lu at y=%d\r\n", err, y); return; }
//
//        err = BSP_LCD_WriteData(0, (uint8_t*)line, 320 * 2);
//        if (err) { printf("WriteData err=%lu at y=%d\r\n", err, y); return; }
//    }
//
//    ci = (ci + 1) % (sizeof(colors)/sizeof(colors[0]));
//    HAL_Delay(250); // slow enough to see

//	LCD_DrawRect(50, 5, 10, 10, 0xF800);
//	LCD_DrawRect(30, 10, 50, 20, 0xE500);
//
//	LCD_PresentFramebuffer();
//
//	HAL_Delay(250);


//    LCD_FrameClear();
//    LCD_DrawRect(0, 0, FB_WIDTH, FB_HEIGHT, 0x0000); // black
//    LCD_DrawRect(0, 0, 5, 5, 0xFFFF);                // white block at origin
//    LCD_PresentFramebuffer();

}

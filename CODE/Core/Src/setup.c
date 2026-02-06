/*
 * setup.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "main.h"
#include "gpio.h"
#include "setup.h"

void setup(void)
{
    uint32_t err = BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
    printf("LCD_Init err=%lu\r\n", err);

    uint32_t w=0, h=0;
    printf("init=%lu\r\n", BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE));
    printf("on=%lu\r\n", BSP_LCD_DisplayOn(0));

    printf("getW=%lu\r\n", BSP_LCD_GetXSize(0, &w));
    printf("getH=%lu\r\n", BSP_LCD_GetYSize(0, &h));
    printf("LCD size = %lu x %lu\r\n", w, h);

    err = BSP_LCD_DisplayOn(0);
    printf("LCD_On err=%lu\r\n", err);
}

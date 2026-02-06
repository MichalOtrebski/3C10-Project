/*
 * display.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "display.h"
#include "lcd_conf.h"

#include "lcd_io.h"
#include "lcd_os.h"

#include "globals.h"

//void BSP_LCD_SignalTransferDone(uint32_t Instance)
//{
//    // Release the lock taken by BSP_LCD_WriteDataDMA / SetDisplayWindow etc.
//    LCD_OS_Unlock(Instance);
//}

//static inline int is_lcd_spi(SPI_HandleTypeDef *hspi)
//{
//    return (hspi == &hspi2); // change if LCD uses hspi2
//}
//
//void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//    if (hspi == &hspi2) LCD_OS_Unlock(0);
//}
//
//void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//    if (hspi == &hspi2) LCD_OS_Unlock(0);
//}
//
//void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
//{
//    if (hspi == &hspi2) LCD_OS_Unlock(0);
//}

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static inline uint16_t u16min(uint16_t a, uint16_t b){ return a<b?a:b; }

void LCD_DrawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    if (x0 >= FB_WIDTH || y0 >= FB_HEIGHT) return;
    x1 = u16min(x1, FB_WIDTH);
    y1 = u16min(y1, FB_HEIGHT);
    if (x1 <= x0 || y1 <= y0) return;

    for (uint16_t y = y0; y < y1; y++)
    {
        uint16_t *row = &framebuffer[y * FB_WIDTH];
        for (uint16_t x = x0; x < x1; x++)
            row[x] = color;
    }
}

void LCD_PresentFramebuffer(void)
{
    uint32_t lcdW=0, lcdH=0;
    BSP_LCD_GetXSize(0, &lcdW);
    BSP_LCD_GetYSize(0, &lcdH);

    // Expect 320x240 for your 2x scaling. If not, stop and fix orientation.
    if (lcdW != 320 || lcdH != 240) {
        printf("bad lcd size %lux%lu\r\n", lcdW, lcdH);
        return;
    }

    for (int y = 0; y < FB_HEIGHT; y++)
    {
        // Build the scaled line (fills ALL 320 pixels)
        for (int x = 0; x < FB_WIDTH; x++)
        {
            uint16_t c = framebuffer[y * FB_WIDTH + x];
            dmaLine[2*x]   = c;
            dmaLine[2*x+1] = c;
        }

        for (int v = 0; v < 2; v++)
        {
            uint32_t yy = (uint32_t)(y*2 + v);

            int32_t err = BSP_LCD_SetDisplayWindow(0, 0, yy, 320, 1);
            if (err) { printf("win err=%ld yy=%lu\r\n", (long)err, yy); return; }

            err = BSP_LCD_WriteDataDMA(0, (uint8_t*)dmaLine, 320 * 2);
            if (err) { printf("dma err=%ld yy=%lu\r\n", (long)err, yy); return; }

            BSP_LCD_WaitForTransferToBeDone(0);
        }
    }
}

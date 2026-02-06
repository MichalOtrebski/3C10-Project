/*
 * display.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "display.h"
#include "globals.h"
#include "lcd_io.h"
#include "lcd_os.h"

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi == &hspi2)
  {
    LCD_CS_HIGH();
    hspi->Init.DataSize = SPI_DATASIZE_8BIT;
    HAL_SPI_Init(hspi);
    LCD_OS_UnlockFromISR(0);
    BSP_LCD_SignalTransferDone(0);
  }
}

void LCD_ClearFrame() {
	for (size_t i = 0; i < FB_WIDTH * FB_HEIGHT; i++) {
		framebuffer[i] = 0;
	}
}

void LCD_DrawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
{

    if (x0 >= FB_WIDTH || y0 >= FB_HEIGHT) return;

    if (x0 + w > FB_WIDTH)  w = FB_WIDTH  - x0;
    if (y0 + h > FB_HEIGHT) h = FB_HEIGHT - y0;
    if (w == 0 || h == 0) return;

    for (uint16_t y = y0; y < (uint16_t)(y0 + h); y++)
    {
        uint16_t *row = &framebuffer[y * FB_WIDTH];
        for (uint16_t x = x0; x < (uint16_t)(x0 + w); x++)
            row[x] = color;
    }
}

void render() {

	BSP_LCD_SetDisplayWindow(0, 0, 0, LCD_WIDTH, LCD_HEIGHT);

	for (size_t y = 0; y < FB_HEIGHT; y++) {
		for (size_t x = 0; x < FB_WIDTH; x++) {
            uint16_t c = framebuffer[y * FB_WIDTH + x];
            buf[2*x]     = c;
            buf[2*x + 1] = c;
		}


		for (uint32_t v = 0; v < 2; v++) {
			uint32_t yy = y * 2 + v;

			// Window: one line tall
			BSP_LCD_SetDisplayWindow(0, 0, yy, LCD_WIDTH, 1);

			// Send 320 pixels = 640 bytes
			BSP_LCD_WriteData(0, (uint8_t*)buf, LCD_WIDTH * 2);
		}
	}
}

void render_dma(void)
{
  for (uint32_t y = 0; y < FB_HEIGHT; y++)
  {
    for (uint32_t v = 0; v < 2; v++)
    {
      uint32_t yy = y*2 + v;

      BSP_LCD_WaitForTransferToBeDone(0);   // <- critical

      for (uint32_t x = 0; x < FB_WIDTH; x++)
      {
        uint16_t c = framebuffer[y*FB_WIDTH + x];
        buf[2*x]     = c;
        buf[2*x + 1] = c;
      }

      BSP_LCD_SetDisplayWindow(0, 0, yy, LCD_WIDTH , 1);
      BSP_LCD_WriteDataDMA(0, (uint8_t*)buf, LCD_WIDTH * 2);
    }
  }

  BSP_LCD_WaitForTransferToBeDone(0);
}












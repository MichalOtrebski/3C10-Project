/*
 * display.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "display.h"
#include "lcd_io.h"
#include "lcd_os.h"
#include "lcd_conf.h"
#include <string.h>

static uint16_t line_buf_a[LCD_WIDTH];
static uint16_t line_buf_b[LCD_WIDTH];

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &hspi2)
    {
        LCD_CS_HIGH();

        /*
         * Keep this only if your LCD stack requires SPI to return to 8-bit mode
         * after each DMA transfer. If not strictly required, removing it can
         * improve performance a lot.
         */
        hspi->Init.DataSize = SPI_DATASIZE_8BIT;
        HAL_SPI_Init(hspi);

        LCD_OS_UnlockFromISR(0);
        BSP_LCD_SignalTransferDone(0);
    }
}

void LCD_ClearFrame(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

void LCD_DrawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
{
    if (x0 >= FB_WIDTH || y0 >= FB_HEIGHT || w == 0 || h == 0)
        return;

    if ((uint32_t)x0 + w > FB_WIDTH)
        w = FB_WIDTH - x0;

    if ((uint32_t)y0 + h > FB_HEIGHT)
        h = FB_HEIGHT - y0;

    uint16_t *row = &framebuffer[y0 * FB_WIDTH + x0];

    for (uint16_t y = 0; y < h; y++)
    {
        uint16_t *p = row;
        uint16_t n = w;

        while (n >= 4)
        {
            p[0] = color;
            p[1] = color;
            p[2] = color;
            p[3] = color;
            p += 4;
            n -= 4;
        }

        while (n--)
        {
            *p++ = color;
        }

        row += FB_WIDTH;
    }
}

static inline void build_scaled_row_fast(const uint16_t *src, uint16_t *dst)
{
    uint32_t n = FB_WIDTH;

    while (n--)
    {
        uint16_t c = *src++;
        *dst++ = c;
        *dst++ = c;
    }
}

void render_dma(void)
{
    uint16_t *cur_buf  = line_buf_a;
    uint16_t *next_buf = line_buf_b;

    const uint16_t *src = framebuffer;
    uint32_t y = 0;

    /* Prebuild first row */
    build_scaled_row_fast(src, cur_buf);

    for (y = 0; y < FB_HEIGHT; y++)
    {
        uint32_t yy = y << 1;

        /* Send first vertical copy */
        BSP_LCD_WaitForTransferToBeDone(0);
        BSP_LCD_SetDisplayWindow(0, 0, yy, LCD_WIDTH, 1);
        BSP_LCD_WriteDataDMA(0, (uint8_t *)cur_buf, LCD_WIDTH * 2);

        /* Send second vertical copy */
        BSP_LCD_WaitForTransferToBeDone(0);
        BSP_LCD_SetDisplayWindow(0, 0, yy + 1, LCD_WIDTH, 1);
        BSP_LCD_WriteDataDMA(0, (uint8_t *)cur_buf, LCD_WIDTH * 2);

        /* Build next row into alternate buffer */
        if (y + 1 < FB_HEIGHT)
        {
            src += FB_WIDTH;

            BSP_LCD_WaitForTransferToBeDone(0);
            build_scaled_row_fast(src, next_buf);

            uint16_t *tmp = cur_buf;
            cur_buf = next_buf;
            next_buf = tmp;
        }
    }

    BSP_LCD_WaitForTransferToBeDone(0);
}

void border(uint8_t t, uint8_t b, uint8_t l, uint8_t r, uint16_t color)
{
    uint16_t mid_h = 0;

    if (t) LCD_DrawRect(0, 0, FB_WIDTH, t, color);
    if (b) LCD_DrawRect(0, FB_HEIGHT - b, FB_WIDTH, b, color);

    if (FB_HEIGHT > (uint16_t)(t + b))
        mid_h = FB_HEIGHT - t - b;

    if (l && mid_h) LCD_DrawRect(0, t, l, mid_h, color);
    if (r && mid_h) LCD_DrawRect(FB_WIDTH - r, t, r, mid_h, color);
}

uint16_t Darken565(uint16_t c, uint8_t percent) {
    // percent = how much brightness remains, e.g. 75 means 75%
    uint16_t r = (c >> 11) & 0x1F;
    uint16_t g = (c >> 5)  & 0x3F;
    uint16_t b =  c        & 0x1F;

    r = (uint16_t)((r * percent) / 100u);
    g = (uint16_t)((g * percent) / 100u);
    b = (uint16_t)((b * percent) / 100u);

    return (uint16_t)((r << 11) | (g << 5) | b);
}

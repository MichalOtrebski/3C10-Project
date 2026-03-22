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
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* DMA line buffers                                                            */
/* Each source FB row is scaled 2x horizontally and duplicated vertically, so  */
/* one source row becomes 2 LCD rows.                                          */
/* -------------------------------------------------------------------------- */

static uint16_t line_buf_a[LCD_WIDTH * 2];
static uint16_t line_buf_b[LCD_WIDTH * 2];

/* -------------------------------------------------------------------------- */
/* Dirty rectangle tracking                                                    */
/* -------------------------------------------------------------------------- */

#define MAX_DIRTY_RECTS 24

typedef struct
{
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} DirtyRect;

static DirtyRect dirty_rects[MAX_DIRTY_RECTS];
static uint16_t dirty_count = 0;
static bool full_frame_dirty = true;

/* -------------------------------------------------------------------------- */
/* SPI DMA complete callback                                                   */
/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */
/* Dirty rectangle helpers                                                     */
/* -------------------------------------------------------------------------- */

static inline bool rect_valid(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    return !(x >= FB_WIDTH || y >= FB_HEIGHT || w == 0 || h == 0);
}

static inline void clip_rect(uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h)
{
    if ((uint32_t)(*x) + *w > FB_WIDTH)
        *w = FB_WIDTH - *x;

    if ((uint32_t)(*y) + *h > FB_HEIGHT)
        *h = FB_HEIGHT - *y;
}

static inline bool rects_overlap_or_touch(const DirtyRect *a, const DirtyRect *b)
{
    uint16_t a_r = a->x + a->w;
    uint16_t a_b = a->y + a->h;
    uint16_t b_r = b->x + b->w;
    uint16_t b_b = b->y + b->h;

    return !(a_r < b->x || b_r < a->x || a_b < b->y || b_b < a->y);
}

static inline DirtyRect rect_union(const DirtyRect *a, const DirtyRect *b)
{
    DirtyRect out;
    uint16_t x1 = (a->x < b->x) ? a->x : b->x;
    uint16_t y1 = (a->y < b->y) ? a->y : b->y;
    uint16_t x2 = ((a->x + a->w) > (b->x + b->w)) ? (a->x + a->w) : (b->x + b->w);
    uint16_t y2 = ((a->y + a->h) > (b->y + b->h)) ? (a->y + a->h) : (b->y + b->h);

    out.x = x1;
    out.y = y1;
    out.w = x2 - x1;
    out.h = y2 - y1;
    return out;
}

static void mark_dirty(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (!rect_valid(x, y, w, h))
        return;

    clip_rect(&x, &y, &w, &h);

    if (full_frame_dirty)
        return;

    DirtyRect nr = { x, y, w, h };

    /* Try to merge into existing rects first */
    for (uint16_t i = 0; i < dirty_count; i++)
    {
        if (rects_overlap_or_touch(&dirty_rects[i], &nr))
        {
            dirty_rects[i] = rect_union(&dirty_rects[i], &nr);

            /* Second pass: collapse any newly-overlapping rects */
            for (uint16_t j = 0; j < dirty_count; )
            {
                if (j != i && rects_overlap_or_touch(&dirty_rects[i], &dirty_rects[j]))
                {
                    dirty_rects[i] = rect_union(&dirty_rects[i], &dirty_rects[j]);

                    for (uint16_t k = j; k + 1 < dirty_count; k++)
                        dirty_rects[k] = dirty_rects[k + 1];

                    dirty_count--;
                    if (j < i) i--;
                }
                else
                {
                    j++;
                }
            }
            return;
        }
    }

    if (dirty_count < MAX_DIRTY_RECTS)
    {
        dirty_rects[dirty_count++] = nr;
    }
    else
    {
        full_frame_dirty = true;
        dirty_count = 0;
    }
}

static void clear_dirty_state(void)
{
    dirty_count = 0;
    full_frame_dirty = false;
}

/* -------------------------------------------------------------------------- */
/* Framebuffer drawing                                                         */
/* -------------------------------------------------------------------------- */

void LCD_ClearFrame(void)
{
    memset(framebuffer, 0, sizeof(framebuffer));
    full_frame_dirty = true;
    dirty_count = 0;
}

void LCD_DrawRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color)
{
    if (!rect_valid(x0, y0, w, h))
        return;

    clip_rect(&x0, &y0, &w, &h);

    uint16_t *row0 = &framebuffer[y0 * FB_WIDTH + x0];

    /* Fill first row */
    uint16_t n = w;
    uint16_t *p = row0;

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

    /* Copy first row into remaining rows */
    size_t bytes = (size_t)w * sizeof(uint16_t);
    for (uint16_t y = 1; y < h; y++)
    {
        memcpy(&framebuffer[(y0 + y) * FB_WIDTH + x0], row0, bytes);
    }

    mark_dirty(x0, y0, w, h);
}

/* -------------------------------------------------------------------------- */
/* Scaling helpers                                                             */
/* -------------------------------------------------------------------------- */

static inline void build_scaled_2rows_full(const uint16_t *src, uint16_t *dst)
{
    uint16_t *dst_row0 = dst;
    uint16_t *dst_row1 = dst + LCD_WIDTH;

    for (uint32_t x = 0; x < FB_WIDTH; x++)
    {
        uint16_t c = src[x];
        uint32_t xx = x << 1;

        dst_row0[xx]     = c;
        dst_row0[xx + 1] = c;
        dst_row1[xx]     = c;
        dst_row1[xx + 1] = c;
    }
}

static inline void build_scaled_2rows_partial(const uint16_t *src, uint16_t *dst, uint16_t src_w)
{
    uint16_t lcd_w = (uint16_t)(src_w << 1);
    uint16_t *dst_row0 = dst;
    uint16_t *dst_row1 = dst + lcd_w;

    for (uint16_t x = 0; x < src_w; x++)
    {
        uint16_t c = src[x];
        uint16_t xx = (uint16_t)(x << 1);

        dst_row0[xx]     = c;
        dst_row0[xx + 1] = c;
        dst_row1[xx]     = c;
        dst_row1[xx + 1] = c;
    }
}

/* -------------------------------------------------------------------------- */
/* Render helpers                                                              */
/* -------------------------------------------------------------------------- */

static void render_full_dma_internal(void)
{
    uint16_t *cur_buf  = line_buf_a;
    uint16_t *next_buf = line_buf_b;
    const uint16_t *src = framebuffer;

    build_scaled_2rows_full(src, cur_buf);

    for (uint32_t y = 0; y < FB_HEIGHT; y++)
    {
        uint32_t yy = y << 1;

        BSP_LCD_WaitForTransferToBeDone(0);
        BSP_LCD_SetDisplayWindow(0, 0, yy, LCD_WIDTH, 2);
        BSP_LCD_WriteDataDMA(0, (uint8_t *)cur_buf, LCD_WIDTH * 2 * sizeof(uint16_t));

        if (y + 1 < FB_HEIGHT)
        {
            src += FB_WIDTH;
            build_scaled_2rows_full(src, next_buf);

            uint16_t *tmp = cur_buf;
            cur_buf = next_buf;
            next_buf = tmp;
        }
    }

    BSP_LCD_WaitForTransferToBeDone(0);
}

static void flush_rect_scaled(const DirtyRect *r)
{
    uint16_t *cur_buf  = line_buf_a;
    uint16_t *next_buf = line_buf_b;

    uint16_t src_x = r->x;
    uint16_t src_y = r->y;
    uint16_t src_w = r->w;
    uint16_t src_h = r->h;

    uint16_t lcd_x = (uint16_t)(src_x << 1);
    uint16_t lcd_y = (uint16_t)(src_y << 1);
    uint16_t lcd_w = (uint16_t)(src_w << 1);

    const uint16_t *src = &framebuffer[src_y * FB_WIDTH + src_x];

    build_scaled_2rows_partial(src, cur_buf, src_w);

    for (uint16_t row = 0; row < src_h; row++)
    {
        uint16_t yy = (uint16_t)(lcd_y + (row << 1));

        BSP_LCD_WaitForTransferToBeDone(0);
        BSP_LCD_SetDisplayWindow(0, lcd_x, yy, lcd_w, 2);
        BSP_LCD_WriteDataDMA(0, (uint8_t *)cur_buf, lcd_w * 2 * sizeof(uint16_t));

        if ((uint16_t)(row + 1) < src_h)
        {
            src += FB_WIDTH;
            build_scaled_2rows_partial(src, next_buf, src_w);

            uint16_t *tmp = cur_buf;
            cur_buf = next_buf;
            next_buf = tmp;
        }
    }

    BSP_LCD_WaitForTransferToBeDone(0);
}

/* -------------------------------------------------------------------------- */
/* Public render entry                                                         */
/* -------------------------------------------------------------------------- */

void render_dma(void)
{
    if (full_frame_dirty)
    {
        render_full_dma_internal();
        clear_dirty_state();
        return;
    }

    if (dirty_count == 0)
        return;

    /*
     * Heuristic:
     * if too many dirty rects, or dirty area is large, full refresh is cheaper.
     */
    uint32_t total_area = 0;
    for (uint16_t i = 0; i < dirty_count; i++)
        total_area += (uint32_t)dirty_rects[i].w * dirty_rects[i].h;

    if (dirty_count > 10 || total_area > ((uint32_t)FB_WIDTH * FB_HEIGHT) / 3U)
    {
        render_full_dma_internal();
        clear_dirty_state();
        return;
    }

    for (uint16_t i = 0; i < dirty_count; i++)
        flush_rect_scaled(&dirty_rects[i]);

    clear_dirty_state();
}

/* -------------------------------------------------------------------------- */
/* Helpers                                                                     */
/* -------------------------------------------------------------------------- */

void border(uint8_t t, uint8_t b, uint8_t l, uint8_t r, uint16_t color)
{
    uint16_t mid_h = 0;

    if (t)
        LCD_DrawRect(0, 0, FB_WIDTH, t, color);

    if (b)
        LCD_DrawRect(0, FB_HEIGHT - b, FB_WIDTH, b, color);

    if (FB_HEIGHT > (uint16_t)(t + b))
        mid_h = FB_HEIGHT - t - b;

    if (l && mid_h)
        LCD_DrawRect(0, t, l, mid_h, color);

    if (r && mid_h)
        LCD_DrawRect(FB_WIDTH - r, t, r, mid_h, color);
}

uint16_t Darken565(uint16_t c, uint8_t percent)
{
    uint16_t r = (c >> 11) & 0x1F;
    uint16_t g = (c >> 5)  & 0x3F;
    uint16_t b =  c        & 0x1F;

    r = (uint16_t)((r * percent) / 100u);
    g = (uint16_t)((g * percent) / 100u);
    b = (uint16_t)((b * percent) / 100u);

    return (uint16_t)((r << 11) | (g << 5) | b);
}

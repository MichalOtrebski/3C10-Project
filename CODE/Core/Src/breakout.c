#include "breakout.h"
#include <stdlib.h>
#include "display.h"
#include "globals.h"
#include "text.h"
#include <stdbool.h>
#include "buttons.h"
#include <math.h>
#include "menu.h"
#include <limits.h>

/*
 * Power-up meanings:
 *  - bloc_pow1: SCORE BOOST   -> block worth double score for a short time
 *  - bloc_pow2: WIDE PADDLE   -> paddle becomes wider for a short time
 *  - bloc_pow3: SLOW BALL     -> ball speed is reduced for a short time
 */

static bool bloc_exst[BLK_YQ][BLK_XQ];
static bool bloc_pow1[BLK_YQ][BLK_XQ];
static bool bloc_pow2[BLK_YQ][BLK_XQ];
static bool bloc_pow3[BLK_YQ][BLK_XQ];

static bool ball_in = true;
static bool game_ran = true;
static bool ball_released = false;

static float prev_ball_x = 0.0f;
static float prev_ball_y = 0.0f;
static float plat_fx = 0.0f;

static uint32_t s_lastGameTickMs;
#define LOCAL_GAME_TICK_MS 8u

static int16_t prev_draw_ball_x = -1;
static int16_t prev_draw_ball_y = -1;
static int16_t prev_draw_plat_x = -1;
static int16_t prev_draw_plat_y = -1;
static uint16_t prev_score_drawn = 0xFFFF;
static uint8_t prev_active_power_mask = 0xFFu;
static uint16_t prev_score_colour = 0xFFFFu;

static uint8_t block_wave = 0;

#define LOCAL_BALL_MIN_VX      6.0f
#define PLATFORM_SPEED         180.0f
#define POWER_STATUS_Y         8
#define POWER_STATUS_H         10
#define POWER_STATUS_W         42
#define POWER_LABEL_SCORE      "2X"
#define POWER_LABEL_WIDE       "WD"
#define POWER_LABEL_SLOW       "SL"
#define PADDLE_WIDE_BONUS      14
#define SLOW_BALL_FACTOR       0.78f
#define SCORE_PER_BLOCK        10u
#define MAX_PHYS_SUBSTEPS      16

#define SCORE_LABEL_X          4
#define SCORE_VALUE_X          38
#define SCORE_VALUE_W          34
#define POWER_STATUS_X         (FB_WIDTH - POWER_STATUS_W - 4)

static uint32_t power_score_until_ms = 0;
static uint32_t power_wide_until_ms = 0;
static uint32_t power_slow_until_ms = 0;

static uint16_t current_plat_wdh = PLT_WDH;
static float current_ball_speed = BALL_VEL;

float ball_velx = 0.0f;
float ball_vely = 0.0f;

float pos_ball[2] = {0.0f, 0.0f};
uint16_t pos_plat[2] = {0, 0};

uint16_t score = 0;

/* -------------------- HELPERS -------------------- */

static bool blocks_remaining(void) {
    for (int j = 0; j < BLK_YQ; j++) {
        for (int i = 0; i < BLK_XQ; i++) {
            if (bloc_exst[j][i]) {
                return true;
            }
        }
    }
    return false;
}

static void clear_block_powers(int j, int i) {
    bloc_pow1[j][i] = false;
    bloc_pow2[j][i] = false;
    bloc_pow3[j][i] = false;
}

static void assign_power_block(int j, int i, int type) {
    clear_block_powers(j, i);

    switch (type) {
        case 1: bloc_pow1[j][i] = true; break;
        case 2: bloc_pow2[j][i] = true; break;
        case 3: bloc_pow3[j][i] = true; break;
        default: break;
    }
}

static uint16_t block_base_colour_for_row(int row) {
    switch (row) {
        case 0: return BLK_RED;
        case 1: return BLK_YLW;
        case 2: return BLK_GRN;
        case 3: return BLK_BLU;
        case 4: return BLK_PRP;
        default: return PLT_WHT;
    }
}

static uint16_t score_text_colour(void) {
    return (HAL_GetTick() < power_score_until_ms) ? PLT_WHT : C_SCORE;
}

static uint8_t active_power_mask(void) {
    uint32_t now = HAL_GetTick();
    uint8_t mask = 0;

    if (now < power_score_until_ms) {
        mask |= 0x01u;
    }
    if (now < power_wide_until_ms) {
        mask |= 0x02u;
    }
    if (now < power_slow_until_ms) {
        mask |= 0x04u;
    }

    return mask;
}

static void position_ball_on_paddle(void) {
    pos_ball[0] = (float)pos_plat[0];
    pos_ball[1] = (float)(pos_plat[1] - (PLT_HGT / 2) - (BALL_HGT / 2) - 1);
    prev_ball_x = pos_ball[0];
    prev_ball_y = pos_ball[1];
}

static void refresh_active_powers(void) {
    uint32_t now = HAL_GetTick();

    current_plat_wdh = PLT_WDH;
    current_ball_speed = BALL_VEL;

    if (now < power_wide_until_ms) {
        current_plat_wdh = (uint16_t)(PLT_WDH + PADDLE_WIDE_BONUS);
    }

    if (now < power_slow_until_ms) {
        current_ball_speed = BALL_VEL * SLOW_BALL_FACTOR;
    }
}

static void enforce_min_horizontal_speed(void) {
    float min_vx = LOCAL_BALL_MIN_VX;

    if (current_ball_speed > 0.0f && min_vx > current_ball_speed * 0.85f) {
        min_vx = current_ball_speed * 0.85f;
    }

    if (ball_velx > -min_vx && ball_velx < min_vx) {
        if (ball_velx < 0.0f) {
            ball_velx = -min_vx;
        } else {
            ball_velx = min_vx;
        }

        {
            float vy_sq = (current_ball_speed * current_ball_speed) - (ball_velx * ball_velx);
            if (vy_sq < 1.0f) {
                vy_sq = 1.0f;
            }

            if (ball_vely < 0.0f) {
                ball_vely = -sqrtf(vy_sq);
            } else {
                ball_vely = sqrtf(vy_sq);
            }
        }
    }
}

static void renormalise_ball_speed(void) {
    float mag = sqrtf((ball_velx * ball_velx) + (ball_vely * ball_vely));

    if (mag < 0.001f) {
        return;
    }

    ball_velx = (ball_velx / mag) * current_ball_speed;
    ball_vely = (ball_vely / mag) * current_ball_speed;
    enforce_min_horizontal_speed();
}

static void draw_power_status_if_needed(void) {
    uint8_t mask = active_power_mask();

    if (mask == prev_active_power_mask) {
        return;
    }

    LCD_DrawRect(POWER_STATUS_X, POWER_STATUS_Y, POWER_STATUS_W, POWER_STATUS_H, C_BG);

    if (mask != 0u) {
        int x = POWER_STATUS_X;

        if (mask & 0x01u) {
            LCD_DrawText(x, POWER_STATUS_Y, POWER_LABEL_SCORE, PLT_WHT, C_BG, 1);
            x += 14;
        }
        if (mask & 0x02u) {
            LCD_DrawText(x, POWER_STATUS_Y, POWER_LABEL_WIDE, BLK_BLU, C_BG, 1);
            x += 14;
        }
        if (mask & 0x04u) {
            LCD_DrawText(x, POWER_STATUS_Y, POWER_LABEL_SLOW, BLK_PRP, C_BG, 1);
        }
    }

    prev_active_power_mask = mask;
}

static void draw_block_cell(int j, int i) {
    if (!bloc_exst[j][i]) {
        return;
    }

    int x = FST_BLK_SPX + i * BLK_WDH;
    int y = FST_BLK_SPY + j * BLK_HGT;
    uint16_t base_colour = block_base_colour_for_row(j);
    uint16_t accent = C_BG;
    bool p1 = bloc_pow1[j][i];
    bool p2 = bloc_pow2[j][i];
    bool p3 = bloc_pow3[j][i];

    LCD_DrawRect(x, y, BLK_WDH, BLK_HGT, base_colour);

    if (p1) {
        /* SCORE BOOST: compact centered X */
        int cx = x + (BLK_WDH / 2);
        int cy = y + (BLK_HGT / 2);

        LCD_DrawRect(cx - 2, cy - 2, 1, 1, accent);
        LCD_DrawRect(cx + 1, cy - 2, 1, 1, accent);
        LCD_DrawRect(cx - 1, cy - 1, 1, 1, accent);
        LCD_DrawRect(cx,     cy - 1, 1, 1, accent);
        LCD_DrawRect(cx,     cy,     1, 1, accent);
        LCD_DrawRect(cx - 1, cy,     1, 1, accent);
        LCD_DrawRect(cx - 2, cy + 1, 1, 1, accent);
        LCD_DrawRect(cx + 1, cy + 1, 1, 1, accent);

    } else if (p2) {
    	/* WIDE PADDLE: plus */
    	int cx = x + (BLK_WDH / 2);
    	int cy = y + (BLK_HGT / 2);

    	LCD_DrawRect(cx - 2, cy, 5, 1, C_BG);
    	LCD_DrawRect(cx, cy - 2, 1, 5, C_BG);

    } else if (p3) {
        /* SLOW BALL: centered minus */
        int cx = x + (BLK_WDH / 2);
        int cy = y + (BLK_HGT / 2);

        LCD_DrawRect(cx - 2, cy, 5, 1, accent);
    }
}

static void draw_static_scene(void) {
    LCD_DrawRect(0, 0, FB_WIDTH, FB_HEIGHT, C_BG);
    LCD_DrawText(SCORE_LABEL_X, 8, "SCORE", C_TEXT, C_BG, 1);
    border(4, 4, 4, 4, C_TEXT);
    LCD_DrawRect(0, UI_TOP, FB_WIDTH, 2, C_TEXT);
}

static void draw_score_if_needed(void) {
    uint16_t col = score_text_colour();

    if (score != prev_score_drawn || col != prev_score_colour) {
        char buf[12];
        LCD_DrawRect(SCORE_VALUE_X, 8, SCORE_VALUE_W, 10, C_BG);
        itoa(score, buf, 10);
        LCD_DrawText(SCORE_VALUE_X, 8, buf, col, C_BG, 1);
        prev_score_drawn = score;
        prev_score_colour = col;
    }
}

static void erase_old_ball(void) {
    if (prev_draw_ball_x >= 0 && prev_draw_ball_y >= 0) {
        int x = prev_draw_ball_x - BALL_WDH / 2;
        int y = prev_draw_ball_y - BALL_HGT / 2;
        int w = BALL_WDH;
        int h = BALL_HGT;

        int field_left   = FIELD_X + 1;
        int field_right  = FIELD_X + FIELD_W_BREAKOUT - 1;
        int field_top    = FIELD_Y + 1;
        int field_bottom = FIELD_Y + FIELD_H_BREAKOUT - 1;

        /* Clip horizontally */
        if (x < field_left) {
            w -= (field_left - x);
            x = field_left;
        }
        if (x + w > field_right) {
            w = field_right - x;
        }

        /* Clip vertically */
        if (y < field_top) {
            h -= (field_top - y);
            y = field_top;
        }
        if (y + h > field_bottom) {
            h = field_bottom - y;
        }

        if (w > 0 && h > 0) {
            LCD_DrawRect(x, y, w, h, C_BG);
        }
    }
}

static void erase_old_paddle(void) {
    if (prev_draw_plat_x >= 0 && prev_draw_plat_y >= 0) {
        int erase_w = (int)(PLT_WDH + PADDLE_WIDE_BONUS + 6);

        int x = prev_draw_plat_x - (erase_w / 2);
        int y = prev_draw_plat_y - (PLT_HGT / 2);
        int w = erase_w;
        int h = PLT_HGT;

        int field_left   = FIELD_X + 1;
        int field_right  = FIELD_X + FIELD_W_BREAKOUT - 1;
        int field_top    = FIELD_Y + 1;
        int field_bottom = FIELD_Y + FIELD_H_BREAKOUT - 1;

        /* Clip horizontally */
        if (x < field_left) {
            w -= (field_left - x);
            x = field_left;
        }
        if (x + w > field_right) {
            w = field_right - x;
        }

        /* Clip vertically (usually not needed, but safe) */
        if (y < field_top) {
            h -= (field_top - y);
            y = field_top;
        }
        if (y + h > field_bottom) {
            h = field_bottom - y;
        }

        if (w > 0 && h > 0) {
            LCD_DrawRect(x, y, w, h, C_BG);
        }
    }
}

static void draw_ball_current(void) {
    int bx = (int)roundf(pos_ball[0]);
    int by = (int)roundf(pos_ball[1]);

    int x = bx - BALL_WDH / 2;
    int y = by - BALL_HGT / 2;
    int w = BALL_WDH;
    int h = BALL_HGT;

    int field_left   = FIELD_X + 1;
    int field_right  = FIELD_X + FIELD_W_BREAKOUT - 1;
    int field_top    = FIELD_Y + 1;
    int field_bottom = FIELD_Y + FIELD_H_BREAKOUT - 1;

    if (x < field_left) {
        w -= (field_left - x);
        x = field_left;
    }
    if (x + w > field_right) {
        w = field_right - x;
    }

    if (y < field_top) {
        h -= (field_top - y);
        y = field_top;
    }
    if (y + h > field_bottom) {
        h = field_bottom - y;
    }

    if (w > 0 && h > 0) {
        LCD_DrawRect(x, y, w, h, PLT_WHT);
    }

    prev_draw_ball_x = (int16_t)bx;
    prev_draw_ball_y = (int16_t)by;
}

static void draw_paddle_current(void) {
    int px = pos_plat[0];
    int py = pos_plat[1];
    uint16_t paddle_colour = (HAL_GetTick() < power_wide_until_ms) ? BLK_BLU : PLT_WHT;

    int x = px - (current_plat_wdh / 2);
    int y = py - (PLT_HGT / 2);
    int w = current_plat_wdh;
    int h = PLT_HGT;

    int field_left   = FIELD_X + 1;
    int field_right  = FIELD_X + FIELD_W_BREAKOUT - 1;
    int field_top    = FIELD_Y + 1;
    int field_bottom = FIELD_Y + FIELD_H_BREAKOUT - 1;

    if (x < field_left) {
        w -= (field_left - x);
        x = field_left;
    }
    if (x + w > field_right) {
        w = field_right - x;
    }

    if (y < field_top) {
        h -= (field_top - y);
        y = field_top;
    }
    if (y + h > field_bottom) {
        h = field_bottom - y;
    }

    if (w > 0 && h > 0) {
        LCD_DrawRect(x, y, w, h, paddle_colour);

        if (w >= 2) {
            LCD_DrawRect(x, y, 2, h, PLT_WHT);
            LCD_DrawRect(x + w - 2, y, 2, h, PLT_WHT);
        } else {
            LCD_DrawRect(x, y, w, h, PLT_WHT);
        }
    }

    prev_draw_plat_x = (int16_t)px;
    prev_draw_plat_y = (int16_t)py;
}

static void respawn_blocks(void) {
    block_wave++;

    for (int j = 0; j < BLK_YQ; j++) {
        for (int i = 0; i < BLK_XQ; i++) {
            bloc_exst[j][i] = true;
            clear_block_powers(j, i);
        }
    }

    {
        int total_blocks = BLK_XQ * BLK_YQ;
        int power_count = (total_blocks / 8) + block_wave;

        if (power_count < 3) {
            power_count = 3;
        }
        if (power_count > total_blocks / 2) {
            power_count = total_blocks / 2;
        }

        for (int n = 0; n < power_count; ) {
            int i = rand() % BLK_XQ;
            int j = rand() % BLK_YQ;
            int type;

            if (bloc_pow1[j][i] || bloc_pow2[j][i] || bloc_pow3[j][i]) {
                continue;
            }

            type = 1 + (rand() % 3);
            assign_power_block(j, i, type);
            n++;
        }
    }

    block_draw();
}

static void handle_block_clear(void) {
    if (!blocks_remaining()) {
        erase_old_ball();
        erase_old_paddle();

        ball_released = false;
        ball_velx = 0.0f;
        ball_vely = 0.0f;

        plat_fx = (float)PLT_SPX;
        pos_plat[0] = PLT_SPX;
        pos_plat[1] = PLT_SPY;

        position_ball_on_paddle();

        respawn_blocks();

        draw_paddle_current();
        draw_ball_current();
    }
}

static void activate_block_power(bool hit_p1, bool hit_p2, bool hit_p3) {
    uint32_t now = HAL_GetTick();

    if (hit_p1) {
        power_score_until_ms = now + POWER_SCORE_MS;
    }
    if (hit_p2) {
        power_wide_until_ms = now + POWER_WIDE_MS;
    }
    if (hit_p3) {
        power_slow_until_ms = now + POWER_SLOW_MS;
    }

    refresh_active_powers();
    renormalise_ball_speed();
    prev_active_power_mask = 0xFFu;
}

/* -------------------- initialising game play -------------------- */

void block_intl(void) {
    block_wave = 0;
    respawn_blocks();
}

void block_draw(void) {
    for (int j = 0; j < BLK_YQ; j++) {
        for (int i = 0; i < BLK_XQ; i++) {
            if (!bloc_exst[j][i]) {
                continue;
            }
            draw_block_cell(j, i);
        }
    }
}

void plat_intl(void) {
    refresh_active_powers();
    plat_fx = (float)PLT_SPX;
    pos_plat[0] = PLT_SPX;
    pos_plat[1] = PLT_SPY;
    draw_paddle_current();
}

void ball_intl(void) {
    position_ball_on_paddle();
    draw_ball_current();
}

void initialise(void) {
    ball_released = false;
    ball_in = true;

    score = 0;
    ball_velx = 0.0f;
    ball_vely = 0.0f;

    power_score_until_ms = 0;
    power_wide_until_ms = 0;
    power_slow_until_ms = 0;
    refresh_active_powers();

    prev_draw_ball_x = -1;
    prev_draw_ball_y = -1;
    prev_draw_plat_x = -1;
    prev_draw_plat_y = -1;
    prev_score_drawn = 0xFFFFu;
    prev_score_colour = 0xFFFFu;
    prev_active_power_mask = 0xFFu;

    plat_fx = (float)PLT_SPX;
    pos_plat[0] = PLT_SPX;
    pos_plat[1] = PLT_SPY;

    position_ball_on_paddle();

    draw_static_scene();
    block_intl();
    draw_score_if_needed();
    draw_power_status_if_needed();
    draw_paddle_current();
    draw_ball_current();

    s_lastGameTickMs = HAL_GetTick();
}

/* -------------------- button controls -------------------- */

void release_ball(uint16_t pressed) {
    if (!ball_released && (pressed & (1u << BTN_A))) {
        float r = ((float)rand() / (float)RAND_MAX);
        refresh_active_powers();

        ball_velx = current_ball_speed * ((r - 0.5f) * 1.55f);
        if (ball_velx > current_ball_speed * 0.90f) ball_velx = current_ball_speed * 0.90f;
        if (ball_velx < -current_ball_speed * 0.90f) ball_velx = -current_ball_speed * 0.90f;

        {
            float vy_sq = (current_ball_speed * current_ball_speed) - (ball_velx * ball_velx);
            if (vy_sq < 1.0f) {
                vy_sq = 1.0f;
            }
            ball_vely = -sqrtf(vy_sq);
        }

        enforce_min_horizontal_speed();
        ball_released = true;
    }
}

/* -------------------- bounce code -------------------- */

void bouncex(void) {
    ball_velx = -ball_velx;
    enforce_min_horizontal_speed();
}

void bouncexplat(float position) {
    float offset = (position - 0.5f) * 2.0f;

    if (offset > -0.08f && offset < 0.08f) {
        offset = (offset < 0.0f) ? -0.08f : 0.08f;
    }

    if (offset < -0.95f) offset = -0.95f;
    if (offset > 0.95f)  offset = 0.95f;

    refresh_active_powers();

    ball_velx = offset * current_ball_speed;

    {
        float vy_sq = (current_ball_speed * current_ball_speed) - (ball_velx * ball_velx);
        if (vy_sq < 1.0f) {
            vy_sq = 1.0f;
        }
        ball_vely = -sqrtf(vy_sq);
    }

    enforce_min_horizontal_speed();
}

void bouncey(void) {
    ball_vely = -ball_vely;
}

void does_collide_blk(void) {
    float ball_left   = pos_ball[0] - BALL_WDH / 2.0f;
    float ball_right  = pos_ball[0] + BALL_WDH / 2.0f;
    float ball_top    = pos_ball[1] - BALL_HGT / 2.0f;
    float ball_bottom = pos_ball[1] + BALL_HGT / 2.0f;

    float prev_left   = prev_ball_x - BALL_WDH / 2.0f;
    float prev_right  = prev_ball_x + BALL_WDH / 2.0f;
    float prev_top    = prev_ball_y - BALL_HGT / 2.0f;
    float prev_bottom = prev_ball_y + BALL_HGT / 2.0f;

    for (int j = 0; j < BLK_YQ; j++) {
        for (int i = 0; i < BLK_XQ; i++) {
            if (!bloc_exst[j][i]) {
                continue;
            }

            float blk_left   = (float)(FST_BLK_SPX + i * BLK_WDH);
            float blk_right  = blk_left + BLK_WDH;
            float blk_top    = (float)(FST_BLK_SPY + j * BLK_HGT);
            float blk_bottom = blk_top + BLK_HGT;
            bool overlap =
                (ball_right >= blk_left) &&
                (ball_left <= blk_right) &&
                (ball_bottom >= blk_top) &&
                (ball_top <= blk_bottom);

            if (!overlap) {
                continue;
            }

            bool hit_p1 = bloc_pow1[j][i];
            bool hit_p2 = bloc_pow2[j][i];
            bool hit_p3 = bloc_pow3[j][i];
            bool came_from_left   = (prev_right <= blk_left)   && (ball_right >= blk_left);
            bool came_from_right  = (prev_left >= blk_right)   && (ball_left <= blk_right);
            bool came_from_top    = (prev_bottom <= blk_top)   && (ball_bottom >= blk_top);
            bool came_from_bottom = (prev_top >= blk_bottom)   && (ball_top <= blk_bottom);

            bloc_exst[j][i] = false;
            clear_block_powers(j, i);

            score += (HAL_GetTick() < power_score_until_ms) ? (SCORE_PER_BLOCK * 2u) : SCORE_PER_BLOCK;

            LCD_DrawRect((int)blk_left, (int)blk_top, BLK_WDH, BLK_HGT, C_BG);

            if (came_from_left) {
                pos_ball[0] = blk_left - BALL_WDH / 2.0f;
                bouncex();
            } else if (came_from_right) {
                pos_ball[0] = blk_right + BALL_WDH / 2.0f;
                bouncex();
            } else if (came_from_top) {
                pos_ball[1] = blk_top - BALL_HGT / 2.0f;
                bouncey();
            } else if (came_from_bottom) {
                pos_ball[1] = blk_bottom + BALL_HGT / 2.0f;
                bouncey();
            } else {
                float overlap_left   = ball_right - blk_left;
                float overlap_right  = blk_right - ball_left;
                float overlap_top    = ball_bottom - blk_top;
                float overlap_bottom = blk_bottom - ball_top;
                float min_x = (overlap_left < overlap_right) ? overlap_left : overlap_right;
                float min_y = (overlap_top < overlap_bottom) ? overlap_top : overlap_bottom;

                if (min_x < min_y) {
                    if (overlap_left < overlap_right) {
                        pos_ball[0] = blk_left - BALL_WDH / 2.0f;
                    } else {
                        pos_ball[0] = blk_right + BALL_WDH / 2.0f;
                    }
                    bouncex();
                } else {
                    if (overlap_top < overlap_bottom) {
                        pos_ball[1] = blk_top - BALL_HGT / 2.0f;
                    } else {
                        pos_ball[1] = blk_bottom + BALL_HGT / 2.0f;
                    }
                    bouncey();
                }
            }

            activate_block_power(hit_p1, hit_p2, hit_p3);
            handle_block_clear();
            return;
        }
    }
}

void does_collide_wall(void) {
    if (pos_ball[0] - BALL_WDH / 2 <= FIELD_X) {
        pos_ball[0] = FIELD_X + BALL_WDH / 2;
        bouncex();
    } else if (pos_ball[0] + BALL_WDH / 2 >= FIELD_X + FIELD_W_BREAKOUT) {
        pos_ball[0] = FIELD_X + FIELD_W_BREAKOUT - BALL_WDH / 2;
        bouncex();
    }

    if (pos_ball[1] - BALL_HGT / 2 <= FIELD_Y) {
        pos_ball[1] = FIELD_Y + BALL_HGT / 2;
        bouncey();
    }
}

void does_collide_plat(void) {
    float plat_left   = pos_plat[0] - current_plat_wdh / 2.0f;
    float plat_right  = pos_plat[0] + current_plat_wdh / 2.0f;
    float plat_top    = pos_plat[1] - PLT_HGT / 2.0f;

    float ball_left      = pos_ball[0] - BALL_WDH / 2.0f;
    float ball_right     = pos_ball[0] + BALL_WDH / 2.0f;
    float ball_bottom    = pos_ball[1] + BALL_HGT / 2.0f;

    float prev_ball_left   = prev_ball_x - BALL_WDH / 2.0f;
    float prev_ball_right  = prev_ball_x + BALL_WDH / 2.0f;
    float prev_ball_bottom = prev_ball_y + BALL_HGT / 2.0f;

    bool x_overlap_now =
        (ball_right >= plat_left) &&
        (ball_left <= plat_right);

    bool x_overlap_prev =
        (prev_ball_right >= plat_left) &&
        (prev_ball_left <= plat_right);

    bool crossed_top =
        (prev_ball_bottom <= plat_top) &&
        (ball_bottom >= plat_top);

    if ((x_overlap_now || x_overlap_prev) && crossed_top && ball_vely > 0.0f) {
        float pos = (pos_ball[0] - plat_left) / current_plat_wdh;

        if (pos < 0.0f) pos = 0.0f;
        if (pos > 1.0f) pos = 1.0f;

        pos_ball[1] = plat_top - BALL_HGT / 2.0f;
        bouncexplat(pos);
    }
}

/* -------------------- movement updates -------------------- */

void ball_update(float dt) {
    if (!ball_released) {
        return;
    }

    refresh_active_powers();
    renormalise_ball_speed();

    float move_x = ball_velx * dt;
    float move_y = ball_vely * dt;
    float max_move = fabsf(move_x);
    int steps;

    if (fabsf(move_y) > max_move) {
        max_move = fabsf(move_y);
    }

    steps = (int)ceilf(max_move / 2.0f);
    if (steps < 1) {
        steps = 1;
    }
    if (steps > MAX_PHYS_SUBSTEPS) {
        steps = MAX_PHYS_SUBSTEPS;
    }

    {
        float step_x = move_x / (float)steps;
        float step_y = move_y / (float)steps;

        for (int s = 0; s < steps; s++) {
            prev_ball_x = pos_ball[0];
            prev_ball_y = pos_ball[1];

            pos_ball[0] += step_x;
            pos_ball[1] += step_y;

            does_collide_wall();
            does_collide_plat();
            does_collide_blk();

            if (!ball_released || !ball_in) {
                break;
            }
        }
    }
}

void platform_update(uint16_t held, float dt) {
    refresh_active_powers();

    if (held & (1u << BTN_LEFT)) {
        plat_fx -= PLATFORM_SPEED * dt;
    }

    if (held & (1u << BTN_RIGHT)) {
        plat_fx += PLATFORM_SPEED * dt;
    }

    if (plat_fx - current_plat_wdh / 2.0f < FIELD_X) {
        plat_fx = FIELD_X + current_plat_wdh / 2.0f;
    }

    if (plat_fx + current_plat_wdh / 2.0f > FIELD_X + FIELD_W_BREAKOUT) {
        plat_fx = FIELD_X + FIELD_W_BREAKOUT - current_plat_wdh / 2.0f;
    }

    pos_plat[0] = (uint16_t)roundf(plat_fx);
    pos_plat[1] = PLT_SPY;

    if (!ball_released) {
        position_ball_on_paddle();
    }
}

/* -------------------- updating and running the game -------------------- */

void break_Tick(void) {
    if (!ball_in) {
        return;
    }

    if (pos_ball[1] + BALL_HGT / 2 >= FIELD_Y + FIELD_H_BREAKOUT) {
        ball_in = false;
    }
}

void break_draw(void) {
    erase_old_ball();
    erase_old_paddle();

    draw_score_if_needed();
    draw_power_status_if_needed();
    draw_paddle_current();

    if (ball_in) {
        draw_ball_current();
    }

    if (!ball_in) {
        LCD_DrawRect(18, 66, 92, 24, C_BG);
        LCD_DrawText(20, 68, "GAME OVER", C_TEXT, C_BG, 2);
    }
}

void Breakout_Update(uint16_t pressed, uint16_t held, uint16_t held_ev) {
    if (game_ran) {
        initialise();
        game_ran = false;
    }

    if (held_ev & (1u << BTN_A)) {
        game_ran = true;
        Menu_Invalidate();
        g_state = STATE_MENU;
        return;
    }

    if (ball_in && !ball_released) {
        release_ball(pressed);
    }

    {
        uint32_t now = HAL_GetTick();

        while (ball_in && (uint32_t)(now - s_lastGameTickMs) >= LOCAL_GAME_TICK_MS) {
            s_lastGameTickMs += LOCAL_GAME_TICK_MS;

            platform_update(held, LOCAL_GAME_TICK_MS / 1000.0f);
            ball_update(LOCAL_GAME_TICK_MS / 1000.0f);
            break_Tick();
            refresh_active_powers();
        }
    }

    break_draw();
}

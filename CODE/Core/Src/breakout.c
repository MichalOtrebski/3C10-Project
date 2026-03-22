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

static bool bloc_exst[BLK_YQ][BLK_XQ];
static bool bloc_pow1[BLK_YQ][BLK_XQ];
static bool bloc_pow2[BLK_YQ][BLK_XQ];
static bool bloc_pow3[BLK_YQ][BLK_XQ];

static bool ball_in = true;
static bool game_ran = true;
static bool ball_released = false;

uint32_t time_coll;

float ball_velx = 0.0f;
float ball_vely = 0.0f;

uint16_t pos_pow[2] = {0, 0};

float pos_ball[2] = {0.0f, 0.0f};
uint16_t pos_plat[2] = {0, 0};

uint16_t line[2][142] = {{0}};
bool coll_blk = false;
int16_t coll_timer = -1;
int16_t coll_i = -1;
int16_t coll_j = -1;
bool bounce_dir = false;

static float prev_ball_x = 0.0f;
static float prev_ball_y = 0.0f;

uint16_t score = 0;

static uint32_t s_lastGameTickMs;
#define GAME_TICK_MS 16

static int16_t prev_draw_ball_x = -1;
static int16_t prev_draw_ball_y = -1;
static int16_t prev_draw_plat_x = -1;
static int16_t prev_draw_plat_y = -1;
static uint16_t prev_score_drawn = 0xFFFF;

/* -------------------- HELPERS -------------------- */

static void draw_static_scene(void) {
    LCD_DrawRect(0, 0, FB_WIDTH, FB_HEIGHT, C_BG);
    LCD_DrawText(4, 4, "SCORE", C_TEXT, C_BG, 1);
    border(2, 2, 2, 2, C_TEXT);
    LCD_DrawRect(0, UI_TOP, FB_WIDTH, 2, C_TEXT);
    block_draw();
}

static void draw_score_if_needed(void) {
    if (score != prev_score_drawn) {
        /* Clear only score number area */
        LCD_DrawRect(60, 4, 40, 10, C_BG);

        char buf[10];
        itoa(score, buf, 10);
        LCD_DrawText(60, 4, buf, C_SCORE, C_BG, 1);

        prev_score_drawn = score;
    }
}

static void erase_old_ball(void) {
    if (prev_draw_ball_x >= 0 && prev_draw_ball_y >= 0) {
        LCD_DrawRect(
            prev_draw_ball_x - BALL_WDH / 2,
            prev_draw_ball_y - BALL_HGT / 2,
            BALL_WDH,
            BALL_HGT,
            C_BG
        );
    }
}

static void erase_old_paddle(void) {
    if (prev_draw_plat_x >= 0 && prev_draw_plat_y >= 0) {
        LCD_DrawRect(
            prev_draw_plat_x - PLT_WDH / 2,
            prev_draw_plat_y - PLT_HGT / 2,
            PLT_WDH,
            PLT_HGT,
            C_BG
        );
    }
}

static void draw_ball_current(void) {
    int bx = (int)roundf(pos_ball[0]);
    int by = (int)roundf(pos_ball[1]);

    LCD_DrawRect(
        bx - BALL_WDH / 2,
        by - BALL_HGT / 2,
        BALL_WDH,
        BALL_HGT,
        PLT_WHT
    );

    prev_draw_ball_x = bx;
    prev_draw_ball_y = by;
}

static void draw_paddle_current(void) {
    int px = pos_plat[0];
    int py = pos_plat[1];

    LCD_DrawRect(
        px - PLT_WDH / 2,
        py - PLT_HGT / 2,
        PLT_WDH,
        PLT_HGT,
        PLT_WHT
    );

    prev_draw_plat_x = px;
    prev_draw_plat_y = py;
}

/* -------------------- initialising game play -------------------- */

void block_intl(void) {
    for (int i = 0; i < BLK_XQ; i++) {
        for (int j = 0; j < BLK_YQ; j++) {
            bloc_exst[j][i] = true;
            bloc_pow1[j][i] = false;
            bloc_pow2[j][i] = false;
            bloc_pow3[j][i] = false;
        }
    }
}

void block_draw(void) {
    for (int i = 0; i < BLK_XQ; i++) {
        for (int j = 0; j < BLK_YQ; j++) {
            if (!bloc_exst[j][i]) continue;

            uint16_t colour = BLK_RED;
            switch (j) {
                case 0: colour = BLK_RED; break;
                case 1: colour = BLK_YLW; break;
                case 2: colour = BLK_GRN; break;
                case 3: colour = BLK_BLU; break;
                case 4: colour = BLK_PRP; break;
                default: colour = PLT_WHT; break;
            }

            LCD_DrawRect(
                FST_BLK_SPX + i * BLK_WDH,
                FST_BLK_SPY + j * BLK_HGT,
                BLK_WDH,
                BLK_HGT,
                colour
            );
        }
    }
}

void plat_intl(void) {
    pos_plat[0] = PLT_SPX;
    pos_plat[1] = PLT_SPY;

    LCD_DrawRect(
        pos_plat[0] - PLT_WDH / 2,
        pos_plat[1] - PLT_HGT / 2,
        PLT_WDH,
        PLT_HGT,
        PLT_WHT
    );
}

void ball_intl(void) {
    pos_ball[0] = BALL_SPX;
    pos_ball[1] = BALL_SPY;

    LCD_DrawRect(
        (int)roundf(pos_ball[0]) - BALL_WDH / 2,
        (int)roundf(pos_ball[1]) - BALL_HGT / 2,
        BALL_WDH,
        BALL_HGT,
        PLT_WHT
    );
}

void initialise(void) {
    ball_released = false;
    ball_in = true;

    score = 0;
    time_coll = INT_MAX;
    coll_timer = -1;
    coll_blk = false;
    coll_i = -1;
    coll_j = -1;
    bounce_dir = 0;

    ball_velx = 0.0f;
    ball_vely = 0.0f;

    prev_ball_x = BALL_SPX;
    prev_ball_y = BALL_SPY;

    prev_draw_ball_x = -1;
    prev_draw_ball_y = -1;
    prev_draw_plat_x = -1;
    prev_draw_plat_y = -1;
    prev_score_drawn = 0xFFFF;

    block_intl();

    pos_plat[0] = PLT_SPX;
    pos_plat[1] = PLT_SPY;

    pos_ball[0] = BALL_SPX;
    pos_ball[1] = BALL_SPY;

    draw_static_scene();
    draw_score_if_needed();
    draw_paddle_current();
    draw_ball_current();

    s_lastGameTickMs = HAL_GetTick();
}

/* -------------------- button controls -------------------- */

void release_ball(uint16_t pressed) {
    if (!ball_released && (pressed & (1u << BTN_A))) {
        ball_velx = BALL_VEL * (((float)rand() / (float)RAND_MAX) - 0.5f) * 2.0f;
        ball_vely = -BALL_VEL;
        ball_released = true;
    }
}

/* -------------------- bounce code -------------------- */

void future_pos(void) {
    float oldxpos = pos_ball[0];
    float oldypos = pos_ball[1];

    for (int i = 0; i < 142; i++) {
        oldxpos += ball_velx * BALL_INT;
        oldypos += ball_vely * BALL_INT;

        if (oldxpos - BALL_WDH / 2 < FIELD_X ||
            oldxpos + BALL_WDH / 2 > FIELD_X + FIELD_W_BREAKOUT ||
            oldypos - BALL_HGT / 2 < FIELD_Y ||
            oldypos + BALL_HGT / 2 > FIELD_Y + FIELD_H_BREAKOUT) {
            break;
        }

        line[0][i] = (uint16_t)roundf(oldxpos);
        line[1][i] = (uint16_t)roundf(oldypos);
    }
}

void bouncex(void) {
    ball_velx = -ball_velx;
}

void bouncexplat(float position) {
    ball_velx = (position - 0.5f) * 2.0f * BALL_VEL;
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
            if (!bloc_exst[j][i]) continue;

            float blk_left   = FST_BLK_SPX + i * BLK_WDH;
            float blk_right  = blk_left + BLK_WDH;
            float blk_top    = FST_BLK_SPY + j * BLK_HGT;
            float blk_bottom = blk_top + BLK_HGT;

            bool overlap =
                (ball_right >= blk_left) &&
                (ball_left <= blk_right) &&
                (ball_bottom >= blk_top) &&
                (ball_top <= blk_bottom);

            if (!overlap) continue;

            bloc_exst[j][i] = false;
            score += 10;

            LCD_DrawRect(
                FST_BLK_SPX + i * BLK_WDH,
                FST_BLK_SPY + j * BLK_HGT,
                BLK_WDH,
                BLK_HGT,
                C_BG
            );

            /* Determine which side was crossed using previous position */
            bool came_from_left   = (prev_right <= blk_left)   && (ball_right >= blk_left);
            bool came_from_right  = (prev_left >= blk_right)   && (ball_left <= blk_right);
            bool came_from_top    = (prev_bottom <= blk_top)   && (ball_bottom >= blk_top);
            bool came_from_bottom = (prev_top >= blk_bottom)   && (ball_top <= blk_bottom);

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
                /* Fallback if deeply embedded: choose smaller penetration */
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
    float plat_left  = pos_plat[0] - PLT_WDH / 2.0f;
    float plat_right = pos_plat[0] + PLT_WDH / 2.0f;
    float plat_top   = pos_plat[1] - PLT_HGT / 2.0f;
    float plat_bottom = pos_plat[1] + PLT_HGT / 2.0f;

    float ball_left      = pos_ball[0] - BALL_WDH / 2.0f;
    float ball_right     = pos_ball[0] + BALL_WDH / 2.0f;
    float ball_top       = pos_ball[1] - BALL_HGT / 2.0f;
    float ball_bottom    = pos_ball[1] + BALL_HGT / 2.0f;

    float prev_ball_left   = prev_ball_x - BALL_WDH / 2.0f;
    float prev_ball_right  = prev_ball_x + BALL_WDH / 2.0f;
    float prev_ball_top    = prev_ball_y - BALL_HGT / 2.0f;
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

    if ((x_overlap_now || x_overlap_prev) && crossed_top && ball_vely > 0) {
        float pos = (pos_ball[0] - plat_left) / PLT_WDH;

        if (pos < 0.0f) pos = 0.0f;
        if (pos > 1.0f) pos = 1.0f;

        pos_ball[1] = plat_top - BALL_HGT / 2.0f;
        bouncexplat(pos);
        bouncey();
    }
}

/* -------------------- movement updates -------------------- */

void ball_update(void) {
    if (!ball_released) return;

    int steps = 4;
    float step_x = (ball_velx * BALL_INT) / steps;
    float step_y = (ball_vely * BALL_INT) / steps;

    for (int s = 0; s < steps; s++) {
        prev_ball_x = pos_ball[0];
        prev_ball_y = pos_ball[1];

        pos_ball[0] += step_x;
        pos_ball[1] += step_y;

        does_collide_wall();
        does_collide_plat();
        does_collide_blk();
    }
}

void platform_update(uint16_t pressed) {
    if ((pressed & (1u << BTN_LEFT)) &&
        (pos_plat[0] - PLT_WDH / 2 > FIELD_X)) {
        pos_plat[0] -= 3;
    }

    if ((pressed & (1u << BTN_RIGHT)) &&
        (pos_plat[0] + PLT_WDH / 2 < FIELD_X + FIELD_W_BREAKOUT)) {
        pos_plat[0] += 3;
    }

    if (pos_plat[0] - PLT_WDH / 2 < FIELD_X) {
        pos_plat[0] = FIELD_X + PLT_WDH / 2;
    }

    if (pos_plat[0] + PLT_WDH / 2 > FIELD_X + FIELD_W_BREAKOUT) {
        pos_plat[0] = FIELD_X + FIELD_W_BREAKOUT - PLT_WDH / 2;
    }

    if (!ball_released) {
        pos_ball[0] = pos_plat[0];
        pos_ball[1] = BALL_SPY;
    }
}

/* -------------------- updating and running the game -------------------- */

void break_Tick(void) {
    if (!ball_in) {
    	return;
    }

    if (pos_ball[1] + BALL_HGT / 2 >= FIELD_Y + FIELD_H_BREAKOUT) {
        ball_in = false;
        return;
    }
}

void break_draw(void) {
    erase_old_ball();
    erase_old_paddle();

    draw_score_if_needed();
    draw_paddle_current();
    draw_ball_current();

    if (!ball_in) {
        LCD_DrawRect(8, 68, 90, 20, C_BG);
        LCD_DrawText(10, 70, "GAME OVER", C_TEXT, C_BG, 2);
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

    if (!ball_released) {
        release_ball(pressed);
    }

    uint32_t now = HAL_GetTick();

    while (ball_in && (uint32_t)(now - s_lastGameTickMs) >= GAME_TICK_MS) {
        s_lastGameTickMs += GAME_TICK_MS;

        platform_update(held);
        ball_update();
        break_Tick();
    }

    break_draw();
}

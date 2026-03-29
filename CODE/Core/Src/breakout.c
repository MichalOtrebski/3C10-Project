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

static float prev_ball_x = 0.0f;
static float prev_ball_y = 0.0f;

static uint32_t s_lastGameTickMs;
#define GAME_TICK_MS 16

static int16_t prev_draw_ball_x = -1;
static int16_t prev_draw_ball_y = -1;
static int16_t prev_draw_plat_x = -1;
static int16_t prev_draw_plat_y = -1;
static uint16_t prev_score_drawn = 0xFFFF;

static uint8_t block_wave = 0;

#define BALL_MIN_VX 6.0f

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
        case 1:
            bloc_pow1[j][i] = true;
            break;
        case 2:
            bloc_pow2[j][i] = true;
            break;
        case 3:
            bloc_pow3[j][i] = true;
            break;
        default:
            break;
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

static void enforce_min_horizontal_speed(void) {
    if (ball_velx > -BALL_MIN_VX && ball_velx < BALL_MIN_VX) {
        if (ball_velx < 0.0f) {
            ball_velx = -BALL_MIN_VX;
        } else {
            ball_velx = BALL_MIN_VX;
        }

        {
            float vy_sq = (BALL_VEL * BALL_VEL) - (ball_velx * ball_velx);
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

static void draw_block_cell(int j, int i) {
    if (!bloc_exst[j][i]) {
        return;
    }

    int x = FST_BLK_SPX + i * BLK_WDH;
    int y = FST_BLK_SPY + j * BLK_HGT;
    uint16_t base_colour = block_base_colour_for_row(j);

    LCD_DrawRect(x, y, BLK_WDH, BLK_HGT, base_colour);

    bool p1 = bloc_pow1[j][i];
    bool p2 = bloc_pow2[j][i];
    bool p3 = bloc_pow3[j][i];

    if (p1 || p2 || p3) {
        uint16_t accent = PLT_WHT;

        if (p1) {
            accent = C_SCORE;
        } else if (p2) {
            accent = BLK_BLU;
        } else if (p3) {
            accent = BLK_PRP;
        }

        LCD_DrawRect(x, y, BLK_WDH, 1, accent);
        LCD_DrawRect(x, y + BLK_HGT - 1, BLK_WDH, 1, accent);
        LCD_DrawRect(x, y, 1, BLK_HGT, accent);
        LCD_DrawRect(x + BLK_WDH - 1, y, 1, BLK_HGT, accent);

        if (BLK_WDH > 6 && BLK_HGT > 6) {
            LCD_DrawRect(x + 2, y + 2, BLK_WDH - 4, BLK_HGT - 4, accent);
            LCD_DrawRect(x + 3, y + 3, BLK_WDH - 6, BLK_HGT - 6, base_colour);
        }

        {
            int cx = x + BLK_WDH / 2;
            int cy = y + BLK_HGT / 2;
            LCD_DrawRect(cx - 1, cy - 1, 3, 3, accent);
        }
    }
}

static void draw_static_scene(void) {
    LCD_DrawRect(0, 0, FB_WIDTH, FB_HEIGHT, C_BG);
    LCD_DrawText(4, 4, "SCORE", C_TEXT, C_BG, 1);
    border(2, 2, 2, 2, C_TEXT);
    LCD_DrawRect(0, UI_TOP, FB_WIDTH, 2, C_TEXT);
}

static void draw_score_if_needed(void) {
    if (score != prev_score_drawn) {
        LCD_DrawRect(60, 4, 50, 10, C_BG);

        char buf[12];
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
        int power_count = total_blocks / 8;
        power_count += block_wave;

        if (power_count < 3) {
            power_count = 3;
        }
        if (power_count > total_blocks / 2) {
            power_count = total_blocks / 2;
        }

        for (int n = 0; n < power_count; n++) {
            int i = rand() % BLK_XQ;
            int j = rand() % BLK_YQ;
            int type = 1 + (rand() % 3);
            assign_power_block(j, i, type);
        }
    }

    block_draw();
}

static void handle_block_clear(void) {
    if (!blocks_remaining()) {
        respawn_blocks();

        ball_released = false;
        ball_velx = 0.0f;
        ball_vely = 0.0f;

        pos_plat[0] = PLT_SPX;
        pos_plat[1] = PLT_SPY;

        pos_ball[0] = pos_plat[0];
        pos_ball[1] = BALL_SPY;

        prev_ball_x = pos_ball[0];
        prev_ball_y = pos_ball[1];

        erase_old_ball();
        erase_old_paddle();
        draw_paddle_current();
        draw_ball_current();
    }
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
    ball_velx = 0.0f;
    ball_vely = 0.0f;

    prev_ball_x = BALL_SPX;
    prev_ball_y = BALL_SPY;

    prev_draw_ball_x = -1;
    prev_draw_ball_y = -1;
    prev_draw_plat_x = -1;
    prev_draw_plat_y = -1;
    prev_score_drawn = 0xFFFF;

    pos_plat[0] = PLT_SPX;
    pos_plat[1] = PLT_SPY;

    pos_ball[0] = BALL_SPX;
    pos_ball[1] = BALL_SPY;

    draw_static_scene();
    block_intl();
    draw_score_if_needed();
    draw_paddle_current();
    draw_ball_current();

    s_lastGameTickMs = HAL_GetTick();
}

/* -------------------- button controls -------------------- */

void release_ball(uint16_t pressed) {
    if (!ball_released && (pressed & (1u << BTN_A))) {
        float r = ((float)rand() / (float)RAND_MAX);
        ball_velx = BALL_VEL * ((r - 0.5f) * 1.6f);
        ball_vely = -sqrtf((BALL_VEL * BALL_VEL) - (ball_velx * ball_velx));

        enforce_min_horizontal_speed();
        ball_released = true;
    }
}

/* -------------------- bounce code -------------------- */

void bouncex(void) {
    ball_velx = -ball_velx;
}

void bouncexplat(float position) {
    float offset = (position - 0.5f) * 2.0f;

    if (offset > -0.08f && offset < 0.08f) {
        offset = (offset < 0.0f) ? -0.08f : 0.08f;
    }

    if (offset < -0.95f) offset = -0.95f;
    if (offset > 0.95f)  offset = 0.95f;

    ball_velx = offset * BALL_VEL;
    ball_vely = -BALL_VEL;

    enforce_min_horizontal_speed();

    {
        float vy_sq = (BALL_VEL * BALL_VEL) - (ball_velx * ball_velx);
        if (vy_sq < 1.0f) {
            vy_sq = 1.0f;
        }
        ball_vely = -sqrtf(vy_sq);
    }
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

            float blk_left   = FST_BLK_SPX + i * BLK_WDH;
            float blk_right  = blk_left + BLK_WDH;
            float blk_top    = FST_BLK_SPY + j * BLK_HGT;
            float blk_bottom = blk_top + BLK_HGT;

            bool overlap =
                (ball_right >= blk_left) &&
                (ball_left <= blk_right) &&
                (ball_bottom >= blk_top) &&
                (ball_top <= blk_bottom);

            if (!overlap) {
                continue;
            }

            bloc_exst[j][i] = false;
            clear_block_powers(j, i);
            score += 10;

            LCD_DrawRect(
                (int)blk_left,
                (int)blk_top,
                BLK_WDH,
                BLK_HGT,
                C_BG
            );

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

            handle_block_clear();
            return;
        }
    }
}

void does_collide_wall(void) {
    if (pos_ball[0] - BALL_WDH / 2 <= FIELD_X) {
        pos_ball[0] = FIELD_X + BALL_WDH / 2;
        bouncex();

        if (ball_velx < BALL_MIN_VX) {
            ball_velx = BALL_MIN_VX;
        }
    } else if (pos_ball[0] + BALL_WDH / 2 >= FIELD_X + FIELD_W_BREAKOUT) {
        pos_ball[0] = FIELD_X + FIELD_W_BREAKOUT - BALL_WDH / 2;
        bouncex();

        if (ball_velx > -BALL_MIN_VX) {
            ball_velx = -BALL_MIN_VX;
        }
    }

    if (pos_ball[1] - BALL_HGT / 2 <= FIELD_Y) {
        pos_ball[1] = FIELD_Y + BALL_HGT / 2;
        bouncey();
    }
}

void does_collide_plat(void) {
    float plat_left   = pos_plat[0] - PLT_WDH / 2.0f;
    float plat_right  = pos_plat[0] + PLT_WDH / 2.0f;
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

    if ((x_overlap_now || x_overlap_prev) && crossed_top && ball_vely > 0) {
        float pos = (pos_ball[0] - plat_left) / PLT_WDH;

        if (pos < 0.0f) pos = 0.0f;
        if (pos > 1.0f) pos = 1.0f;

        pos_ball[1] = plat_top - BALL_HGT / 2.0f;
        bouncexplat(pos);
    }
}

/* -------------------- movement updates -------------------- */

void ball_update(void) {
    if (!ball_released) {
        return;
    }

    {
        int steps = 4;
        float step_x = (ball_velx * BALL_INT) / (float)steps;
        float step_y = (ball_vely * BALL_INT) / (float)steps;

        for (int s = 0; s < steps; s++) {
            prev_ball_x = pos_ball[0];
            prev_ball_y = pos_ball[1];

            pos_ball[0] += step_x;
            pos_ball[1] += step_y;

            does_collide_wall();
            does_collide_plat();
            does_collide_blk();

            if (!ball_released) {
                break;
            }
        }
    }
}

void platform_update(uint16_t held) {
    if ((held & (1u << BTN_LEFT)) &&
        (pos_plat[0] - PLT_WDH / 2 > FIELD_X)) {
        pos_plat[0] -= 3;
    }

    if ((held & (1u << BTN_RIGHT)) &&
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
        prev_ball_x = pos_ball[0];
        prev_ball_y = pos_ball[1];
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
    draw_paddle_current();

    if (ball_in) {
        draw_ball_current();
    }

    if (!ball_in) {
        LCD_DrawRect(8, 68, 95, 20, C_BG);
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

    if (ball_in && !ball_released) {
        release_ball(pressed);
    }

    {
        uint32_t now = HAL_GetTick();

        while (ball_in && (uint32_t)(now - s_lastGameTickMs) >= GAME_TICK_MS) {
            s_lastGameTickMs += GAME_TICK_MS;

            platform_update(held);
            ball_update();
            break_Tick();
        }
    }

    break_draw();
}

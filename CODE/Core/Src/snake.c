/*
 * snake.c
 *
 *  Created on: 6 Feb 2026
 *      Author: motre
 */

#include "snake.h"
#include <stdlib.h>
#include "display.h"
#include "globals.h"
#include "text.h"
#include <stdbool.h>
#include "buttons.h"
#include "snake_sfx.h"

static inline int GX(int gx){ return FIELD_X + gx*CELL; }
static inline int GY(int gy){ return FIELD_Y + gy*CELL; }

static bool once = true;
static uint32_t s_lastTickMs;
static bool s_gameOverSfxPlayed = false;

static inline int idx_wrap(int i)
{
    if(i < 0) return i + SNAKE_MAX;
    if(i >= SNAKE_MAX) return i - SNAKE_MAX;
    return i;
}

static inline Pt snake_head(void){ return snake[head_i]; }

static inline Pt snake_tail(void)
{
    int tail = idx_wrap(head_i - (snake_len - 1));
    return snake[tail];
}

static int snake_contains(int x, int y)
{
    for(int k = 0; k < snake_len; k++)
    {
        int i = idx_wrap(head_i - k);
        if(snake[i].x == x && snake[i].y == y) return 1;
    }
    return 0;
}

static void Snake_SpawnFood(void)
{
    while(1)
    {
        int x = rand() % GRID_W;
        int y = rand() % GRID_H;
        if(!snake_contains(x, y))
        {
            food.x = x;
            food.y = y;
            return;
        }
    }
}

void Snake_Init(void)
{
    alive = 1;
    score = 0;
    dir = DIR_RIGHT;

    snake_len = 3;
    head_i = 0;

    s_gameOverSfxPlayed = false;

    int sx = GRID_W / 2;
    int sy = GRID_H / 2;

    snake[0] = (Pt){sx, sy};
    snake[SNAKE_MAX - 1] = (Pt){sx - 1, sy};
    snake[SNAKE_MAX - 2] = (Pt){sx - 2, sy};

    Snake_SpawnFood();

    s_lastTickMs = HAL_GetTick();
}

void Snake_SetDir(Dir d)
{
    if((dir == DIR_UP    && d == DIR_DOWN) ||
       (dir == DIR_DOWN  && d == DIR_UP)   ||
       (dir == DIR_LEFT  && d == DIR_RIGHT)||
       (dir == DIR_RIGHT && d == DIR_LEFT))
    {
        return;
    }

    dir = d;
}

void Snake_Tick(void)
{
    if(!alive) return;

    Pt h = snake_head();
    int nx = h.x;
    int ny = h.y;

    if(dir == DIR_UP)    ny--;
    if(dir == DIR_DOWN)  ny++;
    if(dir == DIR_LEFT)  nx--;
    if(dir == DIR_RIGHT) nx++;

    /* wall collision */
    if(nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H)
    {
        alive = 0;
        if(!s_gameOverSfxPlayed)
        {
            SnakeSFX_HitWall();
            SnakeSFX_GameOver();
            s_gameOverSfxPlayed = true;
        }
        return;
    }

    Pt nh = {nx, ny};
    Pt tail = snake_tail();

    int growing = (nh.x == food.x && nh.y == food.y);

    if(snake_contains(nh.x, nh.y) &&
      !(!growing && nh.x == tail.x && nh.y == tail.y))
    {
        alive = 0;
        if(!s_gameOverSfxPlayed)
        {
            SnakeSFX_HitWall();
            SnakeSFX_GameOver();
            s_gameOverSfxPlayed = true;
        }
        return;
    }

    head_i = idx_wrap(head_i + 1);
    snake[head_i] = nh;

    if(growing)
    {
        snake_len++;
        score++;
        Snake_SpawnFood();
        SnakeSFX_Eat();
    }
    else
    {
        SnakeSFX_MoveTick();
    }
}

static void Snake_DrawCell(int gx, int gy, uint16_t c)
{
    LCD_DrawRect(GX(gx), GY(gy), CELL, CELL, c);
}

void Snake_Draw(void)
{
    LCD_DrawRect(0, 0, FB_WIDTH, FB_HEIGHT, C_BG);

    /* UI */
    LCD_DrawText(4, 4, "SCORE", C_TEXT, C_BG, 1);

    char buf[10];
    itoa(score, buf, 10);
    LCD_DrawText(60, 4, buf, C_SNAKE, C_BG, 1);

    /* border */
    border(2, 2, 2, 2, C_TEXT);

    LCD_DrawRect(0, UI_TOP, FB_WIDTH, 2, C_TEXT);

    /* food */
    Snake_DrawCell(food.x, food.y, C_FOOD);

    /* snake */
    for(int k = 0; k < snake_len; k++)
    {
        int i = idx_wrap(head_i - k);
        uint16_t col = (k == 0) ? C_HEAD : C_SNAKE;
        Snake_DrawCell(snake[i].x, snake[i].y, col);
    }

    if(!alive)
        LCD_DrawText(10, 70, "GAME OVER", C_TEXT, C_BG, 2);
}

void Snake_Update(uint16_t pressed, uint16_t held)
{
    if (once) {
        Snake_Init();
        s_lastTickMs = HAL_GetTick();
        once = false;
    }

    if (pressed & (1u << BTN_UP)) {
        Snake_SetDir(DIR_UP);
    } else if (pressed & (1u << BTN_DOWN)) {
        Snake_SetDir(DIR_DOWN);
    } else if (pressed & (1u << BTN_LEFT)) {
        Snake_SetDir(DIR_LEFT);
    } else if (pressed & (1u << BTN_RIGHT)) {
    	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10);
        Snake_SetDir(DIR_RIGHT);
    }

    if (held & (1u << BTN_A)) {
        once = true;
        g_state = STATE_MENU;
        return;
    }

    uint32_t now = HAL_GetTick();

    while ((uint32_t)(now - s_lastTickMs) >= 100u) {
        s_lastTickMs += 100u;
        Snake_Tick();
    }

    Snake_Draw();
}

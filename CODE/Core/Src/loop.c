/*
 * loop.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "globals.h"

#include "main.h"
#include "gpio.h"
#include "loop.h"
#include "display.h"

#include "text.h"
#include "menu.h"
#include <stdbool.h>
#include "snake.h"

#include "buttons.h"
#include "audio.h"
#include "tetris_audio.h"
#include "tetris.h"

#include "breakout.h"

bool once = true;
uint32_t last = 0;
uint8_t direction = 0;

static uint16_t GetPressedMask(void)
{
    uint16_t mask = 0;

    for (int i = 0; i < BTN_COUNT; i++) {
        if (Buttons_WasPressed((ButtonId)i)) {
            mask |= (uint16_t)(1u << i);
        }
    }

    return mask;
}

static uint16_t GetDownMask(void)
{
    uint16_t mask = 0;

    for (int i = 0; i < BTN_COUNT; i++) {
        if (Buttons_IsDown((ButtonId)i)) {
            mask |= (uint16_t)(1u << i);
        }
    }

    return mask;
}

static uint16_t GetHeldEventMask(void)
{
    uint16_t mask = 0;

    for (int i = 0; i < BTN_COUNT; i++) {
        if (Buttons_WasHeld((ButtonId)i)) {
            mask |= (uint16_t)(1u << i);
        }
    }

    return mask;
}

void loop(void)
{

    Buttons_BeginFrame();

    uint16_t pressed = GetPressedMask();    // one-shot press
    uint16_t held    = GetDownMask();       // continuous held-down state
    uint16_t held_ev = GetHeldEventMask();  // one-shot long-hold event

    switch (g_state) {
    case STATE_MENU: {
        int chosen = Menu_Update(pressed, held);
        if (chosen >= 0) {
            if (chosen == 0) g_state = STATE_TETRIS;
            if (chosen == 1) g_state = STATE_SNAKE;
            if (chosen == 2) g_state = STATE_BREAKOUT;
        }
    } break;

    case STATE_SNAKE:
        Snake_Update(pressed, held_ev);
        break;

    case STATE_TETRIS:
    	LCD_ClearFrame();
        Tetris_Update(pressed, held, held_ev);
        break;

    case STATE_BREAKOUT:
    	Breakout_Update(pressed, held, held_ev);
    	break;

    default:
        break;
    }

    uint32_t start = DWT->CYCCNT;

    /* code you want to measure */
    render_dma();

    uint32_t end = DWT->CYCCNT;

    uint32_t cycles = end - start;
    uint32_t us = cycles / 170;

    printf("Time: %lu us\r\n", us);
}

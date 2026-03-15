/*
 * buttons.c
 *
 *  Created on: 8 Feb 2026
 *      Author: motre
 */

#include "buttons.h"
#include <stddef.h>

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} BtnPin;

static const BtnPin kBtn[BTN_COUNT] = {
    [BTN_UP]    = { GPIOA, GPIO_PIN_11 },
    [BTN_DOWN]  = { GPIOB, GPIO_PIN_11 },
    [BTN_LEFT]  = { GPIOB, GPIO_PIN_12 },
    [BTN_RIGHT] = { GPIOA, GPIO_PIN_12 },
    [BTN_A]     = { GPIOB, GPIO_PIN_1  },
};

#define DB_N    5
#define HOLD_MS 1000

static uint8_t  s_db_cnt[BTN_COUNT];
static uint16_t s_state = 0;          // debounced current down-state bitmask

static uint16_t s_hold_ms[BTN_COUNT];
static uint16_t s_hold_fired = 0;     // one-shot hold fired for current press

// events accumulated by Tick1ms
static volatile uint16_t g_pressed_events = 0;
static volatile uint16_t g_held_events = 0;

// per-frame copies read by game loop
static uint16_t s_pressed_frame = 0;
static uint16_t s_held_frame = 0;

static inline bool raw_pressed(ButtonId b)
{
    return HAL_GPIO_ReadPin(kBtn[b].port, kBtn[b].pin) == GPIO_PIN_RESET;
}

void Buttons_Init(void)
{
    s_state = 0;
    s_hold_fired = 0;

    g_pressed_events = 0;
    g_held_events = 0;

    s_pressed_frame = 0;
    s_held_frame = 0;

    for (size_t i = 0; i < BTN_COUNT; i++) {
        s_db_cnt[i] = 0;
        s_hold_ms[i] = 0;
    }
}

void Buttons_BeginFrame(void)
{
    __disable_irq();
    s_pressed_frame = g_pressed_events;
    s_held_frame    = g_held_events;
    g_pressed_events = 0;
    g_held_events = 0;
    __enable_irq();
}

void Buttons_Tick1ms(void)
{
    uint16_t cur = 0;

    // Debounce into stable current state
    for (int i = 0; i < BTN_COUNT; i++) {
        if (raw_pressed((ButtonId)i)) {
            if (s_db_cnt[i] < DB_N) {
                s_db_cnt[i]++;
            }
        } else {
            if (s_db_cnt[i] > 0) {
                s_db_cnt[i]--;
            }
        }

        if (s_db_cnt[i] == DB_N) {
            cur |= (uint16_t)(1u << i);
        } else if (s_db_cnt[i] != 0) {
            // keep previous stable state while settling
            cur |= (uint16_t)(s_state & (1u << i));
        }
    }

    uint16_t pressed = (uint16_t)(cur & (uint16_t)~s_state);

    if (pressed != 0) {
    	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10);
    }

    // Hold timing / one-shot hold event
    for (int i = 0; i < BTN_COUNT; i++) {
        uint16_t bit = (uint16_t)(1u << i);

        if (cur & bit) {
            if (s_hold_ms[i] < 0xFFFF) {
                s_hold_ms[i]++;
            }

            if (s_hold_ms[i] >= HOLD_MS && (s_hold_fired & bit) == 0u) {
                g_held_events |= bit;
                s_hold_fired |= bit;
            }
        } else {
            s_hold_ms[i] = 0;
            s_hold_fired &= (uint16_t)~bit;
        }
    }

    g_pressed_events |= pressed;
    s_state = cur;
}

bool Buttons_IsDown(ButtonId b)
{
    return (s_state & (1u << b)) != 0u;
}

bool Buttons_WasPressed(ButtonId b)
{
    return (s_pressed_frame & (1u << b)) != 0u;
}

bool Buttons_WasHeld(ButtonId b)
{
    return (s_held_frame & (1u << b)) != 0u;
}

uint16_t Buttons_HeldMs(ButtonId b)
{
    return s_hold_ms[b];
}

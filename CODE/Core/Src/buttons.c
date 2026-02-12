/*
 * buttons.c
 *
 *  Created on: 8 Feb 2026
 *      Author: motre
 */


#include "buttons.h"
#include <stddef.h>

typedef struct {
	GPIO_TypeDef* 	port;
	uint16_t 		pin;
} BtnPin;

static const BtnPin kBtn[BTN_COUNT] = {

	[BTN_UP]	= { GPIOA, GPIO_PIN_11 },
	[BTN_DOWN]	= { GPIOB, GPIO_PIN_11 },
	[BTN_LEFT]	= { GPIOB, GPIO_PIN_12 },
	[BTN_RIGHT]	= { GPIOA, GPIO_PIN_12 },
	[BTN_A]	= { GPIOB, GPIO_PIN_1 },

};

#define DB_N 5

#define REPEAT_DELAY_MS 250
#define REPEAT_RATE_MS 80

#define HOLD_MS 1000   // threshold for "held" event

static volatile uint16_t g_held = 0;     // one-shot events when threshold reached
static uint16_t s_hold_ms[BTN_COUNT] = {0};
static uint16_t s_hold_fired = 0;        // bitmask: held event already fired while down

static volatile uint16_t g_state = 0;
static volatile uint16_t g_pressed = 0;
static volatile uint16_t g_released = 0;

static uint8_t s_cnt[BTN_COUNT] = { 0 };
static uint16_t s_prev = 0;

static uint16_t s_repeat_t0[BTN_COUNT] = { 0 };
static uint16_t s_repeat_t1[BTN_COUNT] = { 0 };

static uint16_t s_pressed_frame = 0;
static uint16_t s_held_frame = 0;
static uint16_t s_released_frame = 0;

static inline bool raw_pressed(ButtonId i) {
	return (HAL_GPIO_ReadPin(kBtn[i].port, kBtn[i].pin) == GPIO_PIN_RESET);
}

static inline int is_repeat_key(int i)
{
    return (i == BTN_UP || i == BTN_DOWN || i == BTN_LEFT || i == BTN_RIGHT);
}

void Buttons_BeginFrame(void)
{
    __disable_irq();
    s_pressed_frame  = g_pressed;  g_pressed = 0;
    s_released_frame = g_released; g_released = 0;
    s_held_frame     = g_held;     g_held = 0;
    __enable_irq();
}

void Buttons_Init(void) {
	g_state = 0;;
	g_pressed = 0;
	g_released = 0;

	s_prev = 0;

	for (size_t i = 0; i < BTN_COUNT; i++) {
		s_cnt[i] = 0;

		s_repeat_t0[i] = 0;
		s_repeat_t1[i] = 0;
	}

	g_held = 0;
	s_hold_fired = 0;
	for (size_t i = 0; i < BTN_COUNT; i++) {
	    s_hold_ms[i] = 0;
	}

}

void Buttons_Tick1ms(void)
{
    uint16_t cur = 0;

    for (int i = 0; i < BTN_COUNT; i++)
    {
        if (raw_pressed((ButtonId)i))
        {
            if (s_cnt[i] < DB_N) s_cnt[i]++;
        }
        else
        {
            if (s_cnt[i] > 0) s_cnt[i]--;
        }

        if (s_cnt[i] == DB_N)
        {
            cur |= (uint16_t)(1u << i);
        }
        else if (s_cnt[i] != 0)
        {
            cur |= (s_prev & (uint16_t)(1u << i)); // keep previous while settling
        }
    }

    uint16_t changed  = (uint16_t)(cur ^ s_prev);
    uint16_t pressed  = (uint16_t)(changed & cur);
    uint16_t released = (uint16_t)(changed & (uint16_t)~cur);

    g_state = cur;

    // Build extra repeats locally (don’t write globals inside loop)
    uint16_t repeat_pressed = 0;

    for (int i = 0; i < BTN_COUNT; i++)
    {
        if (!is_repeat_key(i)) {
            // still reset timers so state is clean
            if ((cur & (1u << i)) == 0) { s_repeat_t0[i] = 0; s_repeat_t1[i] = 0; }
            continue;
        }

        uint16_t bit = (uint16_t)(1u << i);

        if (cur & bit) // held
        {
            if (pressed & bit)
            {
                // initial press: reset counters
                s_repeat_t0[i] = 0;
                s_repeat_t1[i] = 0;
            }
            else
            {
                if (s_repeat_t0[i] < 0xFFFF) s_repeat_t0[i]++;
                if (s_repeat_t1[i] < 0xFFFF) s_repeat_t1[i]++;

                if (s_repeat_t0[i] >= REPEAT_DELAY_MS &&
                    s_repeat_t1[i] >= REPEAT_RATE_MS)
                {
                    repeat_pressed |= bit;
                    s_repeat_t1[i] = 0;
                }
            }
        }
        else
        {
            // released: clear repeat timers
            s_repeat_t0[i] = 0;
            s_repeat_t1[i] = 0;
        }
    }

    for (int i = 0; i < BTN_COUNT; i++)
    {
        uint16_t bit = (uint16_t)(1u << i);

        if (cur & bit) // currently held down
        {
            if (s_hold_ms[i] < 0xFFFF) s_hold_ms[i]++;

            if (s_hold_ms[i] >= HOLD_MS && (s_hold_fired & bit) == 0)
            {
                g_held |= bit;          // fire held event ONCE
                s_hold_fired |= bit;
            }
        }
        else // not held
        {
            s_hold_ms[i] = 0;
            s_hold_fired &= (uint16_t)~bit;
        }
    }

    g_pressed  |= (uint16_t)(pressed | repeat_pressed);
    g_released |= released;
    s_prev = cur;
}

uint16_t Buttons_State(void) {
	return g_state;
}

uint16_t Buttons_PressedEvents(void) {
	__disable_irq();

	uint16_t v = g_pressed;
	g_pressed = 0;

	__enable_irq();

	return v;
}

uint16_t Buttons_ReleasedEvents(void) {
	__disable_irq();

	uint16_t v = g_released;
	g_released = 0;

	__enable_irq();

	return v;
}

bool Buttons_IsDown(ButtonId b) {
	return (Buttons_State() & (1u << b)) != 0;
}

uint16_t Buttons_HeldEvents(void)
{
    __disable_irq();
    uint16_t v = g_held;
    g_held = 0;
    __enable_irq();
    return v;
}

uint16_t Buttons_HeldMs(ButtonId b)
{
    return s_hold_ms[b];
}


bool Buttons_WasPressed(ButtonId b)  { return (s_pressed_frame  & (1u<<b)) != 0; }
bool Buttons_WasHeld(ButtonId b)     { return (s_held_frame     & (1u<<b)) != 0; }
bool Buttons_WasReleased(ButtonId b) { return (s_released_frame & (1u<<b)) != 0; }

uint16_t Buttons_PressedSnapshot(void) { return s_pressed_frame; }
uint16_t Buttons_HeldSnapshot(void)    { return s_held_frame; }








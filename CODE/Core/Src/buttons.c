/*
 * buttons.c
 *
 *  Created on: 8 Feb 2026
 *      Author: motre
 */


#include "buttons.h"

typedef struct {
	GPIO_TypeDef* 	port;
	uint16_t 		pin;
} BtnPin;

static const BtnPin kBtn[BTN_COUNT] = {

	[BTN_UP]	= { GPIOA, GPIO_PIN_0 },
	[BTN_DOWN]	= { GPIOA, GPIO_PIN_0 },
	[BTN_LEFT]	= { GPIOA, GPIO_PIN_0 },
	[BTN_RIGHT]	= { GPIOA, GPIO_PIN_0 },
	[BTN_A]	= { GPIOA, GPIO_PIN_0 },

};

#define DB_N 5

static volatile uint16_t g_state = 0;
static volatile uint16_t g_pressed = 0;
static volatile uint16_t g_released = 0;

static uint8_t s_cnt[BTN_COUNT] = { 0 };
static uint16_t s_prev = 0;

static inline bool raw_pressed(ButtonId i) {
	return (HAL_GPIO_ReadPin(kBtn[i].port, kBtn[i].pin) == GPIO_PIN_RESET);
}

void Buttons_Init(void) {
	g_state = 0;;
	g_pressed = 0;
	g_released = 0;

	s_prev = 0;

	for (int i = 0; i < BTN_COUNT; i++) {
		s_cnt[i] = 0;
	}
}

void Buttons_Tick1ms(void) {
	uint16_t cur = 0;

	for (int i = 0; i < BTN_COUNT; i++) {
		if (raw_pressed((ButtonId)i)) {
			if (s_cnt[i] < DB_N) {
				s_cnt[i]++;
			}
		} else {
			if (s_cnt[i] > 0) {
				s_cnt[i]--;
			}
		}

		if (s_cnt[i] == DB_N) {
			cur |= (uint16_t)(1u << i);
		} else if (s_cnt[i] != 0) {
			cur |= (s_prev & (uint16_t)(1u << i));
		}
	}

	uint16_t changed = (uint16_t)(cur & s_prev);
	uint16_t pressed = (uint16_t)(changed & cur);
	uint16_t released = (uint16_t)(changed & (uint16_t)~cur);

	g_state = cur;
	g_pressed |= pressed;
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

bool Buttons_WasPressed(ButtonId b) {
	return (Buttons_PressedEvents() & (1u << b)) != 0;
}

















/*
 * buttons.h
 *
 *  Created on: 8 Feb 2026
 *      Author: motre
 */

#ifndef INC_BUTTONS_H_
#define INC_BUTTONS_H_

#include "stm32g4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BTN_UP = 0,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT,
    BTN_A,
    BTN_COUNT
} ButtonId;

void     Buttons_Init(void);
void     Buttons_Tick1ms(void);      // call every 1 ms from timer ISR
void     Buttons_BeginFrame(void);   // call once per main loop

bool     Buttons_IsDown(ButtonId b);     // true while button is held down
bool     Buttons_WasPressed(ButtonId b); // true once when button becomes pressed
bool     Buttons_WasHeld(ButtonId b);    // true once when hold threshold is reached

uint16_t Buttons_HeldMs(ButtonId b);     // held time in ms

#endif /* INC_BUTTONS_H_ */

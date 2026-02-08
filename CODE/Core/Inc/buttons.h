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
} ButtonId;

void     Buttons_Init(void);
void     Buttons_Tick1ms(void);              // call from TIM6/TIM7 ISR
uint16_t Buttons_State(void);                // debounced held state bitmask
uint16_t Buttons_PressedEvents(void);        // edge events since last read (clears on read)
uint16_t Buttons_ReleasedEvents(void);       // edge events since last read (clears on read)
bool     Buttons_IsDown(ButtonId b);
bool     Buttons_WasPressed(ButtonId b);

#endif /* INC_BUTTONS_H_ */

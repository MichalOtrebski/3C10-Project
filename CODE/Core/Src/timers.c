/*
 * timers.c
 *
 *  Created on: 8 Feb 2026
 *      Author: motre
 */

#include "buttons.h"
#include "dac.h"

extern TIM_HandleTypeDef htim6;

volatile uint16_t v = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM6) {
		Buttons_Tick1ms();
//		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10);
	}

    if (htim->Instance == TIM7)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10); // scope this: proves TIM7 is ticking

        v = (v + 64) & 0x0FFF; // ramp
        HAL_DAC_SetValue(&hdac2, DAC_CHANNEL_1, DAC_ALIGN_12B_R, v);
    }
}

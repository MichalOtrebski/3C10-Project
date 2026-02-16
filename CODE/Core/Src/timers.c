/*
 * timers.c
 *
 *  Created on: 8 Feb 2026
 *      Author: motre
 */

#include "buttons.h"

extern TIM_HandleTypeDef htim6;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM6) {
		Buttons_Tick1ms();
//		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10);
	}
}

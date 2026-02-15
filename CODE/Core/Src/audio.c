/*
 * audio.c
 *
 *  Created on: 14 Feb 2026
 *      Author: motre
 */

#include "audio.h"

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10);
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10);
}

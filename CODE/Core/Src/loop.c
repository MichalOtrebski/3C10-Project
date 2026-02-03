/*
 * loop.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "main.h"
#include "gpio.h"
#include "loop.h"

void loop() {

    if (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_SET || BSP_PB_GetState(BUTTON_USER) == 1)
    {
        // Button pressed
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
        BSP_LED_On(LED_GREEN);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
        BSP_LED_Off(LED_GREEN);
    }

}

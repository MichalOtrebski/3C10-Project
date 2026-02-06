/*
 * setup.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "main.h"
#include "gpio.h"
#include "setup.h"

void setup() {

	BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
	BSP_LCD_DisplayOn(0);

}

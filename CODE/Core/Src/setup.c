/*
 * setup.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "tim.h"
#include "dac.h"
#include "dma.h"

#include "main.h"
#include "gpio.h"
#include "setup.h"
#include "audio.h"
#include "tetris_audio.h"
#include "sfx.h"

void setup() {

	HAL_TIM_Base_Start_IT(&htim6);

//	PSG_Init();

//	SFX_Init();

	HAL_TIM_Base_Start(&htim15);

	Audio_Start();





	BSP_LCD_Init(0, LCD_ORIENTATION_PORTRAIT_ROT180);
	BSP_LCD_DisplayOn(0);

	static __attribute__((aligned(4))) uint16_t line[320];

	for (int i=0;i<320;i++) line[i] = 0xF800;

	int32_t e;
	e = BSP_LCD_SetDisplayWindow(0, 0, 0, 320, 1);
	printf("win=%ld\r\n", (long)e);

	e = BSP_LCD_WriteDataDMA(0, (uint8_t*)line, 320*2);
	printf("dma_start=%ld status=%u\r\n", (long)e, BSP_LCD_GetTransferStatus(0));

	BSP_LCD_WaitForTransferToBeDone(0);
	printf("dma_done status=%u\r\n", BSP_LCD_GetTransferStatus(0));
}

/*
 * setup.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "tim.h"
#include "dac.h"

#include "main.h"
#include "gpio.h"
#include "setup.h"
#include "audio.h"

void setup() {

	HAL_TIM_Base_Start_IT(&htim6);

	PSG_Init();

	// Example: 2-square “GB-ish” chord
	PSG_SetVoiceWave(0, 0); PSG_SetVoiceFreq(0, 440.0f); PSG_SetVoiceVol(0, 1200);
	PSG_SetVoiceWave(1, 0); PSG_SetVoiceFreq(1, 660.0f); PSG_SetVoiceVol(1, 800);
	PSG_SetVoiceWave(2, 1); PSG_SetVoiceFreq(2, 220.0f); PSG_SetVoiceVol(2, 600);

	// Pre-fill buffer once so DMA doesn't start with garbage
	PSG_Fill(&audioBuf[0], AUDIO_BUF);

	HAL_TIM_Base_Start(&htim6); // TIM6 set to TRGO @ SR (update event)
	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)audioBuf, AUDIO_BUF, DAC_ALIGN_12B_R);

	BSP_LCD_Init(0, LCD_ORIENTATION_PORTRAIT_ROT180);
	BSP_LCD_DisplayOn(0);

	static __attribute__((aligned(4))) uint16_t line[320];

	for (int i=0;i<320;i++) line[i] = 0xF800; // red

	int32_t e;
	e = BSP_LCD_SetDisplayWindow(0, 0, 0, 320, 1);
	printf("win=%ld\r\n", (long)e);

	e = BSP_LCD_WriteDataDMA(0, (uint8_t*)line, 320*2);
	printf("dma_start=%ld status=%u\r\n", (long)e, BSP_LCD_GetTransferStatus(0));

	BSP_LCD_WaitForTransferToBeDone(0);
	printf("dma_done status=%u\r\n", BSP_LCD_GetTransferStatus(0));
}

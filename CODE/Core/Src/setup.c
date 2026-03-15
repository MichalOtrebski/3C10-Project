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

DMA_HandleTypeDef hdma_dac2_ch1;

void setup() {

	HAL_TIM_Base_Start_IT(&htim6);

	PSG_Init();

//	TetrisAudio_Init(SR);

//	PSG_SetVoiceWave(0, 0);       // square
//	PSG_SetVoiceFreq(0, 100.0f);  // 100 Hz (easy on scope)
//	PSG_SetVoiceVol(0, 400);     // strong but not clipping
//
//	/* disable all other voices */
//	PSG_SetVoiceVol(1, 0);
//	PSG_SetVoiceVol(2, 0);
//	PSG_SetVoiceVol(3, 0);


//	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10); // just once

//	HAL_DAC_Start(&hdac2, DAC_CHANNEL_1);
//	HAL_DAC_SetValue(&hdac2, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 1048);

//	HAL_DAC_Start_DMA(&hdac2, DAC_CHANNEL_1, (uint32_t*)audioBuf, AUDIO_BUF, DAC_ALIGN_12B_R);

//	for (int i = 0; i < 1024; i++) {
////	    audioBuf[i] = (i * 4095) / 1023;   // ramp
//	    audioBuf[i] = 2048;   // ramp
//	}


//	__attribute__((aligned(4))) static uint32_t buf[1024] = { 1024 };

	HAL_TIM_Base_Start(&htim15);

	HAL_StatusTypeDef st = HAL_DAC_Start_DMA(&hdac2, DAC_CHANNEL_1,
	                                        (uint32_t*)audioBuf, 1024,
	                                        DAC_ALIGN_12B_R);
//	printf("StartDMA st=%d DACerr=0x%lx\r\n", (int)st, (unsigned long)hdac2.ErrorCode);

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

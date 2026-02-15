/*
 * audio.h
 *
 *  Created on: 14 Feb 2026
 *      Author: motre
 */

#ifndef INC_AUDIO_H_
#define INC_AUDIO_H_

#define AUDIO_BUF 512

#include "stdint.h"
#include "dac.h"
#include "gpio.h"

uint16_t audioBuf[AUDIO_BUF];

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac);
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac);

#endif /* INC_AUDIO_H_ */

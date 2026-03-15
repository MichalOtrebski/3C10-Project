/*
 * audio.h
 *
 *  Created on: 14 Feb 2026
 *      Author: motre
 */

#ifndef INC_AUDIO_H_
#define INC_AUDIO_H_

#define AUDIO_BUF 512u

#include "stdint.h"
#include "dac.h"
#include "gpio.h"
#include <stdint.h>

#define SR 48000u

extern uint32_t audioBuf[AUDIO_BUF];

typedef enum {
    AUDIO_MODE_SFX = 0,
    AUDIO_MODE_TETRIS
} AudioMode;

void PSG_Init(void);
void PSG_SetVoiceFreq(uint8_t voice, float hz);
void PSG_SetVoiceVol(uint8_t voice, uint16_t vol);
void PSG_SetVoiceWave(uint8_t voice, uint8_t wave);
void PSG_Fill(uint32_t *dst, uint32_t n);

void Audio_Start(void);
void Audio_Stop(void);

void Audio_SetMode(AudioMode mode);
AudioMode Audio_GetMode(void);

#endif /* INC_AUDIO_H_ */

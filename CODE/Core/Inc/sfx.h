/*
 * sfx.h
 *
 *  Created on: 15 Mar 2026
 *      Author: motre
 */

#ifndef INC_SFX_H_
#define INC_SFX_H_

#include <stdint.h>

typedef struct {
    uint16_t freq;
    uint16_t vol;
    uint16_t ms;
    uint8_t wave;
} SFX_Note;

void SFX_Init(void);
void SFX_Update(uint32_t samples_rendered);

void SFX_Play(uint8_t voice, const SFX_Note *seq, uint16_t count);
void SFX_StopVoice(uint8_t voice);
uint8_t SFX_IsVoiceBusy(uint8_t voice);

#endif /* INC_SFX_H_ */

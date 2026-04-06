/*
 * tetris_audio.h
 *
 *  Created on: 22 Feb 2026
 *      Author: motre
 */

#ifndef INC_TETRIS_AUDIO_H_
#define INC_TETRIS_AUDIO_H_

#include <stdint.h>
#include <stddef.h>

void TetrisAudio_Init(uint32_t sample_rate_hz);
void TetrisAudio_Start(void);
void TetrisAudio_Stop(void);
void TetrisAudio_SetBPM(uint32_t bpm);
void TetrisAudio_Update(uint32_t samples_rendered);

uint8_t TetrisAudio_IsPlaying(void);
void TetrisSFX_GameOver(void);

#endif /* INC_TETRIS_AUDIO_H_ */

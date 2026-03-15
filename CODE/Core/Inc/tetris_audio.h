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

#ifdef __cplusplus
extern "C" {
#endif

static uint8_t g_tetris_playing = 0;

void TetrisAudio_Start(void);
void TetrisAudio_Stop(void);
void TetrisAudio_Init(uint32_t sample_rate_hz);
void TetrisAudio_SetBPM(uint32_t bpm);
void TetrisAudio_Fill(uint32_t* dst, size_t n_samples);

#ifdef __cplusplus
}
#endif

#endif /* INC_TETRIS_AUDIO_H_ */

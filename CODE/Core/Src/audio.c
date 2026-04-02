/*
 * audio.c
 *
 *  Created on: 14 Feb 2026
 *      Author: motre
 */

#include "audio.h"
#include "dac.h"
#include "sfx.h"
#include "tetris_audio.h"

uint32_t audioBuf[AUDIO_BUF];

typedef struct {
    uint32_t phase;
    uint32_t inc;
    uint16_t vol;
    uint8_t  wave;   /* 0=square 1=tri 2=saw 3=noise */
} Voice;

static Voice v[4];
static uint16_t lfsr = 0xACE1u;
static AudioMode g_audioMode = AUDIO_MODE_SFX;

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    if (hdac->Instance != DAC2) return;

    if (g_audioMode == AUDIO_MODE_TETRIS) {
        TetrisAudio_Update(AUDIO_BUF / 2);
    } else {
        SFX_Update(AUDIO_BUF / 2);
    }

    PSG_Fill(&audioBuf[0], AUDIO_BUF / 2);
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    if (hdac->Instance != DAC2) return;

    if (g_audioMode == AUDIO_MODE_TETRIS) {
        TetrisAudio_Update(AUDIO_BUF / 2);
    } else {
        SFX_Update(AUDIO_BUF / 2);
    }

    PSG_Fill(&audioBuf[AUDIO_BUF / 2], AUDIO_BUF / 2);
}

static inline uint32_t hz_to_inc(float hz)
{
    double inc = (double)hz * (4294967296.0 / (double)SR);
    if (inc < 0) inc = 0;
    if (inc > 4294967295.0) inc = 4294967295.0;
    return (uint32_t)inc;
}

void PSG_Init(void) {
    for (int i = 0; i < 4; i++) {
        v[i].phase = 0;
        v[i].inc = 0;
        v[i].vol = 0;
        v[i].wave = 0;
    }
}

static void Audio_ClearVoices(void) {
    for (int i = 0; i < 4; i++) {
        v[i].phase = 0;
        v[i].inc = 0;
        v[i].vol = 0;
        v[i].wave = 0;
    }
}

void Audio_Start(void) {
    PSG_Init();
    SFX_Init();
    TetrisAudio_Init(SR);

    g_audioMode = AUDIO_MODE_SFX;

    for (uint32_t i = 0; i < AUDIO_BUF; i++) {
        audioBuf[i] = 0x08000800u;
    }

    HAL_DAC_Start_DMA(&hdac2, DAC_CHANNEL_1, (uint32_t*)audioBuf, 1024, DAC_ALIGN_12B_R);
}

void Audio_Stop(void)
{
    HAL_DAC_Stop_DMA(&hdac2, DAC_CHANNEL_1);
    Audio_ClearVoices();
}

void Audio_SetMode(AudioMode mode)
{
    if (g_audioMode == mode)
        return;

    /* stop whatever was previously active */
    if (g_audioMode == AUDIO_MODE_TETRIS) {
        TetrisAudio_Stop();
    }

    Audio_ClearVoices();

    if (mode == AUDIO_MODE_TETRIS) {
        TetrisAudio_Init(SR);
        TetrisAudio_Start();
    } else {
        SFX_Init();
    }

    g_audioMode = mode;
}

AudioMode Audio_GetMode(void)
{
    return g_audioMode;
}

void PSG_SetVoiceFreq(uint8_t voice, float hz)
{
    if (voice >= 4) return;
    v[voice].inc = hz_to_inc(hz);
}

void PSG_SetVoiceVol(uint8_t voice, uint16_t vol)
{
    if (voice >= 4) return;
    if (vol > 4095) vol = 4095;
    v[voice].vol = vol;
}

void PSG_SetVoiceWave(uint8_t voice, uint8_t wave)
{
    if (voice >= 4) return;
    v[voice].wave = wave;
}

static inline int32_t osc_sample(Voice *o)
{
    o->phase += o->inc;
    uint32_t p = o->phase;

    int32_t s;
    if (o->wave == 0) {
        s = (p & 0x80000000u) ? 2047 : -2047;
    } else if (o->wave == 1) {
        uint32_t x = p >> 20;
        if (p & 0x80000000u) x = 4095 - x;
        s = (int32_t)x - 2048;
    } else {
        s = (int32_t)(p >> 20) - 2048;
    }

    return (s * (int32_t)o->vol) >> 12;
}

static inline int32_t noise_sample(uint16_t vol)
{
    uint16_t bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
    lfsr = (lfsr >> 1) | (bit << 15);
    return (((lfsr & 1) ? 2047 : -2047) * (int32_t)vol) >> 12;
}

void PSG_Fill(uint32_t *dst, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {
        int32_t mix = 0;

        mix += osc_sample(&v[0]);
        mix += osc_sample(&v[1]);
        mix += osc_sample(&v[2]);

        if (v[3].vol) {
            if (v[3].wave == 3) mix += noise_sample(v[3].vol);
            else                mix += osc_sample(&v[3]);
        }

        if (mix > 2047) mix = 2047;
        if (mix < -2047) mix = -2047;

        uint32_t s = (uint16_t)(mix + 2048);
        dst[i] = (s & 0x0FFFu) | ((s & 0x0FFFu) << 16);
    }
}

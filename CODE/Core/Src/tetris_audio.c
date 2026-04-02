/*
 * tetris_audio.c
 *
 *  Created on: 22 Feb 2026
 *      Author: motre
 */

#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "tetris_audio.h"
#include "audio.h"

#ifndef TA_NUM_CH
#define TA_NUM_CH 4
#endif

#ifndef TA_WAVE_SQUARE
#define TA_WAVE_SQUARE 0
#endif
#ifndef TA_WAVE_TRI
#define TA_WAVE_TRI    1
#endif
#ifndef TA_WAVE_SAW
#define TA_WAVE_SAW    2
#endif
#ifndef TA_WAVE_NOISE
#define TA_WAVE_NOISE  3
#endif

typedef struct {
    int8_t  note;     /* MIDI note, -1 = rest */
    uint16_t len16;    /* duration in 1/16th notes */
} TA_Event;

typedef struct {
    const TA_Event *seq;
    size_t          seq_len;
    size_t          idx;
    uint32_t        remain_ticks;
} TA_Track;

static uint8_t  g_tetris_playing = 0;
static uint32_t g_sr = 48000;
static uint32_t g_bpm = 500;
static uint32_t g_samples_per_tick = 0;
static uint32_t g_tick_sample_acc = 0;

static uint16_t g_vol_mel = 2400;
static uint16_t g_vol_har = 1600;
static uint16_t g_vol_bas = 2200;
static uint16_t g_vol_noi = 500;

static TA_Track tr_mel;
static TA_Track tr_har;
static TA_Track tr_bas;
static TA_Track tr_noi;

static float midi_to_freq(int midi_note);
static void TA_LoadNext(TA_Track *tr, uint8_t ch, uint16_t vol, int is_noise);
static void TA_AdvanceOne(TA_Track *tr, uint8_t ch, uint16_t vol, int is_noise);
static void TA_AdvanceTracks(uint32_t ticks_to_advance);
static void TA_ApplyEventToChannel(uint8_t ch, const TA_Event *ev, uint16_t vol, int is_noise);
static void TA_ResetTracks(void);

/* melody */
static const TA_Event g_melody[] = {
    { 76, 4 }, { 71, 2 }, { 72, 2 }, { 74, 4 }, { 72, 2 }, { 71, 2 }, { 69, 6 }, { 72, 2 },
    { 76, 4 }, { 74, 2 }, { 72, 2 }, { 71, 4 }, { -1, 2 }, { 72, 2 }, { 74, 4 }, { 76, 4 },
    { 72, 4 }, { 69, 8 }, { -1, 6 }, { 74, 4 }, { 77, 2 }, { 81, 4 }, { 79, 2 }, { 77, 2 },
    { 76, 6 }, { 72, 2 }, { 76, 4 }, { 74, 2 }, { 72, 2 }, { 71, 6 }, { 72, 2 }, { 74, 4 },
    { 76, 4 }, { 72, 4 }, { 69, 8 }, { -1, 4 }, { 76, 4 }, { 71, 2 }, { 72, 2 }, { 74, 4 },
    { 72, 2 }, { 71, 2 }, { 69, 6 }, { 72, 2 }, { 76, 4 }, { 74, 2 }, { 72, 2 }, { 71, 4 },
    { -1, 2 }, { 72, 2 }, { 74, 4 }, { 76, 4 }, { 72, 4 }, { 69, 8 }, { -1, 6 }, { 74, 4 },
    { 77, 2 }, { 81, 4 }, { 79, 2 }, { 77, 2 }, { 76, 6 }, { 72, 2 }, { 76, 4 }, { 74, 2 },
    { 72, 2 }, { 71, 6 }, { 72, 2 }, { 74, 4 }, { 76, 4 }, { 72, 4 }, { 69, 8 },
};
static const size_t g_melody_len = sizeof(g_melody) / sizeof(g_melody[0]);

/* harmony */
static const TA_Event g_harmony[] = {
    { 71, 4 }, { 68, 2 }, { 69, 2 }, { 71, 2 }, { 76, 1 }, { 74, 1 }, { 69, 2 }, { 68, 2 },
    { 64, 2 }, { -1, 4 }, { 69, 2 }, { 72, 4 }, { 71, 2 }, { 69, 2 }, { 68, 2 }, { 64, 1 },
    { -1, 1 }, { 68, 2 }, { 69, 2 }, { 71, 4 }, { 72, 4 }, { 69, 4 }, { 64, 8 }, { -1, 4 },
    { 50, 2 }, { 65, 4 }, { 69, 2 }, { 72, 4 }, { 71, 2 }, { 69, 2 }, { 67, 6 }, { 64, 2 },
    { 67, 2 }, { 69, 1 }, { 67, 1 }, { 65, 2 }, { 64, 2 }, { 68, 2 }, { 64, 2 }, { 68, 2 },
    { 69, 2 }, { 71, 2 }, { 68, 2 }, { 72, 2 }, { 68, 2 }, { 69, 1 }, { 72, 1 }, { 64, 10 },
    { -1, 4 }, { 71, 4 }, { 68, 2 }, { 69, 2 }, { 71, 2 }, { 76, 1 }, { 74, 1 }, { 69, 2 },
    { 68, 2 }, { 64, 2 }, { -1, 4 }, { 69, 2 }, { 72, 4 }, { 71, 2 }, { 69, 2 }, { 68, 2 },
    { 64, 1 }, { -1, 1 }, { 68, 2 }, { 69, 2 }, { 71, 4 }, { 72, 4 }, { 69, 4 }, { 64, 8 },
    { -1, 4 }, { 50, 2 }, { 65, 4 }, { 69, 2 }, { 72, 4 }, { 71, 2 }, { 69, 2 }, { 67, 6 },
    { 64, 2 }, { 67, 2 }, { 69, 1 }, { 67, 1 }, { 65, 2 }, { 64, 2 }, { 68, 2 }, { 64, 2 },
    { 68, 2 }, { 69, 2 }, { 71, 2 }, { 68, 2 }, { 72, 2 }, { 68, 2 }, { 69, 1 }, { 72, 1 },
    { 64, 10 },
};
static const size_t g_harmony_len = sizeof(g_harmony) / sizeof(g_harmony[0]);

/* bass */
static const TA_Event g_bass[] = {
    { 52, 2 }, { 64, 2 }, { 52, 2 }, { 64, 2 }, { 52, 2 }, { 64, 2 }, { 52, 2 }, { 64, 2 },
    { 57, 2 }, { 69, 2 }, { 57, 2 }, { 69, 2 }, { 57, 2 }, { 69, 2 }, { 57, 2 }, { 69, 2 },
    { 56, 2 }, { 68, 2 }, { 56, 2 }, { 68, 2 }, { 52, 2 }, { 64, 2 }, { 52, 2 }, { 64, 2 },
    { 57, 2 }, { 69, 2 }, { 57, 2 }, { 69, 2 }, { 57, 2 }, { 69, 2 }, { 59, 2 }, { 60, 2 },
    { 62, 2 }, { 50, 2 }, { -1, 2 }, { 50, 2 }, { -1, 2 }, { 50, 2 }, { 57, 2 }, { 53, 2 },
    { 48, 2 }, { 60, 2 }, { -1, 2 }, { 60, 2 }, { -1, 2 }, { 55, 6 }, { 59, 2 }, { 71, 2 },
    { -1, 2 }, { 71, 2 }, { -1, 2 }, { 64, 2 }, { -1, 2 }, { 68, 2 }, { 45, 2 }, { 57, 2 },
    { 45, 2 }, { 57, 2 }, { 45, 6 }, { -1, 2 }, { 52, 2 }, { 64, 2 }, { 52, 2 }, { 64, 2 },
    { 52, 2 }, { 64, 2 }, { 52, 2 }, { 64, 2 }, { 57, 2 }, { 69, 2 }, { 57, 2 }, { 69, 2 },
    { 57, 2 }, { 69, 2 }, { 57, 2 }, { 69, 2 }, { 56, 2 }, { 68, 2 }, { 56, 2 }, { 68, 2 },
    { 52, 2 }, { 64, 2 }, { 52, 2 }, { 64, 2 }, { 57, 2 }, { 69, 2 }, { 57, 2 }, { 69, 2 },
    { 57, 2 }, { 69, 2 }, { 59, 2 }, { 60, 2 }, { 62, 2 }, { 50, 2 }, { -1, 2 }, { 50, 2 },
    { -1, 2 }, { 50, 2 }, { 57, 2 }, { 53, 2 }, { 48, 2 }, { 60, 2 }, { -1, 2 }, { 60, 2 },
    { -1, 2 }, { 55, 6 }, { 59, 2 }, { 71, 2 }, { -1, 2 }, { 71, 2 }, { -1, 2 }, { 64, 2 },
    { -1, 2 }, { 68, 2 }, { 45, 2 }, { 57, 2 }, { 45, 2 }, { 57, 2 }, { 45, 6 }, { -1, 2 },
    { 69, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 },
    { 68, 2 }, { 76, 2 }, { 68, 2 }, { 76, 2 }, { 68, 2 }, { 76, 2 }, { 68, 2 }, { 76, 2 },
    { 69, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 },
    { 68, 2 }, { 76, 2 }, { 68, 2 }, { 76, 2 }, { 68, 2 }, { -1, 6 }, { 69, 2 }, { 76, 2 },
    { 69, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 }, { 68, 2 }, { 76, 2 },
    { 68, 2 }, { 76, 2 }, { 68, 2 }, { 76, 2 }, { 68, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 },
    { 69, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 }, { 69, 2 }, { 76, 2 }, { 68, 2 }, { 76, 2 },
    { 68, 2 }, { 76, 2 }, { 68, 2 },
};
static const size_t g_bass_len = sizeof(g_bass) / sizeof(g_bass[0]);

/* noise/percussion */
static const TA_Event g_noise[] = {
    { -1, 382 },
};
static const size_t g_noise_len = sizeof(g_noise) / sizeof(g_noise[0]);

static float midi_to_freq(int midi_note)
{
    return 440.0f * powf(2.0f, (midi_note - 69) / 12.0f);
}

static void TA_ResetTracks(void)
{
    tr_mel.seq = g_melody;
    tr_mel.seq_len = g_melody_len;
    tr_mel.idx = 0;
    tr_mel.remain_ticks = 0;

    tr_har.seq = g_harmony;
    tr_har.seq_len = g_harmony_len;
    tr_har.idx = 0;
    tr_har.remain_ticks = 0;

    tr_bas.seq = g_bass;
    tr_bas.seq_len = g_bass_len;
    tr_bas.idx = 0;
    tr_bas.remain_ticks = 0;

    tr_noi.seq = g_noise;
    tr_noi.seq_len = g_noise_len;
    tr_noi.idx = 0;
    tr_noi.remain_ticks = 0;

    g_tick_sample_acc = 0;
}

void TetrisAudio_Init(uint32_t sample_rate_hz)
{
    g_sr = (sample_rate_hz == 0) ? SR : sample_rate_hz;

    TetrisAudio_SetBPM(g_bpm);

    /* Set up dedicated Tetris music voices */
    PSG_SetVoiceWave(0, TA_WAVE_SQUARE);
    PSG_SetVoiceWave(1, TA_WAVE_SQUARE);
    PSG_SetVoiceWave(2, TA_WAVE_TRI);
    PSG_SetVoiceWave(3, TA_WAVE_NOISE);

    PSG_SetVoiceVol(0, 0);
    PSG_SetVoiceVol(1, 0);
    PSG_SetVoiceVol(2, 0);
    PSG_SetVoiceVol(3, 0);

    TA_ResetTracks();
}

void TetrisAudio_Start(void)
{
    g_tetris_playing = 1;
    TA_ResetTracks();

    /* Load first notes immediately */
    TA_AdvanceTracks(1);
}

void TetrisAudio_Stop(void)
{
    g_tetris_playing = 0;

    PSG_SetVoiceVol(0, 0);
    PSG_SetVoiceVol(1, 0);
    PSG_SetVoiceVol(2, 0);
    PSG_SetVoiceVol(3, 0);
}

uint8_t TetrisAudio_IsPlaying(void)
{
    return g_tetris_playing;
}

void TetrisAudio_SetBPM(uint32_t bpm)
{
    if (bpm < 40)  bpm = 40;
    if (bpm > 260) bpm = 260;

    g_bpm = bpm;
    g_samples_per_tick = (uint32_t)(((uint64_t)g_sr * 60ULL) / ((uint64_t)g_bpm * 4ULL));
    if (g_samples_per_tick == 0) g_samples_per_tick = 1;
}

void TetrisAudio_Update(uint32_t samples_rendered)
{
    if (!g_tetris_playing || samples_rendered == 0)
        return;

    g_tick_sample_acc += samples_rendered;

    while (g_tick_sample_acc >= g_samples_per_tick)
    {
        g_tick_sample_acc -= g_samples_per_tick;
        TA_AdvanceTracks(1);
    }
}

static void TA_LoadNext(TA_Track *tr, uint8_t ch, uint16_t vol, int is_noise)
{
    const TA_Event *ev;

    if (tr->seq_len == 0)
        return;

    ev = &tr->seq[tr->idx];
    tr->idx = (tr->idx + 1) % tr->seq_len;
    tr->remain_ticks = (ev->len16 == 0) ? 1u : (uint32_t)ev->len16;

    TA_ApplyEventToChannel(ch, ev, vol, is_noise);
}

static void TA_AdvanceOne(TA_Track *tr, uint8_t ch, uint16_t vol, int is_noise)
{
    if (tr->remain_ticks > 0)
        tr->remain_ticks--;

    if (tr->remain_ticks == 0)
        TA_LoadNext(tr, ch, vol, is_noise);
}

static void TA_AdvanceTracks(uint32_t ticks_to_advance)
{
    while (ticks_to_advance--)
    {
        TA_AdvanceOne(&tr_mel, 0, g_vol_mel, 0);
        TA_AdvanceOne(&tr_har, 1, g_vol_har, 0);
        TA_AdvanceOne(&tr_bas, 2, g_vol_bas, 0);
        TA_AdvanceOne(&tr_noi, 3, g_vol_noi, 1);
    }
}

static void TA_ApplyEventToChannel(uint8_t ch, const TA_Event *ev, uint16_t vol, int is_noise)
{
    if (is_noise)
    {
        if (ev->note < 0)
        {
            PSG_SetVoiceVol(ch, 0);
        }
        else
        {
            uint16_t v = vol;

            if (ev->note == 1) v = (uint16_t)((uint32_t)vol * 3u / 2u);
            if (ev->note >= 2) v = (uint16_t)((uint32_t)vol * 2u);
            if (v > 4095) v = 4095;

            PSG_SetVoiceWave(ch, TA_WAVE_NOISE);
            PSG_SetVoiceVol(ch, v);
        }
        return;
    }

    if (ev->note < 0)
    {
        PSG_SetVoiceVol(ch, 0);
        return;
    }

    PSG_SetVoiceFreq(ch, midi_to_freq((int)ev->note));
    PSG_SetVoiceVol(ch, vol);
}

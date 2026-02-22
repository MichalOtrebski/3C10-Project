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
#include "setup.h"
#include "dac.h"
#include "tim.h"

#ifndef TA_NUM_CH
#define TA_NUM_CH 4
#endif

// --- PSG waves ---
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

// --- Utility: MIDI note -> frequency (A4=440Hz) ---
static float midi_to_freq(int midi_note)
{
    // midi_note: 69 -> 440Hz
    return 440.0f * powf(2.0f, (midi_note - 69) / 12.0f);
}

// --- Song representation ---
typedef struct {
    int8_t  note;     // MIDI note, -1 = rest
    uint8_t len16;    // duration in 1/16th notes
} TA_Event;

// Simplified “Korobeiniki” melody (Tetris A theme) in 1/16ths
// Key: E minor-ish; you can transpose if you like.
static const TA_Event g_melody[] = {
    // Phrase A
    { 76,2 }, { 71,2 }, { 72,2 }, { 74,2 }, { 72,2 }, { 71,2 }, { 69,4 }, { -1,2 }, { 69,2 },
    { 72,2 }, { 76,2 }, { 74,2 }, { 72,2 }, { 71,4 }, { -1,2 }, { 71,2 },
    { 72,2 }, { 74,2 }, { 76,2 }, { 72,2 }, { 69,2 }, { 69,4 }, { -1,2 }, { -1,2 },

    // Phrase B
    { 74,2 }, { 77,2 }, { 81,2 }, { 79,2 }, { 77,2 }, { 76,2 }, { 72,4 }, { -1,2 }, { 72,2 },
    { 76,2 }, { 74,2 }, { 72,2 }, { 71,4 }, { -1,2 }, { 71,2 },
    { 72,2 }, { 74,2 }, { 76,2 }, { 72,2 }, { 69,2 }, { 69,4 }, { -1,2 }, { -1,2 },
};

static const size_t g_melody_len = sizeof(g_melody) / sizeof(g_melody[0]);

// Harmony (simple thirds / supporting notes) — intentionally sparse
static const TA_Event g_harmony[] = {
    { 64,4 }, { -1,4 }, { 64,4 }, { -1,4 },
    { 66,4 }, { -1,4 }, { 67,4 }, { -1,4 },

    { 69,4 }, { -1,4 }, { 69,4 }, { -1,4 },
    { 67,4 }, { -1,4 }, { 66,4 }, { -1,4 },
};

static const size_t g_harmony_len = sizeof(g_harmony) / sizeof(g_harmony[0]);

// Bassline (roots)
static const TA_Event g_bass[] = {
    { 40,4 }, { 40,4 }, { 43,4 }, { 43,4 },
    { 45,4 }, { 45,4 }, { 43,4 }, { 43,4 },

    { 40,4 }, { 40,4 }, { 43,4 }, { 43,4 },
    { 45,4 }, { 45,4 }, { 43,4 }, { 43,4 },
};
static const size_t g_bass_len = sizeof(g_bass) / sizeof(g_bass[0]);

// “Hats” / ticks on noise channel: note field ignored (use -1 as rest), len16 sets rhythm
// Pattern: 1/16 on, 1/16 off repeating-ish
static const TA_Event g_noise[] = {
    {  0,1 }, { -1,1 }, {  0,1 }, { -1,1 }, {  0,1 }, { -1,1 }, {  0,1 }, { -1,1 },
    {  0,1 }, { -1,1 }, {  0,1 }, { -1,1 }, {  0,1 }, { -1,1 }, {  0,1 }, { -1,1 },
};
static const size_t g_noise_len = sizeof(g_noise) / sizeof(g_noise[0]);

// --- Sequencer state ---
typedef struct {
    const TA_Event* seq;
    size_t          seq_len;
    size_t          idx;
    uint32_t        remain_ticks; // in 1/16th-note ticks
} TA_Track;

static TA_Track tr_mel, tr_har, tr_bas, tr_noi;

static uint32_t g_sr = 22000;
static uint32_t g_bpm = 140; // Tetris theme often ~140-150 BPM; tweak to taste

// We advance at 1/16th notes.
static uint32_t g_samples_per_tick = 0;
static uint32_t g_tick_sample_acc  = 0;

// Volumes per channel (0..4095)
static uint16_t g_vol_mel = 2600;
static uint16_t g_vol_har = 1600;
static uint16_t g_vol_bas = 2200;
static uint16_t g_vol_noi = 900;

// Forward
static void TA_AdvanceTracks_IfNeeded(uint32_t ticks_to_advance);
static void TA_ApplyEventToChannel(uint8_t ch, const TA_Event* ev, uint16_t vol, int is_noise);

// Call once
void TetrisAudio_Init(uint32_t sample_rate_hz)
{
    g_sr = (sample_rate_hz == 0) ? 22000 : sample_rate_hz;

    // 1/16 note duration in seconds:
    // quarter note period = 60/BPM
    // 1/16 note period = (60/BPM)/4
    // samples_per_tick = sr * 60 / (BPM * 4)
    g_samples_per_tick = (uint32_t)(( (uint64_t)g_sr * 60ULL ) / ((uint64_t)g_bpm * 4ULL));
    if (g_samples_per_tick == 0) g_samples_per_tick = 1;

    // Init PSG + voice waves
    PSG_Init();

    // CH0 Melody: square
    PSG_SetVoiceWave(0, TA_WAVE_SQUARE);
    // CH1 Harmony: square or triangle (square gives classic chiptune)
    PSG_SetVoiceWave(1, TA_WAVE_SQUARE);
    // CH2 Bass: triangle (less buzzy bass)
    PSG_SetVoiceWave(2, TA_WAVE_TRI);
    // CH3 Noise: noise
    PSG_SetVoiceWave(3, TA_WAVE_NOISE);

    // Setup tracks
    tr_mel = (TA_Track){ g_melody,  g_melody_len,  0, 0 };
    tr_har = (TA_Track){ g_harmony, g_harmony_len, 0, 0 };
    tr_bas = (TA_Track){ g_bass,    g_bass_len,    0, 0 };
    tr_noi = (TA_Track){ g_noise,   g_noise_len,   0, 0 };

    // Force initial events
    tr_mel.remain_ticks = 0;
    tr_har.remain_ticks = 0;
    tr_bas.remain_ticks = 0;
    tr_noi.remain_ticks = 0;

    g_tick_sample_acc = 0;

    // Apply first events immediately
    TA_AdvanceTracks_IfNeeded(1);
}

// Optional: change tempo at runtime
void TetrisAudio_SetBPM(uint32_t bpm)
{
    if (bpm < 40) bpm = 40;
    if (bpm > 260) bpm = 260;
    g_bpm = bpm;

    g_samples_per_tick = (uint32_t)(( (uint64_t)g_sr * 60ULL ) / ((uint64_t)g_bpm * 4ULL));
    if (g_samples_per_tick == 0) g_samples_per_tick = 1;
}

// Fill audio buffer (call from DAC DMA half/full callbacks)
void TetrisAudio_Fill(uint32_t* dst, size_t n_samples)
{
    if (!dst || n_samples == 0) return;

    // small temp buffer for PSG's native 16-bit sample format
    static __attribute__((aligned(4))) uint16_t tmp16[256];

    size_t remaining = n_samples;
    while (remaining > 0)
    {
        uint32_t to_tick = g_samples_per_tick - g_tick_sample_acc;
        if (to_tick == 0) to_tick = g_samples_per_tick;

        uint32_t chunk = (remaining < to_tick) ? (uint32_t)remaining : to_tick;

        // render in smaller blocks to tmp16 then widen to dst
        size_t todo = chunk;
        while (todo)
        {
            size_t m = (todo > (sizeof(tmp16)/sizeof(tmp16[0]))) ? (sizeof(tmp16)/sizeof(tmp16[0])) : todo;

            PSG_Fill(tmp16, m);                 // PSG generates 0..4095 in uint16_t
            for (size_t i = 0; i < m; i++)
                dst[i] = (uint32_t)tmp16[i];    // store in low bits of uint32_t

            dst += m;
            todo -= m;
        }

        remaining -= chunk;

        g_tick_sample_acc += chunk;
        if (g_tick_sample_acc >= g_samples_per_tick)
        {
            g_tick_sample_acc = 0;
            TA_AdvanceTracks_IfNeeded(1);
        }
    }
}

// --- Internals ---
static void TA_LoadNext(TA_Track* tr, uint8_t ch, uint16_t vol, int is_noise)
{
    if (tr->seq_len == 0) return;

    const TA_Event* ev = &tr->seq[tr->idx];
    tr->idx = (tr->idx + 1) % tr->seq_len;

    tr->remain_ticks = (ev->len16 == 0) ? 1 : ev->len16;

    TA_ApplyEventToChannel(ch, ev, vol, is_noise);
}

static void TA_AdvanceOne(TA_Track* tr, uint8_t ch, uint16_t vol, int is_noise)
{
    if (tr->remain_ticks > 0)
    {
        tr->remain_ticks--;
    }

    if (tr->remain_ticks == 0)
    {
        TA_LoadNext(tr, ch, vol, is_noise);
    }
}

static void TA_AdvanceTracks_IfNeeded(uint32_t ticks_to_advance)
{
    while (ticks_to_advance--)
    {
        TA_AdvanceOne(&tr_mel, 0, g_vol_mel, 0);
        TA_AdvanceOne(&tr_har, 1, g_vol_har, 0);
        TA_AdvanceOne(&tr_bas, 2, g_vol_bas, 0);
        TA_AdvanceOne(&tr_noi, 3, g_vol_noi, 1);
    }
}

static void TA_ApplyEventToChannel(uint8_t ch, const TA_Event* ev, uint16_t vol, int is_noise)
{
    if (is_noise)
    {
        // For noise channel, we treat ev->note as “on/off”.
        if (ev->note < 0)
        {
            PSG_SetVoiceVol(ch, 0);
        }
        else
        {
            // You can also “tune” noise by setting a dummy freq if your noise uses it.
            PSG_SetVoiceVol(ch, vol);
        }
        return;
    }

    if (ev->note < 0)
    {
        PSG_SetVoiceVol(ch, 0);
        return;
    }

    float f = midi_to_freq((int)ev->note);
    PSG_SetVoiceFreq(ch, f);
    PSG_SetVoiceVol(ch, vol);
}

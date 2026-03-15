/*
 * sfx.c
 *
 *  Created on: 15 Mar 2026
 *      Author: motre
 */

#include "sfx.h"
#include "audio.h"

typedef struct {
    const SFX_Note *seq;
    uint16_t count;
    uint16_t index;
    uint32_t samples_left;
    uint8_t active;
} SFX_Channel;

static SFX_Channel ch[4];

static uint32_t ms_to_samples(uint16_t ms)
{
    return ((uint32_t)ms * SR) / 1000u;
}

static void sfx_apply_step(uint8_t voice)
{
    const SFX_Note *n;
    SFX_Channel *c = &ch[voice];

    if (!c->active || c->seq == 0 || c->index >= c->count) {
        PSG_SetVoiceVol(voice, 0);
        c->active = 0;
        return;
    }

    n = &c->seq[c->index];

    PSG_SetVoiceWave(voice, n->wave);

    if (n->freq == 0 || n->vol == 0) {
        PSG_SetVoiceVol(voice, 0);
        PSG_SetVoiceFreq(voice, 0);
    } else {
        PSG_SetVoiceFreq(voice, (float)n->freq);
        PSG_SetVoiceVol(voice, n->vol);
    }

    c->samples_left = ms_to_samples(n->ms);
    if (c->samples_left == 0) c->samples_left = 1;
}

void SFX_Init(void)
{
    for (uint8_t i = 0; i < 4; i++) {
        ch[i].seq = 0;
        ch[i].count = 0;
        ch[i].index = 0;
        ch[i].samples_left = 0;
        ch[i].active = 0;

        PSG_SetVoiceVol(i, 0);
        PSG_SetVoiceFreq(i, 0);
        PSG_SetVoiceWave(i, 0);
    }
}

void SFX_Play(uint8_t voice, const SFX_Note *seq, uint16_t count)
{
    if (voice >= 4 || seq == 0 || count == 0) return;

    ch[voice].seq = seq;
    ch[voice].count = count;
    ch[voice].index = 0;
    ch[voice].active = 1;

    sfx_apply_step(voice);
}

void SFX_StopVoice(uint8_t voice)
{
    if (voice >= 4) return;

    ch[voice].active = 0;
    ch[voice].seq = 0;
    ch[voice].count = 0;
    ch[voice].index = 0;
    ch[voice].samples_left = 0;

    PSG_SetVoiceVol(voice, 0);
    PSG_SetVoiceFreq(voice, 0);
}

uint8_t SFX_IsVoiceBusy(uint8_t voice)
{
    if (voice >= 4) return 0;
    return ch[voice].active;
}

void SFX_Update(uint32_t samples_rendered)
{
    for (uint8_t i = 0; i < 4; i++) {
        SFX_Channel *c = &ch[i];

        if (!c->active) continue;

        if (samples_rendered < c->samples_left) {
            c->samples_left -= samples_rendered;
            continue;
        }

        while (c->active && samples_rendered >= c->samples_left) {
            samples_rendered -= c->samples_left;
            c->index++;

            if (c->index >= c->count) {
                SFX_StopVoice(i);
                break;
            }

            sfx_apply_step(i);
        }
    }
}

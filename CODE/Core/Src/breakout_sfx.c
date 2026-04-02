/*
 * breakout_sfx.c
 *
 *  Created on: 31 Mar 2026
 *      Author: motre
 */

#include "breakout_sfx.h"
#include "sfx.h"

static const SFX_Note breakout_bounce_seq[] = {
    { 60,  1600, 24, 1 },
    { 40,  2000, 18, 1 }
};

static const SFX_Note breakout_block_bounce_seq[] = {
    { 80,  1200, 30, 1 },
    { 70,  1800, 26, 1 },
    { 60,  1400, 20, 1 }
};

static const SFX_Note breakout_powerup_seq[] = {
    { 70,  1000, 20, 2 },
    { 70,  1400, 24, 2 },
    { 70,  1800, 28, 2 },
    { 90,  2400, 32, 2 }
};

static const SFX_Note breakout_gameover_seq[] = {
	    { 700, 2500, 35, 0 },
	    { 520, 2300, 35, 0 },
	    { 380, 2100, 45, 0 },
	    { 240, 1900, 70, 3 }
};

void BreakoutSFX_Bounce(void)
{
    SFX_Play(1, breakout_bounce_seq, sizeof(breakout_bounce_seq) / sizeof(breakout_bounce_seq[0]));
}

void BreakoutSFX_BlockHit(void)
{
    SFX_Play(1, breakout_block_bounce_seq, sizeof(breakout_block_bounce_seq) / sizeof(breakout_block_bounce_seq[0]));
}

void BreakoutSFX_PowerUp(void)
{
    SFX_Play(1, breakout_powerup_seq, sizeof(breakout_powerup_seq) / sizeof(breakout_powerup_seq[0]));
}

void BreakoutSFX_GameOver(void)
{
    SFX_Play(1, breakout_gameover_seq, sizeof(breakout_gameover_seq) / sizeof(breakout_gameover_seq[0]));
}

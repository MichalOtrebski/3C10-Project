/*
 * breakout_sfx.c
 *
 *  Created on: 31 Mar 2026
 *      Author: motre
 */

#include "breakout_sfx.h"
#include "sfx.h"

static const SFX_Note breakout_bounce_seq[] = {
	{ 1000, 1800, 60, 1 },
	{ 800, 1600, 55, 1 },
};

static const SFX_Note breakout_block_bounce_seq[] = {
	{ 1200, 1200, 50, 1 },
	{ 1500, 1000, 40, 1}
};

static const SFX_Note breakout_powerup_seq[] = {
    { 500, 900, 70, 2 },
    { 700, 1200, 70, 2 },
    { 900, 1600, 70, 2 },
    { 1100, 2000, 90, 2 }
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

/*
 * snake_sfx.c
 *
 *  Created on: 15 Mar 2026
 *      Author: motre
 */


#include "snake_sfx.h"
#include "sfx.h"

static const SFX_Note snake_move_seq[] = {
    { 300, 500, 8, 0 }
};

static const SFX_Note snake_eat_seq[] = {
    { 700, 2200, 18, 1 },
    { 1100, 2600, 18, 1 },
    { 1500, 3000, 22, 1 }
};

static const SFX_Note snake_hit_seq[] = {
    { 220, 3000, 35, 2 },
    { 160, 2600, 45, 2 }
};

static const SFX_Note snake_gameover_seq[] = {
    { 700, 2500, 35, 0 },
    { 520, 2300, 35, 0 },
    { 380, 2100, 45, 0 },
    { 240, 1900, 70, 3 }
};

void SnakeSFX_MoveTick(void)
{
    SFX_Play(1, snake_move_seq, sizeof(snake_move_seq) / sizeof(snake_move_seq[0]));
}

void SnakeSFX_Eat(void)
{
    SFX_Play(1, snake_eat_seq, sizeof(snake_eat_seq) / sizeof(snake_eat_seq[0]));
}

void SnakeSFX_HitWall(void)
{
    SFX_Play(1, snake_hit_seq, sizeof(snake_hit_seq) / sizeof(snake_hit_seq[0]));
}

void SnakeSFX_GameOver(void)
{
    SFX_Play(1, snake_gameover_seq, sizeof(snake_gameover_seq) / sizeof(snake_gameover_seq[0]));
}

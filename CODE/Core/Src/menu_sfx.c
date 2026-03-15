/*
 * menu_sfx.c
 *
 *  Created on: 15 Mar 2026
 *      Author: motre
 */


#include "menu_sfx.h"
#include "sfx.h"

static const SFX_Note menu_move_seq[] = {
    { 900, 2200, 20, 0 },
    { 0,      0,  8, 0 }
};

static const SFX_Note menu_select_seq[] = {
    { 700, 2200, 25, 0 },
    { 1000, 2400, 25, 0 },
    { 1400, 2600, 35, 0 }
};

static const SFX_Note menu_back_seq[] = {
    { 1200, 2000, 20, 0 },
    { 800,  1800, 30, 0 }
};

void MenuSFX_Move(void)
{
    SFX_Play(0, menu_move_seq, sizeof(menu_move_seq) / sizeof(menu_move_seq[0]));
}

void MenuSFX_Select(void)
{
    SFX_Play(0, menu_select_seq, sizeof(menu_select_seq) / sizeof(menu_select_seq[0]));
}

void MenuSFX_Back(void)
{
    SFX_Play(0, menu_back_seq, sizeof(menu_back_seq) / sizeof(menu_back_seq[0]));
}

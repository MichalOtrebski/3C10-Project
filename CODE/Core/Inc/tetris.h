/*
 * tetris.h
 *
 *  Created on: 6 Feb 2026
 *      Author: motre
 */

#ifndef INC_TETRIS_H_
#define INC_TETRIS_H_

#include "globals.h"
#include "display.h"
#include "text.h"

#define CELL_T 6

#define PLAY_W_T 10
#define PLAY_H_T 20

#define GRID_W_T (PLAY_W_T + 2)   // left/right walls
#define GRID_H_T (PLAY_H_T + 2)   // top/bottom walls

#define UI_TOP_T 35
#define UI_SIDE_W_T 28
#define GAP_T 10

#define FIELD_X_T (GAP_T)
#define FIELD_Y_T (UI_TOP_T)

#define FIELD_W_T (PLAY_W_T * CELL_T)
#define FIELD_H_T (PLAY_H_T * CELL_T)

#define NEXT_X_T  (FIELD_X_T + FIELD_W_T + GAP_T + 2)
#define NEXT_Y_T  (FIELD_Y_T + 8)

#define HOLD_X_T  (FIELD_X_T)
#define HOLD_Y_T  (3)

#define SCORE_X_T (48)
#define SCORE_Y_T (3)

#define SPAWN_X  (GRID_W_T / 2)
#define SPAWN_Y  3

#define PREDICT  3


#define C_BG 0x0000
#define C_BD 0xFFFF
#define C_B1 0x07FE //cyan
#define C_B2 0xFD40 //orange
#define C_B3 0x181F //blue
#define C_B4 0x07E0 //green
#define C_B5 0xF800 //red
#define C_B6 0xFFE0 //yellow
#define C_B7 0xD01F //purple

int B_init(int state, int anch_x, int anch_y, int B);
void B1_init(int state, int anch_x, int anch_y);
void B2_init(int state, int anch_x, int anch_y);
void B3_init(int state, int anch_x, int anch_y);
void B4_init(int state, int anch_x, int anch_y);
void B5_init(int state, int anch_x, int anch_y);
void B6_init(int state, int anch_x, int anch_y);
void B7_init(int state, int anch_x, int anch_y);

void B_tick(void);

void Grid_init(void);

void Tetris_Update(uint16_t pressed, uint16_t down, uint16_t held_ev);

#endif /* INC_TETRIS_H_ */

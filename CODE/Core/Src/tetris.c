/*
 * tetris.c
 *
 *  Created on: 6 Feb 2026
 *      Author: motre
 */

#include "globals.h"
#include "display.h"
#include "text.h"
#include "tetris.h"
#include "buttons.h"
#include <stdbool.h>
#include "main.h"
#include <stdio.h>
#include "menu.h"
#include <stdlib.h>
#include "rng.h"
#include "audio.h"
#include "tetris_audio.h"

int predict_block[PREDICT] = {-1};

static bool once = true;
static uint32_t s_lastTickMs;

//grid value at black = 0
//grid value at white = 1
//grid value at falling blocks = 2
//grid value at placed block I = 3
//grid value at placed block L = 4
//grid value at placed block J = 5
//grid value at placed block S = 6
//grid value at placed block Z = 7
//grid value at placed block square = 8
//grid value at placed block T = 9

int grid[GRID_W_T][GRID_H_T] = {0};

static inline int GX(int gx) { return FIELD_X_T + (gx - 1) * CELL_T; }
static inline int GY(int gy) { return FIELD_Y_T + (gy - 1) * CELL_T; }

float scale;

static int cur_x, cur_y;
static int cur_state;
static int cur_type;
static int has_piece;
static int game_over;
static int score;
static int block;
static int hold_used;
static int store_block;

bool game_over_sound = true;

static uint16_t FallingColour(void) {
    switch (cur_type) {
    case 0: return C_B1;
    case 1: return C_B2;
    case 2: return C_B3;
    case 3: return C_B4;
    case 4: return C_B5;
    case 5: return C_B6;
    case 6: return C_B7;
    default: return C_BG;
    }
}

static uint16_t BlockColourFromCell(int cell) {
    switch (cell) {
    case 2: return FallingColour();
    case 3: return C_B1;
    case 4: return C_B2;
    case 5: return C_B3;
    case 6: return C_B4;
    case 7: return C_B5;
    case 8: return C_B6;
    case 9: return C_B7;
    case 1: return C_BD;
    case 0:
    default:
        return C_BG;
    }
}

static void DrawCell(int gx, int gy, uint16_t c) {
    int x = GX(gx);
    int y = GY(gy);

    if (c == C_BG) {
        LCD_DrawRect(x, y, CELL_T, CELL_T, C_BG);
        return;
    }

    uint16_t border = Darken565(c, 60);

    LCD_DrawRect(x, y, CELL_T, CELL_T, border);

    if (CELL_T > 2) {
        LCD_DrawRect(x + 1, y + 1, CELL_T - 2, CELL_T - 2, c);
    }
}

static void DrawMiniCell(int x, int y, int s, uint16_t c) {
    if (c == C_BG) {
        LCD_DrawRect(x, y, s, s, C_BG);
        return;
    }

    uint16_t border = Darken565(c, 60);

    LCD_DrawRect(x, y, s, s, border);

    if (s > 2) {
        LCD_DrawRect(x + 1, y + 1, s - 2, s - 2, c);
    }
}

static void DrawDoublePanelFrame(int x, int y, int w, int h) {
    // outer border
    LCD_DrawRect(x - 2, y - 2, w + 4, h + 4, C_BD);

    // clear the gap between borders
    LCD_DrawRect(x - 1, y - 1, w + 2, h + 2, C_BG);

    // inner border
    LCD_DrawRect(x, y, w, h, C_BD);

    // panel interior
    if (w > 2 && h > 2) {
        LCD_DrawRect(x + 1, y + 1, w - 2, h - 2, C_BG);
    }
}

static void DrawSinglePanelFrame(int x, int y, int w, int h) {
    LCD_DrawRect(x, y, w, h, C_BD);

    if (w > 2 && h > 2) {
        LCD_DrawRect(x + 1, y + 1, w - 2, h - 2, C_BG);
    }
}

static void DrawFuturePanels(void) {
    DrawDoublePanelFrame(HOLD_X_T, HOLD_Y_T, 24, 20);
    DrawSinglePanelFrame(NEXT_X_T, NEXT_Y_T, 24, 50);
}

static void DrawStoredBlock(void) {
    if (store_block < 0) return;

    int hx = HOLD_X_T + 4;
    int hy = HOLD_Y_T + 4;
    int s = 4;
    uint16_t c;

    switch (store_block) {
        case 0: c = C_B1; break;
        case 1: c = C_B2; break;
        case 2: c = C_B3; break;
        case 3: c = C_B4; break;
        case 4: c = C_B5; break;
        case 5: c = C_B6; break;
        case 6: c = C_B7; break;
        default: return;
    }

    switch(store_block){
        case 0: hy += 4; break;
        case 1: hx += 4; break;
        case 2: hx += 4; break;
        case 3: hx += 2; hy += 2; break;
        case 4: hx += 2; hy += 2; break;
        case 5: hx += 4; hy += 2; break;
        case 6: hx += 2; hy += 6; break;
    }

    switch(store_block) {
        case 0: // I
            DrawMiniCell(hx,     hy, s, c);
            DrawMiniCell(hx+s,   hy, s, c);
            DrawMiniCell(hx+2*s, hy, s, c);
            DrawMiniCell(hx+3*s, hy, s, c);
            break;

        case 1: // L
            DrawMiniCell(hx,   hy,     s, c);
            DrawMiniCell(hx,   hy+s,   s, c);
            DrawMiniCell(hx,   hy+2*s, s, c);
            DrawMiniCell(hx+s, hy+2*s, s, c);
            break;

        case 2: // J
            DrawMiniCell(hx+s, hy,     s, c);
            DrawMiniCell(hx+s, hy+s,   s, c);
            DrawMiniCell(hx+s, hy+2*s, s, c);
            DrawMiniCell(hx,   hy+2*s, s, c);
            break;

        case 3: // S
            DrawMiniCell(hx+s,   hy,   s, c);
            DrawMiniCell(hx+2*s, hy,   s, c);
            DrawMiniCell(hx,     hy+s, s, c);
            DrawMiniCell(hx+s,   hy+s, s, c);
            break;

        case 4: // Z
            DrawMiniCell(hx,     hy,   s, c);
            DrawMiniCell(hx+s,   hy,   s, c);
            DrawMiniCell(hx+s,   hy+s, s, c);
            DrawMiniCell(hx+2*s, hy+s, s, c);
            break;

        case 5: // O
            DrawMiniCell(hx,   hy,   s, c);
            DrawMiniCell(hx+s, hy,   s, c);
            DrawMiniCell(hx,   hy+s, s, c);
            DrawMiniCell(hx+s, hy+s, s, c);
            break;

        case 6: // T
            DrawMiniCell(hx,     hy,   s, c);
            DrawMiniCell(hx+s,   hy,   s, c);
            DrawMiniCell(hx+2*s, hy,   s, c);
            DrawMiniCell(hx+s,   hy-s, s, c);
            break;
    }
}

static void DrawPredictedBlocks(void) {
    int s = 4;
    int base_x = NEXT_X_T + 4;
    int base_y = NEXT_Y_T + 4;

    for (int n = 0; n < PREDICT; n++) {
        int type = predict_block[n];
        if (type < 0) continue;

        int bx = base_x;
        int by = base_y + n * 15;

        uint16_t c;
        switch (type) {
            case 0: c = C_B1; break;
            case 1: c = C_B2; break;
            case 2: c = C_B3; break;
            case 3: c = C_B4; break;
            case 4: c = C_B5; break;
            case 5: c = C_B6; break;
            case 6: c = C_B7; break;
            default: continue;
        }

        switch (type) {
            case 0: by += 2; break;
            case 1: bx += 4; break;
            case 2: bx += 4; break;
            case 3: bx += 2; break;
            case 4: bx += 2; break;
            case 5: bx += 4; break;
            case 6: bx += 2; break;
        }

        switch (type) {
            case 0: // I
                DrawMiniCell(bx,     by, s, c);
                DrawMiniCell(bx+s,   by, s, c);
                DrawMiniCell(bx+2*s, by, s, c);
                DrawMiniCell(bx+3*s, by, s, c);
                break;

            case 1: // L
                DrawMiniCell(bx,   by,     s, c);
                DrawMiniCell(bx,   by+s,   s, c);
                DrawMiniCell(bx,   by+2*s, s, c);
                DrawMiniCell(bx+s, by+2*s, s, c);
                break;

            case 2: // J
                DrawMiniCell(bx+s, by,     s, c);
                DrawMiniCell(bx+s, by+s,   s, c);
                DrawMiniCell(bx+s, by+2*s, s, c);
                DrawMiniCell(bx,   by+2*s, s, c);
                break;

            case 3: // S
                DrawMiniCell(bx+s,   by,   s, c);
                DrawMiniCell(bx+2*s, by,   s, c);
                DrawMiniCell(bx,     by+s, s, c);
                DrawMiniCell(bx+s,   by+s, s, c);
                break;

            case 4: // Z
                DrawMiniCell(bx,     by,   s, c);
                DrawMiniCell(bx+s,   by,   s, c);
                DrawMiniCell(bx+s,   by+s, s, c);
                DrawMiniCell(bx+2*s, by+s, s, c);
                break;

            case 5: // O
                DrawMiniCell(bx,   by,   s, c);
                DrawMiniCell(bx+s, by,   s, c);
                DrawMiniCell(bx,   by+s, s, c);
                DrawMiniCell(bx+s, by+s, s, c);
                break;

            case 6: // T
                DrawMiniCell(bx,     by,   s, c);
                DrawMiniCell(bx+s,   by,   s, c);
                DrawMiniCell(bx+2*s, by,   s, c);
                DrawMiniCell(bx+s,   by+s, s, c);
                break;
        }
    }
}

void Grid_init(void) {
    for (int i = 0; i < GRID_W_T; i++) {
        for (int j = 0; j < GRID_H_T; j++) {
            if (i == 0 || i == GRID_W_T - 1 || j == 0 || j == GRID_H_T - 1) {
                grid[i][j] = 1;
            } else {
                grid[i][j] = 0;
            }
        }
    }
}

void Tetris_init(void) {
	TetrisAudio_Init(SR);
	Grid_init();
	game_over_sound = true;
	store_block = -1;
	hold_used = 0;
	int last_block = -1;
	int block;
	uint32_t random;
	for (int i = 0; i < PREDICT; i++){
		do{
			HAL_RNG_GenerateRandomNumber(&hrng, &random);
			block = random %7;
		} while(last_block == block);

		last_block = block;
		predict_block[i] = block;

	}
}

void Draw_Field(void) {
    LCD_DrawRect(FIELD_X_T - 2, FIELD_Y_T - 2, FIELD_W_T + 4, FIELD_H_T + 4, C_BD);
    LCD_DrawRect(FIELD_X_T, FIELD_Y_T, FIELD_W_T, FIELD_H_T, C_BG);

    for (int i = 1; i <= PLAY_W_T; i++) {
        for (int j = 1; j <= PLAY_H_T; j++) {
            DrawCell(i, j, BlockColourFromCell(grid[i][j]));
        }
    }
}

void B1_init(int state, int anch_x, int anch_y) {//I shape
	if (state == 1 || state == 3){//vertical
		grid[anch_x][anch_y] = 2;
		grid[anch_x][anch_y - 1] = 2;
		grid[anch_x][anch_y - 2] = 2;
		grid[anch_x][anch_y + 1] = 2;
	}
	else{//horizontal
		grid[anch_x][anch_y] = 2;
		grid[anch_x + 1][anch_y] = 2;
		grid[anch_x - 1][anch_y] = 2;
		grid[anch_x - 2][anch_y] = 2;
	}
}

void B2_init(int state, int anch_x, int anch_y) {//L shape
	if (state == 0){
		grid[anch_x][anch_y] = 2;
		grid[anch_x][anch_y - 1] = 2;
		grid[anch_x][anch_y + 1] = 2;
		grid[anch_x + 1][anch_y + 1] = 2;
	}
	else if (state == 1){//pointing down
		grid[anch_x][anch_y] = 2;
		grid[anch_x + 1][anch_y] = 2;
		grid[anch_x - 1][anch_y] = 2;
		grid[anch_x - 1][anch_y + 1] = 2;
	}
	else if (state == 2){//upside down
		grid[anch_x][anch_y] = 2;
		grid[anch_x][anch_y + 1] = 2;
		grid[anch_x][anch_y - 1] = 2;
		grid[anch_x - 1][anch_y - 1] = 2;
	}
	else{//pointing up
		grid[anch_x][anch_y] = 2;
		grid[anch_x - 1][anch_y] = 2;
		grid[anch_x + 1][anch_y] = 2;
		grid[anch_x + 1][anch_y - 1] = 2;
	}
}

void B3_init(int state, int anch_x, int anch_y) {//J shape
	if (state == 0){
		grid[anch_x][anch_y] = 2;
		grid[anch_x][anch_y - 1] = 2;
		grid[anch_x][anch_y + 1] = 2;
		grid[anch_x - 1][anch_y + 1] = 2;
	}
	else if (state == 1){//pointing up
		grid[anch_x][anch_y] = 2;
		grid[anch_x + 1][anch_y] = 2;
		grid[anch_x - 1][anch_y] = 2;
		grid[anch_x - 1][anch_y - 1] = 2;
	}
	else if (state == 2){//upside down
		grid[anch_x][anch_y] = 2;
		grid[anch_x][anch_y + 1] = 2;
		grid[anch_x][anch_y - 1] = 2;
		grid[anch_x + 1][anch_y - 1] = 2;
	}
	else{//pointing down
		grid[anch_x][anch_y] = 2;
		grid[anch_x - 1][anch_y] = 2;
		grid[anch_x + 1][anch_y] = 2;
		grid[anch_x + 1][anch_y + 1] = 2;
	}
}

void B4_init(int state, int anch_x, int anch_y) {//S shape
	if (state == 0 || state == 2){//horizontal
		grid[anch_x][anch_y] = 2;
		grid[anch_x - 1][anch_y] = 2;
		grid[anch_x][anch_y - 1] = 2;
		grid[anch_x + 1][anch_y - 1] = 2;
	}
	else{//vertical
		grid[anch_x][anch_y] = 2;
		grid[anch_x][anch_y + 1] = 2;
		grid[anch_x - 1][anch_y] = 2;
		grid[anch_x - 1][anch_y - 1] = 2;
	}
}

void B5_init(int state, int anch_x, int anch_y) {//Z shape
	if (state == 0 || state == 2){//horizontal
		grid[anch_x][anch_y] = 2;
		grid[anch_x + 1][anch_y] = 2;
		grid[anch_x - 1][anch_y - 1] = 2;
		grid[anch_x][anch_y - 1] = 2;
	}
	else{//vertical
		grid[anch_x][anch_y] = 2;
		grid[anch_x][anch_y + 1] = 2;
		grid[anch_x + 1][anch_y] = 2;
		grid[anch_x + 1][anch_y - 1] = 2;
	}
}

void B6_init(int state, int anch_x, int anch_y) {//square shape
	grid[anch_x][anch_y] = 2;
	grid[anch_x - 1][anch_y] = 2;
	grid[anch_x][anch_y - 1] = 2;
	grid[anch_x - 1][anch_y - 1] = 2;
}

void B7_init(int state, int anch_x, int anch_y) {//T shape
	if (state == 0){//pointing down
		grid[anch_x][anch_y] = 2;
		grid[anch_x - 1][anch_y] = 2;
		grid[anch_x + 1][anch_y] = 2;
		grid[anch_x][anch_y - 1] = 2;
	}
	else if (state == 1){//pointing left
		grid[anch_x][anch_y] = 2;
		grid[anch_x][anch_y + 1] = 2;
		grid[anch_x][anch_y - 1] = 2;
		grid[anch_x + 1][anch_y] = 2;
	}
	else if (state == 2){//pointing up
		grid[anch_x][anch_y] = 2;
		grid[anch_x - 1][anch_y] = 2;
		grid[anch_x + 1][anch_y] = 2;
		grid[anch_x][anch_y + 1] = 2;
	}
	else{//pointing right
		grid[anch_x][anch_y] = 2;
		grid[anch_x][anch_y - 1] = 2;
		grid[anch_x][anch_y + 1] = 2;
		grid[anch_x - 1][anch_y] = 2;
	}
}

int B_init(int state, int anch_x, int anch_y, int B) {
	static int last_block = -1;
		static int rand_block;
		if(B == -1){
			block = predict_block[0];
			for(int i = 0; i < PREDICT - 1; i++){
				predict_block[i] = predict_block[i+1];
			}
			uint32_t random;
			do{
				HAL_RNG_GenerateRandomNumber(&hrng, &random);
				rand_block = random %7;
			} while(last_block == rand_block);
			last_block = rand_block;
			predict_block[PREDICT - 1] = rand_block;
		}
	else {
		block = B;
	}
	switch (block) {
	case 0:
		 B1_init(state, anch_x, anch_y); break;
	case 1:
		 B2_init(state, anch_x, anch_y); break;
	case 2:
		 B3_init(state, anch_x, anch_y); break;
	case 3:
		 B4_init(state, anch_x, anch_y); break;
	case 4:
		 B5_init(state, anch_x, anch_y); break;
	case 5:
		 B6_init(state, anch_x, anch_y); break;
	case 6:
		 B7_init(state, anch_x, anch_y); break;
	}
	return block;
}

static int cell_ok(int x,int y) {
    if(x<0||x>=GRID_W_T||y<0||y>=GRID_H_T) return 0;
    if(grid[x][y]==1) return 0;
    if(grid[x][y]>2) return 0;
    return 1;
}

static int can_place_B1(int state,int ax,int ay) {
    if(state==1 || state==3){ // vertical
        return cell_ok(ax,ay)
            && cell_ok(ax,ay-1)
            && cell_ok(ax,ay-2)
            && cell_ok(ax,ay+1);
    }
    else { // horizontal
        return cell_ok(ax,ay)
            && cell_ok(ax+1,ay)
            && cell_ok(ax-1,ay)
            && cell_ok(ax-2,ay);
    }
}

static int can_place_B2(int state,int ax,int ay) {
    if(state==0){
        return cell_ok(ax,ay)
            && cell_ok(ax,ay-1)
            && cell_ok(ax,ay+1)
            && cell_ok(ax+1,ay+1);
    }
    else if(state==1){
        return cell_ok(ax,ay)
            && cell_ok(ax+1,ay)
            && cell_ok(ax-1,ay)
            && cell_ok(ax-1,ay+1);
    }
    else if(state==2){
        return cell_ok(ax,ay)
            && cell_ok(ax,ay+1)
            && cell_ok(ax,ay-1)
            && cell_ok(ax-1,ay-1);
    }
    else{
        return cell_ok(ax,ay)
            && cell_ok(ax-1,ay)
            && cell_ok(ax+1,ay)
            && cell_ok(ax+1,ay-1);
    }
}

static int can_place_B3(int state,int ax,int ay) {
    if(state==0){
        return cell_ok(ax,ay)
            && cell_ok(ax,ay-1)
            && cell_ok(ax,ay+1)
            && cell_ok(ax-1,ay+1);
    }
    else if(state==1){
        return cell_ok(ax,ay)
            && cell_ok(ax+1,ay)
            && cell_ok(ax-1,ay)
            && cell_ok(ax-1,ay-1);
    }
    else if(state==2){
        return cell_ok(ax,ay)
            && cell_ok(ax,ay+1)
            && cell_ok(ax,ay-1)
            && cell_ok(ax+1,ay-1);
    }
    else{
        return cell_ok(ax,ay)
            && cell_ok(ax-1,ay)
            && cell_ok(ax+1,ay)
            && cell_ok(ax+1,ay+1);
    }
}

static int can_place_B4(int state,int ax,int ay) {
    if(state==0 || state==2){
        return cell_ok(ax,ay)
            && cell_ok(ax-1,ay)
            && cell_ok(ax,ay-1)
            && cell_ok(ax+1,ay-1);
    }
    else{
        return cell_ok(ax,ay)
            && cell_ok(ax,ay+1)
            && cell_ok(ax-1,ay)
            && cell_ok(ax-1,ay-1);
    }
}

static int can_place_B5(int state,int ax,int ay) {
    if(state==0 || state==2){
        return cell_ok(ax,ay)
            && cell_ok(ax+1,ay)
            && cell_ok(ax-1,ay-1)
            && cell_ok(ax,ay-1);
    }
    else{
        return cell_ok(ax,ay)
            && cell_ok(ax,ay+1)
            && cell_ok(ax+1,ay)
            && cell_ok(ax+1,ay-1);
    }
}

static int can_place_B6(int state,int ax,int ay) {
    return cell_ok(ax,ay)
        && cell_ok(ax-1,ay)
        && cell_ok(ax,ay-1)
        && cell_ok(ax-1,ay-1);
}

static int can_place_B7(int state,int ax,int ay) {
    if(state==0){
        return cell_ok(ax,ay)
            && cell_ok(ax-1,ay)
            && cell_ok(ax+1,ay)
            && cell_ok(ax,ay-1);
    }
    else if(state==1){
        return cell_ok(ax,ay)
            && cell_ok(ax,ay+1)
            && cell_ok(ax,ay-1)
            && cell_ok(ax+1,ay);
    }
    else if(state==2){
        return cell_ok(ax,ay)
            && cell_ok(ax-1,ay)
            && cell_ok(ax+1,ay)
            && cell_ok(ax,ay+1);
    }
    else{
        return cell_ok(ax,ay)
            && cell_ok(ax,ay-1)
            && cell_ok(ax,ay+1)
            && cell_ok(ax-1,ay);
    }
}

int can_move_down(void) {
	for(int j = GRID_H_T - 2; j >= 0; j--){
		 for(int i = 0; i < GRID_W_T; i++){
			 if (grid[i][j] == 2){
				 if(grid[i][j + 1] > 2 || grid[i][j + 1] == 1){
					 return 0;
				 }
			 }
		 }
	 }
	return 1;
}

int can_move_right(void) {
	for(int j = GRID_H_T - 2; j >= 0; j--){
			 for(int i = 0; i < GRID_W_T; i++){
				 if (grid[i][j] == 2){
					 if(grid[i + 1][j] > 2 || grid[i + 1][j] == 1){
						 return 0;
					 }
				 }
			 }
		 }
	return 1;
}

int can_move_left(void) {
	for(int j = GRID_H_T - 2; j >= 0; j--){
			 for(int i = 0; i < GRID_W_T; i++){
				 if (grid[i][j] == 2){
					 if(grid[i - 1][j] > 2 || grid[i - 1][j] == 1){
						 return 0;
					 }
				 }
			 }
		 }
	return 1;
}

void erase_B(void) {
	for(int j = 0; j < GRID_H_T; j++){
		for(int i = 0; i < GRID_W_T; i++){
			if(grid[i][j] == 2){
				grid[i][j] = 0;
			}
		}
	}
}

void place(void) {
	for(int j = 0; j < GRID_H_T; j++){
		for(int i = 0; i < GRID_W_T; i++){
			if(grid[i][j] == 2){
				switch(cur_type){
				case 0:
					grid[i][j] = 3;
					break;
				case 1:
					grid[i][j] = 4;
					break;
				case 2:
					grid[i][j] = 5;
					break;
				case 3:
					grid[i][j] = 6;
					break;
				case 4:
					grid[i][j] = 7;
					break;
				case 5:
					grid[i][j] = 8;
					break;
				case 6:
					grid[i][j] = 9;
					break;
				}
			}
		}
	}
}


void hold(void) {
    if (!has_piece || hold_used) {
        return;
    }

    erase_B();

    if (store_block == -1) {
        store_block = cur_type;
        has_piece = 0;
        cur_state = 0;
    } else {
        int temp = store_block;
        store_block = cur_type;

        cur_type = temp;
        cur_state = 0;
        cur_x = SPAWN_X;
        cur_y = SPAWN_Y;

        if (grid[cur_x][cur_y] >= 3) {
            game_over = 1;
            return;
        }

        B_init(cur_state, cur_x, cur_y, cur_type);
        has_piece = 1;
    }

    hold_used = 1;
}

void rotate(void) {
	 int new_state = (cur_state + 1) % 4;

	    erase_B();

	    int ok = 0;

	    switch(cur_type){
	    case 0: ok = can_place_B1(new_state,cur_x,cur_y); break;
	    case 1: ok = can_place_B2(new_state,cur_x,cur_y); break;
	    case 2: ok = can_place_B3(new_state,cur_x,cur_y); break;
	    case 3: ok = can_place_B4(new_state,cur_x,cur_y); break;
	    case 4: ok = can_place_B5(new_state,cur_x,cur_y); break;
	    case 5: ok = can_place_B6(new_state,cur_x,cur_y); break;
	    case 6: ok = can_place_B7(new_state,cur_x,cur_y); break;
	    }

	    if(ok){
	        cur_state = new_state;
	    }

	    B_init(cur_state,cur_x,cur_y,cur_type);
}

int check_line(int j) {
	int gap = 0;
	for(int i = 1; i < GRID_W_T - 1; i++){
		if (grid[i][j] < 3){
			gap++;
			break;
		}
	}
	return gap;
}

void move_line(int k) {
	for(int j = k; j > 0; j--){
		for(int i = 1; i < GRID_W_T - 1; i++){
			if (j == 1){
				grid[i][j] = 0;
			}
			else{
				grid[i][j] = grid[i][j - 1];
			}
		}
	}
}

void clear_line(void) {
	for(int j = GRID_H_T - 2; j > 0; j--){
		if(check_line(j) == 0){
			int counter = 0;
			do{
			for(int i = 1; i < GRID_W_T - 1; i++){
				if (grid[i][j] >= 3){
					grid[i][j] = 0;
					counter++;
				}
			}

			switch(counter){
				case 1:
					scale = 1;
					break;
				case 2:
					scale = 1.3;
					break;
				case 3:
					scale = 1.6;
					break;
				case 4:
					scale = 2;
					break;
				default:
					scale = 1;
					break;
				}

				move_line(j);
				score = score + (counter) * 10 * scale;
			} while(check_line(j) == 0);
		}
	}
}

static void move_down(void) {
    cur_y++;
    erase_B();
    B_init(cur_state, cur_x, cur_y, cur_type);
}

static void move_right(void) {
	cur_x++;
	erase_B();
	B_init(cur_state, cur_x, cur_y, cur_type);
}

static void move_left(void) {
	cur_x--;
	erase_B();
	B_init(cur_state, cur_x, cur_y, cur_type);
}

void B_tick(void){

	if (game_over == 1) {
		return;
	}

	if(once){
		Tetris_init();
		Audio_SetMode(AUDIO_MODE_TETRIS);
		s_lastTickMs = HAL_GetTick();
		once = false;
	}

	if(game_over) {
		return;
	}

	if (!has_piece) {
		has_piece = 1;
		cur_state = 0;
		cur_x = SPAWN_X;
		cur_y = SPAWN_Y;
		hold_used = 0;

		if (grid[cur_x][cur_y] >= 3) {
			game_over = 1;
		} else {
			cur_type = B_init(cur_state, cur_x, cur_y, -1);
		}

		return;
	 }

	 if(can_move_down()){
		 move_down();
	 }
	 else{
		 has_piece = 0;
		 place();
		 clear_line();
	 }
}

static void DrawGameOverPanel(void) {

	LCD_DrawText(10, 70, "GAME OVER", C_BD, C_BG, 2);
    LCD_DrawText(20, 90, "SCORE", C_BD, C_BG, 1);

    char score_buf[10];
    itoa(score, score_buf, 10);
    LCD_DrawText(70, 90, score_buf, C_B4, C_BG, 1);
}

//static void DrawGameOverPanel(void) {
//    char score_buf[10];
//
//    DrawDoublePanelFrame(18, 62, 92, 36);
//
//    LCD_DrawText(28, 68, "GAME OVER", C_BD, C_BG, 1);
//    LCD_DrawText(32, 82, "SCORE", C_BD, C_BG, 1);
//
//    itoa(score, score_buf, 10);
//    LCD_DrawText(68, 82, score_buf, C_B4, C_BG, 1);
//}

static void DrawScorePanel(void) {
    char buf[10];

    DrawDoublePanelFrame(SCORE_X_T, SCORE_Y_T, 70, 14);

    LCD_DrawText(SCORE_X_T + 5, SCORE_Y_T + 4, "SCORE", 0x632c, C_BG, 1);
    LCD_DrawText(SCORE_X_T + 4, SCORE_Y_T + 3, "SCORE", C_BD, C_BG, 1);


    itoa(score, buf, 10);
    LCD_DrawText(SCORE_X_T + 40, SCORE_Y_T + 3, buf, C_B4, C_BG, 1);
}

void Tetris_Update(uint16_t pressed, uint16_t down, uint16_t held_ev) {
    static uint32_t left_repeat_ms = 0;
    static uint32_t right_repeat_ms = 0;
    static bool down_was_held = false;

    uint32_t now = HAL_GetTick();
    bool down_held = (down & (1u << BTN_DOWN)) != 0u;

    if (held_ev & (1u << BTN_A)) {
    	Menu_Invalidate();
        g_state = STATE_MENU;
        game_over = 0;
        once = true;
        score = 0;
        store_block = -1;
        hold_used = 0;
        left_repeat_ms = 0;
        right_repeat_ms = 0;
        down_was_held = false;

        return;
    }

    if (game_over) {
    	TetrisAudio_Stop();
    	Audio_SetMode(AUDIO_MODE_SFX);

    	if (game_over_sound) {
        	TetrisSFX_GameOver();
        	game_over_sound = false;
    	}

        DrawGameOverPanel();
        return;
    }

    if (!has_piece) {
        B_tick();
        DrawScorePanel();
        DrawFuturePanels();
        DrawPredictedBlocks();
        DrawStoredBlock();
        Draw_Field();
        return;
    }

    if (pressed & (1u << BTN_LEFT)) {
        if (can_move_left()) {
            move_left();
        }
        left_repeat_ms = now + 200u;
    } else if ((down & (1u << BTN_LEFT)) && now >= left_repeat_ms) {
        if (can_move_left()) {
            move_left();
        }
        left_repeat_ms = now + 70u;
    } else if ((down & (1u << BTN_LEFT)) == 0u) {
        left_repeat_ms = 0;
    }

    if (pressed & (1u << BTN_RIGHT)) {
        if (can_move_right()) {
            move_right();
        }
        right_repeat_ms = now + 200u;
    } else if ((down & (1u << BTN_RIGHT)) && now >= right_repeat_ms) {
        if (can_move_right()) {
            move_right();
        }
        right_repeat_ms = now + 70u;
    } else if ((down & (1u << BTN_RIGHT)) == 0u) {
        right_repeat_ms = 0;
    }

    if (pressed & (1u << BTN_UP)) {
        hold();
    }

    if (pressed & (1u << BTN_A)) {
        rotate();
    }

    if (down_held != down_was_held) {
        s_lastTickMs = now;
        down_was_held = down_held;
    }

    {
        uint32_t step_ms = down_held ? 75u : 350u;

        while ((uint32_t)(now - s_lastTickMs) >= step_ms) {
            s_lastTickMs += step_ms;
            B_tick();
        }
    }

    DrawScorePanel();
    DrawFuturePanels();
    DrawStoredBlock();
    DrawPredictedBlocks();
    Draw_Field();
//    DrawTetrisIcon();
}

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
#include <stdlib.h>
#include "audio.h"
#include "tetris_audio.h"

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

static inline int GX(int gx){ return FIELD_X_T + gx*CELL_T; }
static inline int GY(int gy){ return FIELD_Y_T + gy*CELL_T; }

static int cur_x, cur_y;
static int cur_state;
static int cur_type;
static int has_piece;
static int game_over;
static int score;

static uint16_t FallingColour(void){
    switch(cur_type){
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

static void DrawCell(int gx,int gy,uint16_t c)
{
    LCD_DrawRect(GX(gx),GY(gy),CELL_T,CELL_T,c);
}

void Grid_init(void){
	for(int i = 0; i < GRID_W_T; i++){//row increment
		if (i == 0 || i == GRID_W_T - 1){//left and right borders
			for(int j = 0; j < GRID_H_T; j++){// column increment
				grid[i][j] = 1;
			}
		}
		else{
			for(int j = 0; j < GRID_H_T; j++){
				if(j == 0 || j == GRID_H_T -1){
				  grid[i][j] = 1;
				}
				else{
				  grid[i][j] = 0;
				}
			}
		}
	}
}


void Tetris_init(void){
	Grid_init();
	TetrisAudio_Init(SR);
}

void Draw_Field(void){
	for(int i = 0; i < GRID_W_T; i++){
		for(int j = 0; j < GRID_H_T; j++){
			switch(grid[i][j]){
			case 0:
				DrawCell(i, j, C_BG);
				break;
			case 1:
				DrawCell(i, j, C_BD);
				break;
			case 2:
				DrawCell(i, j, FallingColour());
				break;
			case 3:
				DrawCell(i, j, C_B1);
				break;
			case 4:
				DrawCell(i, j, C_B2);
				break;
			case 5:
				DrawCell(i, j, C_B3);
				break;
			case 6:
				DrawCell(i, j, C_B4);
				break;
			case 7:
				DrawCell(i, j, C_B5);
				break;
			case 8:
				DrawCell(i, j, C_B6);
				break;
			case 9:
				DrawCell(i, j, C_B7);
				break;
			}
		}
	}
}
void B1_init(int state, int anch_x, int anch_y){//I shape
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

void B2_init(int state, int anch_x, int anch_y){//L shape
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

void B3_init(int state, int anch_x, int anch_y){//J shape
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

void B4_init(int state, int anch_x, int anch_y){//S shape
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

void B5_init(int state, int anch_x, int anch_y){//Z shape
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

void B6_init(int state, int anch_x, int anch_y){//square shape
	grid[anch_x][anch_y] = 2;
	grid[anch_x - 1][anch_y] = 2;
	grid[anch_x][anch_y - 1] = 2;
	grid[anch_x - 1][anch_y - 1] = 2;
}

void B7_init(int state, int anch_x, int anch_y){//T shape
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

int B_init(int state, int anch_x, int anch_y, int B){
	int block;

	if(B == -1){
		block = rand()%7;
	}
	else{
		block = B;
	}

	switch(block){
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

static int cell_ok(int x,int y){
    if(x<0||x>=GRID_W_T||y<0||y>=GRID_H_T) return 0;
    if(grid[x][y]==1) return 0;
    if(grid[x][y]>2) return 0;
    return 1;
}

static int can_place_B1(int state,int ax,int ay)
{
    if(state==1 || state==3){ // vertical
        return cell_ok(ax,ay)
            && cell_ok(ax,ay-1)
            && cell_ok(ax,ay-2)
            && cell_ok(ax,ay+1);
    }
    else{ // horizontal
        return cell_ok(ax,ay)
            && cell_ok(ax+1,ay)
            && cell_ok(ax-1,ay)
            && cell_ok(ax-2,ay);
    }
}

static int can_place_B2(int state,int ax,int ay)
{
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

static int can_place_B3(int state,int ax,int ay)
{
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

static int can_place_B4(int state,int ax,int ay)
{
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

static int can_place_B5(int state,int ax,int ay)
{
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

static int can_place_B6(int state,int ax,int ay)
{
    return cell_ok(ax,ay)
        && cell_ok(ax-1,ay)
        && cell_ok(ax,ay-1)
        && cell_ok(ax-1,ay-1);
}

static int can_place_B7(int state,int ax,int ay)
{
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

int can_move_down(void){
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

int can_move_right(void){
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

int can_move_left(void){
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

void erase_B(void){
	for(int j = 0; j < GRID_H_T; j++){
		for(int i = 0; i < GRID_W_T; i++){
			if(grid[i][j] == 2){
				grid[i][j] = 0;
			}
		}
	}
}

void place(void){
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

void rotate(void){
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

int check_line(int j){
	int gap = 0;
	for(int i = 1; i < GRID_W_T - 1; i++){
		if (grid[i][j] < 3){
			gap++;
			break;
		}
	}
	return gap;
}

void move_line(int k){
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

void clear_line(void){
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
				move_line(j);
				score = score + (counter + 1) * 10;
			} while(check_line(j) == 0);
		}
	}
}

static void move_down(void)
{
    cur_y++;
    erase_B();
    B_init(cur_state, cur_x, cur_y, cur_type);
}

static void move_right(void){
	cur_x++;
	erase_B();
	B_init(cur_state, cur_x, cur_y, cur_type);
}

static void move_left(void){
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
		TetrisAudio_Start();
		s_lastTickMs = HAL_GetTick();
		once = false;
	}

	if(game_over){
		return;
	}

	if (!has_piece){
		has_piece = 1;
		cur_state = 0;
		cur_x = SPAWN_X;
		cur_y = SPAWN_Y;

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

void Tetris_Update(uint16_t pressed, uint16_t held){

	if (held & (1u << BTN_A)) {
		TetrisAudio_Stop();
		g_state = STATE_MENU;
		game_over = 0;
		once = true;
		score = 0;
		return;
	}

	if (game_over == 1) {
		LCD_DrawText(10, 70, "GAME OVER", C_BD, C_BG, 2);
		return;
	}

	if(!has_piece){
		B_tick();
		Draw_Field();
		return;
	}

	 if (pressed & (1u << BTN_LEFT)){
		 if(can_move_left() == 1){
			move_left();
		 }
	 }

	 if (pressed & (1u << BTN_RIGHT)){
		 if(can_move_right() == 1){
			move_right();
		 }
	 }

	 if (pressed & (1u << BTN_A)){
		 rotate();
	 }

	 LCD_DrawText(4,4,"SCORE",C_BD,C_BG,1);
	 char buf[10];
	 itoa(score, buf, 10);
	 LCD_DrawText(60,4,buf,C_BD,C_BG,1);

	 uint32_t now = HAL_GetTick();


	 if (held & (1u << BTN_DOWN)) {
		while ((uint32_t)(now - s_lastTickMs) >= 50){
			s_lastTickMs += 50;
			B_tick();
		}
	  } else {
		while ((uint32_t)(now - s_lastTickMs) >= 250)
		{
			s_lastTickMs += 250;
			B_tick();
		}
	  }

	 Draw_Field();
}

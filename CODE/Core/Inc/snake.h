/*
 * snake.h
 *
 *  Created on: 6 Feb 2026
 *      Author: motre
 */

#ifndef INC_SNAKE_H_
#define INC_SNAKE_H_

#include <stdint.h>
#include "globals.h"

#define UI_TOP 20
#define BORDER 2
#define CELL 4

#define FIELD_X BORDER
#define FIELD_Y (UI_TOP + BORDER)

#define FIELD_W (FB_WIDTH - BORDER * 2)
#define FIELD_H (FB_HEIGHT - UI_TOP - BORDER * 2)

#define GRID_W FIELD_W / CELL
#define GRID_H FIELD_H / CELL

// colors

#define C_BG     0x0000
#define C_SNAKE  0x07E0
#define C_HEAD   0x00FF
#define C_FOOD   0xF800
#define C_TEXT   0xFFFF

// SNAKE

typedef struct { uint8_t x,y; } Pt;

#define SNAKE_MAX (GRID_W*GRID_H)

static Pt snake[SNAKE_MAX];
static int snake_len;
static int head_i;

typedef enum {DIR_UP,DIR_DOWN,DIR_LEFT,DIR_RIGHT} Dir;
static Dir dir;

static Pt food;
static int alive;
static int score;

// FUNCTIONS

void Snake_Init(void);
void Snake_Tick(void);
void Snake_Draw(void);

// HELPERS


#endif /* INC_SNAKE_H_ */

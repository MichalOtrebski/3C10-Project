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
#define BORDER_S 2
#define CELL 8

/*
 * Play area available below the UI.
 */
#define PLAY_W (FB_WIDTH - BORDER_S * 2)
#define PLAY_H (FB_HEIGHT - UI_TOP - BORDER_S * 2)

/*
 * Grid size in cells.
 */
#define GRID_W (PLAY_W / CELL)
#define GRID_H (PLAY_H / CELL)

/*
 * Actual pixel size of the grid.
 */
#define FIELD_W (GRID_W * CELL)
#define FIELD_H (GRID_H * CELL)

/*
 * Center the grid in the available play area.
 */
#define FIELD_X_S ((FB_WIDTH - FIELD_W) / 2)
#define FIELD_Y_S (UI_TOP + BORDER_S + ((PLAY_H - FIELD_H) / 2))

// colors
#define C_BG     0x0000
#define C_SNAKE  0x07E0
#define C_HEAD   0x47F9
#define C_FOOD   0xF800
#define C_TEXT   0xFFFF

// SNAKE
typedef struct { uint8_t x,y; } Pt;

#define SNAKE_MAX (GRID_W * GRID_H)



typedef enum {DIR_UP,DIR_DOWN,DIR_LEFT,DIR_RIGHT} Dir;


// FUNCTIONS
void Snake_Init(void);
void Snake_Tick(void);
void Snake_Draw(void);
void Snake_Update(uint16_t pressed, uint16_t held);

#endif /* INC_SNAKE_H_ */

#ifndef INC_BREAKOUT_H_
#define INC_BREAKOUT_H_

#include <stdint.h>
#include <stdbool.h>
#include "globals.h"

// UI DIMENSIONS AND COLOURS
#define C_TEXT   0xFFFF
#define C_SCORE  0x07E0
#define C_BG     0x0000
#define UI_TOP 20
#define BORDER 2
#define BALL_WDH 4
#define BALL_HGT 4
#define BLK_WDH 16
#define BLK_HGT 8
#define BLK_XQ 7
#define BLK_YQ 5
#define PLT_WDH 32
#define PLT_HGT 2

#define FIELD_X BORDER
#define FIELD_Y (UI_TOP + BORDER)
#define FIELD_W (FB_WIDTH - (BORDER * 2))
#define FIELD_H (FB_HEIGHT - UI_TOP - (BORDER * 2))

//INTIAL POSITION
#define FST_BLK_SPX 5
#define FST_BLK_SPY (FIELD_Y + (FIELD_H / 8))
#define BALL_SPX 62 //(FIELD_X + (FIELD_W / 2))
#define BALL_SPY 146 //(FIELD_Y + FIELD_H - (3 * PLT_HGT))
#define PLT_SPX (FIELD_X + (FIELD_W / 2) + BALL_WDH)
#define PLT_SPY (FIELD_Y + FIELD_H - (3 * PLT_HGT) - BALL_HGT)

//INTIAL Velocity
#define BALL_VEL 32
#define BALL_INT 0.1f

// BLOCK colors
#define BLK_RED 0xF800
#define BLK_YLW 0xFFE0
#define BLK_GRN 0x07E0
#define BLK_BLU 0x001F
#define BLK_PRP 0xA154

// PLATFORM COLOUR
#define PLT_WHT 0xFFFF

static float ball_velx;
static float ball_vely;

//SCORE

//POWER UPS
static uint16_t pos_pow[2];

//BALL
static float pos_ball[2];

//PLATFORM
static uint16_t pos_plat[2];

// FUNCTIONS
void block_intl(void);
void block_draw(void);
void plat_intl(void);
void ball_intl(void);
void initialise(void);
void release_ball(uint16_t pressed);
void bouncex(void);
void bouncexplat(float position);
void bouncey(void);
void does_collide_blk(void);
void does_collide_wall(void);
void does_collide_plat(void);
void ball_update(void);
void platform_update(uint16_t pressed);
void break_draw(void);
void break_update(uint16_t pressed, uint16_t held, uint16_t held_ev);
#endif /* INC_BREAKOUT_H_ */

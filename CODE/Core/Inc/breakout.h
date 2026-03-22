#ifndef INC_BREAKOUT_H_
#define INC_BREAKOUT_H_

#include <stdint.h>
#include <stdbool.h>
#include "globals.h"

/* -------------------- UI DIMENSIONS AND COLOURS -------------------- */
#define C_TEXT   0xFFFF
#define C_SCORE  0x07E0
#define C_BG     0x0000

#define UI_TOP   20
#define BORDER   2

#define BALL_WDH 4
#define BALL_HGT 4

#define BLK_WDH  16
#define BLK_HGT  8
#define BLK_XQ   7
#define BLK_YQ   5

#define PLT_WDH  32
#define PLT_HGT  2

#define FIELD_X           BORDER
#define FIELD_Y           (UI_TOP + BORDER)
#define FIELD_W_BREAKOUT  (FB_WIDTH - (BORDER * 2))
#define FIELD_H_BREAKOUT  (FB_HEIGHT - UI_TOP - (BORDER * 2))

/* -------------------- INITIAL POSITIONS -------------------- */
#define FST_BLK_SPX (FIELD_X + BLK_WDH)
#define FST_BLK_SPY (FIELD_Y + (FIELD_H_BREAKOUT / 8))

#define BALL_SPX    62
#define BALL_SPY    146

#define PLT_SPX     (FIELD_X + (FIELD_W_BREAKOUT / 2))
#define PLT_SPY     (FIELD_Y + FIELD_H_BREAKOUT - (3 * PLT_HGT) - BALL_HGT)

/* -------------------- INITIAL VELOCITY -------------------- */
#define BALL_VEL 24.0f
#define BALL_INT 0.05f

/* -------------------- BLOCK COLOURS -------------------- */
#define BLK_RED 0xF800
#define BLK_YLW 0xFFE0
#define BLK_GRN 0x07E0
#define BLK_BLU 0x001F
#define BLK_PRP 0xA154

/* -------------------- PLATFORM COLOUR -------------------- */
#define PLT_WHT 0xFFFF

/* -------------------- GAME STATE VARIABLES -------------------- */
/* Defined in breakout.c */
extern float ball_velx;
extern float ball_vely;

/* POWER UPS */
extern uint16_t pos_pow[2];

/* BALL */
extern float pos_ball[2];

/* PLATFORM */
extern uint16_t pos_plat[2];

/* BOUNCE / BLOCK COLLISION VALUES */
extern uint16_t line[2][142];
extern bool coll_blk;
extern int16_t coll_timer;
extern int16_t coll_i;
extern int16_t coll_j;
extern bool bounce_dir;   /* 0 = x, 1 = y */

/* -------------------- FUNCTIONS -------------------- */
void block_intl(void);
void block_draw(void);
void plat_intl(void);
void ball_intl(void);
void initialise(void);

void release_ball(uint16_t pressed);

void bouncex(void);
void bouncexplat(float position);
void bouncey(void);

void future_pos(void);
void does_collide_blk(void);
void does_collide_wall(void);
void does_collide_plat(void);

void ball_update(void);
void platform_update(uint16_t pressed);

void break_Tick(void);
void break_draw(void);
void Breakout_Update(uint16_t pressed, uint16_t held, uint16_t held_ev);

#endif /* INC_BREAKOUT_H_ */

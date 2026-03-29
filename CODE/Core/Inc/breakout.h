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

/* -------------------- BALL -------------------- */
#define BALL_WDH 4
#define BALL_HGT 4

/* -------------------- BLOCKS -------------------- */
#define BLK_WDH  16
#define BLK_HGT  8
#define BLK_XQ   7
#define BLK_YQ   5

/* -------------------- PLATFORM -------------------- */
#define PLT_WDH  32
#define PLT_HGT  2

/* -------------------- FIELD -------------------- */
#define FIELD_X           BORDER
#define FIELD_Y           (UI_TOP + BORDER)
#define FIELD_W_BREAKOUT  (FB_WIDTH - (BORDER * 2))
#define FIELD_H_BREAKOUT  (FB_HEIGHT - UI_TOP - (BORDER * 2))

/* -------------------- BLOCK POSITIONING -------------------- */
/* Center blocks nicely instead of offsetting */
#define BLOCKS_TOTAL_W   (BLK_XQ * BLK_WDH)
#define FST_BLK_SPX      (FIELD_X + ((FIELD_W_BREAKOUT - BLOCKS_TOTAL_W) / 2))
#define FST_BLK_SPY      (FIELD_Y + (FIELD_H_BREAKOUT / 8))

/* -------------------- INITIAL POSITIONS -------------------- */
#define BALL_SPX    62
#define BALL_SPY    146

#define PLT_SPX     (FIELD_X + (FIELD_W_BREAKOUT / 2))
#define PLT_SPY     (FIELD_Y + FIELD_H_BREAKOUT - (3 * PLT_HGT) - BALL_HGT)

/* -------------------- BALL PHYSICS -------------------- */
#define BALL_VEL 24.0f
#define BALL_INT 0.05f

/* -------------------- BLOCK COLOURS -------------------- */
#define BLK_RED 0xF800
#define BLK_YLW 0xFFE0
#define BLK_GRN 0x07E0
#define BLK_BLU 0x001F
#define BLK_PRP 0xA154

/* -------------------- PLATFORM / BALL COLOUR -------------------- */
#define PLT_WHT 0xFFFF

/* -------------------- GAME STATE VARIABLES -------------------- */
/* Defined in breakout.c */
extern float ball_velx;
extern float ball_vely;

/* BALL */
extern float pos_ball[2];

/* PLATFORM */
extern uint16_t pos_plat[2];

/* -------------------- FUNCTIONS -------------------- */

/* Initialisation */
void block_intl(void);
void block_draw(void);
void plat_intl(void);
void ball_intl(void);
void initialise(void);

/* Input / Launch */
void release_ball(uint16_t pressed);

/* Physics / Bounce */
void bouncex(void);
void bouncexplat(float position);
void bouncey(void);

/* Collision */
void does_collide_blk(void);
void does_collide_wall(void);
void does_collide_plat(void);

/* Updates */
void ball_update(void);
void platform_update(uint16_t held);

/* Game loop */
void break_Tick(void);
void break_draw(void);
void Breakout_Update(uint16_t pressed, uint16_t held, uint16_t held_ev);

#endif /* INC_BREAKOUT_H_ */

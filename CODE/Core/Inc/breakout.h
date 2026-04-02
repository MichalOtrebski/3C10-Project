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
#define BORDER_H   4

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
#define FIELD_X           BORDER_H
#define FIELD_Y           (UI_TOP + BORDER_H)
#define FIELD_W_BREAKOUT  (FB_WIDTH - (BORDER_H * 2))
#define FIELD_H_BREAKOUT  (FB_HEIGHT - UI_TOP - (BORDER_H * 2))

/* -------------------- BLOCK POSITIONING -------------------- */
#define BLOCKS_TOTAL_W   (BLK_XQ * BLK_WDH)
#define FST_BLK_SPX      (FIELD_X + ((FIELD_W_BREAKOUT - BLOCKS_TOTAL_W) / 2))
#define FST_BLK_SPY      (FIELD_Y + (FIELD_H_BREAKOUT / 8))

/* -------------------- INITIAL POSITIONS -------------------- */
#define BALL_SPX    62
#define BALL_SPY    146

#define PLT_SPX     (FIELD_X + (FIELD_W_BREAKOUT / 2))
#define PLT_SPY     (FIELD_Y + FIELD_H_BREAKOUT - (3 * PLT_HGT) - BALL_HGT)

/* -------------------- BALL PHYSICS -------------------- */
#define BALL_VEL         120.0f
#define BALL_MIN_VX      30.0f
#define GAME_TICK_MS     16

/* -------------------- POWER-UP DURATIONS -------------------- */
#define POWER_SCORE_MS   8000u
#define POWER_WIDE_MS    10000u
#define POWER_SLOW_MS    8000u

/* -------------------- POWER-UP TUNING -------------------- */
#define POWER_WIDE_EXTRA_WDH  12
#define POWER_SLOW_FACTOR     0.78f

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

/* PLATFORM
 * The improved .c tracks the platform internally as float for smoother motion,
 * but keeps this exported integer position for compatibility with the rest
 * of the project.
 */
extern uint16_t pos_plat[2];

/* SCORE */

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
void bouncex(uint8_t typ);
void bouncexplat(float position);
void bouncey(uint8_t typ);

/* Collision */
void does_collide_blk(void);
void does_collide_wall(void);
void does_collide_plat(void);

/* Updates */
void ball_update(float dt);
void platform_update(uint16_t held, float dt);

/* Game loop */
void break_Tick(void);
void break_draw(void);
void Breakout_Update(uint16_t pressed, uint16_t held, uint16_t held_ev);

#endif /* INC_BREAKOUT_H_ */

#include "breakout.h"
#include <stdlib.h>
#include "display.h"
#include "globals.h"
#include "text.h"
#include <stdbool.h>
#include "buttons.h"
#include <math.h>

static bool bloc_exst [BLK_YQ][BLK_XQ];
static bool bloc_pow1 [BLK_YQ][BLK_XQ];
static bool bloc_pow2 [BLK_YQ][BLK_XQ];
static bool bloc_pow3 [BLK_YQ][BLK_XQ];
static bool ball_in = true;
static bool game_ran = true;
static bool ball_released = false;
static uint32_t s_lastTickMs;

uint16_t score;

// intialising game play
void block_intl(void) { //sets all the block exists to true interatively
	for (int i = 0; i < BLK_XQ; i++) {
		for (int j = 0; j < BLK_YQ; j++) bloc_exst [j][i] = 1;
	}
}

void block_draw(void) { //draws all the blocks that currently exists
	for (int i = 0; i < BLK_XQ; i++) {
		for (int j = 0; j < BLK_YQ; j++) {
			if (bloc_exst [j][i] == 1) {
				switch (j) {
					case 0:
					LCD_DrawRect(FST_BLK_SPX + i * BLK_WDH, FST_BLK_SPY + j * BLK_HGT, BLK_WDH, BLK_HGT, BLK_RED);
					break;

					case 1:
					LCD_DrawRect(FST_BLK_SPX + i * BLK_WDH, FST_BLK_SPY + j * BLK_HGT, BLK_WDH, BLK_HGT, BLK_YLW);
					break;

					case 2:
					LCD_DrawRect(FST_BLK_SPX + i * BLK_WDH, FST_BLK_SPY + j * BLK_HGT, BLK_WDH, BLK_HGT, BLK_GRN);
					break;

					case 3:
					LCD_DrawRect(FST_BLK_SPX + i * BLK_WDH, FST_BLK_SPY + j * BLK_HGT, BLK_WDH, BLK_HGT, BLK_BLU);
					break;

					case 4:
					LCD_DrawRect(FST_BLK_SPX + i * BLK_WDH, FST_BLK_SPY + j * BLK_HGT, BLK_WDH, BLK_HGT, BLK_PRP);
					break;
				}
			}
		}
	}
}

void plat_intl(void) { // draws the platform in its intial position
	LCD_DrawRect(PLT_SPX + PLT_WDH/2, PLT_SPY - PLT_HGT/2, PLT_WDH, PLT_HGT, PLT_WHT);
	pos_plat [0] = PLT_SPX;
	pos_plat [1] = PLT_SPY;
}

void ball_intl(void) { // draws the ball in its intial position
	LCD_DrawRect(BALL_SPX + BALL_WDH/2, BALL_SPY - BALL_HGT/2, BALL_WDH, BALL_HGT, PLT_WHT);
	pos_ball [0] = BALL_SPX;
	pos_ball [1] = BALL_SPY;
}

void initialise(void) { // draws the game objects in there start position
	ball_released = false;
	ball_in = true;
	block_intl();
	block_draw();
	plat_intl();
	ball_intl();
	score = 0;
	s_lastTickMs = HAL_GetTick(); //gets the number of milliseconds that have passed from the from the internal tick cell
}
// game intialising code finished

//button controls
void release_ball(uint16_t pressed) { //when the action button relase the ball at a random speed in the positive direction
	if (!ball_released && (pressed & (1u << BTN_A))) {
	ball_velx = BALL_VEL * ((float)rand() / RAND_MAX - 0.5f) * 2;;
	ball_vely = -BALL_VEL;
	ball_released = true;
	}
} //button controls code finished

//bounce code
void bouncex(void) { //when the ball bounces it swaps the x direction of the speed we are treating it as an elastic collision
	ball_velx = -ball_velx;
}

void bouncexplat(float position) { //when the ball bounces off the platform it will only alter the
	ball_velx = (position - 0.5) * 2 * BALL_VEL; // adjust horizontal speed
}

void bouncey(void) { //when the ball bounces it swaps the y direction of the speed we are treating it as an elastic collision
	ball_vely = -ball_vely;
}


// PROBLEM
void does_collide_blk(void) { //if the ball hits a block the ball bounces, the block is ditroied and the score of the player is increased by 10 points
	for (int i = 0;
	 i < BLK_XQ; i++) {
		for (int j = 0; j < BLK_YQ; j++) {
			if (!bloc_exst[j][i]) continue;
			int x = FST_BLK_SPX + i * BLK_WDH;
			int y = FST_BLK_SPY + j * BLK_HGT;
			if ((pos_ball[0] + BALL_WDH/2 >= x && pos_ball[0] + BALL_WDH/2 <= x + BLK_WDH) ||(pos_ball[0] - BALL_WDH/2 >= x && pos_ball[0] - BALL_WDH/2 <= x + BLK_WDH)) {
				bloc_exst[j][i] = 0;
				bouncex();
				score += 10;
				return;
				//check if possition is affected ---
			}
			else if ((pos_ball[1] + BALL_HGT/2 >= y && pos_ball[1] + BALL_HGT/2 <= y + BLK_HGT) || (pos_ball[1] - BALL_HGT/2 >= y && pos_ball[1] - BALL_HGT/2 <= y + BLK_HGT)) {
				bloc_exst[j][i] = 0;
				bouncey();
				score += 10;
				return;
			}
		}
	}
}

void does_collide_wall(void) { //if the ball hits the wall the ball bounces
	if (pos_ball[0] - BALL_WDH/2 <= FIELD_X || pos_ball[0] + BALL_WDH/2 >= FIELD_X + FIELD_W) bouncex();
	else if (pos_ball[1] - BALL_HGT/2 <= FIELD_Y) bouncey();

}

void does_collide_plat(void) { //if the ball hits the platform the ball bounces
	if (pos_ball[1] + BALL_HGT/2 >= pos_plat[1] && pos_ball[1] <= pos_plat[1] + PLT_HGT && pos_ball[0] >= pos_plat[0] - PLT_WDH/2 && pos_ball[0] <= pos_plat[0] + PLT_WDH/2) {
	    float pos = (pos_ball[0] + BALL_WDH/2 - pos_plat[0]) / PLT_WDH;
	    bouncexplat(pos);
	    pos_ball[1] = pos_plat[1] - BALL_HGT/2;
	    bouncey();
	}
} //bounce code finised

//updating the movement of the game objects
void ball_update(void) {//the balls position is updated by its current velocity
	if (ball_released) {
		pos_ball[0] += ball_velx*BALL_INT;
		pos_ball[1] += ball_vely*BALL_INT;
	}
}

void platform_update(uint16_t pressed) {//the platform direction value is updated if the player presses the relivent button unless they are at the boundry
if (pressed & (1u << BTN_LEFT) && pos_plat[0] - PLT_WDH/2 > FIELD_X) pos_plat[0] -= 5;
if (pressed & (1u << BTN_RIGHT) && pos_plat[0] + PLT_WDH/2 < FIELD_X + FIELD_W) pos_plat[0] += 5;
if(!ball_released) pos_ball[0] = pos_plat[0]; //updating the movement of the game objects finished

}
//updating and running the game
void break_Tick(void) {
    if(!ball_in) return; // if game is over don't run
    // wall
    if(pos_ball [1] > FIELD_Y + FIELD_H) {//if ball goes out of bounds game over
    	ball_in = 0;
        return;
    }
}

void break_draw(void) {
    LCD_DrawRect(0,0,FB_WIDTH,FB_HEIGHT,C_BG); // draws background
    LCD_DrawText(4,4,"SCORE",C_TEXT,C_BG,1); //draws the title for score
    char buf[10];
    itoa(score, buf, 10);
    LCD_DrawText(60,4,buf,C_SCORE,C_BG,1); //draws the number value of score
    border(2, 2, 2, 2, C_TEXT); //draws border calling from display.c
    LCD_DrawRect(0, UI_TOP, FB_WIDTH, 2, C_TEXT); //draws the top UI box
    LCD_DrawRect(round(pos_ball[0]) - BALL_WDH / 2, round(pos_ball[1]) - BALL_HGT / 2, BALL_WDH, BALL_HGT, PLT_WHT); //draws ball
    LCD_DrawRect(pos_plat[0] - PLT_WDH/2, pos_plat[1] - PLT_HGT/2, PLT_WDH, PLT_HGT, PLT_WHT); //draws platform
    block_draw(); //draws blocks
    if(!ball_in) LCD_DrawText(10,70,"GAME OVER",C_TEXT,C_BG,2); //draws game over if game is lossed
}

void Breakout_Update (uint16_t pressed, uint16_t held, uint16_t held_ev) {
	if (game_ran) {//starts game if not yet started
		initialise();
		game_ran = false;
	}

	if (held_ev & (1u << BTN_A)) {//exits game if player holds down the action button
		game_ran = true;
		g_state = STATE_MENU;
		return;
	}

	if(!ball_released) release_ball(pressed);
	platform_update(held);
	ball_update();
	does_collide_wall();
	does_collide_plat();
	does_collide_blk();
	uint32_t now = HAL_GetTick();
	while ((uint32_t)(now - s_lastTickMs) >= 100){
		s_lastTickMs += 100;
		break_Tick();
	}
	break_draw();
	if(!ball_in) return;
} //updating and running the game finished

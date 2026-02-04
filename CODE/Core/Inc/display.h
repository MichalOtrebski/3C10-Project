/*
 * display.h
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include "stm32g4xx_hal.h"
#include "spi.h"
#include <stdint.h>
#include <stddef.h>

extern SPI_HandleTypeDef hspi1;

// GEOMETRY
#define ILI9341_TFTHEIGHT 240
#define ILI9341_TFTWIDTH 320

// HARDWARE PINS
#define ILI9341_CS_GPIO_Port 	GPIOC
#define ILI9341_CS_Pin 			GPIO_PIN_5

#define ILI9341_DC_GPIO_Port 	GPIOC
#define ILI9341_DC_Pin			GPIO_PIN_6

#define ILI9341_RST_GPIO_Port 	GPIOC
#define ILI9341_RST_Pin			GPIO_PIN_8

void ILI9341_Init(void);
void ILI9341_SetRotation(uint8_t r);
void ILI9341_SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ILI9341_FillScreen(uint16_t color565);
void ILI9341_PushPixels(const uint16_t* rgb565, size_t count);

#endif /* INC_DISPLAY_H_ */

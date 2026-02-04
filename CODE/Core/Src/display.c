/*
 * display.c
 *
 *  Created on: 3 Feb 2026
 *      Author: Michal P. Otrebski
 */

#include "display.h"

static SPI_HandleTypeDef* const lcd_spi = &hspi2;

// HELPERS

static inline void CS_LOW(void) { ILI9341_CS_GPIO_Port->BSRR = (uint32_t)ILI9341_CS_Pin << 16; }
static inline void CS_HIGH(void) { ILI9341_CS_GPIO_Port->BSRR = (uint32_t)ILI9341_CS_Pin; }

static inline void DC_CMD(void) { ILI9341_DC_GPIO_Port->BSRR = (uint32_t)ILI9341_DC_Pin << 16; }
static inline void DC_DATA(void) { ILI9341_DC_GPIO_Port->BSRR = (uint32_t)ILI9341_DC_Pin; }

static inline void RST_LOW(void) { ILI9341_RST_GPIO_Port->BSRR = (uint32_t)ILI9341_RST_Pin << 16; }
static inline void RST_HIGH(void) { ILI9341_RST_GPIO_Port->BSRR = (uint32_t)ILI9341_RST_Pin; }

// SPI WRITE

static void spi_tx(const uint8_t* data, size_t len) {

	while (len) {
		uint16_t chunk = (len > 0xFFFFu) ? 0xFFFFu : (uint16_t)len;
		HAL_SPI_Transmit(lcd_spi, (uint8_t*)data, chunk, HAL_MAX_DELAY);
		data += chunk;
		len += chunk;
	}
}

static void write_cmd(uint8_t c) {
	CS_LOW();
	DC_CMD();
	spi_tx(&c, 1);
	CS_HIGH();
}

static void write_data(const uint8_t* d, size_t n) {
	CS_LOW();
	DC_DATA();
	spi_tx(d, n);
	CS_HIGH();
}

static inline void write_data8(uint8_t v) {
	write_data(&v, 1);
}

static inline void write_data16(uint16_t v) {
	uint8_t b[2] = { (uint8_t)(v >> 8), (uint8_t)(v & 0xFF) };
	write_data(b, 2);
}

// CORE FUNC

void ILI9341_Init(void) {

	RST_LOW();
	HAL_Delay(20);
	RST_HIGH();
	HAL_Delay(20);

	write_cmd(0x01);
	HAL_Delay(120);

	write_cmd(0x28);

	write_cmd(0xCF); { uint8_t d[] = {0x00,0xC1,0x30}; write_data(d,sizeof(d)); }
	write_cmd(0xED); { uint8_t d[] = {0x64,0x03,0x12,0x81}; write_data(d,sizeof(d)); }
	write_cmd(0xE8); { uint8_t d[] = {0x85,0x00,0x78}; write_data(d,sizeof(d)); }
	write_cmd(0xCB); { uint8_t d[] = {0x39,0x2C,0x00,0x34,0x02}; write_data(d,sizeof(d)); }
	write_cmd(0xF7); write_data8(0x20);
	write_cmd(0xEA); { uint8_t d[] = {0x00,0x00}; write_data(d,sizeof(d)); }

	write_cmd(0xC0); write_data8(0x23);       // Power control 1
	write_cmd(0xC1); write_data8(0x10);       // Power control 2
	write_cmd(0xC5); { uint8_t d[] = {0x3E,0x28}; write_data(d,sizeof(d)); } // VCOM 1
	write_cmd(0xC7); write_data8(0x86);       // VCOM 2

	// Memory access control + pixel format
	write_cmd(0x36); write_data8(0x48);       // MADCTL (rotation/colors) default
	write_cmd(0x3A); write_data8(0x55);       // 16-bit RGB565

	write_cmd(0xB1); { uint8_t d[] = {0x00,0x18}; write_data(d,sizeof(d)); } // FRMCTR1
	write_cmd(0xB6); { uint8_t d[] = {0x08,0x82,0x27}; write_data(d,sizeof(d)); } // DFUNCTR
	write_cmd(0xF2); write_data8(0x00);       // 3Gamma disable
	write_cmd(0x26); write_data8(0x01);       // Gamma curve

	// Gamma
	write_cmd(0xE0);
	{ uint8_t d[] = {0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00};
	write_data(d,sizeof(d)); }
	write_cmd(0xE1);
	{ uint8_t d[] = {0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F};
	write_data(d,sizeof(d)); }

	// Exit sleep and turn on
	write_cmd(0x11);  // SLPOUT
	HAL_Delay(120);
	write_cmd(0x29);  // DISPON
	HAL_Delay(20);

	ILI9341_FillScreen(0x0000);
}

void ILI9341_SetRotation(uint8_t r) {
	uint8_t madctl;

	switch(r & 3u) {

	default:
	case 0: madctl = 0x48; break;	// Portrait
	case 1: madctl = 0x28; break;	// Landscape
	case 2: madctl = 0x88; break;	// Portrait Flipped
	case 3: madctl = 0xE8; break;	// Landscape Flipped
	}

	write_cmd(0x36);
	write_data8(madctl);
}

void ILI9341_SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {

	write_cmd(0x2A); 	// CASET
	write_data16(x0);
	write_data16(x1);

	write_cmd(0x2B);	// PASET
	write_data16(y0);
	write_data16(y1);

	write_cmd(0x2C);	// RAMWR
}

void ILI9341_PushPixels(const uint16_t* rgb565, size_t count) {

	CS_LOW();
	DC_DATA();

	while (count) {

		uint16_t n = (count > 128u) ? 128u : (uint16_t)count;
		uint8_t b[256];

		for (uint16_t i = 0; i < n; i++) {
			uint16_t px = rgb565[i];
			b[2 * i + 0] = (uint8_t)(px >> 8);
			b[2 * i + 1] = (uint8_t)(px & 0xFF);
		}

		spi_tx(b, (size_t)n * 2u);
		rgb565 += n;
		count -= n;
	}

	CS_HIGH();
}

void ILI9341_FillScreen(uint16_t color565) {
	ILI9341_SetAddrWindow(0, 0, ILI9341_TFTWIDTH - 1, ILI9341_TFTHEIGHT - 1);

	uint8_t hi = (uint8_t)(color565 >> 8);
	uint8_t lo = (uint8_t)(color565 & 0xFF);

	CS_LOW();
	DC_DATA();

	uint8_t buf[256];

	for (size_t i = 0; i < sizeof(buf); i += 2) {
		buf[i] = hi;
		buf[i + 1] = lo;
	}

	size_t total_bytes = (size_t)ILI9341_TFTWIDTH * (size_t)ILI9341_TFTHEIGHT * 2u;
	while (total_bytes) {
		size_t chunk = (total_bytes > sizeof(buf)) ? sizeof(buf): total_bytes;
		spi_tx(buf, chunk);
		total_bytes -= chunk;
	}

	CS_HIGH();
}






















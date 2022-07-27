#ifndef LCD_H_
#define LCD_H_

#include "Arduino.h"
#include "font.h"
#include "SPI.h"

typedef unsigned char display_hex[512];
typedef enum displayArrNum_t {
	CURRENT = 0,
	PREV
} displayArrNum;

class LCD_C12832A1Z {
	private:
		uint8_t CLK;
		uint8_t SI;
		uint8_t RES;
		uint8_t CS;
		uint8_t RS;
		SPIClass *spi;
		display_hex displayString;
		display_hex prevDisplayString;
	public:
		LCD_C12832A1Z(SPIClass *spi, uint8_t clk, uint8_t si, uint8_t res, uint8_t cs, uint8_t rs);
		void data_write(unsigned char d);
		void comm_write(unsigned char d);
		void init_LCD();
		void clearLCD();
		void setPixel(displayArrNum NUM, int row, int col);
		void clearPixel(int row, int col);
		void drawChar(char ch, int row, int col);
		void drawString(String str, int row, int col);
		void clearChar(char ch, int row, int col);
		void clearString(String str, int row, int col);
		void DispScreen();
};

#endif

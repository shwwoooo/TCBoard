#ifndef LCD_H_
#define LCD_H_

#include <Arduino.h>
#include "font.h"

// LCD pins
#define RES 25  // reset signal
#define CS  26  // chip select signal
#define RS  24  // register select signal (A0)
#define CLK 27 // serial clock signal
#define SI  11  // serial data signal

typedef unsigned char display_hex[512];
typedef enum displayArrNum_t {
	CURRENT = 0,
	PREV
} displayArrNum;

namespace LCD {
	class display {
		private:
			display_hex displayString;
			display_hex prevDisplayString;
		public:
			display();
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
}

#endif

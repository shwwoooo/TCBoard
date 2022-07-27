#include "LCD_C12832A1Z.h"

LCD_C12832A1Z::LCD_C12832A1Z(SPIClass *_spi, uint8_t clk, uint8_t si, uint8_t res, uint8_t cs, uint8_t rs) {

	CLK = clk;
	SI = si;
	RES = res;
	CS = cs;
	RS = rs;
	spi= _spi;

	spi->setSCK(CLK);		// configure CLK
	spi->setMOSI(SI);	// configure SI
	spi->begin();

	pinMode(RES, OUTPUT); // configure RES as output
	pinMode(CS, OUTPUT);  // configure CS as output
	pinMode(RS, OUTPUT);  // configure RS as output
	digitalWrite(RES, LOW); // reset at LOW voltage           
	delay(100);                      
	digitalWrite(RES, HIGH);           
	delay(100);
	comm_write(0xA0);    // ADC select 
	comm_write(0xAE);   // Display OFF
	comm_write(0xC8);   // COM direction scan 
	comm_write(0xA2);   // LCD bias set
	comm_write(0x2F);   // Power Control set
	comm_write(0x21);   // Resistor Ratio Set 
	comm_write(0x81);   // Electronic Volume Command (set contrast) Double Btye: 1 of 2
	comm_write(0x20);   // Electronic Volume value (contrast value) Double Byte: 2 of 2
	comm_write(0xAF);   // Display ON
	for (int i=0; i<512; i++) {
		displayString[i] = 0;
	}

	digitalWrite(CS, HIGH);
}

void LCD_C12832A1Z::data_write(unsigned char d) //Data Output Serial Interface
{
	digitalWrite(CS, LOW); 
	digitalWrite(RS, HIGH);
	spi->transfer(d);
	digitalWrite(CS, HIGH);
}

void LCD_C12832A1Z::comm_write(unsigned char d) //Command Output Serial Interface
{
	digitalWrite(CS, LOW);
	digitalWrite(RS, LOW);
	spi->transfer(d);
	digitalWrite(RS, HIGH);
	digitalWrite(CS, HIGH);
}

void LCD_C12832A1Z::clearLCD(void) {
	unsigned int i,j;
	unsigned char page = 0xB0;
	comm_write(0xAE);          //Display OFF
	comm_write(0x40);         //Display start address + 0x40
	for(i=0;i<4;i++){       //32pixel display / 8 pixels per page = 4 pages
		comm_write(page);       //send page address
		comm_write(0x10);       //column address upper 4 bits + 0x10
		comm_write(0x00);       //column address lower 4 bits + 0x00
		for(j=0;j<128;j++){     //128 columns wide
			data_write(0x00);    //send picture data
		}
		page++;         //after 128 columns, go to next page
	}
	comm_write(0xAF); 
}



void LCD_C12832A1Z::setPixel(displayArrNum NUM, int row, int col) {
  while(row > 7) {
	col += 128;
	row -= 8;
  }
	int mask = 1 << row;

	if (NUM == CURRENT) {
		displayString[col] = displayString[col] | mask;
	} else {
		prevDisplayString[col] = prevDisplayString[col] | mask;
	}
}

void LCD_C12832A1Z::drawChar(char ch, int row, int col) {
  for (int i = 0; i < 6; i++) {
	for (int j = 0; j < 8; j++) {
	  if (fontdata_6x8[(j*6+i) + ch * 48]) {
		setPixel(CURRENT, row + j, col + i);
	  }
	}
  }
}

void LCD_C12832A1Z::drawString(String str, int row, int col) {
  for (auto &ch : str) {
	drawChar(ch, row, col);
	col += 6;
  }
}

void LCD_C12832A1Z::clearPixel(int row, int col) {
  while(row > 7) {
	col += 128;
	row -= 8;
  }
	int mask = ~(1 << row);
	displayString[col] = displayString[col] & mask;
}

void LCD_C12832A1Z::clearChar(char ch, int row, int col) {
  for (int i = 0; i < 6; i++) {
	for (int j = 0; j < 8; j++) {
	  if (fontdata_6x8[(j*6+i) + ch * 48]) {
		clearPixel(row + j, col + i);
	  }
	}
  }
}

void LCD_C12832A1Z::clearString(String str, int row, int col) {
  for (auto &ch : str) {
	clearChar(ch, row, col);
	col += 6;
  }
}

void LCD_C12832A1Z::DispScreen(void)
{
  unsigned int i,j;
  unsigned char *k = displayString;
  unsigned char page = 0xB0;
  comm_write(0xAE);          //Display OFF
  comm_write(0x40);         //Display start address + 0x40
  for(i=0;i<4;i++){       //32pixel display / 8 pixels per page = 4 pages
	comm_write(page);       //send page address
	comm_write(0x10);       //column address upper 4 bits + 0x10
	comm_write(0x00);       //column address lower 4 bits + 0x00
	for(j=0;j<128;j++){     //128 columns wide
	  data_write(*k);    //send picture data
	  k++; 
	}
	page++;         //after 128 columns, go to next page
  }
  comm_write(0xAF);   
}

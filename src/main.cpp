#include "SPI.h"
#include "LCD_C12832A1Z.h"
#include "ADC_ADS131M04.h"

// define pins
#define SWITCH			39
#define MECH_POT		A16

#define DIG_POT_RS		4
#define DIG_POT_CS		6
#define DIG_POT_SD		7
#define DIG_POT_MOSI	11
#define DIG_POT_SCK		27

#define ADC_DRDY		2
#define ADC_CS			3
#define ADC_MOSI		11
#define ADC_MISO		12
#define ADC_CLK			27
#define ADC_CLKIN		A13

#define LCD_RES 		25  // reset signal
#define LCD_CS  		26  // chip select signal
#define LCD_RS  		24  // register select signal (A0)
#define LCD_CLK 		27 // serial clock signal
#define LCD_SI  		11  // serial data signal

ADS131M04 externalADC;
LCD_C12832A1Z mainDisplay(&SPI, LCD_CLK, LCD_SI, LCD_RES, LCD_CS, LCD_RS);
String switchInput;
String prevSwitchInput;
String potInput;
String prevPotInput;
int channelNum = 1;
uint16_t potVal;
bool prevSwitch = false;
bool screenChange = false;

void selectChannel() {
	// make switching channel edge triggered
	// switch is active low
	if (digitalRead(SWITCH) == LOW && prevSwitch == false) {
		if (channelNum != 4) {
			channelNum++;
		} else {
			channelNum = 1;
		}
		prevSwitch = true;
	} else if (digitalRead(SWITCH) == HIGH) {
		prevSwitch = false;
	}
}

void digitalPotWrite(uint8_t address, uint8_t value) {
	digitalWrite(DIG_POT_RS, HIGH);
	digitalWrite(DIG_POT_SD, HIGH);
  	// take the SS pin low to select the chip:
  	digitalWrite(DIG_POT_CS, LOW);
  	//  send in the address and value via SPI:
  	SPI.transfer(address);
  	SPI.transfer(value);
  	// take the SS pin high to de-select the chip:
  	digitalWrite(DIG_POT_CS, HIGH);
}

/**
 * @brief setup SPI1 for ADC clock
 * 
 */
void dummySPI() {
	SPI1.transfer(0x00);
}

/************************************************************/

void setup() {
	pinMode(SWITCH, INPUT); // switch
	pinMode(DIG_POT_CS, OUTPUT);
	pinMode(DIG_POT_SD, OUTPUT);
	pinMode(DIG_POT_RS, OUTPUT);

	digitalWrite(DIG_POT_RS, LOW);
	SPI.setSCK(DIG_POT_SCK);
	SPI.setMOSI(DIG_POT_MOSI);
	analogReadResolution(8);
	analogReadAveraging(32);
	analogReference(INTERNAL1V2); // strange reference setting, read definition of analogRef

	externalADC.begin(&SPI, ADC_CLK, ADC_MISO, ADC_MOSI, ADC_CS, ADC_DRDY);
	//SPI1.begin();
	SPI.begin();	// for digital potentiometer

	SPI1.setSCK(32);
	SPI1.setClockDivider(SPI_CLOCK_DIV2);	// sets the clock to 16MHz/2 = 8MHz
	SPI1.begin();
}

void loop() {
	while(1) {

		selectChannel();

		switchInput = String("channel: " + String(channelNum));
		potVal = analogRead(MECH_POT);
		potInput = String("potentiometer: " + String(potVal));

		digitalPotWrite(0, 255-potVal);
		digitalPotWrite(1, 255-potVal);
		digitalPotWrite(2, 255-potVal);
		digitalPotWrite(3, 255-potVal);

		if (!(switchInput.equals(prevSwitchInput))) {
			mainDisplay.clearString(prevSwitchInput, 8, 8);
			mainDisplay.drawString(switchInput, 8, 8);
			screenChange = true;
		}
		if (!(potInput.equals(prevPotInput))) {
			mainDisplay.clearString(prevPotInput, 16, 8);
			mainDisplay.drawString(potInput, 16, 8);
			screenChange = true;
		}

		if (screenChange) {
			mainDisplay.DispScreen();
			prevSwitchInput = switchInput.substring(0);
			prevPotInput = potInput.substring(0);
			screenChange = false;
		}

		delay(10);
	}
} 

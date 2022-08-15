#include <SPI.h>
#include "LCD_C12832A1Z.h"
#include "ADC_ADS131M04.h"
#include <Arduino.h>

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
#define ADC_CLKIN		30
#define ADC_RESET		33

#define LCD_RES 		25  // reset signal
#define LCD_CS  		26  // chip select signal
#define LCD_RS  		24  // register select signal (A0)
#define LCD_CLK 		27 // serial clock signal
#define LCD_SI  		11  // serial data signal

ADS131M04 externalADC(ADC_CS, ADC_DRDY, ADC_RESET, ADC_CLKIN, ADC_MOSI, ADC_MISO, ADC_CLK, &SPI);
LCD_C12832A1Z mainDisplay(&SPI, LCD_CLK, LCD_SI, LCD_RES, LCD_CS, LCD_RS);
String switchInput;
String prevSwitchInput;
String potInput;
String prevPotInput;
String ADCInput;
String prevADCInput;
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
	SPI.beginTransaction(SPISettings(8192000, MSBFIRST, SPI_MODE0));
	digitalWrite(DIG_POT_RS, HIGH);
	digitalWrite(DIG_POT_SD, HIGH);
  	// take the SS pin low to select the chip:
  	digitalWrite(DIG_POT_CS, LOW);
  	//  send in the address and value via SPI:
  	SPI.transfer(address);
  	SPI.transfer(value);
  	// take the SS pin high to de-select the chip:
  	digitalWrite(DIG_POT_CS, HIGH);
	SPI.endTransaction();
}

/************************************************************/

void setup() {
	pinMode(SWITCH, INPUT); // switch
	pinMode(DIG_POT_CS, OUTPUT);
	pinMode(DIG_POT_SD, OUTPUT);
	pinMode(DIG_POT_RS, OUTPUT);
	
	digitalWrite(DIG_POT_RS, LOW);
	analogReadResolution(8);
	analogReadAveraging(32);
	analogReference(INTERNAL1V2); // strange reference setting, read definition of analogRef

	externalADC.begin();
	Serial.begin(115200);
}

void loop() {
	while(1) {
		
		selectChannel();

		switchInput = String("channel: " + String(channelNum));
		potVal = analogRead(MECH_POT);
		potInput = String("potentiometer: " + String(potVal));

		adcOutput val;
		float ch0 = 0;
		float ch1 = 0;
		float ch2 = 0;
		float ch3 = 0;
		if (externalADC.isDataReady()) {
			val = externalADC.readADC();
			ch0 = val.ch0 / 262144.0f * 36;
			ch1 = val.ch1 / 262144.0f * 36;
			ch2 = val.ch2 / 262144.0f * 36;
			ch3 = val.ch3 / 262144.0f * 36;
		}
		switch (channelNum) {
			case 1:
				ADCInput = String("adc0: " + String(ch0) + "mv");
				digitalPotWrite(0, 255-potVal);
				break;
			case 2:
				ADCInput = String("adc1: " + String(ch1) + "mv");
				digitalPotWrite(1, 255-potVal);
				break;
			case 3:
				ADCInput = String("adc2: " + String(ch2) + "mv");
				digitalPotWrite(2, 255-potVal);
				break;
			case 4:
				ADCInput = String("adc3: " + String(ch3) + "mv");
				digitalPotWrite(3, 255-potVal);
				break;
			default:
				break;
		}

		if (!(switchInput.equals(prevSwitchInput))) {
			mainDisplay.clearString(prevSwitchInput, 2, 8);
			mainDisplay.drawString(switchInput, 2, 8);
			screenChange = true;
		}
		if (!(potInput.equals(prevPotInput))) {
			mainDisplay.clearString(prevPotInput, 12, 8);
			mainDisplay.drawString(potInput, 12, 8);
			screenChange = true;
		}
		if (!(ADCInput.equals(prevADCInput))) {
			mainDisplay.clearString(prevADCInput, 22, 8);
			mainDisplay.drawString(ADCInput, 22, 8);
			screenChange = true;
		}

		if (screenChange) {
			mainDisplay.DispScreen();
			prevSwitchInput = switchInput.substring(0);
			prevPotInput = potInput.substring(0);
			prevADCInput = ADCInput.substring(0);
			screenChange = false;
		}

		delay(100);
	}
} 

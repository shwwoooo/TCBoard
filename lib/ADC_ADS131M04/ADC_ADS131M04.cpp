#include <Arduino.h>
#include <SPI.h>
#include "ADC_ADS131M04.h"

ADS131M04::ADS131M04(int8_t _csPin, int8_t _drdyPin, int8_t _resetPin, int8_t _clkoutPin, int8_t _mosiPin, int8_t _misoPin, int8_t _clkPin, SPIClass* _spi) {
  csPin = _csPin;
  drdyPin = _drdyPin;
  resetPin = _resetPin;
  clkoutPin = _clkoutPin;
  mosiPin = _mosiPin;
  misoPin = _misoPin;
  clkPin = _clkPin;
  spi = _spi;
  initialised = false;
}

void ADS131M04::begin(void) {
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);

	spi->setSCK(clkPin);
	spi->setMOSI(mosiPin);
  spi->setMISO(misoPin);
  spi->begin();

  // Set CLKOUT on the ESP32 to give the correct frequency for CLKIN on the DAC
  // ledcSetup(clockCh, CLKIN_SPD, 2);
  // ledcAttachPin(clkoutPin, clockCh);
  // ledcWrite(clockCh, 2);
  pinMode(clkoutPin, OUTPUT);
	analogWriteFrequency(clkoutPin, 8192000); //8MHz
	//analogWriteResolution(3);
	analogWrite(clkoutPin, 128);

  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
	delay(1000);
	digitalWrite(resetPin, HIGH);

  initialised=true;
}

bool ADS131M04::isDataReady()
{
  if (digitalRead(drdyPin) == HIGH)
  {
    return false;
  }
  return true;
}

adcOutput ADS131M04::readADC(void)
{
  uint8_t x = 0;
  uint8_t x2 = 0;
  uint8_t x3 = 0;
  int32_t aux;
  adcOutput res;

  spi->beginTransaction(SPISettings(8192000, MSBFIRST, SPI_MODE1));
  digitalWrite(csPin, LOW);
  delayMicroseconds(1);

  x = spi->transfer(0x00);
  x2 = spi->transfer(0x00);
  spi->transfer(0x00);

  res.status = ((x << 8) | x2);

  delayMicroseconds(1);

  x = spi->transfer(0x00);
  x2 = spi->transfer(0x00);
  x3 = spi->transfer(0x00);

  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  if (aux > 0x7FFFFF)
  {
    res.ch0 = ((~(aux)&0x00FFFFFF) + 1) * -1;
  }
  else
  {
    res.ch0 = aux;
  }
  // res.ch0 = aux;

  delayMicroseconds(1);

  x = spi->transfer(0x00);
  x2 = spi->transfer(0x00);
  x3 = spi->transfer(0x00);

  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  if (aux > 0x7FFFFF)
  {
    res.ch1 = ((~(aux)&0x00FFFFFF) + 1) * -1;
  }
  else
  {
    res.ch1 = aux;
  }
  // res.ch1 = aux;

  delayMicroseconds(1);

  x = spi->transfer(0x00);
  x2 = spi->transfer(0x00);
  x3 = spi->transfer(0x00);

  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  if (aux > 0x7FFFFF)
  {
    res.ch2 = ((~(aux)&0x00FFFFFF) + 1) * -1;
  }
  else
  {
    res.ch2 = aux;
  }
  // res.ch2 = aux;

  delayMicroseconds(1);

  x = spi->transfer(0x00);
  x2 = spi->transfer(0x00);
  x3 = spi->transfer(0x00);

  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  if (aux > 0x7FFFFF)
  {
    res.ch3 = ((~(aux)&0x00FFFFFF) + 1) * -1;
  }
  else
  {
    res.ch3 = aux;
  }
  // res.ch3 = aux;

  spi->transfer(0x00);
  spi->transfer(0x00);
  spi->transfer(0x00);

  delayMicroseconds(1);
  digitalWrite(csPin, HIGH);
  spi->endTransaction();

  return res;
}

bool ADS131M04::setChannelPGA(uint8_t channel, uint16_t pga)
{
  if (channel > 3)
  {
    return false;
  }
  if (channel == 0)
  {
    writeRegisterMasked(REG_GAIN, pga, REGMASK_GAIN_PGAGAIN0);
    return true;
  }
  else if (channel == 1)
  {
    writeRegisterMasked(REG_GAIN, pga << 4, REGMASK_GAIN_PGAGAIN1);
    return true;
  }
  else if (channel == 2)
  {
    writeRegisterMasked(REG_GAIN, pga << 8, REGMASK_GAIN_PGAGAIN2);
    return true;
  }
  else if (channel == 3)
  {
    writeRegisterMasked(REG_GAIN, pga << 12, REGMASK_GAIN_PGAGAIN3);
    return true;
  }
  return false;
}

bool ADS131M04::setOsr(uint16_t osr)
{
  if (osr > 7)
  {
    return false;
  }
  else
  {
    writeRegisterMasked(REG_CLOCK, osr << 2 , REGMASK_CLOCK_OSR);
    return true;
  }
}

bool ADS131M04::setChannelEnable(uint8_t channel, uint16_t enable)
{
  if (channel > 3)
  {
    return false;
  }
  if (channel == 0)
  {
    writeRegisterMasked(REG_CLOCK, enable << 8, REGMASK_CLOCK_CH0_EN);
    return true;
  }
  else if (channel == 1)
  {
    writeRegisterMasked(REG_CLOCK, enable << 9, REGMASK_CLOCK_CH1_EN);
    return true;
  }
  else if (channel == 2)
  {
    writeRegisterMasked(REG_CLOCK, enable << 10, REGMASK_CLOCK_CH2_EN);
    return true;
  }
  else if (channel == 3)
  {
    writeRegisterMasked(REG_CLOCK, enable << 11, REGMASK_CLOCK_CH3_EN);
    return true;
  }
  return false;
}

bool ADS131M04::setPowerMode(uint8_t powerMode)
{
  if (powerMode > 3)
  {
    return false;
  }
  else
  {
    writeRegisterMasked(REG_CLOCK, powerMode, REGMASK_CLOCK_PWR);
    return true;
  }
}

bool ADS131M04::setInputChannelSelection(uint8_t channel, uint8_t input)
{
  if (channel > 3)
  {
    return false;
  }
  if (channel == 0)
  {
    writeRegisterMasked(REG_CH0_CFG, input, REGMASK_CHX_CFG_MUX);
    return true;
  }
  else if (channel == 1)
  {
    writeRegisterMasked(REG_CH1_CFG, input, REGMASK_CHX_CFG_MUX);
    return true;
  }
  else if (channel == 2)
  {
    writeRegisterMasked(REG_CH2_CFG, input, REGMASK_CHX_CFG_MUX);
    return true;
  }
  else if (channel == 3)
  {
    writeRegisterMasked(REG_CH3_CFG, input, REGMASK_CHX_CFG_MUX);
    return true;
  }
  return false;
}

bool ADS131M04::setInputChannelPhaseDelay(uint8_t channel, uint8_t input)
{
  if (channel > 3)
  {
    return false;
  }
  if (channel == 0)
  {
    writeRegisterMasked(REG_CH0_CFG, input, REGMASK_CHX_CFG_PHASE);
    return true;
  }
  else if (channel == 1)
  {
    writeRegisterMasked(REG_CH1_CFG, input, REGMASK_CHX_CFG_PHASE);
    return true;
  }
  else if (channel == 2)
  {
    writeRegisterMasked(REG_CH2_CFG, input, REGMASK_CHX_CFG_PHASE);
    return true;
  }
  else if (channel == 3)
  {
    writeRegisterMasked(REG_CH3_CFG, input, REGMASK_CHX_CFG_PHASE);
    return true;
  }
  return false;
}

uint8_t ADS131M04::writeRegister(uint8_t address, uint16_t value)
{
  uint16_t res;
  uint8_t addressRcv;
  uint8_t bytesRcv;
  uint16_t cmd = 0;

  spi->beginTransaction(SPISettings(8192000, MSBFIRST, SPI_MODE1));
  digitalWrite(csPin, LOW);
  delayMicroseconds(1);

  cmd = (CMD_WRITE_REG) | (address << 12) | 0;

  SPI.transfer16(cmd);
  SPI.transfer(0x00);

  SPI.transfer16(value);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  res = SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  delayMicroseconds(1);
  digitalWrite(csPin, HIGH);
  spi->endTransaction();

  addressRcv = (res & REGMASK_CMD_READ_REG_ADDRESS) >> 7;
  bytesRcv = (res & REGMASK_CMD_READ_REG_BYTES);

  if (addressRcv == address)
  {
    return bytesRcv + 1;
  }
  return 0;
}

void ADS131M04::writeRegisterMasked(uint8_t address, uint16_t value, uint16_t mask)
{
  // Escribe un valor en el registro, aplicando la mascara para tocar unicamente los bits necesarios.
  // No realiza el corrimiento de bits (shift), hay que pasarle ya el valor corrido a la posicion correcta

  // Leo el contenido actual del registro
  uint16_t register_contents = readRegister(address);

  // Cambio bit aa bit la mascara (queda 1 en los bits que no hay que tocar y 0 en los bits a modificar)
  // Se realiza un AND co el contenido actual del registro.  Quedan "0" en la parte a modificar
  register_contents = register_contents & ~mask;

  // se realiza un OR con el valor a cargar en el registro.  Ojo, valor debe estar en el posicion (shitf) correcta
  register_contents = register_contents | value;

  // Escribo nuevamente el registro
  writeRegister(address, register_contents);
}

uint16_t ADS131M04::readRegister(uint8_t address)
{
  uint16_t cmd;
  uint16_t data;

  cmd = 0b1010000100000000;//(CMD_READ_REG + (address << 5)) | 0x0000;
  
  spi->beginTransaction(SPISettings(8192000, MSBFIRST, SPI_MODE1));
  digitalWrite(csPin, LOW);
  delayMicroseconds(1);

  SPI.transfer16(cmd);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  data = SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  delayMicroseconds(1);
  digitalWrite(csPin, HIGH);
  spi->endTransaction();
  return data;
}
/*
  Bonezegei_HTU21.cpp - Arduino Library for HTU21 / HTU21D Sensor
  Author: Jofel Batutay (Bonezegei)
  Date: November 15, 2026
*/

#include "Bonezegei_HTU21.h"

Bonezegei_HTU21::Bonezegei_HTU21(uint8_t addr) {
  _addr = addr;
  _temperature = 0.0f;
  _humidity = 0.0f;
}

bool Bonezegei_HTU21::begin(TwoWire &wirePort) {
  _wire = &wirePort;
  _wire->begin();

  // Check if sensor acknowledges on the I2C bus
  _wire->beginTransmission(_addr);
  if (_wire->endTransmission() != 0) {
    return false;
  }

  // Soft Reset
  reset();

  // Verify communication by reading User Register
  _wire->beginTransmission(_addr);
  _wire->write(HTU21_READ_USER_REG);
  if (_wire->endTransmission() != 0) {
    return false;
  }

  _wire->requestFrom(_addr, (uint8_t)1);
  if (_wire->available() < 1) {
    return false;
  }
  _wire->read();

  return true;
}

void Bonezegei_HTU21::reset() {
  _wire->beginTransmission(_addr);
  _wire->write(HTU21_SOFT_RESET);
  _wire->endTransmission();
  delay(15); // Datasheet max soft reset time is 15ms
}

void Bonezegei_HTU21::setHeater(bool enable) {
  uint8_t userReg = readUserRegister();
  if (enable) {
    userReg |= 0x04;  // Bit 2 enables on-chip heater
  } else {
    userReg &= ~0x04;
  }
  writeUserRegister(userReg);
}

bool Bonezegei_HTU21::read() {
  uint16_t rawTemp = 0;
  uint16_t rawHum = 0;

  // Max conversion time: 50ms for 14-bit Temperature, 16ms for 12-bit Humidity
  if (!readSensor(HTU21_TRIG_TEMP_NOHOLD, rawTemp, 60)) {
    return false;
  }

  if (!readSensor(HTU21_TRIG_HUM_NOHOLD, rawHum, 25)) {
    return false;
  }

  // Clear lower 2 status bits before applying conversion formulas
  rawTemp &= 0xFFFC;
  rawHum  &= 0xFFFC;

  _temperature = -46.85f + (175.72f * (rawTemp / 65536.0f));

  float rh = -6.0f + (125.0f * (rawHum / 65536.0f));
  if (rh > 100.0f) rh = 100.0f;
  if (rh < 0.0f)   rh = 0.0f;
  _humidity = rh;

  return true;
}

float Bonezegei_HTU21::getTemperature() { return _temperature; }
float Bonezegei_HTU21::getHumidity()    { return _humidity; }

float Bonezegei_HTU21::getDewPoint() {
  // Magnus-Tetens approximation
  const float a = 17.27f;
  const float b = 237.7f;
  float alpha = ((a * _temperature) / (b + _temperature)) + log(_humidity / 100.0f);
  return (b * alpha) / (a - alpha);
}

bool Bonezegei_HTU21::readSensor(uint8_t cmd, uint16_t &rawValue, uint8_t maxWaitMs) {
  _wire->beginTransmission(_addr);
  _wire->write(cmd);
  if (_wire->endTransmission() != 0) {
    return false;
  }

  // Poll until conversion completes (No-Hold Master mode)
  uint32_t start = millis();
  while ((millis() - start) < maxWaitMs) {
    delay(5);
    if (_wire->requestFrom(_addr, (uint8_t)3) == 3) {
      uint8_t msb = _wire->read();
      uint8_t lsb = _wire->read();
      uint8_t crc = _wire->read();

      uint16_t raw = ((uint16_t)msb << 8) | lsb;
      if (checkCRC(raw, crc) != 0) {
        return false; // CRC mismatch
      }

      rawValue = raw;
      return true;
    }
  }

  return false; // Timeout
}

uint8_t Bonezegei_HTU21::checkCRC(uint16_t data, uint8_t crc) {
  uint32_t remainder = ((uint32_t)data << 8) | crc;
  uint32_t divisor = 0x988000; // Polynomial x^8 + x^5 + x^4 + 1 (0x131)

  for (int i = 0; i < 16; i++) {
    if (remainder & ((uint32_t)1 << (23 - i))) {
      remainder ^= divisor;
    }
    divisor >>= 1;
  }

  return (uint8_t)remainder;
}

void Bonezegei_HTU21::writeUserRegister(uint8_t value) {
  _wire->beginTransmission(_addr);
  _wire->write(HTU21_WRITE_USER_REG);
  _wire->write(value);
  _wire->endTransmission();
}

uint8_t Bonezegei_HTU21::readUserRegister() {
  _wire->beginTransmission(_addr);
  _wire->write(HTU21_READ_USER_REG);
  _wire->endTransmission();
  _wire->requestFrom(_addr, (uint8_t)1);
  return _wire->read();
}
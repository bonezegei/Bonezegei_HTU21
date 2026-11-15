/*
  Bonezegei_HTU21.h - Arduino Library for HTU21 / HTU21D Sensor
  Author: Jofel Batutay (Bonezegei)
  Date: November 15, 2026
*/

#ifndef BONEZEGEI_HTU21_H
#define BONEZEGEI_HTU21_H

#include <Arduino.h>
#include <Wire.h>

#define HTU21_I2C_ADDR            0x40

#define HTU21_TRIG_TEMP_HOLD      0xE3
#define HTU21_TRIG_HUM_HOLD       0xE5
#define HTU21_TRIG_TEMP_NOHOLD    0xF3
#define HTU21_TRIG_HUM_NOHOLD     0xF5
#define HTU21_WRITE_USER_REG      0xE6
#define HTU21_READ_USER_REG       0xE7
#define HTU21_SOFT_RESET          0xFE

class Bonezegei_HTU21 {
public:
  Bonezegei_HTU21(uint8_t addr = HTU21_I2C_ADDR);

  bool begin(TwoWire &wirePort = Wire);
  bool read();
  void reset();
  void setHeater(bool enable);

  float getTemperature();  // Returns Temperature in Celsius
  float getHumidity();     // Returns Relative Humidity in %
  float getDewPoint();     // Returns Dew Point in Celsius

private:
  uint8_t _addr;
  TwoWire *_wire;

  float _temperature;
  float _humidity;

  bool readSensor(uint8_t cmd, uint16_t &rawValue, uint8_t maxWaitMs);
  uint8_t checkCRC(uint16_t data, uint8_t crc);
  void writeUserRegister(uint8_t value);
  uint8_t readUserRegister();
};

#endif
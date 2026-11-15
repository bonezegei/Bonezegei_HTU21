/*
  Read All Sensor value
  Author: Jofel Batutay (Bonezegei)
  Date: November 15, 2026
*/
#include <Wire.h>
#include <Bonezegei_HTU21.h>

// Initialize HTU21 with default I2C address (0x40)
Bonezegei_HTU21 htu(0x40);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!htu.begin()) {
    Serial.println("Could not find a valid HTU21 sensor, check wiring!");
    while (1);
  }

  Serial.println("HTU21 Sensor Initialized!");
}

void loop() {
  if (htu.read()) {
    Serial.print("Temperature: ");
    Serial.print(htu.getTemperature());
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(htu.getHumidity());
    Serial.println(" %");

    Serial.print("Dew Point: ");
    Serial.print(htu.getDewPoint());
    Serial.println(" °C");

    Serial.println("------------------------------------");
  } else {
    Serial.println("Failed to read from HTU21 sensor!");
  }

  delay(2000);
}
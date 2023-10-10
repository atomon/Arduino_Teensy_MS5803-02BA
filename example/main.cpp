#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <MS5803.h>

// Chip Select pin for SPI
#define CS_PIN 10

// Use this constructor for SPI
MS5803 ms5803 = MS5803(CS_PIN);


void setup() {
  // Start the serial ports.
  Serial.begin(115200);
  ms5803.init();
  while(!Serial);
}

void loop() {
  ms5803.read();

  Serial.print("Pressure = ");
  Serial.print(ms5803.get_pressure(ms5803.hPa));
  Serial.print(ms5803.unit(ms5803.hPa));
  
  Serial.print("  depth = ");
  Serial.print(ms5803.get_depth(ms5803.mm));
  Serial.print(ms5803.unit(ms5803.mm));
  
  Serial.print("  delta press = ");
  Serial.print(ms5803.get_pressure() - ms5803.get_base_atm());
  Serial.print(" hPa");

  Serial.print("  Temperature = ");
  Serial.print(ms5803.get_temperature());
  Serial.println(" C");
  
  delay(100); 
}




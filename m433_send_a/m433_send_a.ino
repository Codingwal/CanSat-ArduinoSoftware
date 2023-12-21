#include <RCSwitch.h>
#include <Adafruit_BMP280.h>
#define BMP280_I2C_ADDRESS 0x76

Adafruit_BMP280 bmp;
RCSwitch mySwitch = RCSwitch();

void setup() {
  mySwitch.enableTransmit(10);  // Der Sender wird an Pin 10 angeschlossen
  bmp.begin(BMP280_I2C_ADDRESS);

  Serial.begin(9600);
}
void loop() {
  float temp = bmp.readTemperature() * 100;
  float pres = bmp.readPressure() * 100;
  mySwitch.send((1UL << 24) + temp, 32);
  mySwitch.send((2UL << 24) + pres, 32);

  delay(1000);
}
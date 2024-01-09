#include <RCSwitch.h>
#include <Adafruit_BMP280.h>
#define BMP280_I2C_ADDRESS 0x76

Adafruit_BMP280 bmp;
RCSwitch mySwitch = RCSwitch();

void setup() {
  mySwitch.enableTransmit(10);  // Der Sender wird an Pin 10 angeschlossen

  Serial.begin(9600);
}
void loop() {

  mySwitch.send("Hello, World");

  delay(1000);
}

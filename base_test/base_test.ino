#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP280_I2C_ADDRESS 0x76

#define DATA_BLOCK_START 0xFFFFFF  // maximaler 32b Wert bzw. nur noch 6 F
#define DATA_BLOCK_END 0xFFFFFE    // maximaler 32b Wert minus 1, bzw. nur noch 5 F und ein E

Adafruit_BMP280 bmp;

int counter = 0;

void setup() {
  Serial.begin(9600);

  if (!bmp.begin(BMP280_I2C_ADDRESS)) { // Init BMP (Luftdruck & Temperatur)
    Serial.println(501);
  }
}

void loop() {
  send(DATA_BLOCK_START);
  send(counter);
  send(bmp.readTemperature());
  send(bmp.readPressure());
  send(0.000);
  send(0.000);
  send(0.000);
  send(0.000);
  send(0.000);
  send(0.000);
  send(0.000);
  send(0.000);
  send(0.000);
  send(0.000);
  send(DATA_BLOCK_END);

  counter++;

  delay(700);
}

void send(float x) {
  Serial.println(x);
}

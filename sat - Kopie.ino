#include <RCSwitch.h>
#include <Adafruit_BMP280.h>
#define BMP280_I2C_ADDRESS 0x76

Adafruit_BMP280 bmp;
RCSwitch mySwitch = RCSwitch();

float groundalti; // Höhe beim Start
float temp;
float pres;
float alti;

void setup() {
  mySwitch.enableTransmit(10);  // Der Sender wird an Pin 10 angeschlossen
  bmp.begin(BMP280_I2C_ADDRESS);

  Serial.begin(9600);

  presGroundalti = bmp.readAltitude(); // Höhe beim einschalten (Luftdruck)
}
void loop() {
  temp = bmp.readTemperature() * 100;
  pres = bmp.readPressure() * 100;

  presAlti = bmp.readAltitude();
  float presHeight = presAlti - presGroundalti; // Höhe mit Luftdruck in Metern, relativ zur Höhe beim Start
  Serial.println(presHeight);

  // Platz für Höhenmessung mit der Intertialplatform und dem GPS

  if (presHeight < 50) { // Beispielwert
    // Schalte den PC-Lüfter an (für den Airbag)
  }


  sendData();
  delay(250);
}

void sendData() {
  mySwitch.send((1UL << 24) + temp, 32);  // Temperatur senden, fängt mit 1 an
  mySwitch.send((2UL << 24) + pres, 32);  // Druck senden, fängt mit 2 an
}

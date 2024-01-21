// #include <stdfloat.h>

#include <RCSwitch.h>
#include <Adafruit_BMP280.h>
#define BMP280_I2C_ADDRESS 0x76

Adafruit_BMP280 bmp;
RCSwitch mySwitch = RCSwitch();

using namespace std;

void setup() {
  mySwitch.enableTransmit(10);  // Der Sender wird an Pin 10 angeschlossen

  bmp.begin(2);

  Serial.begin(9600);
}
void loop() {
  float temp = getTemp();
  float pres = getPres();

  temp = 40.5f;

  Serial.println(temp);

  // Ersten Datenblock (Beispiel) übertragen und 1s warten
  sendData(1, 0xFFFF & (int)temp);
  delay(1000);
  sendData(2, (int)temp >> 16);
  delay(1000);
}

void sendData(unsigned char head, unsigned long data) {
  // Kopfzeile als oberen Byte mit den unteren 3 Bytes der Daten kombinieren und als 4 Byte Block senden
  mySwitch.send(((unsigned long)head << 24) + (data & 0xFFFFFF), 32);
}

float getTemp() {
  float temp = bmp.readTemperature();
  return temp;
}
float getPres() {
  float pres = bmp.readPressure();
  return pres;
}
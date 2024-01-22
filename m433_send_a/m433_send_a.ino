// #include <stdfloat.h>

#include <RCSwitch.h>
#include <Adafruit_BMP280.h>
#define BMP280_I2C_ADDRESS 0x76

Adafruit_BMP280 bmp;
RCSwitch mySwitch = RCSwitch();

using namespace std;

void setup() {
  mySwitch.enableTransmit(10);  // Der Sender wird an Pin 10 angeschlossen

  bmp.begin(BMP280_I2C_ADDRESS);

  Serial.begin(9600);
}
void loop() {
  float temp = getTemp();
  float pres = getPres();

  // Ersten Datenblock (Beispiel) übertragen und 1s warten
  // Maximal zwei Nachkommastellen (x100), bei 3 (x1000) wird z.B. 40.5 zu 32.77
  sendData(1, (int)(temp * 100));
  sendData(2, (int)(pres / 100));
  delay(1000);
}

void sendData(unsigned char head, unsigned long data) {
  // Kopfzeile als oberen Byte mit den unteren 3 Bytes der Daten kombinieren und als 4 Byte Block senden
  mySwitch.send(((unsigned long)head << 24) + (data & 0xFFFFFF), 32);
}

float getTemp() {
  float temp = bmp.readTemperature();
  Serial.println("Temperature: " + String(temp));
  return temp;
}
float getPres() {
  float pres = bmp.readPressure();
  Serial.println("Pressure: " + String(pres));
  return pres;
}
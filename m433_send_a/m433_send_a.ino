#include <RCSwitch.h>
#include <Adafruit_BMP280.h>
#define BMP280_I2C_ADDRESS 0x76

Adafruit_BMP280 bmp;
RCSwitch mySwitch = RCSwitch();

void setup() {
  mySwitch.enableTransmit(10);  // Der Sender wird an Pin 10 angeschlossen

  bmp.begin(2);

  Serial.begin(9600);
}
void loop() {
  // Ersten Datenblock (Beispiel) übertragen und 1s warten
  sendData(100, 0);
  delay(1000);

  // Zweiten Datenblock (Beispiel) übertragen und 1s warten
  sendData(999, 1);
  delay(1000);
}

void sendData(unsigned long data, unsigned char head)
{
  // Kopfzeile als oberen Byte mit den unteren 3 Bytes der Daten kombinieren und als 4 Byte Block senden
  mySwitch.send(((unsigned long)head << 24) + (data & 0xFFFFFF), 32);
}
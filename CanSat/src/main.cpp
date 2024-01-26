#include <RCSwitch.h>
#include <Adafruit_BMP280.h>
#include "myVector.h"

#define BMP280_I2C_ADDRESS 0x76

Adafruit_BMP280 bmp;
RCSwitch mySwitch = RCSwitch();

using namespace std;

// Eine Liste in der alle gemessenen Temperaturen gespeichert werden
// Von Florian programmiert, nachdem das aktuell verwendete Array voll ist wird ein doppelt
// so großes erstellt, Startgröße ist in dem Fall 5 (5->10->20->40->...)
// Source Code ist unter ../CanSat/lib/vector/myVector.h
Vector<float> tempVec = Vector<float>(5);
Vector<float> presVec = Vector<float>(5);

void sendData(unsigned char head, unsigned long data)
{
  // Kopfzeile als oberen Byte mit den unteren 3 Bytes der Daten kombinieren und als 4 Byte Block senden
  mySwitch.send(((unsigned long)head << 24) + (data & 0xFFFFFF), 32);
}

float getTemp()
{
  float temp = bmp.readTemperature();
  return temp;
}
float getPres()
{
  float pres = bmp.readPressure();
  return pres;
}

void setup()
{
  mySwitch.enableTransmit(10); // Der Sender wird an Pin 10 angeschlossen

  bmp.begin(BMP280_I2C_ADDRESS);

  Serial.begin(9600);
}
void loop()
{
  float temp = getTemp();
  float pres = getPres();

  tempVec.pushBack(temp);
  presVec.pushBack(pres);

  // DEBUG: Jede 10 Iterationen werden alle gespeicherten Temperaturen ausgegeben
  if (tempVec.size() % 10 == 0)
  {
    Serial.println("---Temperature------------");
    for (size_t i = 0; i < tempVec.size(); i++)
    {
      Serial.println(tempVec[i]);
    }
  }

  // DEBUG: Jede 10 Iterationen werden alle gespeicherten Druckmesswerte ausgegeben
  if (presVec.size() % 10 == 0)
  {
    Serial.println("---Pressure---------------");
    for (size_t i = 0; i < presVec.size(); i++)
    {
      Serial.println(presVec[i]);
    }
    Serial.println("--------------------------");
  }

  // Ersten Datenblock (Beispiel) übertragen und 1s warten
  // Maximal zwei Nachkommastellen (x100), bei 3 (x1000) wird z.B. 40.5 zu 32.77
  sendData(1, (int)(temp * 100));
  sendData(2, (int)(pres / 100));
  delay(1000);
}
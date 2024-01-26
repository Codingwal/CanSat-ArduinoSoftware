#include <RCSwitch.h>
#include "myVector.h"

RCSwitch mySwitch = RCSwitch();

unsigned long data;
unsigned char head;
unsigned char prevHead = -1;

// Eine Liste in der alle gemessenen Temperaturen gespeichert werden
// Von Florian programmiert, nachdem das aktuell verwendete Array voll ist wird ein doppelt
// so großes erstellt, Startgröße ist in dem Fall 5 (5->10->20->40->...)
// Source Code ist unter ../CanSat/lib/vector/myVector.h
Vector<float> tempVec = Vector<float>(5);
Vector<float> presVec = Vector<float>(5);

void setup()
{
  Serial.begin(9600);
  mySwitch.enableReceive(0); // Empfänger ist an Interrupt-Pin "0" - Das ist am UNO der Pin2
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
}

bool receiveData()
{
  // Auf verfügbares Signal prüfen
  if (!mySwitch.available())
  {
    return false;
  }

  // Empfangene Daten lesen
  const unsigned long msg = mySwitch.getReceivedValue();

  // Status der Library ob Daten angekommen sind zurücksetzen
  mySwitch.resetAvailable();

  // Bei invalidem Signal abbrechen
  if (msg == 0)
  {
    return false;
  }

  // Datenblock und Kopfzeile trennen und aktualisieren
  data = msg & 0xFFFFFF;
  head = msg >> 24;

  // Jede Information nur einmal lesen bis etwas anderes gesendet wurde
  if (head == prevHead)
  {
    return false;
  }
  prevHead = head;

  // Erfolgreich Daten empfangen
  return true;
}

float temp = -1;
float pres = -1;

void loop()
{
  // Wenn nicht erfolgreich Daten empfangen wurden, abbrechen
  if (!receiveData())
  {
    return;
  }

  // Temperatur wurde gesendet
  if (head == 1)
  {
    temp = (float)data / 100;
    tempVec.pushBack(temp);
    // Serial.println("Temperature: " + String(temp));
  }

  // Druck wurde gesendet
  if (head == 2)
  {
    pres = (float)data * 100;
    presVec.pushBack(pres);
    // Serial.println("Pressure: " + String(pres));
  }

  // DEBUG: Jede 10 Iterationen werden alle gespeicherten Temperaturen und Druckwerte ausgegeben
  if (tempVec.size() % 10 == 0 && presVec.size() % 10 == 0)
  {
    Serial.println("---Temperature------------");
    for (size_t i = 0; i < tempVec.size(); i++)
    {
      Serial.println(tempVec[i]);
    }
    Serial.println("---Pressure---------------");
    for (size_t i = 0; i < presVec.size(); i++)
    {
      Serial.println(presVec[i]);
    }
    Serial.println("--------------------------");
  }
}
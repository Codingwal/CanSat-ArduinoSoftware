#include <RCSwitch.h>
#include "dataSystem.h"

RCSwitch mySwitch = RCSwitch();

unsigned long data;
unsigned long prevData = -1;

DataBlock dataBlock = DataBlock();
int index = -1;

bool receiveData()
{
  // Auf verfügbares Signal prüfen
  if (!mySwitch.available())
  {
    return false;
  }

  // Empfangene Daten lesen
  data = mySwitch.getReceivedValue();

  // Status der Library ob Daten angekommen sind zurücksetzen
  mySwitch.resetAvailable();

  // Jede Information nur einmal lesen bis etwas anderes gesendet wurde
  if (data == prevData)
  {
    return false;
  }
  prevData = data;

  // Erfolgreich Daten empfangen
  return true;
}
float longToFloat(unsigned long data)
{
  float value;
  memcpy(&value, &data, 4);
  return value;
}
void writeVector3(Vector3 vector)
{
  Serial.println(vector.x);
  Serial.println(vector.y);
  Serial.println(vector.z);
}

void writeDataBlock(DataBlock data)
{
  Serial.println(data.temperature);
  Serial.println(data.pressure);
  writeVector3(data.acceleration);
  writeVector3(data.position);
  Serial.println("---------------------");
}

void getDataBlock()
{
  if (data == DATA_BLOCK_START)
  {
    if (index != -1)
    {
      writeDataBlock(dataBlock);
      dataBlock = DataBlock();
    }
    index = 0;
    return;
  }

  if (data == DATA_BLOCK_END)
  {
    writeDataBlock(dataBlock);
    dataBlock = DataBlock();

    index = -1;
    return;
  }

  switch (index)
  {
  case 0:
    dataBlock.temperature = longToFloat(data);
    break;
  case 1:
    dataBlock.pressure = longToFloat(data);
    break;
  case 2:
    dataBlock.acceleration.x = longToFloat(data);
    break;
  case 3:
    dataBlock.acceleration.y = longToFloat(data);
    break;
  case 4:
    dataBlock.acceleration.z = longToFloat(data);
    break;
  case 5:
    dataBlock.position.x = longToFloat(data);
    break;
  case 6:
    dataBlock.position.y = longToFloat(data);
    break;
  case 7:
    dataBlock.position.z = longToFloat(data);
    break;
  default:
    dataBlock = DataBlock();
    index = -1;
    break;
  }
  index++;
}

void setup()
{
  Serial.begin(9600);
  mySwitch.enableReceive(0); // Empfänger ist an Interrupt-Pin "0" - Das ist am UNO der Pin2
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
}
void loop()
{
  // Wenn nicht erfolgreich Daten empfangen wurden, abbrechen
  if (!receiveData())
  {
    return;
  }

  getDataBlock();
}
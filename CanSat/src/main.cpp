#include <RCSwitch.h>
#include <Adafruit_BMP280.h>
#include <SD.h>
#include <SPI.h>

#include "dataSystem.h"

#define BMP280_I2C_ADDRESS 0x76

Adafruit_BMP280 bmp;
RCSwitch mySwitch = RCSwitch();

File myFile;

// Die bits der float werden mit RCSwitch gesendet
void sendFloat(float value)
{
  // Ändert den Typ von float zu long ohne bits zu ändern
  unsigned long data;
  memcpy(&data, &value, 4);

  mySwitch.send(data, 32);
}
// Sendet einen Vector3 (in dataSystem.h definiert) mithilfe der sendFloat Funktion
void sendVector3(Vector3 vector)
{
  sendFloat(vector.x);
  sendFloat(vector.y);
  sendFloat(vector.z);
}
// Sendet alle Daten als einen Datenblock mit den obigen Sendefunktionen
void sendDataBlock(DataBlock data)
{
  mySwitch.send(DATA_BLOCK_START, 32);
  sendFloat(data.temperature);
  sendFloat(data.pressure);
  sendVector3(data.acceleration);
  sendVector3(data.position);
  mySwitch.send(DATA_BLOCK_END, 32);
}
// Schreibt einen Vector3 (in dataSystem.h definiert) an den angeschlossenen Computer (zum debuggen)
void writeVector3(Vector3 vector)
{
  // myFile.print(String(vector.x) + ", " + String(vector.y) + ", " + String(vector.z));

  Serial.println(vector.x);
  Serial.println(vector.y);
  Serial.println(vector.z);
}
// Schreibt alle Daten als einen Datenblock an den angeschlossenen Computer (zum debuggen)
void writeDataBlock(DataBlock data)
{
  // myFile.print(String(data.temperature) + ", " + String(data.pressure) + ", ");
  // writeVector3(data.acceleration);
  // myFile.print(", ");
  // writeVector3(data.position);
  // myFile.println();

  Serial.println(data.temperature);
  Serial.println(data.pressure);
  writeVector3(data.acceleration);
  writeVector3(data.position);
  Serial.println("---------------------");
}

// Liest die Sensordaten mithilfe der Adafruit_BMP Bibiothek
float getTemperature()
{
  return bmp.readTemperature();
}
float getPressure()
{
  return bmp.readPressure();
}

void setup()
{
  mySwitch.enableTransmit(10); // Der Sender wird an Pin 10 angeschlossen

  bmp.begin(BMP280_I2C_ADDRESS);

  Serial.begin(9600);
}
void loop()
{
  // Daten werden gelesen und gespeichert
  DataBlock data = DataBlock();
  data.temperature = getTemperature();
  data.pressure = getPressure();

  // Daten werden an die Bodenstation gesendet
  sendDataBlock(data);

  // Daten werden an den Computer gesendet
  writeDataBlock(data);

  // Kurze Pause
  delay(100);
}

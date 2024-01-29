#include <RCSwitch.h>
#include <Adafruit_BMP280.h>
#include <SD.h>
#include <SPI.h>

#include "dataSystem.h"

#define BMP280_I2C_ADDRESS 0x76

Adafruit_BMP280 bmp;
RCSwitch mySwitch = RCSwitch();

File myFile;

void sendFloat(float value)
{
  unsigned long data;
  memcpy(&data, &value, 4);

  mySwitch.send(data, 32);
}
void sendVector3(Vector3 vector)
{
  sendFloat(vector.x);
  sendFloat(vector.y);
  sendFloat(vector.z);
}
void sendDataBlock(DataBlock data)
{
  mySwitch.send(DATA_BLOCK_START, 32);
  sendFloat(data.temperature);
  sendFloat(data.pressure);
  sendVector3(data.acceleration);
  sendVector3(data.position);
  mySwitch.send(DATA_BLOCK_END, 32);
}
void writeVector3(Vector3 vector)
{
  // myFile.print(String(vector.x) + ", " + String(vector.y) + ", " + String(vector.z));

  Serial.println(vector.x);
  Serial.println(vector.y);
  Serial.println(vector.z);
}
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
  DataBlock data = DataBlock();
  data.temperature = getTemperature();
  data.pressure = getPressure();

  sendDataBlock(data);
  writeDataBlock(data);
  delay(100);
}

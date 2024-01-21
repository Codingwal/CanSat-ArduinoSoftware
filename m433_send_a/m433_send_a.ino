#include <RCSwitch.h>
#include <Adafruit_BMP280.h>
#define BMP280_I2C_ADDRESS 0x76

Adafruit_BMP280 bmp;
RCSwitch mySwitch = RCSwitch();

float temp_arr[1000];
float pres_arr[1000];

int i = 0;

void setup() {
  mySwitch.enableTransmit(10);  // Der Sender wird an Pin 10 angeschlossen

  bmp.begin(2);

  Serial.begin(9600);
}
void loop() {
  float temp = getTemp();
  temp_arr[i] = temp;

  sendData(100, 0);
  delay(1000);
  sendData(999, 1);

  delay(1000);
}

void sendData(unsigned long data, unsigned char head)
{
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
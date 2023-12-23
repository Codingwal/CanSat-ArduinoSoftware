#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Empfänger ist an Interrupt-Pin "0" - Das ist am UNO der Pin2
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
}

void loop() {
  if (!mySwitch.available())
  {
    return;
  }
  //unsigned long data = mySwitch.getReceivedValue();

  unsigned long msg = mySwitch.getReceivedValue();

  uint8_t header = (uint8_t)(msg >> 24); // Get top Byte
  float data = (msg & 0xFFFFFF); // Get bottom 3 Bytes

  if (header == 1) {
    Serial.print("Temperatur: ");
    Serial.print(data / 100);
    Serial.println("°C");
  }

  if (header == 2) {
    Serial.print("Druck: ");
    Serial.print(data / 100);
    Serial.println("Pa");
  }

  mySwitch.resetAvailable();
}

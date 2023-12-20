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
  unsigned long data = mySwitch.getReceivedValue();

  Serial.print(data >> 32); // Get top 4 Bytes
  Serial.print(": ");
  Serial.println(data & 0xFFFFFFFF); // Get bottom 4 Bytes

  mySwitch.resetAvailable();
}
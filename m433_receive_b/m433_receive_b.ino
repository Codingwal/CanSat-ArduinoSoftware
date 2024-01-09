#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Empfänger ist an Interrupt-Pin "0" - Das ist am UNO der Pin2
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
}

void loop() {
  if (!mySwitch.available()) {
    return;
  }
  const unsigned int *msg = mySwitch.getReceivedRawdata();

  print((char *)msg, mySwitch.getReceivedBitlength());

  mySwitch.resetAvailable();
}

void print(const char *msg, unsigned int size) {
  for (int i = 0; i < size; i += 8) {
    Serial.print(msg[i]);
  }
}

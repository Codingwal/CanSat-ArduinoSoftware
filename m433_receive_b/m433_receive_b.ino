#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

unsigned long data;
unsigned long prevData;
unsigned char head;
unsigned char prevHead = -1;

char prevChar;

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Empfänger ist an Interrupt-Pin "0" - Das ist am UNO der Pin2
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
}

bool receiveData() {
  if (!mySwitch.available()) {
    return false;
  }

  const unsigned long msg = mySwitch.getReceivedValue();
  data = msg & 0xFFFFFF;
  head = msg >> 24;

  if (head == prevHead) {
    return false;
  }
  prevHead = head;
  prevData = data;

  return true;
}      

void loop() {
  if (!receiveData()) {
    return;
  }
   
  // Serial.println(head);
  Serial.println(head);
  Serial.println(data);
  Serial.println();

  mySwitch.resetAvailable();
}

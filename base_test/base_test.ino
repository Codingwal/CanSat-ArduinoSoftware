#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

#define RX 2
#define TX 3

TinyGPSPlus gps;
SoftwareSerial ss(RX, TX);

void setup() {
  Serial.begin(9600);
  ss.begin(9600);
}

void loop() {
  while (ss.available()) {
    gps.encode(ss.read());
  }
  send(gps.location.lng());
  send(gps.location.lng());
  send(gps.altitude.meters());

  delay(1000);
}

void send(float x) {
  Serial.println(x);
}

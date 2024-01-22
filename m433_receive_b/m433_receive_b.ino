#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

unsigned long data;
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
  // Auf verfügbares Signal prüfen
  if (!mySwitch.available()) {
    return false;
  }

  // Empfangene Daten lesen
  const unsigned long msg = mySwitch.getReceivedValue();

  // Status der Library ob Daten angekommen sind zurücksetzen
  mySwitch.resetAvailable();

  // Bei invalidem Signal abbrechen
  if (msg == 0) {
    return false;
  }

  // Datenblock und Kopfzeile trennen und aktualisieren
  data = msg & 0xFFFFFF;
  head = msg >> 24;

  // Jede Information nur einmal lesen bis etwas anderes gesendet wurde
  if (head == prevHead) {
    return false;
  }
  prevHead = head;

  // Erfolgreich Daten empfangen
  return true;
}

float temp = -1;
float pres = -1;

void loop() {
  // Wenn nicht erfolgreich Daten empfangen wurden, abbrechen
  if (!receiveData()) {
    return;
  }

  // Temperatur wurde gesendet
  if (head == 1) {
    temp = (float)data / 100;
    Serial.println("Temperature: " + String(temp));
  }

  // Druck wurde gesendet
  if (head == 2) {
    pres = (float)data * 100;
    Serial.println("Pressure: " + String(pres));
  }
}

#include <SPI.h>
#include <SD.h>

#define SD_CS_PIN 10
#define FILEPATH "data.txt"

File file;

void setup() {
  Serial.begin(9600);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(504);
  }
}

void loop() {
  String input = Serial.readString();
  if (input == "del") {
    file = SD.open(FILEPATH);
    if (file) {
      while (file.available()) {
        Serial.write(file.read());
      }
      file.close();
    }
  }
}

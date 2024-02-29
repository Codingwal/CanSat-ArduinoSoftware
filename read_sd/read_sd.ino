#include <SPI.h>
#include <SD.h>

#define SD_CS_PIN 10

File file;

void setup() {
  Serial.begin(9600);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(504);
  }

  Serial.println("Files: ");
  int counter = 0;
  while (SD.exists(String(counter))) {
    //Serial.println(counter);
    counter++;
  }
  Serial.print("Select file to open: 0-");
  Serial.println(counter - 1);
}

void loop() {
  while (!Serial.available()) {
    delay(10);
  }

  String input = Serial.readString();
  input.trim();
  if (!SD.exists(input)) {
    Serial.print("File not found: ");
    Serial.println(input);
  } else {
    file = SD.open(input, FILE_READ);
    if (file) {
      while (file.available()) {
        Serial.write(file.read());
        file.close(); // Schließt die Datei, war eine mögliche Fehlerquelle
      }
      file.close();
    } else {
      Serial.println("Can't open file!");
    }
  }
}

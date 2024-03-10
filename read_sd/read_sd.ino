 #include <SPI.h>
#include <SD.h>

#define SD_CS_PIN 10
#define SPEAKER_PIN 8

#include "pitches.h"

File file;
int melody[] = {261, 293, 329, 349, 391, 391, 440, 440, 440, 440, 391, 440, 440, 440, 440, 391, 349, 349, 349, 349, 329, 329, 391, 391, 391, 391,  261};
int durations[] = {250, 250, 250, 250, 500, 500, 250, 250, 250,  250, 500, 250, 250, 250, 250, 500, 250, 250, 250, 250, 500, 500, 250, 250, 250,  250, 500, 500};
int melodysize = sizeof(melody) / sizeof(melody[0]);
int duration = 500;
long time; // ago...
long lasttone;
int m = 0;

void setup() {
  //Serial.begin(9600);
  Serial.begin(115200);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(504);
  }

  Serial.println("Files: ");
  int counter = 0;
  while (SD.exists(String(counter))) {
    Serial.print(counter);
    Serial.print(" ");
    counter++;
  }
  Serial.print("Select file to open");

  time = millis();
  lasttone = time;
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

        time = millis();
        if (time - lasttone > durations[(m - 1) % melodysize]) {
          //tone(SPEAKER_PIN, melody[m % melodysize], durations[m % melodysize]);
          m++;
          lasttone = time;
        }
      }
      file.close();
    } else {
      Serial.println("Can't open file!");
    }
  }
}

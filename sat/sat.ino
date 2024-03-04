#include <Adafruit_BMP280.h>
#include <Adafruit_BNO055.h>
#include <SoftwareSerial.h>
//#include <TinyGPSPlus.h>
//#include <TinyGPSMinus.h>
#include <RH_RF95.h>
#include <SdFat.h>

// SPI Pins Arduino Nano: CS 10, MOSI 11, MISO 12, SCK 13
// https://www.arduino.cc/reference/en/language/functions/communication/spi/
// https://funduino.de/nr-28-das-sd-karten-modul

// I2C Adressen
#define BMP280_I2C_ADDRESS 0x76
#define BNO055_I2C_ADDRESS 0x28

// PIN Belegungen
#define GPS_TX_PIN 2 // Es beleiben PIN D2 und D3 für das GPS-Modul übrig
#define GPS_RX_PIN 3
#define FAN_PIN 4
#define LORA_RX_PIN 5
#define LORA_TX_PIN 6
#define SPEAKER_PIN 8
#define LED_PIN 9
#define SD_CS_PIN 10
#define VOLTAGE_PIN A0

// Fehlercodes
#define ERROR_BMP 501
#define ERROR_BNO 502
#define ERROR_RF95 503
#define ERROR_SD_CONNECT 504
#define ERROR_SD_OPEN 505

// Kontrollwerte vor und nach der Datenübertragung
#define DATA_BLOCK_START 0xFFFFFF  // maximaler 32b Wert bzw. nur noch 6 F
#define DATA_BLOCK_END 0xFFFFFE    // maximaler 32b Wert minus 1, bzw. nur noch 5 F und ein E

#define FREQUENCY 433.0  // Datenübertragungsfrequenz (in MHz)

#define STARTALTITUDE 30  // Höhe vor dem Start vom Flugplatz

#define TOLERANCE 5 // Toleranz in Metern, um Höhenunterschiede mit dem BMP zu messen
#define FAN_STARTING_HEIGHT 400 // Ab welcher Höhe der Lüfter den Airbag aufblässt
#define BMP_ALTITUDES_SIZE 8 // Wie viele letzte Temperaturen gespeichert werden sollen

Adafruit_BMP280 bmp;
Adafruit_BNO055 bno(55, BNO055_I2C_ADDRESS, &Wire);
// TinyGPSPlus gps;
//TinyGPSMinus gps;
SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);
SoftwareSerial rf(LORA_RX_PIN, LORA_TX_PIN);
RH_RF95 rf95(rf);

float sealevelhpa;
short bmp_altitude = STARTALTITUDE;
short bmp_altitudes[BMP_ALTITUDES_SIZE];

bool fan = false;
bool ejected = false;
bool landed = false;

int counter = 0;

long time;

SdFat32 SD;
File32 file;

void setup() {
  Serial.begin(9600);
  Serial.println(100);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  if (!bmp.begin(BMP280_I2C_ADDRESS)) { // Init BMP (Luftdruck & Temperatur)
    error(ERROR_BMP);
  }
  sealevelhpa = bmp.seaLevelForAltitude(STARTALTITUDE, bmp.readPressure());

  if (!bno.begin()) { // Init BNO (Inertialplatform)
    error(ERROR_BNO);
  }
  if (!rf95.init()) { // RF95 (Funkmodul) initialisieren
    error(ERROR_RF95);
  }
  rf95.setFrequency(FREQUENCY);

  ss.begin(9600); // GPS

  if (!SD.begin(SD_CS_PIN)) { // Init SD (Speicher)
    error(ERROR_SD_CONNECT);
  }
  {
    // Datei mit höchstem Wert als Namen finden und dann mit Wert + 1 als Namen eine Datei erstellen
    // So muss nicht nach jedem Test die SD Karte geleert werden, sondern die Dateien sind chronologisch sortiert
    int filecounter = 0;
    while (SD.exists(String(filecounter))) {
      filecounter++;
    }
    file = SD.open(String(filecounter), FILE_WRITE);
    if (file) {
      file.println(""); // Irgendetwas in die erste Zeile geschrieben werden, damit die Zahlen gespeichert werden können
    } else {
      error(ERROR_SD_OPEN);
    }
  }

  Serial.println(200);
  digitalWrite(LED_PIN, HIGH); // LED an
  time = millis();

  // Nur bei genügend Speicher einbauen
  /*  tone(SPEAKER_PIN, 200);
      delay(100);
      tone(SPEAKER_PIN, 400);
      delay(100);
      tone(SPEAKER_PIN, 600);
      delay(100);
      noTone(SPEAKER_PIN);*/
}

void loop() {
  // Sendet einen Datenblock, die Unterfunktionen senden die jeweiligen Werte selber
  // Am Anfang und am Ende wird ein Kontrollwert gesendet
  send(DATA_BLOCK_START);
  send(counter);

  { // BMP
    //float pressure = bmp.readPressure();

    bmp_altitude = 44300 * (1 - (pow(bmp.readPressure() / sealevelhpa, 0.1903)));

    send(bmp.readTemperature());  // Temperatur senden
    send(bmp.readPressure());
  }

  { // BNO
    sensors_event_t accelerometer, gyroscope;
    bno.getEvent(&accelerometer, Adafruit_BNO055::VECTOR_LINEARACCEL);  // Acceleration - Gravity
    send(accelerometer.acceleration.x);
    send(accelerometer.acceleration.y);
    send(accelerometer.acceleration.z);

    bno.getEvent(&gyroscope, Adafruit_BNO055::VECTOR_EULER);
    send(gyroscope.gyro.x);
    send(gyroscope.gyro.y);
    send(gyroscope.gyro.z);
  }

  { // GPS
    if (!landed) {
      char* gprmcString[128];
      byte index = 0;
      while (ss.available()) {
        char c = ss.read();
        if (c == '\n') {
          gprmcString[index] = '\0'; // Nullterminator setzen, um den String zu beenden
          index = 0;
          /*send(gpscoord2float(gps.get_latitude(), 6));
            send(gpscoord2float(gps.get_longitude(), 7));
            send(gps.f_altitude());*/
          double latitude, longitude, altitude;

          // Extrahieren von Koordinaten aus dem GPRMC-String
          extractCoordinates(gprmcString, &latitude, &longitude, &altitude);
        } else {
          // Hinzufügen des Zeichens zum GPRMC-String
          gprmcString[index++] = c;
        }
      }
    }

    { // Fumktion zum schauen, ob ausgeworfen oder gelandet
      short x = bmp_altitudes[counter % BMP_ALTITUDES_SIZE] - TOLERANCE;
      if (counter > BMP_ALTITUDES_SIZE) { // Erst mit Vergleichen beginnen, wenn die Liste an Vergleichswerten voll ist
        if (x > bmp_altitude) { // Falls die Höhe fällt
          ejected = true;
        } else if (x < bmp_altitude) { // Falls die Höhe steigt oder eher gleichbleit, dafür ist die Toleranz da
          if (ejected == true) { // Falls schon ausgeworfen, muss also gelandet sein :)
            landed = true;
          }
        }
      }
    }

    // Serial.println(analogRead(VOLTAGE_PIN) / 1023 * 5 * 2); // Spannungsüberwachung

    if (ejected) {// Falls ausgeworfen, piept der akustische Signalgeber
      tone(SPEAKER_PIN, 1000, 500); // 0.5 Sekunden den Ton abspielen
    }

    if (bmp_altitude < FAN_STARTING_HEIGHT && ejected) {
      fan = true;
      digitalWrite(FAN_PIN, HIGH);
    }

    if (landed) { // Falls geladet, Lüfter ausschalten
      digitalWrite(FAN_PIN, LOW);
    }

    send((fan << 0) + (ejected << 1) + (landed << 2));  // Sendet alle Bools in einem Byte


    file.println(millis() - time, DEC);
    time = millis(); // Zeit zurücksetzten, um jedesmal die Zeit, die die Schleife (loop()) gebraucht hat zu bestimmen

    send(DATA_BLOCK_END);

    bmp_altitudes[counter % BMP_ALTITUDES_SIZE] = bmp_altitude; // Aktueller Höhenmeter-Wert speichern

    counter++;
  }
}

void send(float val) {
  {
    uint8_t tosend[sizeof(float)]; // Ändert den Typ von float zu uint8_t[], ohne tatsächlich Bits zu modifizieren
    memcpy(&tosend, &val, sizeof(float));
    rf95.send(tosend, sizeof(float));
  }
  {
    file.println(val, DEC);
    file.flush(); // Tatsächlich Speichern, wäre ansonsten evtl. nur im Buffer, was zu Fehlern führen kann :(
  }
}

void error(int errorCode) {
  Serial.println(errorCode); // Fehler code zum Debuggen an den Laptop schicken

  // Dauerhaftes Fehler-Piepen und blinkende LED
  byte countdown = 60; // 60 Sekunden = 1 Minute
  while (countdown > 0) {
    digitalWrite(LED_PIN, HIGH);
    tone(SPEAKER_PIN, 1000, 500);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);

    countdown--;
  }
}

float gpscoord2float(char* x, byte length) {
  float result;
  for (int i = 0; i < length; i++) {
    result += char2float(x[i]) * pow(10, i);
  }
}

float char2float(char x) {
  switch (x) {
    case 48: return 0;
    case 49: return 1;
    case 50: return 2;
    case 51: return 3;
    case 52: return 4;
    case 53: return 5;
    case 54: return 6;
    case 55: return 7;
    case 56: return 8;
    case 57: return 9;
    default: 0;
  }
}


// By ChatGPT
bool extractCoordinates(const char* gprmcString, double & latitude, double & longitude, double & altitude) {
  char* token;
  char* copy = strdup(gprmcString); // Kopie des Strings erstellen

  // Erstes Komma überspringen
  token = strtok(copy, ",");
  for (int i = 0; i < 3; i++) {
    token = strtok(NULL, ",");
    if (token == NULL) {
      free(copy);
      return false;
    }
  }

  // Breitengrad extrahieren
  token = strtok(NULL, ",");
  if (token == NULL) {
    free(copy);
    return false;
  }
  latitude = atof(token);

  // N/S-Indikator überspringen
  token = strtok(NULL, ",");
  if (token == NULL) {
    free(copy);
    return false;
  }

  // Längengrad extrahieren
  token = strtok(NULL, ",");
  if (token == NULL) {
    free(copy);
    return false;
  }
  longitude = atof(token);

  // E/W-Indikator überspringen
  token = strtok(NULL, ",");
  if (token == NULL) {
    free(copy);
    return false;
  }

  // Überspringen der restlichen GPRMC-Felder
  for (int i = 0; i < 6; i++) {
    token = strtok(NULL, ",");
    if (token == NULL) {
      free(copy);
      return false;
    }
  }

  // Höhe extrahieren
  token = strtok(NULL, ",");
  if (token == NULL) {
    free(copy);
    return false;
  }
  altitude = atof(token);

  free(copy);
  return true;
}

// Delta T in der Datei speichern!

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BNO055.h>
#include <SoftwareSerial.h>
//#include <TinyGPS.h>
//#include <TinyGPSPlus.h>
#include <RH_RF95.h>
#include <SD.h>

/* SPI Pins Arduino Nano
  CS 10
  MOSI 11
  MISO 12
  SCK 13
  https://www.arduino.cc/reference/en/language/functions/communication/spi/
  https://funduino.de/nr-28-das-sd-karten-modul
*/


// I2C Adressen
#define BMP280_I2C_ADDRESS 0x76
#define BNO055_I2C_ADDRESS 0x28

// PIN Belegungen
#define FAN_PIN 4
#define GPS_RX_PIN 00
#define GPS_TX_PIN 00
#define LORA_RX_PIN 5
#define LORA_TX_PIN 6
#define LED_PIN 2
#define SPEAKER_PIN 8
#define SD_CS_PIN 10

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
#define FAN_STARTING_HEIGHT 100
#define BMP_ALTITUDES_SIZE 8 // Wie viele letzte Temperaturen gespeichert werden sollen

Adafruit_BMP280 bmp;
Adafruit_BNO055 bno(55, BNO055_I2C_ADDRESS, &Wire);
//SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
//SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);
//TinyGPSPlus gps;
SoftwareSerial rf(LORA_RX_PIN, LORA_TX_PIN);
RH_RF95 rf95(rf);

float sealevelhpa;
float bmp_altitude = STARTALTITUDE;
float bmp_altitudes[BMP_ALTITUDES_SIZE];

bool fan = false;
bool ejected = false;
bool landed = false;

int counter = 0;

File file;

void setup() {
  Serial.begin(9600);
  Serial.println(100);

  bool problem = false;

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  /*
    pinMode(GPS_RX_PIN, OUTPUT); // GPS-Pin auf Output setzen
    pinMode(GPS_TX_PIN, OUTPUT); // GPS-Pin auf Output setzen
    digitalWrite(GPS_RX_PIN, LOW); // Spannung auf GPS-Pin ausschalten
    digitalWrite(GPS_TX_PIN, LOW); // Spannung auf GPS-Pin ausschalten
  */

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  // Init BMP (Luftdruck & Temperatur)
  if (!bmp.begin(BMP280_I2C_ADDRESS)) {
    error(ERROR_BMP);
  }

  float pressure = bmp.readPressure();
  sealevelhpa = bmp.seaLevelForAltitude(STARTALTITUDE, pressure);


  // Init BNO (Inertialplatform)
  /*if (!bno.begin()) {
    error(ERROR_BNO);
    }*/

  // Init RF95 (Funkmodul)
  if (!rf95.init()) {
    error(ERROR_RF95);
  }
  rf95.setFrequency(FREQUENCY);

  // Init SD (Speicher)
  if (!SD.begin(SD_CS_PIN)) {
    error(ERROR_SD_CONNECT);
  }
  {
    // Datei mit höchstem Wert als Namen finden und dann mit Wert + 1 als Namen eine Datei erstellen
    // So muss nicht nach jedem Test die SD Karte geleert werden, sondern die Dateien sind chronologisch sortiert
    int counter = 0;
    while (SD.exists(String(counter))) {
      counter++;
    }
    file = SD.open(String(counter), FILE_WRITE);
    if (file) {
      file.println("");  // Es muss irgendetwas in die erste Zeile geschrieben werden, damit die Zahlen gespeichert werden können
    } else {
      error(ERROR_SD_OPEN);
    }
  }

  // Init GPS
  //gpsSerial.begin(9600);
  //ss.begin(9600);

  // Erfolg-Tonabfolge, LED aus
  Serial.println(200);
  tone(SPEAKER_PIN, 200);
  delay(100);
  tone(SPEAKER_PIN, 400);
  delay(100);
  tone(SPEAKER_PIN, 600);
  delay(100);
  noTone(SPEAKER_PIN);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // Sendet einen Datenblock, die Unterfunktionen senden die jeweiligen Werte selber
  // Am Anfang und am Ende wird ein Kontrollwert gesendet
  send(DATA_BLOCK_START);
  send(counter);
  BMP();
  BNO();
  GPS();

  if (counter > BMP_ALTITUDES_SIZE) { // Falls der Zähler über der Länge an gespeicherten Höhenmetern ist, weiter machen, sonst würde der Satellit schon fliegen, weil die Werte noch 0.00 sind, und dem entsprechend der Satellit schon hochgeflogen sein muss
    if (bmp_altitudes[counter % BMP_ALTITUDES_SIZE] - TOLERANCE > bmp_altitude) { // Falls die Höhe fällt
      ejected = true;
    } else if (bmp_altitudes[counter % BMP_ALTITUDES_SIZE] - TOLERANCE < bmp_altitude) { // Falls die Höhe steigt oder eher gleichbleit, dafür ist die Toleranz da
      if (ejected == true) { // Falls schon ausgeworfen, muss also gelandet sein
        landed = true;
      }
    }
  }

  if (bmp_altitude < FAN_STARTING_HEIGHT) {
    fan = true;
    digitalWrite(FAN_PIN, LOW);
  }

  bmp_altitudes[counter % BMP_ALTITUDES_SIZE] = bmp_altitude;

  // Nach dem Auswerfen Piepen (jedes mal Wechsel zwischen Ton und kein Ton, wird anhand der gesendeten Nachrichten bestimmt :)
  if (ejected) {
    if (counter % 2 == 0) {
      tone(SPEAKER_PIN, 1000);
    } else {
      noTone(SPEAKER_PIN);
    }
  }

  send((fan << 0) + (ejected << 1) + (landed << 2));  // Sendet alle Bools in einem Byte
  send(DATA_BLOCK_END);

  counter++;
}

void BMP() {
  float pressure = bmp.readPressure();

  bmp_altitude = calcAltitude(pressure);

  send(bmp.readTemperature());  // Temperatur senden
  send(pressure);
}

float calcAltitude(float pressure) {
  // https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.cpp [Zeile 321] oder im Heft Lernen mit ARDUINO!
  return 44300 * (1 - (pow(pressure / sealevelhpa, 0.1903)));
}

void BNO() {
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

void GPS() {
  float latitude;
  float longitude;
  float altitude;

  /*while (gpsSerial.available()) {
    if (gps.encode(gpsSerial.read())) {
      gps.f_get_position(&latitude, &longitude);
      altitude = gps.f_altitude();
    }
    }

    while (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      latitude = gps.location.lat(), 3;
      longitude = gps.location.lng(), 3;
    }
    }*/

  send(latitude);
  send(longitude);
  send(altitude);
}

void send(float val) {
  // Ändert den Typ von float zu uint8_t[], ohne tatsächlich Bits zu modifizieren
  uint8_t tosend[sizeof(float)];
  memcpy(&tosend, &val, sizeof(float));
  rf95.send(tosend, sizeof(float));

  // Wert als eine Zeile in die Datei schreiben
  file.println(val, DEC);

  // Tatsächlich physisch Speichern, wäre ansonsten evtl. nur im Buffer was zu Fehlern führen kann
  file.flush();
}

void error(int errorCode) {
  // Fehler code zum Debuggen an den Laptop schicken
  Serial.println(errorCode);

  // Dauerhaftes Fehler-Piepen und blinkende LED
  while (true) {
    digitalWrite(LED_PIN, LOW);
    tone(SPEAKER_PIN, 1000);
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    noTone(SPEAKER_PIN);
    delay(500);
  }
}

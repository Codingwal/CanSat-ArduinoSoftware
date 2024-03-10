#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BNO055.h>
#include <SoftwareSerial.h>
#include <RH_RF95.h>
#include <SdFat.h>

// I2C Adressen
#define BMP280_I2C_ADDRESS 0x76
#define BNO055_I2C_ADDRESS 0x28

// PIN Belegungen
#define GPS_RX_PIN 2
#define GPS_TX_PIN 3
#define FAN_PIN 4
#define LORA_RX_PIN 5
#define LORA_TX_PIN 6
#define SPEAKER_PIN 8
#define LED_PIN 9
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

#define TOLERANCE 20 // Toleranz in Metern, um Höhenunterschiede mit dem BMP zu messen
#define FAN_STARTING_HEIGHT 250
#define BMP_ALTITUDES_SIZE 8 // Wie viele letzte Temperaturen gespeichert werden sollen

#define DEBUG true
#define USE_SDCARD false
#define USE_GPS true
#define USE_BMP true
#define USE_BNO true
#define USE_LORA true
#define SILENT true

#if USE_BMP
// Adafruit_BMP280 bmp;
#endif
#if USE_BNO
Adafruit_BNO055 bno(55, BNO055_I2C_ADDRESS, &Wire);
#endif
#if USE_LORA
SoftwareSerial rf(LORA_RX_PIN, LORA_TX_PIN);
RH_RF95 rf95(rf);
#endif
#if USE_GPS
//SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);
#endif

float sealevelhpa;
float bmp_altitude = STARTALTITUDE;
int bmp_altitudes[BMP_ALTITUDES_SIZE];

bool fan = false;
bool ejected = false;
bool landed = false;

int counter = 0;

#if USE_SDCARD
SdFat32 SD;
//File32 file;
int filecounter = 0;
#endif

long time;

void setup() {
  Serial.begin(9600);
  Serial.println(100);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
#if DEBUG
  Serial.println("a");
#endif
#if USE_SDCARD
  // Init SD (Speicher)
  if (!SD.begin(SD_CS_PIN)) {
    error(ERROR_SD_CONNECT);
  }
#endif
#if DEBUG
  Serial.println("b");
#endif
#if USE_SDCARD
  {
    // Datei mit höchstem Wert als Namen finden und dann mit Wert + 1 als Namen eine Datei erstellen
    // So muss nicht nach jedem Test die SD Karte geleert werden, sondern die Dateien sind chronologisch sortiert
    while (SD.exists(String(filecounter))) {
      filecounter++;
    }
    File32 file = SD.open(String(filecounter), FILE_WRITE);
    if (file) {
      file.println(""); // Es muss irgendetwas in die erste Zeile geschrieben werden, damit die Zahlen gespeichert werden können
      file.close();
    } else {
      error(ERROR_SD_OPEN);
    }
  }
#endif
#if DEBUG
  Serial.println("c");
#endif

#if USE_BMP
  {
    Adafruit_BMP280 bmp;
    // Init BMP (Luftdruck & Temperatur)
    if (!bmp.begin(BMP280_I2C_ADDRESS)) {
      error(ERROR_BMP);
    }
    float pressure = bmp.readPressure();
    sealevelhpa = bmp.seaLevelForAltitude(STARTALTITUDE, pressure);
  }
#endif

#if DEBUG
  Serial.println("d");
#endif
#if USE_BNO
  // Init BNO (Inertialplatform)
  if (!bno.begin()) {
    error(ERROR_BNO);
  }
#endif
#if DEBUG
  Serial.println("e");
#endif
#if USE_LORA
  // Init RF95 (Funkmodul)
  if (!rf95.init()) {
    error(ERROR_RF95);
  }
#endif
#if DEBUG
  Serial.println("f");
#endif
#if USE_LORA
  rf95.setFrequency(FREQUENCY);
#endif

#if USE_GPS
  //ss.begin(9600);
#endif

#if DEBUG
  Serial.println("f2");
#endif

  // Erfolg-Tonabfolge, LED an
  Serial.println(200);
  tone(SPEAKER_PIN, 200);
  delay(100);
  tone(SPEAKER_PIN, 400);
  delay(100);
  tone(SPEAKER_PIN, 600);
  delay(100);
  noTone(SPEAKER_PIN);
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
#if USE_LORA
  rf.listen();
#endif
#if DEBUG
  Serial.println("g");
#endif
  // Sendet einen Datenblock, die Unterfunktionen senden die jeweiligen Werte selber
  // Am Anfang und am Ende wird ein Kontrollwert gesendet
  send(DATA_BLOCK_START);
  send(counter);
  BMP();
  BNO();

#if USE_LORA
  rf.listen();
#endif

  if (counter > BMP_ALTITUDES_SIZE) { // Falls der Zähler über der Länge an gespeicherten Höhenmetern ist, weiter machen, sonst würde der Satellit schon fliegen, weil die Werte noch 0.00 sind, und dem entsprechend der Satellit schon hochgeflogen sein muss
    Serial.println(bmp_altitudes[counter % BMP_ALTITUDES_SIZE]);
    Serial.println(bmp_altitude);
    Serial.println("---");
    if (bmp_altitudes[counter % BMP_ALTITUDES_SIZE] - bmp_altitude > TOLERANCE) { // Falls die Höhe fällt
      ejected = true;
    } else if (bmp_altitudes[counter % BMP_ALTITUDES_SIZE] - bmp_altitude <= 0) { // Falls gelandet
      if (ejected == true) { // Falls schon ausgeworfen, muss also gelandet sein
        landed = true;
      }
    }
  }

  if (landed) {
    fan = false;
    digitalWrite(FAN_PIN, LOW);
  } else if (ejected && bmp_altitude < FAN_STARTING_HEIGHT) {
    fan = true;
    digitalWrite(FAN_PIN, HIGH);
  }

  bmp_altitudes[counter % BMP_ALTITUDES_SIZE] = bmp_altitude;

  // Nach dem Auswerfen Piepen (jedes mal Wechsel zwischen Ton und kein Ton, wird anhand der gesendeten Nachrichten bestimmt :)
  if (ejected) {
    if (counter % 2 == 0) {
#if !SILENT
      tone(SPEAKER_PIN, 1000);
#endif
    } else {
      noTone(SPEAKER_PIN);
    }
  }

  send((fan << 0) + (ejected << 1) + (landed << 2));  // Sendet alle Bools in einem Byte
#if DEBUG
  Serial.println(fan + 1000);
  Serial.println(ejected + 2000);
  Serial.println(landed + 3000);
#endif

#if USE_SDCARD
  {
    File32 file = SD.open(String(filecounter), FILE_WRITE);
    file.println(millis() - time, DEC);
    // Tatsächlich physisch Speichern, wäre ansonsten evtl. nur im Buffer was zu Fehlern führen kann
    file.flush();
    file.close();
  }
#endif
  time = millis();  // Zeit zurücksetzten, um jedesmal die Zeit, die die Schleife (loop()) gebraucht hat zu bestimmen

  send(DATA_BLOCK_END);

  counter++;
}

void BMP() {
  float pressure = 200000;
  float temperature = 50;
#if USE_BMP
  Adafruit_BMP280 bmp;
  // Init BMP (Luftdruck & Temperatur)
  if (bmp.begin(BMP280_I2C_ADDRESS)) {
    pressure = bmp.readPressure();
    temperature = bmp.readTemperature();
    bmp_altitude = calcAltitude(pressure);
  }
#else
  float altitudes[30] = {30, 30, 35, 55, 155, 300, 500, 700, 710, 700, 680, 690, 670, 610, 550, 500, 440, 390, 310, 250, 200, 140, 100, 60, 30, 30, 35, 37, 36, 33};
  pressure = 100000;
  temperature = -10;
  bmp_altitude = altitudes[min(29, counter)];
#endif
#if DEBUG
  Serial.println(bmp_altitude);
#endif
  send(temperature);
  send(pressure);
}

float calcAltitude(float pressure) {
  // https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.cpp [Zeile 321] oder im Heft Lernen mit ARDUINO!
  return 44300 * (1 - (pow(pressure / sealevelhpa, 0.1903)));
}

void BNO() {
#if USE_BNO
  sensors_event_t accelerometer, gyroscope;
  bno.getEvent(&accelerometer, Adafruit_BNO055::VECTOR_LINEARACCEL);  // Acceleration - Gravity
  send(accelerometer.acceleration.x);
  send(accelerometer.acceleration.y);
  send(accelerometer.acceleration.z);

  bno.getEvent(&gyroscope, Adafruit_BNO055::VECTOR_EULER);
  send(gyroscope.gyro.x);
  send(gyroscope.gyro.y);
  send(gyroscope.gyro.z);
#endif
}

void send(float val) {
  {
#if USE_LORA
    // Ändert den Typ von float zu uint8_t[], ohne tatsächlich Bits zu modifizieren
    uint8_t tosend[sizeof(float)];
    memcpy(&tosend, &val, sizeof(float));
    rf95.send(tosend, sizeof(float));
#endif
  }

  {
#if SDCARD
    File32 file = SD.open(String(filecounter), FILE_WRITE);
    file.println(val, DEC);
    // Tatsächlich physisch Speichern, wäre ansonsten evtl. nur im Buffer was zu Fehlern führen kann
    file.flush();
    file.close();
#endif
  }
}

void error(int errorCode) {
  // Fehler code zum Debuggen an den Laptop schicken
  Serial.println(errorCode);

  // Dauerhaftes Fehler-Piepen und blinkende LED
  byte countdown = 60; // 60 Sekunden = 1 Minute
  while (countdown > 0) {
    digitalWrite(LED_PIN, LOW);
#if !SILENT
    tone(SPEAKER_PIN, 1000);
#endif
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    noTone(SPEAKER_PIN);
    delay(500);

    countdown--;
  }
}

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
#define BMP_ALTITUDES_SIZE 4 // Wie viele letzte Temperaturen gespeichert werden sollen

#define DEBUG false
#define USE_SDCARD true
#define USE_GPS true
#define USE_BMP true
#define USE_BNO true
#define USE_LORA true
#define SILENT true

#if USE_LORA
SoftwareSerial rf(LORA_RX_PIN, LORA_TX_PIN);
RH_RF95 rf95(rf);
#endif

#if USE_BNO
Adafruit_BNO055 bno(55, BNO055_I2C_ADDRESS, &Wire);
#endif

float sealevelhpa;
float bmp_altitude = STARTALTITUDE;
int bmp_altitudes[BMP_ALTITUDES_SIZE];

bool fan = false;
bool ejected = false;
bool landed = false;

int loopcounter = 0;

#if USE_SDCARD
SdFat32 SD;
byte filecounter = 0;
#endif

//long time;

float datatosend[15];

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

#if USE_BNO
  if (!bno.begin()) {
    error(ERROR_BNO);
  }
#endif

#if DEBUG
  Serial.println("d");
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
  //gps.begin(9600);
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
  datatosend[0] = DATA_BLOCK_START;
  datatosend[1] = loopcounter;
  /*send(DATA_BLOCK_START);
    send(loopcounter);*/
  BMP();
  BNO();

#if USE_LORA
  // rf.listen();
#endif

  if (loopcounter > BMP_ALTITUDES_SIZE) { // Falls der Zähler über der Länge an gespeicherten Höhenmetern ist, weiter machen, sonst würde der Satellit schon fliegen, weil die Werte noch 0.00 sind, und dem entsprechend der Satellit schon hochgeflogen sein muss
#if DEBUG
    Serial.println(bmp_altitudes[loopcounter % BMP_ALTITUDES_SIZE]);
    Serial.println(bmp_altitude);
    Serial.println("---");
#endif
    if (bmp_altitudes[loopcounter % BMP_ALTITUDES_SIZE] - bmp_altitude > TOLERANCE) { // Falls die Höhe fällt
      ejected = true;
    } else if (bmp_altitudes[loopcounter % BMP_ALTITUDES_SIZE] - bmp_altitude <= 0) { // Falls gelandet
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

  bmp_altitudes[loopcounter % BMP_ALTITUDES_SIZE] = bmp_altitude;

  // Nach dem Auswerfen Piepen (jedes mal Wechsel zwischen Ton und kein Ton, wird anhand der gesendeten Nachrichten bestimmt :)
  if (ejected) {
    if (loopcounter % 2 == 0) {
#if !SILENT
      tone(SPEAKER_PIN, 1000);
#endif
    } else {
      noTone(SPEAKER_PIN);
    }
  }

  GPS();

  datatosend[13] = (fan << 0) + (ejected << 1) + (landed << 2);  // Sendet alle Bools in einem Byte

#if DEBUG
  Serial.println(fan + 1000);
  Serial.println(ejected + 2000);
  Serial.println(landed + 3000);
#endif

#if USE_SDCARD
  {
    File32 file = SD.open(String(filecounter), FILE_WRITE);
    //file.println(millis() - time, DEC);
    file.println(millis(), DEC);
    // Tatsächlich physisch Speichern, wäre ansonsten evtl. nur im Buffer was zu Fehlern führen kann
    file.flush();
    file.close();
  }
#endif
  //time = millis();  // Zeit zurücksetzten, um jedesmal die Zeit, die die Schleife (loop()) gebraucht hat zu bestimmen

  datatosend[14] = (DATA_BLOCK_END);

  loopcounter++;

  for (byte i = 0; i < 15; i++) {
    send(datatosend[i]);
  }
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
  bmp_altitude = altitudes[min(29, loopcounter)];
#endif
#if DEBUG
  Serial.println(bmp_altitude);
#endif
  datatosend[2] = temperature;
  datatosend[3] = pressure;
}

float calcAltitude(float pressure) {
  // https://github.com/adafruit/Adafruit_BMP280_Library/blob/master/Adafruit_BMP280.cpp [Zeile 321] oder im Heft Lernen mit ARDUINO!
  return 44300 * (1 - (pow(pressure / sealevelhpa, 0.1903)));
}

void BNO() {
#if USE_BNO
  sensors_event_t accelerometer, gyroscope;
  bno.getEvent(&accelerometer, Adafruit_BNO055::VECTOR_LINEARACCEL);  // Acceleration - Gravity
  /*send(accelerometer.acceleration.x);
    send(accelerometer.acceleration.y);
    send(accelerometer.acceleration.z);*/
  datatosend[4] = accelerometer.acceleration.x;
  datatosend[5] = accelerometer.acceleration.y;
  datatosend[6] = accelerometer.acceleration.z;

  bno.getEvent(&gyroscope, Adafruit_BNO055::VECTOR_EULER);
  datatosend[7] = gyroscope.gyro.x;
  datatosend[8] = gyroscope.gyro.y;
  datatosend[9] = gyroscope.gyro.z;
  /*send(gyroscope.gyro.x);
    send(gyroscope.gyro.y);
    send(gyroscope.gyro.z);*/
#else
  datatosend[4] = 1.23;
  datatosend[5] = 1.23;
  datatosend[6] = 1.23;

  datatosend[7] = 1.23;
  datatosend[8] = 1.23;
  datatosend[9] = 1.23;
#endif
}

void GPS() {
  float latitude, longitude, altitude;
#if USE_GPS
  SoftwareSerial gps(GPS_RX_PIN, GPS_TX_PIN);
  //Serial.println("---GPS---");
  gps.begin(9600);
  //delay(700);
  char input[13]; // was 80
  byte y = 0;

  bool isGPGGA = false;
  bool isDollarSignFound = false;

  byte field = 0;

  unsigned long start = millis();
  do
  {
    while (gps.available() > 0) {
      char x = gps.read();
#if DEBUG
      //Serial.print("rcvd: ");
      //Serial.write(x);
      //Serial.println(x);
#endif
      if (x == '$') {
        //memset(input, 0, sizeof(input));
        y = 0;
        isDollarSignFound = true;
        isGPGGA = false;
        field = 0;
#if DEBUG
        //Serial.println("isDollarSignFound");
#endif
      }

      if (isDollarSignFound) {
        input[y] = x;
        y++;
        if (y >= 6 && strncmp(input + 1, "GPGGA", 5) == 0) {
          isGPGGA = true;
#if DEBUG
          Serial.println("isGPGGA");
#endif
        }
        if (x == ',') {
          if (isGPGGA) {
#if DEBUG
          Serial.print(", found, field: ");
          Serial.println(field);
          Serial.println(input);
#endif
            float gpsdata[2];
            gpsStringToFloats(gpsdata, input, field, y);

            if (field == 2) {
              latitude = gpsdata[0] + gpsdata[1] / 100.0;
#if DEBUG
              Serial.println(latitude);
#endif
            }
            if (field == 4) {
              longitude = gpsdata[0] + gpsdata[1] / 100.0;
#if DEBUG
              Serial.println(longitude);
#endif
            }
            if (field == 9) {
              altitude = gpsdata[0];
#if DEBUG
              Serial.println(altitude);
#endif
            }
          }

          y = 0;
          field++;
          memset(input, 0, sizeof(input));
        }
      }
      if (x == '\n' || y == 12) {
        isDollarSignFound = false;
#if DEBUG
          Serial.println("NL or ==12");
#endif
      }
    }
  } while (millis() - start < 1000);
  //Serial.println("done reading.");

#endif
#if USE_LORA
  rf.listen();
#endif
  /*send(latitude);
    send(longitude);
    send(altitude);*/
  datatosend[10] = latitude;
  datatosend[11] = longitude;
  datatosend[12] = altitude;
#if DEBUG
  Serial.println(latitude, DEC);
  Serial.println(longitude, DEC);
  Serial.println(altitude, DEC);
#endif
}

void send(float val) {
  {
#if USE_LORA
    // Ändert den Typ von float zu uint8_t[], ohne tatsächlich Bits zu modifizieren
    uint8_t tosend[sizeof(float)];
    memcpy(&tosend, &val, sizeof(float));
    rf95.send(tosend, sizeof(float));
    rf95.waitPacketSent();
#endif
  }
// #if DEBUG
  Serial.print("Val: ");
  Serial.println(val);
// #endif

  {
#if USE_SDCARD
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

int charToInt(char c)
{
  return c - '0';
}

void gpsStringToFloats(float * dest, const char *str, byte field, byte len)
{
  //byte field = 0;
  byte fieldstart = 0;

  if (field == 2) {
    dest[0] = charToInt(str[fieldstart]) * 10 + charToInt(str[fieldstart + 1]);
    dest[1] = (charToInt(str[fieldstart + 2]) * 10 + charToInt(str[fieldstart + 3]) + charToInt(str[fieldstart + 5]) * 0.1 + charToInt(str[fieldstart + 6]) * 0.01 + charToInt(str[fieldstart + 7]) * 0.001 + charToInt(str[fieldstart + 8]) * 0.0001) / 0.6f;
  } else if (field == 4) {
    dest[0] = charToInt(str[fieldstart]) * 100 + charToInt(str[fieldstart + 1]) * 10 + charToInt(str[fieldstart + 2]);
    dest[1] = (charToInt(str[fieldstart + 3]) * 10 + charToInt(str[fieldstart + 4]) + charToInt(str[fieldstart + 6]) * 0.1 + charToInt(str[fieldstart + 7]) * 0.01 + charToInt(str[fieldstart + 8]) * 0.001 + charToInt(str[fieldstart + 9]) * 0.0001) / 0.6f;
  } else if (field == 9) {
    byte pointpos = fieldstart;
    while (pointpos < len && str[pointpos] != '.') {
      pointpos++;
    }
#if DEBUG
    Serial.println("Pointpos:");
    Serial.println(fieldstart);
    Serial.println(pointpos);
#endif
    if (pointpos == fieldstart) {
      dest[0] = 0;
    } else if (pointpos == fieldstart + 1) {
      dest[0] = charToInt(str[fieldstart]);
    } else if (pointpos == fieldstart + 2) {
      dest[0] = charToInt(str[fieldstart]) * 10 + charToInt(str[fieldstart + 1]);
    } else if (pointpos == fieldstart + 3) {
      dest[0] = charToInt(str[fieldstart]) * 100 + charToInt(str[fieldstart + 1]) * 10 + charToInt(str[fieldstart + 2]);
    } else if (pointpos == fieldstart + 4) {
      dest[0] = charToInt(str[fieldstart]) * 1000 + charToInt(str[fieldstart + 1]) * 100 + charToInt(str[fieldstart + 2]) * 10 + charToInt(str[fieldstart + 3]);
    }
    if (pointpos < len - 1) {
      dest[0] += charToInt(str[pointpos + 1]) * 0.1;
    }
  }
}
/*
  void gpsStringToFloats(float * dest, const char *str)
  {
  byte field = 0;
  byte counter = 0;
  byte fieldstart = 0;

  while (counter < 80 && field < 10) {
    if (str[counter] == ',') {
      if (field == 2) {
        dest[0] = charToInt(str[fieldstart]) * 10 + charToInt(str[fieldstart + 1]);
        dest[1] = (charToInt(str[fieldstart + 2]) * 10 + charToInt(str[fieldstart + 3]) + charToInt(str[fieldstart + 5]) * 0.1 + charToInt(str[fieldstart + 6]) * 0.01 + charToInt(str[fieldstart + 7]) * 0.001 + charToInt(str[fieldstart + 8]) * 0.0001) / 0.6f;
      } else if (field == 4) {
        dest[2] = charToInt(str[fieldstart]) * 100 + charToInt(str[fieldstart + 1]) * 10 + charToInt(str[fieldstart + 2]);
        dest[3] = (charToInt(str[fieldstart + 3]) * 10 + charToInt(str[fieldstart + 4]) + charToInt(str[fieldstart + 6]) * 0.1 + charToInt(str[fieldstart + 7]) * 0.01 + charToInt(str[fieldstart + 8]) * 0.001 + charToInt(str[fieldstart + 9]) * 0.0001) / 0.6f;
      } else if (field == 9) {
        byte pointpos = fieldstart;
        while (pointpos < counter && str[pointpos] != '.') {
          pointpos++;
        }
  #if DEBUG
        Serial.println("Pointpos:");
        Serial.println(fieldstart);
        Serial.println(pointpos);
  #endif
        if (pointpos == fieldstart) {
          dest[4] = 0;
        } else if (pointpos == fieldstart + 1) {
          dest[4] = charToInt(str[fieldstart]);
        } else if (pointpos == fieldstart + 2) {
          dest[4] = charToInt(str[fieldstart]) * 10 + charToInt(str[fieldstart + 1]);
        } else if (pointpos == fieldstart + 3) {
          dest[4] = charToInt(str[fieldstart]) * 100 + charToInt(str[fieldstart + 1]) * 10 + charToInt(str[fieldstart + 2]);
        } else if (pointpos == fieldstart + 4) {
          dest[4] = charToInt(str[fieldstart]) * 1000 + charToInt(str[fieldstart + 1]) * 100 + charToInt(str[fieldstart + 2]) * 10 + charToInt(str[fieldstart + 3]);
        }
        if (pointpos < counter - 1) {
          dest[4] += charToInt(str[pointpos + 1]) * 0.1;
        }
      }

      field++;
      fieldstart = counter + 1;
    }
    counter++;
  }
  }
*/

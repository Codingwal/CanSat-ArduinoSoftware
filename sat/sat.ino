#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BNO055.h>
#include <SoftwareSerial.h>
//#include <TinyGPS.h>
//#include <TinyGPSPlus.h>
#include <RH_RF95.h>
#include <SD.h>

#define BMP280_I2C_ADDRESS 0x76
#define BNO055_I2C_ADDRESS 0x28
#define FAN_PIN 4
#define GPS_RX_PIN 00
#define GPS_TX_PIN 00
#define LORA_RX_PIN 5
#define LORA_TX_PIN 6
#define LED_PIN 2
#define SPEAKER_PIN 8
#define SD_CS_PIN 10

/*
  SPI Pins Arduino Nano
  CS 10
  MOSI 11
  MISO 12
  SCK 13
  https://www.arduino.cc/reference/en/language/functions/communication/spi/
  https://funduino.de/nr-28-das-sd-karten-modul
*/

#define FREQUENCY 433.0

#define STARTALTITUDE 30 // Höhe vor dem Start vom Flugplatz

#define FILEPATH "data.txt"

Adafruit_BMP280 bmp;
Adafruit_BNO055 bno;
//SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
//SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);
//TinyGPSPlus gps;
SoftwareSerial rf(LORA_RX_PIN, LORA_TX_PIN);
RH_RF95 rf95(rf);

float pressure;
float sealevelhpa;

bool fan = false;
bool ejected = false;
bool landed = false;

int messages = 0;

File file;

void setup() {
  Serial.begin(9600);
  Serial.println(100);

  bool problem = false;

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  /*pinMode(GPS_RX_PIN, OUTPUT); // GPS-Pin auf Output setzen
    pinMode(GPS_TX_PIN, OUTPUT); // GPS-Pin auf Output setzen
    digitalWrite(GPS_RX_PIN, LOW); // Spannung auf GPS-Pin ausschalten
    digitalWrite(GPS_TX_PIN, LOW); // Spannung auf GPS-Pin ausschalten*/

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  if (!bmp.begin(BMP280_I2C_ADDRESS)) {
    Serial.println(501);
    problem = true;
  }
  if (!bno.begin()) {
    Serial.println(502);
    problem = true;
  }
  if (!rf95.init())
  {
    Serial.println(503);
    problem = true;
  }
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(504);
    problem = true;
  }

  file = SD.open(FILEPATH, FILE_WRITE);
  if (file) {
    file.println(""); // Es muss irgendetwas in die erste Zeile geschrieben werden, damit die Zahlen gespeichert werden können
  } else {
    Serial.println(505);
    problem = true;
  }

  /*gpsSerial.begin(9600);
    ss.begin(9600);*/

  rf95.setFrequency(FREQUENCY);
  sealevelhpa = bmp.seaLevelForAltitude(STARTALTITUDE, bmp.readPressure());

  if (problem) {
    while (true) {
      digitalWrite(LED_PIN, LOW);
      tone(SPEAKER_PIN, 1000);
      delay(500);
      digitalWrite(LED_PIN, HIGH);
      noTone(SPEAKER_PIN);
      delay(500);
    }
  } else {
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
}

float height = 700; // Dient als Test, wird durchgehend runtergesetzt
void loop() {
  /*if (Serial.readString() == "getdata") {
    transmitData();
    }*/

  if (height < 500) { // Beispielwert
    digitalWrite(FAN_PIN, HIGH); // Lüfter einschalten
    fan = 1;
  }

  if (height > 0) { // Nur für Test-Zwecke
    height = height - 200;
  } else {
    //file.close();
  }

  if (ejected) {
    if (messages % 2 == 0) {
      tone(SPEAKER_PIN, 1000);
    } else {
      noTone(SPEAKER_PIN);
    }
  }

  send(9999990);
  send(messages);
  BMP();
  BNO();
  GPS();
  send(fan);
  send(landed);
  send(9999991);
  messages++;
}

void BMP() {
  pressure = bmp.readPressure();

  send(bmp.readTemperature()); // Temperatur senden
  send(pressure);
}

void BNO() {
  sensors_event_t accelerometer, gyroscope;
  bno.getEvent(&accelerometer, Adafruit_BNO055::VECTOR_LINEARACCEL); // Acceleration - Gravity
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
    }*/

  /*while (ss.available() > 0) {
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
  uint8_t tosend[sizeof(float)];
  memcpy(&tosend, &val, sizeof(float));
  rf95.send(tosend, sizeof(float));

  file.println(val, DEC);
  file.flush();
}

void transmitData() {
  while (file.available()) {
    Serial.write(file.read());
  }
}

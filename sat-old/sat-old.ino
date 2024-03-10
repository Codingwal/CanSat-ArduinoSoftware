#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <Adafruit_BNO055.h>
#include <SoftwareSerial.h>
//#include <TinyGPS.h>
//#include <TinyGPSPlus.h>
#include <RH_RF95.h>

#define BMP280_I2C_ADDRESS 0x76
#define BNO055_I2C_ADDRESS 0x28
#define FAN_PIN 4
#define GPS_RX_PIN 11
#define GPS_TX_PIN 12
#define LORA_RX_PIN 5
#define LORA_TX_PIN 6
#define LED_PIN 13
#define SPEAKER_PIN 10

#define FREQUENCY 433.0

Adafruit_BMP280 bmp;
Adafruit_BNO055 bno = Adafruit_BNO055(55, BNO055_I2C_ADDRESS, &Wire);
//SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
//SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);
//TinyGPSPlus gps;
SoftwareSerial rf(LORA_RX_PIN, LORA_TX_PIN);
RH_RF95 rf95(rf);

float startaltitude; // Höhe vor dem Start vom GPS
float startpressure; // Luftdruck vor dem Start
float startaltitude2; // Höhe vor dem Start vom BMP
float startX; // GPS-Position vor dem Start, bzw. wenn es nicht mehr die indischen Koordinaten sind
float startY; // GPS-Position vor dem Start, bzw. wenn es nicht mehr die indischen Koordinaten sind
float startZ; // GPS-Position vor dem Start, bzw. wenn es nicht mehr die indischen Koordinaten sind

float temperature;
float pressure;
float altitude2;

float accelerationX;
float accelerationY;
float accelerationZ;
float rotationX;
float rotationY;
float rotationZ;
float temperature2;

float latitude;
float longitude;
float altitude;

bool fan = 0;
bool working = false;
bool landed = 0;

int sended = 0;

bool problem = false;

void setup() {
  Serial.begin(9600);

  Serial.println("Starting...");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(GPS_RX_PIN, OUTPUT); // GPS-Pin auf Output setzen
  pinMode(GPS_TX_PIN, OUTPUT); // GPS-Pin auf Output setzen
  digitalWrite(GPS_RX_PIN, LOW); // Spannung auf GPS-Pin ausschalten
  digitalWrite(GPS_TX_PIN, LOW); // Spannung auf GPS-Pin ausschalten

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  if (!bmp.begin(BMP280_I2C_ADDRESS)) {
    Serial.println("Please connect BMP280!");
    problem = true;
  }
  if (!bno.begin()) {
    Serial.println("Please connect BNO055!");
    problem = true;
  }

  if (!rf95.init())
  {
    Serial.println("Please connect LoRa!");
    problem = true;
  }

  //gpsSerial.begin(9600);
  //ss.begin(9600);

  rf95.setFrequency(FREQUENCY);

  if (bmp.takeForcedMeasurement()) {
    startpressure = bmp.readPressure(); // Luftdruck beim einschalten (Luftdruck)
    startaltitude2 = bmp.readAltitude(); // Höhe beim einschalten (Luftdruck)
  }

  if (problem) {
    while (true) {
      digitalWrite(LED_PIN, LOW);
      delay(500);
      digitalWrite(LED_PIN, HIGH);
      delay(500);
    }
  } else {
    Serial.println("Successfully started!");
    digitalWrite(LED_PIN, LOW);
    working = true;
  }
}

float height = 700; // Dient als Test, wird durchgehend runtergesetzt
void loop() {
  Serial.println("looooooop"); // Zum Debuggen

  // BMP
  //if (bmp.takeForcedMeasurement()) {
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure();
  altitude2 = bmp.readAltitude();
  //}

  // BNO
  sensors_event_t accelerometer, gyroscope;
  //bno.getEvent(&accelerometer, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  bno.getEvent(&accelerometer, Adafruit_BNO055::VECTOR_LINEARACCEL); // Acceleration - Gravity
  accelerationX = accelerometer.acceleration.x;
  accelerationY = accelerometer.acceleration.y;
  accelerationZ = accelerometer.acceleration.z;

  bno.getEvent(&gyroscope, Adafruit_BNO055::VECTOR_EULER);
  rotationX = gyroscope.gyro.x;
  rotationY = gyroscope.gyro.y;
  rotationZ = gyroscope.gyro.z;

  // GPS
  //while (gpsSerial.available()) {
  // if (gps.encode(gpsSerial.read())) {
  //  gps.f_get_position(&latitude, &longitude);
  //  altitude = gps.f_altitude();
  // }
  //}

  //while (ss.available() > 0) {
  //gps.encode(ss.read());
  //if (gps.location.isUpdated()) {
  //latitude = gps.location.lat(), 3;
  //longitude = gps.location.lng(), 3;
  //}
  //}

  float bmpHeight = startaltitude2 - altitude2; // Höhe mit Luftdruck in Metern, relativ zur Höhe beim Start
  float imuHeight;
  float gpsHeight;

  if (height < 500) { // Beispielwert
    digitalWrite(FAN_PIN, HIGH); // Lüfter einschalten
    fan = 1;
  }

  sendData();

  if (height > 0) { // Nur für Test-Zwecke
    height = height - 2.5;
  }
  
  tone(SPEAKER_PIN, 1000);
  delay(150);
  noTone(SPEAKER_PIN);
}

void sendData() {
  send(9999990);
  send(temperature);
  send(pressure);
  send(altitude2);
  //send(9999991);
  send(accelerationX);
  send(accelerationY);
  send(accelerationZ);
  //send(9999992);
  send(rotationX);
  send(rotationY);
  send(rotationZ);
  //send(9999993);
  send(latitude);
  send(longitude);
  send(altitude);
  //send(9999994);
  send(fan);
  send(working);
  send(landed);
  //send(9999995);
  send(sended);
  send(9999991);

  sended++;
}

void send(double val) {
  uint8_t data[sizeof(double)];
  memcpy(&data, &val, sizeof(double));
  rf95.send(data, sizeof(double));
}

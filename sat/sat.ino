#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <Adafruit_BNO055.h>
#include <SoftwareSerial.h>
//#include <TinyGPS.h>
#include <TinyGPSPlus.h>

#define BMP280_I2C_ADDRESS 0x76
#define BNO055_I2C_ADDRESS 0x28
#define FAN_PIN 4
#define GPS_RX_PIN 11
#define GPS_TX_PIN 12

Adafruit_BMP280 bmp;
Adafruit_BNO055 bno = Adafruit_BNO055(55, BNO055_I2C_ADDRESS, &Wire);
//SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);
TinyGPSPlus gps;

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
bool started = 0;
bool landed = 0;

void setup() {
  Serial.begin(9600);

  // bmp.begin(BMP280_I2C_ADDRESS);

  if (!bmp.begin(BMP280_I2C_ADDRESS)) {
    Serial.println("Please connect BMP280!");
  }

  if (!bno.begin()) {
    Serial.println("Please connect BNO055!");
  }

  //gpsSerial.begin(9600);
  ss.begin(9600);

  pinMode(GPS_RX_PIN, OUTPUT); // GPS-Pin auf Output setzen
  pinMode(GPS_TX_PIN, OUTPUT); // GPS-Pin auf Output setzen
  digitalWrite(GPS_RX_PIN, LOW); // Spannung auf GPS-Pin ausschalten
  digitalWrite(GPS_TX_PIN, LOW); // Spannung auf GPS-Pin ausschalten

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  if (bmp.takeForcedMeasurement()) {
    startpressure = bmp.readPressure(); // Luftdruck beim einschalten (Luftdruck)
    startaltitude2 = bmp.readAltitude(); // Höhe beim einschalten (Luftdruck)
  }
}

float height = 700; // Dient als Test, wird durchgehend runtergesetzt
void loop() {
  // BMP
  //if (bmp.takeForcedMeasurement()) {
    temperature = bmp.readTemperature();
    pressure = bmp.readPressure();
    altitude2 = bmp.readAltitude();
  //}

  // BNO
  sensors_event_t accelerometer, gyroscope;
  bno.getEvent(&accelerometer, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  accelerationX = accelerometer.acceleration.x;
  accelerationY = accelerometer.acceleration.y;
  accelerationZ = accelerometer.acceleration.z;

  bno.getEvent(&gyroscope, Adafruit_BNO055::VECTOR_GYROSCOPE);
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

  height = height - 10; // Nur für Test-Zwecke
  delay(1000);
}

void sendData() { // Zurzeit noch über den USB-Serial-Converter
  Serial.println("99999990");
  Serial.println(temperature);
  Serial.println(pressure);
  Serial.println(altitude2);
  Serial.println("99999991");
  Serial.println(accelerationX);
  Serial.println(accelerationY);
  Serial.println(accelerationZ);
  Serial.println("99999992");
  Serial.println(rotationX);
  Serial.println(rotationY);
  Serial.println(rotationZ);
  Serial.println("99999993");
  Serial.println(latitude);
  Serial.println(longitude);
  Serial.println(altitude);
  Serial.println("99999994");
  Serial.println(fan);
  Serial.println(started);
  Serial.println(landed);
  Serial.println("99999995");
}

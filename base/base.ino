#include <RH_RF95.h>

// PIN Belegungen
#define LORA_RX_PIN 5
#define LORA_TX_PIN 6

//
#define FREQUENCY 433.0 // Datenübertragungsfrequenz (in MHz)

SoftwareSerial rf(LORA_RX_PIN, LORA_TX_PIN);
RH_RF95 rf95(rf);

void setup() {
  Serial.begin(9600);

  // Init RF95 (Funkmodul)
  if (!rf95.init())
  {
    Serial.println("Please connect LoRa!");
  }
  rf95.setFrequency(FREQUENCY);
}

void loop()
{
  // Wenn ein Signal angekommen ist...
  if (rf95.available())
  {
    uint8_t buf[sizeof(float)];   // Buffer für die empfagenen Daten
    uint8_t receivedDateLength;   // Unused

    if (rf95.recv(buf, &receivedDateLength))
    {
      // Ändert den Typ von uint8_t[] zu float, ohne tatsächlich Bits zu modifizieren
      float data;
      memcpy(&data, &buf, sizeof(float));

      // Sendet die Daten an den Laptop weiter
      Serial.println(data);
    }
  }
}

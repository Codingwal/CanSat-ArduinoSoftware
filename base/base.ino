#include <RH_RF95.h>

#define LORA_RX_PIN 5
#define LORA_TX_PIN 6

#define FREQUENCY 433.0

SoftwareSerial rf(LORA_RX_PIN, LORA_TX_PIN);
RH_RF95 rf95(rf);

void setup() {
  Serial.begin(9600);

  if (!rf95.init())
  {
    Serial.println("Please connect LoRa!");
  }

  rf95.setFrequency(FREQUENCY);
}

void loop()
{
  if (rf95.available())
  {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      float data;
      memcpy(&data, &buf, len);
      Serial.println(data);
    }
  }
}

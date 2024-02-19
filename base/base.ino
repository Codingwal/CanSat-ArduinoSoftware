#include <RH_RF95.h>

#define LORA_RX_PIN 5
#define LORA_TX_PIN 6
#define USB_RX_PIN 0
#define USB_TX_PIN 1

#define FREQUENCY 434.0

SoftwareSerial rf(LORA_RX_PIN, LORA_TX_PIN);
RH_RF95 rf95(rf);

SoftwareSerial usb(USB_RX_PIN, USB_TX_PIN);

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
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      double data;
      memcpy(&data, &buf, len);
      Serial.println(data);
    }
  }
}

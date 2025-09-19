#include <Arduino.h>
#include "Device.h"

// ---- Pines ----
#define PIN_DHT       33      // DATA del DHT22
#define DHTTYPE       DHT22
#define I2C_SDA       21
#define I2C_SCL       22
#define PIN_POT 32
#define PIN_LED 23
#define ENC_SW 19 

// ---- Objetos ----
Device device(127, 64, -1, PIN_DHT, DHT22, PIN_POT, ENC_SW, PIN_LED);



void setup() {

  Serial.begin(115200);
  delay(100);
  device.begin(I2C_SDA, I2C_SCL);
}

void loop() {
  device.handleEncoderButton();
  if(device.screenIdx == 0){
    device.showScreenTemp();
  } else {
    device.showScreenHum();
    device.actualizarLedIntermitente();
  }
}

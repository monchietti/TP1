#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---- Pines ----
#define PIN_DHT       33      // DATA del DHT22
#define DHTTYPE       DHT22
#define I2C_SDA       21
#define I2C_SCL       22
#define OLED_ADDRESS  0x3C
#define PIN_POT 32
#define PIN_LED 23
#define ENC_SW 19 

// ---- Objetos ----
DHT dht(PIN_DHT, DHTTYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ---- Variables globales ----
uint8_t screenIdx = 0; // pantalla actual
unsigned long lastBtnMs = 0; 
const unsigned long DEBOUNCE_MS = 40; 


//---- Prototipos de funciones ----
float mapPotToTempC(int raw);
void escribirPantalla(String texto);
void showScreenTemp();
void showScreenHum();
void handleEncoderButton();

void setup() {
  Serial.begin(115200);
  delay(100);

  // I2C + OLED
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("ERROR: No se detecta OLED (0x3C)."));
    while (true) delay(10);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // DHT
  dht.begin();
  Serial.println("DHT22 iniciado");
  // inicialización pines
  pinMode(PIN_LED, OUTPUT);
  pinMode(ENC_SW, INPUT_PULLUP); // botón del encoder
}

void loop() {
  handleEncoderButton();
  if (screenIdx == 0) showScreenTemp(); else showScreenHum();
}

float mapPotToTempC(int raw) {
  return 0.0f + (45.0f - 0.0f) * (float)raw / 4095.0f;
}


void escribirPantalla(String texto){
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(texto);
  display.display();
}

void showScreenTemp() {
  // Leer sensor
   int valor = analogRead(PIN_POT);
   float tempC_pot = mapPotToTempC(valor);
   Serial.println(tempC_pot);

  float tempC = dht.readTemperature(); // en Celsius
  Serial.println(tempC);
  float hum   = dht.readHumidity();
  String strHumedad = String(hum, 2); 
  String texto = "Pantalla Temperatura \n Temp: " + String(tempC, 2) + " C\n Temp Pot: " + String(tempC_pot, 2) + " C";
  
  if(tempC > tempC_pot){
    Serial.println("en pantalla ventilacion");
    digitalWrite(PIN_LED, HIGH);
    escribirPantalla(texto + "\n Ventilacion ON");
  } else {
    digitalWrite(PIN_LED, LOW);
    escribirPantalla(texto + "\n Ventilacion OFF");
  }

  delay(500);  // refresco cada 1 s

}

void showScreenHum() {
  escribirPantalla("Pantalla Humedad");
  delay(1000);  // refresco cada 1 s
}

void handleEncoderButton() {
  unsigned long now = millis();
  if (now - lastBtnMs < DEBOUNCE_MS) return;
  if (digitalRead(ENC_SW) == LOW) {       // INPUT_PULLUP: presionado = LOW
    lastBtnMs = now;
    screenIdx = (screenIdx + 1) % 2;
    Serial.printf("[UI] Cambio a Pantalla", screenIdx + 1 + "\n");
    // refrescar inmediatamente y aplicar modo de LED de la pantalla
  }
}

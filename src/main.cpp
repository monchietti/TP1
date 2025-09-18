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
const unsigned long DEBOUNCE_MS = 200; 
float humedadMinimaDeseada = 0.0;    // valor inicial aleatorio
float humedadActual = dht.readHumidity();    // última humedad mostrada
boolean ventilacionEncendida = false;
boolean riegoEncendido = false;


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

  // Inicializar generador de aleatorios
  randomSeed(analogRead(0));

  // Generar umbral inicial entre 40 y 60
  humedadMinimaDeseada = random(40, 61);
  humedadActual = dht.readHumidity();
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
  float tempC = dht.readTemperature(); // en Celsius
  float hum   = dht.readHumidity();
  String strHumedad = String(hum, 2); 
  String texto = "Pantalla Temperatura \n Temp: " + String(tempC, 2) + " C\n Temp Pot: " + String(tempC_pot, 2) + " C";
  
  if(tempC > tempC_pot){
    digitalWrite(PIN_LED, HIGH);
    escribirPantalla(texto + "\n Ventilacion ON");
    
    if (!ventilacionEncendida){
      ventilacionEncendida = true;
      Serial.println("Ventilacion encendida");
    }

  } else {
    digitalWrite(PIN_LED, LOW);
    escribirPantalla(texto + "\n Ventilacion OFF");
    if (ventilacionEncendida){
      ventilacionEncendida = false;
      Serial.println("Ventilacion apagada");
    }

  }

}

void showScreenHum() {
  humedadActual = dht.readHumidity();
  String textoRiego = "Humedad min deseada:\n" + String(humedadMinimaDeseada, 0) + "%\n\n" + "Humedad actual:\n" + humedadActual + "%";

  if(humedadActual < humedadMinimaDeseada) {
    digitalWrite(PIN_LED, HIGH);
    delay(200);
    digitalWrite(PIN_LED, LOW);
    delay(200);
    textoRiego += "\n\n Riego Encendido";
    if (!riegoEncendido){
      riegoEncendido = true;
      Serial.println("Riego encendido");
    }
  }else{
    textoRiego += "\n\n Riego Apagado";
    if(riegoEncendido){
      riegoEncendido = false;
      Serial.println("Riego apagado");
    }
  }
  escribirPantalla(textoRiego);
}

void handleEncoderButton() {
  unsigned long now = millis();
  if (now - lastBtnMs < DEBOUNCE_MS) return;
  if (digitalRead(ENC_SW) == LOW) {       // INPUT_PULLUP: presionado = LOW
    lastBtnMs = now;
    screenIdx = (screenIdx + 1) % 2;
    Serial.println("[UI] Cambio a Pantalla"+ String(screenIdx + 1));
    // refrescar inmediatamente y aplicar modo de LED de la pantalla
  }
}

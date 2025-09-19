#include "Device.h"

// Inicializamos las variables con algo llamado lista de inicializacion, propio de C++
// Se diferencia de la inicializacion normal en que en la normal se setean valores por defecto, cadenas en " ", numeros en 0, etc.
// Cuando se definen con la lista es mas performante, ocupa menos espacio y no asigna valores por defecto, se crea con los valores que vos le pasas
Device::Device(int w, int h, int reset,int pinDHT, const uint8_t model, int pinPOT, int pinBtn, int pinLED) : _sensor(pinDHT, model), _display(w, h, &Wire, reset)
{
    PIN_POT = pinPOT;
    ENC_SW = pinBtn;
    PIN_LED = pinLED;
}

void Device::begin(int I2C_SDA, int I2C_SCL)
{   
    Serial.println("INICIANDO...");
    // inicializacion de la pantalla
    Wire.begin(I2C_SDA, I2C_SCL);
    if (!_display.begin(0x3C, true))
    {
        Serial.println(F("ERROR: No se detecta OLED (0x3C)."));
        while (true)
            delay(10);
    }

    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setTextColor(SH110X_WHITE);
    // inicializacion del sensor
    _sensor.begin();

    // inicializacion pines
    pinMode(PIN_LED, OUTPUT);
    pinMode(ENC_SW, INPUT_PULLUP); // botón del encoder
    randomSeed(millis());
    // Generar umbral inicial entre 40 y 60
    //HARDCODEAR EN 100 para que titile siempre
    humedadMinimaDeseada = random(40, 61);
    Serial.println("Humedad minima deseada: " + String(humedadMinimaDeseada) + "%");
}

void Device::escribirPantalla(String texto){
    _display.clearDisplay();
    _display.setCursor(0, 0);
    _display.println(texto);
    _display.display();
}

float Device::readTemp()
{
    return _sensor.readTemperature();
}
float Device::readHum()
{
    return _sensor.readHumidity();
}

float Device::readPot()
{
    return analogRead(PIN_POT);
}
float Device::mapPotToTempC(int raw)
{
    return 0.0f + (45.0f - 0.0f) * (float)raw / 4095.0f;
}
void Device::handleEncoderButton()
{
    unsigned long now = millis();
    if (now - lastBtnMs < DEBOUNCE_MS)
        return;
    if (digitalRead(ENC_SW) == LOW)
    { // INPUT_PULLUP: presionado = LOW
        lastBtnMs = now;
        screenIdx = (screenIdx + 1) % 2;
        Serial.println("[UI] Cambio a Pantalla" + String(screenIdx + 1));
        pararLedIntermitente();
        // refrescar inmediatamente y aplicar modo de LED de la pantalla
    }
}

void Device::prenderLed()
{
    digitalWrite(PIN_LED, HIGH);
}
void Device::apagarLed(){
    digitalWrite(PIN_LED, LOW);
}

void Device::ledIntermitente(int intervalo){
    ledInterval = intervalo;
    ledMillis = millis();
    ledState = true;
    digitalWrite(PIN_LED, HIGH);
}

void Device::pararLedIntermitente(){
    ledState = false;
    digitalWrite(PIN_LED, LOW);
}

void Device::actualizarLedIntermitente(){
    if (ledState) {
        unsigned long now = millis();
        if (now - ledMillis >= ledInterval) {
            ledMillis = now;
            // Cambia el estado del LED
            digitalWrite(PIN_LED, !digitalRead(PIN_LED));
        }
    }
}

void Device::showScreenTemp()
{
    // Leer sensor
    int valor = readPot();
    float tempC_pot = mapPotToTempC(valor);
    float tempC = readTemp(); // en Celsius
    String texto = "Pantalla Temperatura \n \n Temp: " + String(tempC, 2) + " C\n\n  Temp Ref: " + String(tempC_pot, 2) + " C";

    if (tempC > tempC_pot)
    {
        prenderLed();
        escribirPantalla(texto + "\n Ventilacion ON");

        if (!ventilacionEncendida)
        {
            ventilacionEncendida = true;
            Serial.println("Ventilacion encendida");
        }
    }
    else
    {
        apagarLed();
        escribirPantalla(texto + "\n Ventilacion OFF");
        if (ventilacionEncendida)
        {
            ventilacionEncendida = false;
            Serial.println("Ventilacion apagada");
        }
    }
}

void Device::showScreenHum(){
    float humedadActual = readHum();
    String textoRiego = "Humedad min deseada:\n" + String(humedadMinimaDeseada, 0) + "%\n\n" + "Humedad actual:\n" + humedadActual + "%";

    if (humedadActual < humedadMinimaDeseada){
        if(!ledState){
            ledIntermitente(200);
        }
        textoRiego += "\n\n Riego Encendido";
        if (!riegoEncendido){
            riegoEncendido = true;
            Serial.println("Riego encendido");
        }
    }
    else{
        pararLedIntermitente();
        textoRiego += "\n\n Riego Apagado";
        if (riegoEncendido){
            riegoEncendido = false;
            Serial.println("Riego apagado");
        }
    }
    escribirPantalla(textoRiego);
}



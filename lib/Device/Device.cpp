#include "Device.h"


Device::Device(int w, int h, int reset,int pinDHT, const uint8_t model, int pinPOT, int pinBtn, int pinLED, int pinCLK, int pinDT) 
: 
_sensor(pinDHT, model), 
_display(w, h, &Wire, reset), 
encoder(pinDT, pinCLK, RotaryEncoder::LatchMode::FOUR3)
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
    mostrarMenuPrincipal();
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



void Device::actualizarTemperatura(){
    bool estadoAnterior = ventilacionEncendida;
    float valor = readPot();
    tempPot = mapPotToTempC(valor);
    temp = readTemp(); 
    ventilacionEncendida = (temp > tempPot);
    
    if (ventilacionEncendida && !estadoAnterior){
        Serial.println("Ventilacion encendida");
    } else if (!ventilacionEncendida && estadoAnterior){
        Serial.println("Ventilacion apagada");
    }

}

void Device::mostrarPantallaTemperatura(){
    // Leer sensor
    
    String texto = "Pantalla Temperatura \n \n Temp: " + String(temp, 2) + " C\n\n  Temp Ref: " + String(tempPot, 2) + " C";

    if (ventilacionEncendida)
    {
        prenderLed();
        escribirPantalla(texto + "\n Ventilacion ON");
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

void Device::actualizarHumedad(){
    bool estadoAnterior = riegoEncendido;
    humedad = readHum();
    riegoEncendido = (humedad < humedadMinimaDeseada);
    if (riegoEncendido && !estadoAnterior){
        Serial.println("riego encendido");
    } else if (!riegoEncendido && estadoAnterior){
        Serial.println("riego apagado");
    }

}

void Device::mostrarPantallaHumedad(){
    String textoRiego = "Humedad min deseada:\n" + String(humedadMinimaDeseada, 0) + "%\n\n" + "Humedad actual:\n" + humedad + "%";

    if (riegoEncendido){
        if(!ledState){
            ledIntermitente(200);
        }
        textoRiego += "\n\n Riego Encendido";
        if (!riegoEncendido){
            Serial.println("Riego encendido");
        }
    }
    else{
        pararLedIntermitente();
        textoRiego += "\n\n Riego Apagado";
        if (riegoEncendido){
            Serial.println("Riego apagado");
        }
    }
    escribirPantalla(textoRiego);
}

void Device::mostrarPantallaMonitor(){
    String texto = "=== MONITOR ===\n\n";
    texto += "Temp: " + String(temp, 1) + "C\n";
    texto += "Humedad: " + String(humedad, 1) + "%\n\n";
    texto += "Ventilador: " + String(ventilacionEncendida ? "ON" : "OFF") + "\n";
    texto += "Riego: " + String(riegoEncendido ? "ON" : "OFF") + "\n\n";
    texto += "Giro antihorario: SALIR";

    escribirPantalla(texto);
}


// ========== FUNCIONES DEL ENCODER ==========
//valida si hubo o no rotacion y en que direccion
void Device::checkRotaryEncoder() {
    encoder.tick();
    
    int posicionActual = encoder.getPosition();
    
    if (posicionActual != posicionAnterior) {
        RotaryEncoder::Direction direction = encoder.getDirection();
        
        if (direction == RotaryEncoder::Direction::CLOCKWISE) {
            handleRotation("horario");
            Serial.println("Encoder: Giro horario");
        } 
        else if (direction == RotaryEncoder::Direction::COUNTERCLOCKWISE) {
            handleRotation("antihorario");
            Serial.println("Encoder: Giro antihorario");
        }
        
        posicionAnterior = posicionActual;
    }
}

void Device::handleRotation(String sentido) {
    
    if (!dentroDeOpcion) {
        // En el menú: solo giro horario entra en opción
        if (sentido == "horario") {
            // Giro horario: ENTRAR en la opción seleccionada
            dentroDeOpcion = true;
            mostrarOpcion(opcionActual);
            Serial.println("ENTRANDO en opcion: " + String(opcionActual));
        }
        // Giro antihorario en menú: no hace nada
        
    } else {
        // Dentro de opción: solo antihorario sale
        if (sentido == "antihorario") {
            // Giro antihorario: SALIR al menú
            dentroDeOpcion = false;
            pararLedIntermitente();
            mostrarMenuPrincipal();
            Serial.println("VOLVIENDO al menu");
        }
        // Giro horario dentro de opción: no hace nada
    }
}

// ========== FUNCIONES DEL MENÚ ==========

void Device::actualizarMenu() {
    // Esta función va en el loop() principal
    checkRotaryEncoder();
    leerBoton();
    actualizarLedIntermitente();

    actualizarTemperatura();
    actualizarHumedad();
    
    
    // Actualizar pantalla si estamos dentro de una opción
    if (dentroDeOpcion) {
        static unsigned long ultimaActualizacion = 0;
        if (millis() - ultimaActualizacion > 1000) { // Cada segundo
            mostrarOpcion(opcionActual);
            ultimaActualizacion = millis();
        }
    }
}

void Device::leerEncoder() {
    int estadoActual = digitalRead(PIN_CLK);
    
    // Detectar cambio en CLK
    if (estadoActual != lastStateCLK && estadoActual == LOW) {
        if (millis() - ultimoEncoder > 50) { // Debounce
            
            if (!dentroDeOpcion) {
                // En el menú: solo giro horario entra en opción
                if (digitalRead(PIN_DT) != estadoActual) {
                    // Giro horario: ENTRAR en la opción seleccionada
                    dentroDeOpcion = true;
                    mostrarOpcion(opcionActual);
                    Serial.println("ENTRANDO en opcion: " + String(opcionActual));
                }
                // Giro antihorario en menú: no hace nada
                
            } else {
                // Dentro de opción: solo antihorario sale
                if (digitalRead(PIN_DT) == estadoActual) {
                    // Giro antihorario: SALIR al menú
                    dentroDeOpcion = false;
                    pararLedIntermitente();
                    mostrarMenuPrincipal();
                    Serial.println("VOLVIENDO al menu");
                }
                // Giro horario dentro de opción: no hace nada
            }
            
            ultimoEncoder = millis();
        }
    }
    lastStateCLK = estadoActual;
}

void Device::leerBoton() {
    if (digitalRead(ENC_SW) == LOW) {
        if (millis() - ultimoBoton > 300) { // Debounce
            
            if (!dentroDeOpcion) {
                // En menú: NAVEGAR a siguiente opción
                opcionActual = (opcionActual + 1) % totalOpciones;
                mostrarMenuPrincipal();
                Serial.println("NAVEGANDO a opcion: " + String(opcionActual));
            }
            // Si estamos dentro de opción, el botón no hace nada
            
            ultimoBoton = millis();
        }
    }
}

void Device::mostrarMenuPrincipal() {
    String menu = " == MENU PRINCIPAL ==\n\n";
    
    String opciones[] = {"Monitor", "Temperatura", "Humedad"};
    
    for (int i = 0; i < totalOpciones; i++) {
        if (i == opcionActual) {
            menu += "> " + opciones[i] + " <\n";
        } else {
            menu += "  " + opciones[i] + "\n";
        }
    }
    
    menu += "\nBoton: NAVEGAR\nGiro horario: ENTRAR";
    escribirPantalla(menu);
}

void Device::mostrarOpcion(int cual) {
    switch (cual) {
        case 0: // Monitor
        {
            mostrarPantallaMonitor();
            break;
        }
        case 1: // Temperatura
            mostrarPantallaTemperatura();
            break;
        case 2: // Humedad  
            mostrarPantallaHumedad();
            break;
    }
}
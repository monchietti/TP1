//Archivo .h
#ifndef DEVICE
#define DEVICE 

//para que tenga las constantes y funciones de arduino
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_SH110X.h>



class Device{
    //Por convencion de C++, primero la parte publica y luego la privada
    public:
        Device(int w, int h, int reset,int pinDHT, const uint8_t model, int pinPOT, int pinBtn, int pinLED);
        void begin(int I2C_SDA,int I2C_SCL); //Inicia el atributo sensor y el atributo display
        void escribirPantalla(String texto); //puntero a un array de letras
        float readTemp();
        float readHum();
        float readPot();
        float mapPotToTempC(int raw);
        void handleEncoderButton();
        void showScreenTemp();
        void showScreenHum();
        void prenderLed();
        void apagarLed();
        void ledIntermitente(int lapso);
        void pararLedIntermitente();
        void actualizarLedIntermitente();

        //void loop(); //podria tener toda la logica metida en un loop
    private:
        unsigned long ledMillis = 0;
        bool ledState = false;
        int ledInterval = 200;
        DHT _sensor;
        Adafruit_SH1106G _display;
        unsigned long lastBtnMs = 0; 
        const unsigned long DEBOUNCE_MS = 200; 
        float humedadMinimaDeseada = 0.0; 
        bool btnPrev = HIGH;
        
    public:
        uint8_t screenIdx = 0; // pantalla actual
        bool ventilacionEncendida = false;
        bool riegoEncendido = false;
        int PIN_POT;
        int ENC_SW;
        int PIN_LED;

}; //no olvidarse del ;


#endif
//Archivo .h
#ifndef DEVICE
#define DEVICE 

//para que tenga las constantes y funciones de arduino
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_SH110X.h>
#include <RotaryEncoder.h>



class Device{
    //Por convencion de C++, primero la parte publica y luego la privada
    public:
        Device(int w, int h, int reset,int pinDHT, const uint8_t model, int pinPOT, int pinBtn, int pinLED, int pinCLK, int pinDT);
        void begin(int I2C_SDA,int I2C_SCL); //Inicia el atributo sensor y el atributo display
        void escribirPantalla(String texto); //puntero a un array de letras
        float readTemp();
        float readHum();
        float readPot();
        float mapPotToTempC(int raw);
        //funciones de actualizacion de datos
        void actualizarHumedad();
        void actualizarTemperatura();
        
        //funciones del encoder
        void tick();
        void chequearEncoder();
        void manejarRotacion(String sentido);

        //funciones del menu
        void mostrarPantallaTemperatura();
        void mostrarPantallaHumedad();
        void mostrarPantallaMonitor();

        void actualizarMenu();
        void leerEncoder();
        void leerBoton();
        void mostrarMenuPrincipal();
        void mostrarOpcion(int cual);
        void prenderLed();
        void apagarLed();
        void ledIntermitente(int lapso);
        void pararLedIntermitente();
        void actualizarLedIntermitente();

    private:
        RotaryEncoder encoder;
        DHT _sensor;
        Adafruit_SH1106G _display;
        unsigned long ledMillis = 0;
        bool ledState = false;
        int ledInterval = 200;
        
        unsigned long lastBtnMs = 0; 
        const unsigned long DEBOUNCE_MS = 200; 
        float humedadMinimaDeseada = 0.0; 
        bool btnPrev = HIGH;
        // Pines del encoder
        int PIN_CLK = 18;
        int PIN_DT = 5;
        int lastStateCLK;
        // Variables del menú
        int opcionActual = 0;        // Qué opción está seleccionada
        int totalOpciones = 3;       // Cuántas opciones hay
        bool dentroDeOpcion = false; // ¿Estamos viendo una opción o en el menú?
        unsigned long ultimoEncoder = 0;
        unsigned long ultimoBoton = 0;

        //variables del sensor
        float humedad = 0.0;
        float temp = 0.0;
        float tempPot = 0.0;

        int posicionAnterior;
        
    public:
        uint8_t screenIdx = 0; // pantalla actual
        bool ventilacionEncendida = false;
        bool riegoEncendido = false;
        int PIN_POT;
        int ENC_SW;
        int PIN_LED;

}; //no olvidarse del ;


#endif
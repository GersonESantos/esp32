/*
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 * - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 * - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif 

#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
  #include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProDimSwitch.h"

#define WIFI_SSID         "BRUGER_2G"     
#define WIFI_PASS         "Gersones68"



#define APP_KEY           "89cda427-c430-4127-9a60-afd89f2364d7"       
#define APP_SECRET        "a4993c6e-91d9-4b84-97f6-373c1d78789b-72975cde-8f6e-4ff5-b9a1-b67a298291b5"   
#define DIMSWITCH_ID      "68cb08fdad5e91d6f437c17e"

// -----------------------------------------------------------------
// INÍCIO DAS MODIFICAÇÕES - PINO DO LED
// -----------------------------------------------------------------

// Define o pino GPIO 13 para o LED (conforme solicitado para o ESP32)
#define LED_PIN 13 

// -----------------------------------------------------------------
// FIM DAS MODIFICAÇÕES - PINO DO LED
// -----------------------------------------------------------------


#define BAUD_RATE         115200             // Change baudrate to your need

// we use a struct to store all states and values for our dimmable switch
struct {
  bool powerState = false;
  int powerLevel = 0;
} device_state;


// -----------------------------------------------------------------
// INÍCIO DAS MODIFICAÇÕES - CALLBACKS DE CONTROLE
// -----------------------------------------------------------------

/**
 * @brief Função auxiliar para definir o brilho do LED (0-100%)
 * * Converte o valor 0-100 (do SinricPro) para a escala de PWM
 * do ESP32 (0-255) e controla o pino do LED.
 */
void setPower(int powerLevel) {
  // 1. Limita o valor de entrada entre 0 (desligado) e 100 (máximo)
  if (powerLevel < 0)   powerLevel = 0;
  if (powerLevel > 100) powerLevel = 100;

  // 2. Atualiza as variáveis de estado globais
  device_state.powerLevel = powerLevel;
  device_state.powerState = (powerLevel > 0); // Se o brilho for > 0, está ligado

  // 3. Converte a escala 0-100 (SinricPro) para 0-255 (PWM do ESP32)
  int dutyCycle = map(device_state.powerLevel, 0, 100, 0, 255);
  
  // 4. Aplica o brilho (dutyCycle) ao pino 13
  analogWrite(LED_PIN, dutyCycle);
}


/**
 * @brief Callback chamado ao Ligar/Desligar o dispositivo
 */
bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Device %s power turned %s \r\n", deviceId.c_str(), state?"on":"off");
  
  if (state) { // Se o comando for LIGAR
    // Se estava desligado (nível 0), liga no máximo (100).
    // Se já tinha um nível (ex: 50), apenas restaura esse nível.
    if (device_state.powerLevel == 0) {
      setPower(100);
    } else {
      setPower(device_state.powerLevel);
    }
  } else { // Se o comando for DESLIGAR
    setPower(0);
  }
  
  return true; // request handled properly
}

/**
 * @brief Callback chamado ao definir um nível de brilho (ex: "Alexa, 50%")
 */
bool onPowerLevel(const String &deviceId, int &powerLevel) {
  Serial.printf("Device %s power level changed to %d\r\n", deviceId.c_str(), powerLevel);
  setPower(powerLevel); // Apenas define o novo nível de brilho
  return true;
}

/**
 * @brief Callback chamado ao ajustar o brilho (ex: "Alexa, aumente o brilho")
 */
bool onAdjustPowerLevel(const String &deviceId, int &levelDelta) {
  // Calcula o novo nível (nível atual + ajuste)
  int newLevel = device_state.powerLevel + levelDelta;
  Serial.printf("Device %s power level changed about %i to %d\r\n", deviceId.c_str(), levelDelta, newLevel);
  
  setPower(newLevel); // Define o novo nível
  
  levelDelta = device_state.powerLevel; // Retorna o nível final para o SinricPro
  return true;
}

// -----------------------------------------------------------------
// FIM DAS MODIFICAÇÕES - CALLBACKS DE CONTROLE
// -----------------------------------------------------------------


void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");

  #if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP); 
    WiFi.setAutoReconnect(true);
  #elif defined(ESP32)
    WiFi.setSleep(false); 
    WiFi.setAutoReconnect(true);
  #endif

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %d.%d.%d.%d\r\n", localIP[0], localIP[1], localIP[2], localIP[3]);
}

void setupSinricPro() {
  SinricProDimSwitch &myDimSwitch = SinricPro[DIMSWITCH_ID];

  // set callback function to device
  myDimSwitch.onPowerState(onPowerState);
  myDimSwitch.onPowerLevel(onPowerLevel);
  myDimSwitch.onAdjustPowerLevel(onAdjustPowerLevel);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");

  // -----------------------------------------------------------------
  // INÍCIO DAS MODIFICAÇÕES - SETUP DO PINO
  // -----------------------------------------------------------------
  
  // Configura o pino 13 como SAÍDA
  pinMode(LED_PIN, OUTPUT);     
  // Garante que o LED começa desligado
  digitalWrite(LED_PIN, LOW); 
  
  // -----------------------------------------------------------------
  // FIM DAS MODIFICAÇÕES - SETUP DO PINO
  // -----------------------------------------------------------------
  
  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();
}
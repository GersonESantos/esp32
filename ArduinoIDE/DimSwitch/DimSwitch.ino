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
#define DIMSWITCH_ID      "68d98569c6b3a7ebd1b62c43"

// MODIFICADO: Definindo o pino do LED para fácil alteração
#define LED_PIN           13

#define BAUD_RATE         115200          // Change baudrate to your need

// MODIFICADO: Renomeado para myDeviceState para evitar conflitos com a biblioteca
struct {
  bool powerState = false;
  int powerLevel = 0;
} myDeviceState;

// MODIFICADO: Esta função agora controla o LED
bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Device %s power turned %s \r\n", deviceId.c_str(), state?"on":"off");
  myDeviceState.powerState = state;
  
  if (state) {
    // Se ligar e o brilho for 0, define como 100%
    if (myDeviceState.powerLevel == 0) {
      myDeviceState.powerLevel = 100;
    }
    // Mapeia 0-100 para 0-255 e aplica ao LED
    int dutyCycle = map(myDeviceState.powerLevel, 0, 100, 0, 255);
    analogWrite(LED_PIN, dutyCycle);
  } else {
    // Se desligar, define o brilho como 0
    analogWrite(LED_PIN, 0);
  }

  return true; // request handled properly
}

// MODIFICADO: Esta função agora controla o brilho do LED
bool onPowerLevel(const String &deviceId, int &powerLevel) {
  myDeviceState.powerLevel = powerLevel;
  Serial.printf("Device %s power level changed to %d\r\n", deviceId.c_str(), myDeviceState.powerLevel);
  
  // Mapeia o novo brilho (0-100) para a escala PWM (0-255) e aplica ao LED
  int dutyCycle = map(myDeviceState.powerLevel, 0, 100, 0, 255);
  analogWrite(LED_PIN, dutyCycle);

  return true;
}

// MODIFICADO: Esta função agora ajusta o brilho do LED
bool onAdjustPowerLevel(const String &deviceId, int &levelDelta) {
  // Ajusta o nível de brilho e garante que ele fique entre 0 e 100
  myDeviceState.powerLevel += levelDelta;
  if (myDeviceState.powerLevel < 0) myDeviceState.powerLevel = 0;
  if (myDeviceState.powerLevel > 100) myDeviceState.powerLevel = 100;

  Serial.printf("Device %s power level changed about %i to %d\r\n", deviceId.c_str(), levelDelta, myDeviceState.powerLevel);
  
  // Mapeia o novo brilho para a escala PWM e aplica ao LED
  int dutyCycle = map(myDeviceState.powerLevel, 0, 100, 0, 255);
  analogWrite(LED_PIN, dutyCycle);
  
  levelDelta = myDeviceState.powerLevel; // Retorna o novo nível para o SinricPro
  return true;
}

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
  
  // MODIFICADO: Configura o pino do LED como saída
  pinMode(LED_PIN, OUTPUT);
  // MODIFICADO: Garante que o LED comece desligado
  digitalWrite(LED_PIN, LOW);

  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();
}
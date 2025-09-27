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
// -----------------------------------------------------------------
// INÍCIO DAS MODIFICAÇÕES - SENSOR DE UMIDADE
// -----------------------------------------------------------------
#include "SinricProTemperaturesensor.h" // Usaremos este tipo de dispositivo para reportar a umidade
// -----------------------------------------------------------------
// FIM DAS MODIFICAÇÕES - SENSOR DE UMIDADE
// -----------------------------------------------------------------

#define WIFI_SSID         "BRUGER_2G"     
#define WIFI_PASS         "Gersones68"

#define APP_KEY           "89cda427-c430-4127-9a60-afd89f2364d7"      
#define APP_SECRET        "a4993c6e-91d9-4b84-97f6-373c1d78789b-72975cde-8f6e-4ff5-b9a1-b67a298291b5"   

// --- Dispositivo Dimmer (Luz)
#define DIMSWITCH_ID      "68cb08fdad5e91d6f437c17e"
#define LED_PIN 13 

// -----------------------------------------------------------------
// INÍCIO DAS MODIFICAÇÕES - SENSOR DE UMIDADE
// -----------------------------------------------------------------
// --- Dispositivo Sensor de Umidade
#define SOIL_SENSOR_ID    "COLE_SEU_ID_AQUI" // << IMPORTANTE: Substitua pelo ID do seu dispositivo no SinricPro
#define SOIL_SENSOR_PIN   34                 // Pino GPIO34 para o sensor de umidade do solo

// --- Calibração do Sensor (AJUSTE ESTES VALORES!)
// Para calibrar:
// 1. Verifique o valor lido com o sensor no ar (seco) e coloque em DRY_VALUE.
// 2. Verifique o valor lido com o sensor 100% submerso em água e coloque em WET_VALUE.
// Sensor capacitivo: valor maior = mais seco; valor menor = mais úmido.
#define DRY_VALUE 2800 // Valor analógico para solo seco (exemplo)
#define WET_VALUE 1300 // Valor analógico para solo 100% úmido (exemplo)

unsigned long lastSensorRead = 0;       // Variável para controlar o tempo de envio dos dados
const long readInterval = 10000;      // Intervalo para enviar dados do sensor (10 segundos)
// -----------------------------------------------------------------
// FIM DAS MODIFICAÇÕES - SENSOR DE UMIDADE
// -----------------------------------------------------------------

#define BAUD_RATE         115200          // Change baudrate to your need

// we use a struct to store all states and values for our dimmable switch
struct {
  bool powerState = false;
  int powerLevel = 0;
} device_state;


// Função auxiliar para definir o brilho do LED (0-100%)
void setPower(int powerLevel) {
  if (powerLevel < 0)   powerLevel = 0;
  if (powerLevel > 100) powerLevel = 100;

  device_state.powerLevel = powerLevel;
  device_state.powerState = (powerLevel > 0);

  int dutyCycle = map(device_state.powerLevel, 0, 100, 0, 255);
  analogWrite(LED_PIN, dutyCycle);
}

// Callback chamado ao Ligar/Desligar o dispositivo
bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Device %s power turned %s \r\n", deviceId.c_str(), state?"on":"off");
  
  if (state) {
    if (device_state.powerLevel == 0) {
      setPower(100);
    } else {
      setPower(device_state.powerLevel);
    }
  } else {
    setPower(0);
  }
  
  return true; // request handled properly
}

// Callback chamado ao definir um nível de brilho
bool onPowerLevel(const String &deviceId, int &powerLevel) {
  Serial.printf("Device %s power level changed to %d\r\n", deviceId.c_str(), powerLevel);
  setPower(powerLevel);
  return true;
}

// Callback chamado ao ajustar o brilho
bool onAdjustPowerLevel(const String &deviceId, int &levelDelta) {
  int newLevel = device_state.powerLevel + levelDelta;
  Serial.printf("Device %s power level changed about %i to %d\r\n", deviceId.c_str(), levelDelta, newLevel);
  
  setPower(newLevel);
  
  levelDelta = device_state.powerLevel;
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
  // Configuração do Dimmer Switch
  SinricProDimSwitch &myDimSwitch = SinricPro[DIMSWITCH_ID];
  myDimSwitch.onPowerState(onPowerState);
  myDimSwitch.onPowerLevel(onPowerLevel);
  myDimSwitch.onAdjustPowerLevel(onAdjustPowerLevel);

  // -----------------------------------------------------------------
  // INÍCIO DAS MODIFICAÇÕES - SENSOR DE UMIDADE
  // -----------------------------------------------------------------
  // Nenhuma callback é necessária aqui, pois o sensor apenas ENVIA dados.
  // A biblioteca já prepara o dispositivo para enviar eventos.
  SinricProTemperaturesensor &mySoilSensor = SinricPro[SOIL_SENSOR_ID];
  // -----------------------------------------------------------------
  // FIM DAS MODIFICAÇÕES - SENSOR DE UMIDADE
  // -----------------------------------------------------------------

  // Configuração geral do SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
  
  // Configura o pino do LED como SAÍDA
  pinMode(LED_PIN, OUTPUT);     
  digitalWrite(LED_PIN, LOW); 
  
  // O pino do sensor (GPIO34) é apenas de entrada (ADC), não precisa de pinMode.
  
  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();

  // -----------------------------------------------------------------
  // INÍCIO DAS MODIFICAÇÕES - SENSOR DE UMIDADE
  // -----------------------------------------------------------------
  // Verifica se já passou o tempo definido em 'readInterval'
  if (millis() - lastSensorRead > readInterval) {
    // Lê o valor analógico do sensor
    int rawValue = analogRead(SOIL_SENSOR_PIN);
    
    // Mapeia o valor bruto para uma porcentagem de 0 a 100
    // A função map inverte os valores: DRY_VALUE (maior) -> 0%, WET_VALUE (menor) -> 100%
    float humidity = map(rawValue, DRY_VALUE, WET_VALUE, 0, 100);
    
    // Garante que o valor fique entre 0 e 100
    humidity = constrain(humidity, 0, 100);

    Serial.printf("Leitura do sensor: %d | Umidade: %.2f%%\r\n", rawValue, humidity);

    // Pega a referência do nosso sensor no SinricPro
    SinricProTemperaturesensor &mySoilSensor = SinricPro[SOIL_SENSOR_ID];

    // Envia o evento de umidade para o SinricPro
    // O primeiro parâmetro é temperatura (não usamos), o segundo é umidade.
    bool success = mySoilSensor.sendHumidityEvent(humidity);

    if (success) {
      Serial.printf("Umidade enviada para o SinricPro com sucesso!\r\n");
    } else {
      Serial.printf("Falha ao enviar umidade para o SinricPro!\r\n");
    }

    // Atualiza o tempo da última leitura
    lastSensorRead = millis();
  }
  // -----------------------------------------------------------------
  // FIM DAS MODIFICAÇÕES - SENSOR DE UMIDADE
  // -----------------------------------------------------------------
}
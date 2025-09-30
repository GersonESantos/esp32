/*
 * Example
 ...
 */

#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
#endif

#include <SinricPro.h>
#include "SensordeUmidadeDoSolo.h" // Garanta que este arquivo .h está na pasta do projeto

#define APP_KEY    "89cda427-c430-4127-9a60-afd89f2364d7"
#define APP_SECRET "a4993c6e-91d9-4b84-97f6-373c1d78789b-72975cde-8f6e-4ff5-b9a1-b67a298291b5"
#define DEVICE_ID  "68daf7175918d860c09da1fb"

#define SSID       "BRUGER_2G"
#define PASS       "Gersones68"

#define BAUD_RATE  115200

// NOVO: Definições do sensor de umidade
#define PINO_SENSOR_UMIDADE 34 // Pino analógico onde o sensor está conectado (GPIO 34)
#define SENSOR_SECO         2800 // Valor ADC para solo seco (VOCÊ PRECISA CALIBRAR ISSO!)
#define SENSOR_MOLHADO      1300 // Valor ADC para solo 100% úmido (VOCÊ PRECISA CALIBRAR ISSO!)

// NOVO: Controle de tempo para a leitura do sensor
unsigned long ultimaLeitura = 0;
const unsigned long intervaloLeitura = 5000; // Ler a cada 5 segundos (5000 ms)

SensordeUmidadeDoSolo &sensordeUmidadeDoSolo = SinricPro[DEVICE_ID];

/*************
 * Variables *
 *************/
std::map<String, int> globalRangeValues;
std::map<String, String> globalModes;

/*************
 * Callbacks *
 *************/
bool onRangeValue(const String &deviceId, const String& instance, int &rangeValue) { /* ...código original... */ return true; }
bool onAdjustRangeValue(const String &deviceId, const String& instance, int &valueDelta) { /* ...código original... */ return true; }
bool onSetMode(const String& deviceId, const String& instance, String &mode) { /* ...código original... */ return true; }

/**********
 * Events *
 **********/
void updateRangeValue(String instance, int value) {
  sensordeUmidadeDoSolo.sendRangeValueEvent(instance, value);
}
void updateMode(String instance, String mode) {
  sensordeUmidadeDoSolo.sendModeEvent(instance, mode, "PHYSICAL_INTERACTION");
}

/*********
 * Setup *
 *********/
void setupSinricPro() {
  sensordeUmidadeDoSolo.onRangeValue("rangeInstance1", onRangeValue);
  sensordeUmidadeDoSolo.onAdjustRangeValue("rangeInstance1", onAdjustRangeValue);
  sensordeUmidadeDoSolo.onSetMode("modeInstance1", onSetMode);

  SinricPro.onConnected([]{ Serial.printf("[SinricPro]: Connected\r\n"); });
  SinricPro.onDisconnected([]{ Serial.printf("[SinricPro]: Disconnected\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
};

void setupWiFi() {
  WiFi.begin(SSID, PASS);
  Serial.printf("[WiFi]: Connecting to %s", SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress ip = WiFi.localIP();
  Serial.printf("connected, IP: %s\r\n", ip.toString().c_str());
}

void setup() {
  Serial.begin(BAUD_RATE);
  // NOVO: O pino do sensor, por ser analógico, não precisa de pinMode no ESP32.
  setupWiFi();
  setupSinricPro();
}

/********
 * Loop *
 ********/
void loop() {
  SinricPro.handle();

  // NOVO: Lógica para ler e imprimir a umidade periodicamente
  if (millis() - ultimaLeitura >= intervaloLeitura) {
    // 1. Lê o valor bruto do sensor
    int valorBruto = analogRead(PINO_SENSOR_UMIDADE);

    // 2. Converte o valor bruto para porcentagem (0-100%)
    //    Nota: a lógica é invertida, pois um valor ADC maior significa solo mais seco.
    int umidadePercentual = map(valorBruto, SENSOR_SECO, SENSOR_MOLHADO, 0, 100);

    // 3. Garante que o valor fique entre 0 e 100
    if (umidadePercentual < 0) umidadePercentual = 0;
    if (umidadePercentual > 100) umidadePercentual = 100;

    // 4. Imprime os valores no Monitor Serial
    Serial.printf("Leitura do Sensor: [Bruto: %d] -> [Umidade: %d %%]\n", valorBruto, umidadePercentual);

    // 5. Envia os dados para o Sinric Pro
    updateRangeValue("rangeInstance1", umidadePercentual); // Envia a porcentagem

    // Define o modo "Dry" ou "Wet" com base em um limite (ex: 30%)
    if (umidadePercentual < 30) {
      updateMode("modeInstance1", "Dry");
    } else {
      updateMode("modeInstance1", "Wet");
    }
    
    ultimaLeitura = millis(); // Reseta o temporizador para a próxima leitura
  }
}
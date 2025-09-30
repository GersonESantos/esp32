/***********************************************************************
 * CÓDIGO MESCLADO: MONITOR AMBIENTAL + DIMMER DE LED
 * * Funcionalidades:
 * - Controla um LED dimmer (liga/desliga e brilho) via Sinric Pro.
 * - Lê um sensor DHT22 (temperatura e umidade) e um LDR (luminosidade).
 * - Envia os dados dos sensores para o Sinric Pro.
 * - Permite ligar/desligar o envio de dados via comando ou botão físico.
 ***********************************************************************/

// --- Bibliotecas (Junção de ambos os códigos) ---
#include <WiFi.h>
#include <TimeLib.h>
#include <DHT.h>
#include <SinricPro.h>
#include "SinricProDimSwitch.h"
#include "TempUmidLumiLed.h"

// --- Configurações de Rede e Sinric Pro ---
#define WIFI_SSID         "BRUGER_2G"
#define WIFI_PASS         "Gersones68"

// Use apenas UM conjunto de chaves da sua conta Sinric Pro
#define APP_KEY           "89cda427-c430-4127-9a60-afd89f2364d7"
#define APP_SECRET        "a4993c6e-91d9-4b84-97f6-373c1d78789b-72975cde-8f6e-4ff5-b9a1-b67a298291b5"

// --- IDs dos Dispositivos (Um para cada função) ---
#define MONITOR_ID        "68daca785918d860c09d9254" // ID do dispositivo "Temperature Sensor"
#define DIMSWITCH_ID      "68d98569c6b3a7ebd1b62c43" // ID do dispositivo "Dimmable Switch"

// --- Configurações de Hardware (Junção de ambos os códigos) ---
#define LED_PIN           13  // Pino do LED do Dimmer
#define DHT_PIN           27  // Pino do sensor DHT22 do Monitor
#define LDR_PIN           33  // Pino do sensor LDR do Monitor
#define BOTAO_PIN         0   // Pino do botão físico do Monitor

// --- Constantes ---
#define BAUD_RATE         115200
const uint16_t  INTERVALO_SENSOR      = 60; // Intervalo de atualização dos sensores (60s)
const char* NTP_SERVER            = "pool.ntp.org";
const int8_t    DATAHORA_FUSO         = -3;

// --- Variáveis Globais e Objetos (Junção de ambos os códigos) ---

// Variáveis do Monitor Ambiental
bool            monitorLigado         = true; // Renomeado de "ligado" para evitar conflito
time_t          proxAtualizacao       = 0;
float           temperatura;
float           umidade;
uint8_t         luminosidade;
DHT             dht(DHT_PIN, DHT22);

// Variáveis do Dimmer
struct {
  bool powerState = false;
  int powerLevel = 0;
} dimmerState; // Renomeado de "myDeviceState" para clareza

// --- Funções de Apoio (do código do Monitor) ---
String dateTimeStr(time_t t, const int8_t tz = 0, const bool flBr = true);
void log(const String &s) { Serial.println(dateTimeStr(time(NULL), DATAHORA_FUSO) + " " + s); }

// --- Funções de Callback (MESCLADAS) ---

// Callback para ligar/desligar o ENVIO DE DADOS DO MONITOR
bool onMonitorPowerState(const String &deviceId, bool &state) {
  log("Comando de energia para o MONITOR: " + String(state ? "LIGADO" : "DESLIGADO"));
  monitorLigado = state;
  return true;
}

// Callback para ligar/desligar o LED DO DIMMER
bool onDimmerPowerState(const String &deviceId, bool &state) {
  Serial.printf("Comando de energia para o DIMMER: %s\n", state ? "LIGADO" : "DESLIGADO");
  dimmerState.powerState = state;
  if (state) {
    if (dimmerState.powerLevel == 0) {
      dimmerState.powerLevel = 100;
    }
    int dutyCycle = map(dimmerState.powerLevel, 0, 100, 0, 255);
    analogWrite(LED_PIN, dutyCycle);
  } else {
    analogWrite(LED_PIN, 0);
  }
  return true;
}

// Callback para definir o brilho do LED
bool onPowerLevel(const String &deviceId, int &powerLevel) {
  dimmerState.powerLevel = powerLevel;
  Serial.printf("Nível de brilho do Dimmer alterado para %d\n", dimmerState.powerLevel);
  int dutyCycle = map(dimmerState.powerLevel, 0, 100, 0, 255);
  analogWrite(LED_PIN, dutyCycle);
  return true;
}

// Callback para ajustar o brilho do LED
bool onAdjustPowerLevel(const String &deviceId, int &levelDelta) {
  dimmerState.powerLevel += levelDelta;
  if (dimmerState.powerLevel < 0) dimmerState.powerLevel = 0;
  if (dimmerState.powerLevel > 100) dimmerState.powerLevel = 100;
  Serial.printf("Nível de brilho do Dimmer ajustado para %d\n", dimmerState.powerLevel);
  int dutyCycle = map(dimmerState.powerLevel, 0, 100, 0, 255);
  analogWrite(LED_PIN, dutyCycle);
  levelDelta = dimmerState.powerLevel;
  return true;
}

// --- Funções de Configuração (MESCLADAS) ---

void setupWiFi() {
  Serial.print("\nConectando WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi conectado, IP "); Serial.println(WiFi.localIP());
}

void setupSinricPro() {
  // Configura o dispositivo MONITOR AMBIENTAL
  TemperaturaUmidadeLuminosidade &meuMonitor = SinricPro[MONITOR_ID];
  meuMonitor.onPowerState(onMonitorPowerState); // Usa o callback renomeado

  // Configura o dispositivo DIMMER
  SinricProDimSwitch &meuDimmer = SinricPro[DIMSWITCH_ID];
  meuDimmer.onPowerState(onDimmerPowerState); // Usa o callback renomeado
  meuDimmer.onPowerLevel(onPowerLevel);
  meuDimmer.onAdjustPowerLevel(onAdjustPowerLevel);

  // Inicia a conexão
  SinricPro.onConnected([](){ Serial.println("Sinric Pro conectado"); }); 
  SinricPro.onDisconnected([](){ Serial.println("Sinric Pro desconectado"); });
  SinricPro.restoreDeviceStates(true);
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// --- Função Principal de SETUP (MESCLADA) ---
void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("\n\n*** PROJETO MESCLADO: Monitor Ambiental + Dimmer ***");

  // --- ADICIONADO DO CÓDIGO DO DIMMER ---
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // --- ADICIONADO DO CÓDIGO DO MONITOR ---
  pinMode(BOTAO_PIN, INPUT_PULLUP); // Configura o pino do botão com resistor interno
  dht.begin();
  configTime(DATAHORA_FUSO * 3600, 0, NTP_SERVER);
  
  // --- MESCLADO ---
  setupWiFi();
  setupSinricPro();
}

// --- Função Principal de LOOP (MESCLADA) ---
void loop() {
  SinricPro.handle(); // Essencial para a comunicação

  // --- LÓGICA DO MONITOR AMBIENTAL ---
  if (time(NULL) > proxAtualizacao) {
    if (monitorLigado) {
      temperatura = dht.readTemperature();
      umidade = dht.readHumidity();
      if (isnan(temperatura) || isnan(umidade)) {
        log("Falha lendo DHT22");
      } else {
        luminosidade = map(analogRead(LDR_PIN), 0, 4095, 0, 100);
        
        // Envia os eventos para o dispositivo MONITOR
        TemperaturaUmidadeLuminosidade &meuMonitor = SinricPro[MONITOR_ID];
        meuMonitor.sendTemperatureEvent(temperatura, umidade);
        meuMonitor.sendRangeValueEvent("luminosidade", luminosidade);

        log("Dados atualizados: t=" + String(temperatura, 1) + "°C u=" + String(umidade, 1) + "% l=" + String(luminosidade) + "%");
      }
    } else {
      log("Monitor desligado, envio de dados pausado.");
    }
    proxAtualizacao = time(NULL) + INTERVALO_SENSOR;
  }

  // --- LÓGICA DO BOTÃO FÍSICO (DO MONITOR) ---
  if (!digitalRead(BOTAO_PIN)) {
    delay(100); // Debounce
    while (!digitalRead(BOTAO_PIN)) {
      yield();
    }
    monitorLigado = !monitorLigado; // Inverte o estado do monitor
    log("Botão pressionado. Estado do monitor: " + String(monitorLigado ? "LIGADO" : "DESLIGADO"));
    
    // Envia o novo estado para o Sinric Pro
    TemperaturaUmidadeLuminosidade &meuMonitor = SinricPro[MONITOR_ID];
    meuMonitor.sendPowerStateEvent(monitorLigado);
  }
}


// --- Implementação da Função de Apoio (do código do Monitor) ---
String dateTimeStr(time_t t, const int8_t tz, const bool flBr) {
  if (t == 0) return F("N/A");
  t += tz * 3600;
  char buff[20];
  if (flBr) {
    sprintf(buff, "%02d/%02d/%d %02d:%02d:%02d", day(t), month(t), year(t), hour(t), minute(t), second(t));
  } else {
    sprintf(buff, "%d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
  }
  return String(buff);
}
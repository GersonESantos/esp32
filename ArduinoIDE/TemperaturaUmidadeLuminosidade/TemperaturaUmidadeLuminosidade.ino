/********************************************************
 * CANAL INTERNET E COISAS
 * youtube.com/@internetecoisas
 * ESP32/ESP8266 - Monitor Ambiental via Alexa
 * 09/2025 - André Michelon
 */

// Versão do software -----------------------------------
#define       FIRMWARE_VERSION        "1.0.0"

// IDs Sinric Pro ---------------------------------------
// CORRIGIDO: Removido o bloco de chaves duplicado. 
// Use apenas o conjunto de chaves correto para o seu dispositivo.
#define       APP_KEY                 "89cda427-c430-4127-9a60-afd89f2364d7"      
#define       APP_SECRET              "a4993c6e-91d9-4b84-97f6-373c1d78789b-72975cde-8f6e-4ff5-b9a1-b67a298291b5"   
#define       DEVICE_ID               "68daca785918d860c09d9254"

// Bibliotecas ------------------------------------------
#ifdef ESP32
  #include <WiFi.h>                   // Biblioteca para ESP32
#elif defined ESP8266
  #include <ESP8266WiFi.h>            // Biblioteca para ESP8266
#else
  #error Placa inválida               // Erro, placa incorreta
#endif

#include <TimeLib.h>
#include <DHT.h>
#include <SinricPro.h>
#include "TemperaturaUmidadeLuminosidade.h"

// Wi-Fi ------------------------------------------------
const char* SSID                  = "BRUGER_2G";
const char* PASSWORD              = "Gersones68";

// Constantes -------------------------------------------
const uint16_t  INTERVALO             = 60; // Intervalo de atualização dos dados (60s)
#ifdef ESP32
  const uint8_t DHT_PIN               = 27;
  const uint8_t LDR_PIN               = 33;
#else // ESP8266
  const uint8_t DHT_PIN               = 12;
  const uint8_t LDR_PIN               = A0;
#endif
const char* NTP_SERVER            = "pool.ntp.org";
const int8_t    DATAHORA_FUSO         = -3;

// Variaveis globais ------------------------------------
bool            ligado                = true;
time_t          proxAtualizacao       = 0;
float           temperatura;
float           umidade;
uint8_t         luminosidade;

// Instâncias -------------------------------------------
DHT             dht(DHT_PIN, DHT22);
TemperaturaUmidadeLuminosidade &temperaturaUmidadeLuminosidade = SinricPro[DEVICE_ID];

// Funcoes Genericas ------------------------------------
const char* platform() { /* ...código original sem alterações... */ return "DESCONHECIDO"; }
String dateTimeStr(time_t t, const int8_t tz = 0, const bool flBr = true) { /* ...código original sem alterações... */ return ""; }
uint16_t analogAverage(const uint8_t &a) { /* ...código original sem alterações... */ return 0; }
void log(const String &s) { Serial.println(dateTimeStr(time(NULL), DATAHORA_FUSO) + " " + s); }

// Funcoes Sinric Pro -----------------------------------
bool onPowerState(const String &deviceId, bool &state) {
  ligado = state;
  Serial.printf("Estado: %s\n", ligado ? "ligado" : "desligado");
  return true;
}

void setupSinricPro() {
  temperaturaUmidadeLuminosidade.onPowerState(onPowerState);
  SinricPro.onConnected([](){ Serial.printf("Sinric Pro conectado\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Sinric Pro desconectado\n"); });
  SinricPro.restoreDeviceStates(true);
  SinricPro.begin(APP_KEY, APP_SECRET);
};

// Setup ------------------------------------------------
void setup() {
  #ifdef ESP32
    Serial.begin(115200);
  #else
    Serial.begin(74880);
  #endif

  Serial.println("\n\n*** CANAL INTERNET E COISAS ***\nMonitor Ambiental via ALexa");
  Serial.print("Plataforma: "); Serial.println(platform());
  
  configTime(DATAHORA_FUSO * 3600, 0, NTP_SERVER); // Ajuste para o fuso horário

  Serial.print("Conectando WiFi");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi conectado, IP "); Serial.println(WiFi.localIP());

  setupSinricPro();
  dht.begin();
}

// Loop -------------------------------------------------
void loop() {
  SinricPro.handle();

  if (time(NULL) > proxAtualizacao) {
    if (ligado) {
      if (isnan(dht.readTemperature())) {
        log("Falha lendo DHT22");
      } else {
        temperatura   = dht.readTemperature();
        umidade       = dht.readHumidity();
        #ifdef ESP32
          luminosidade  = analogAverage(LDR_PIN) * 100L / 4095;
        #else
          luminosidade  = analogAverage(LDR_PIN) * 100L / 1023;
        #endif
        
        temperaturaUmidadeLuminosidade.sendTemperatureEvent(temperatura, umidade);
        temperaturaUmidadeLuminosidade.sendRangeValueEvent("luminosidade", luminosidade);

        log("Dados atualizados t=" + String(temperatura, 1) + "°C u=" + String(umidade, 1) + "% l=" + String(luminosidade) + "%");
      }
    } else {
      log("Desligado");
    }
    proxAtualizacao = time(NULL) + INTERVALO;
  }

  // Verifica o botão no pino 0
  // Lembre-se que o pino 0 precisa de um resistor de pull-up para funcionar bem
  if (!digitalRead(0)) {
    delay(100); // Debounce
    while (!digitalRead(0)) {
      yield();
    }
    ligado = !ligado;
    Serial.println("Botão pressionado");
    Serial.printf("Estado: %s\n", ligado ? "ligado" : "desligado");
    temperaturaUmidadeLuminosidade.sendPowerStateEvent(ligado);
  }
}
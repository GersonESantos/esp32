/********************************************************
 * CANAL INTERNET E COISAS
 * youtube.com/@internetecoisas
 * ESP32/ESP8266 - Monitor Ambiental via Alexa
 * 09/2025 - André Michelon
 */

// Versão do software -----------------------------------
#define         FIRMWARE_VERSION        "1.0.0"

// IDs Sinric Pro ---------------------------------------
#define         APP_KEY                 "e71be708-4b89-4368-bb36-246791e3e8e8"
#define         APP_SECRET              "818424e2-cd8c-4f78-af36-6edaa53639eb-14c8c67b-e45f-415b-a20d-0526aa43be10"
#define         DEVICE_ID               "68d17bb1c6e94834159cad62"

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
const char*     SSID                  = "home";
const char*     PASSWORD              = "Home@135711";

// Constantes -------------------------------------------
// Intervalo de atualização dos dados (60s)
const uint16_t  INTERVALO             = 60;
#ifdef ESP32
  // Definição de pinos dos sensores para ESP32
  // DHT22
  const uint8_t DHT_PIN               = 27;
  // LDR
  const uint8_t LDR_PIN               = 33;
#else
  // Definição de pinos dos sensores para ESP8266
  // DHT22
  const uint8_t DHT_PIN               = 12;
  // LDR
  const uint8_t LDR_PIN               = A0;
#endif
// Servidor NTP
const char*     NTP_SERVER            = "pool.ntp.org";
// Fuso horário
const int8_t    DATAHORA_FUSO         = -3;

// Variaveis globais ------------------------------------
// Liga/Desliga
bool            ligado                = true;
// Proxima atualizacao de dados
time_t          proxAtualizacao       = 0;
// Temperatura
float           temperatura;
// Umidade
float           umidade;
// Luminosidade
uint8_t         luminosidade;

// Instâncias -------------------------------------------
DHT             dht(DHT_PIN, DHT22);
TemperaturaUmidadeLuminosidade &temperaturaUmidadeLuminosidade = SinricPro[DEVICE_ID];

// Funcoes Genericas ------------------------------------
const char* platform() {
  // Obtém a plataforma de hardware em uso
  #if defined(__AVR_ATmega328P__)
    return "UNO/NANO";
  #elif defined(__AVR_ATmega2560__)
    return "MEGA";
  #elif defined(__AVR_ATmega32U4__)
    return "LEONARDO/MICRO";
  #elif defined(ARDUINO_BOARD)
    return ARDUINO_BOARD;
  #else
    return "DESCONHECIDO";
  #endif
}

String dateTimeStr(time_t t, const int8_t tz = 0, const bool flBr = true) {
  // Retorna time_t como "yyyy-mm-dd hh:mm:ss" ou "dd/mm/yyyy hh:mm:ss"
  if (t == 0) {
    return F("N/A");
  }
  t += tz * 3600;
  String sFn;
  if (flBr) {
    // dd/mm/yyyy hh:mm:ss
    sFn = "";
    if (day(t) < 10) {
      sFn += '0';
    }
    sFn += String(day(t)) + '/';
    if (month(t) < 10) {
      sFn += '0';
    }
    sFn += String(month(t)) + '/' + String(year(t)) + ' ';
  } else {
    // yyyy-mm-dd hh:mm:ss
    sFn = String(year(t)) + '-';
    if (month(t) < 10) {
      sFn += '0';
    }
    sFn += String(month(t)) + '-';
    if (day(t) < 10) {
      sFn += '0';
    }
    sFn += String(day(t)) + ' ';
  }
  if (hour(t) < 10) {
    sFn += '0';
  }
  sFn += String(hour(t)) + ':';
  if (minute(t) < 10) {
    sFn += '0';
  }
  sFn += String(minute(t)) + ':';
  if (second(t) < 10) {
    sFn += '0';
  }
  sFn += String(second(t));
  return sFn;
}

uint16_t analogAverage(const uint8_t &a) {
  // Retornar média de 100 leituras da porta analógica
  uint32_t l = 0;
  for (uint8_t b = 0; b < 100; b++) {
    l += analogRead(a);
    yield();
  }
  return l / 100;
}

void log(const String &s) {
  // Gera log no Monitor Serial
  Serial.println(dateTimeStr(time(NULL), DATAHORA_FUSO) + " " + s);
}

// Funcoes Sinric Pro -----------------------------------
bool onPowerState(const String &deviceId, bool &state) {
  // Define estado - ligado / desligado
  ligado = state;
  Serial.printf("Estado: %s\n", ligado ? "ligado" : "desligado");
  return true;
}

void setupSinricPro() {
  // Inicialização
  temperaturaUmidadeLuminosidade.onPowerState(onPowerState);

  SinricPro.onConnected([](){ Serial.printf("Sinric Pro conectado\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Sinric Pro desconectado\n"); });
  SinricPro.restoreDeviceStates(true);
  SinricPro.begin(APP_KEY, APP_SECRET);
};


// Setup ------------------------------------------------
void setup() {
  // Incializa
  #ifdef ESP32
    Serial.begin(115200); // Velocidade padrão para ESP32
  #else
    Serial.begin(74880);  // Velocidade padrão para ESP8266
  #endif

  Serial.println("\n\n*** CANAL INTERNET E COISAS ***\n"
                  "Monitor Ambiental via ALexa");
  Serial.print("Plataforma: "); Serial.println(platform());

  // Inicializa o cliente NTP
  configTime(0, 0, NTP_SERVER);

  // Conecta Wi-Fi
  Serial.print("Conectando WiFi");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi conectado, IP "); Serial.println(WiFi.localIP());

  // Sinric Pro
  setupSinricPro();

  // DHT22
  dht.begin();
}

// Loop -------------------------------------------------
void loop() {
  // Processa Sinric Pro
  SinricPro.handle();

  // Envia dados, se ligado e após intervalo
  if (time(NULL) > proxAtualizacao) {
    if (ligado) {
      // Ligado
      if (isnan(dht.readTemperature())) {
        // Falha na leitura
        log("Falha lendo DHT22");
      } else {
        // Atualiza leitura dos sensores
        temperatura     = dht.readTemperature();
        umidade         = dht.readHumidity();
        #ifdef ESP32
          // ESP32 - resolução 12 bits - 0 a 4095
          luminosidade  = analogAverage(LDR_PIN) * 100L / 4095;
        #else
          // ESP8266 - resolução 10 bits - 0 a 1023
          luminosidade  = analogAverage(LDR_PIN) * 100L / 1023;
        #endif

        // Atualiza dados
        temperaturaUmidadeLuminosidade.sendTemperatureEvent(temperatura, umidade);
        temperaturaUmidadeLuminosidade.sendRangeValueEvent("luminosidade", luminosidade);

        log("Dados atualizados t=" + String(temperatura, 1) +
              "°C u=" + String(umidade, 1) +
              "% l=" + String(luminosidade) + "%");
      }
    } else {
      // Desligado
      log("Desligado");
    }
    // Proxima atualizacao
    proxAtualizacao = time(NULL) + INTERVALO;
  }

  // Verifica botão liga/desliga
  if (!digitalRead(0)) {
    // Botão pressionado
    delay(100);
    // Loop aguarda até o botão ser liberado
    while (!digitalRead(0)) {
      yield();
    }
    // Inverte estado
    ligado = !ligado;
    Serial.println("Botão pressionado");
    Serial.printf("Estado: %s\n", ligado ? "ligado" : "desligado");

    temperaturaUmidadeLuminosidade.sendPowerStateEvent(ligado);
  }
}

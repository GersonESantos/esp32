/***********************************************************************
 * CÓDIGO MESCLADO E INCREMENTADO:
 * Monitor Ambiental + Dimmer de LED + Painel de Controle Web
 ***********************************************************************/

// --- Bibliotecas ---
#include <WiFi.h>
#include <WebServer.h> // NOVO: Para o servidor web
#include <TimeLib.h>
#include <DHT.h>
#include <SinricPro.h>
#include "SinricProDimSwitch.h"
#include "TempUmidLumiLed.h"

// --- Configurações de Rede e Sinric Pro ---
#define WIFI_SSID         "BRUGER_2G"
#define WIFI_PASS         "Gersones68"
#define APP_KEY           "89cda427-c430-4127-9a60-afd89f2364d7"
#define APP_SECRET        "a4993c6e-91d9-4b84-97f6-373c1d78789b-72975cde-8f6e-4ff5-b9a1-b67a298291b5"

// --- IDs dos Dispositivos ---
#define MONITOR_ID        "68daca785918d860c09d9254"
#define DIMSWITCH_ID      "68d98569c6b3a7ebd1b62c43"

// --- Configurações de Hardware ---
#define LED_PIN           13
#define DHT_PIN           27
#define LDR_PIN           33
#define BOTAO_PIN         0

// --- Constantes ---
#define BAUD_RATE         115200
const uint16_t  INTERVALO_SENSOR      = 60;
const char* NTP_SERVER            = "pool.ntp.org";
const int8_t    DATAHORA_FUSO         = -3;

// --- Variáveis Globais e Objetos ---
WebServer       server(80); // NOVO: Objeto do servidor web
bool            monitorLigado         = true;
time_t          proxAtualizacao       = 0;
float           temperatura = 0.0;
float           umidade = 0.0;
uint8_t         luminosidade = 0;
DHT             dht(DHT_PIN, DHT22);
struct { bool powerState = false; int powerLevel = 0; } dimmerState;

// =================================================================
// --- PÁGINA WEB (HTML, CSS, JAVASCRIPT) ---
// =================================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8" /><meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Painel de Controle IoT</title>
    <style>
        :root { --cor-primaria: #0b9dac; --cor-fundo: #001f21; --cor-card: #012526; --cor-texto: #e4e4e4; }
        body { font-family: sans-serif; background: var(--cor-fundo); color: var(--cor-texto); display: flex; flex-direction: column; align-items: center; padding: 20px; text-align: center; }
        .container { max-width: 800px; width: 100%; }
        header h1 { color: #fff; }
        nav { display: flex; margin-bottom: 20px; background: var(--cor-card); border-radius: 8px; padding: 5px; }
        nav button { flex: 1; background: transparent; color: #9ca3af; border: none; padding: 12px 10px; cursor: pointer; border-radius: 6px; font-size: 0.9rem; transition: all 0.3s; }
        nav button.active { background: var(--cor-primaria); color: white; font-weight: 700; }
        .card { background: var(--cor-card); padding: 25px; border-radius: 8px; border-top: 3px solid var(--cor-primaria); display: none; flex-direction: column; align-items: center; }
        .card.active { display: flex; }
        .card > h2 { width:100%; margin-bottom: 20px; font-weight: 400; border-bottom: 1px solid rgba(255,255,255,0.1); padding-bottom: 10px;}
        .content-grid { display: flex; flex-wrap: wrap; justify-content: center; align-items: center; gap: 40px; }
        .led-container { display: flex; flex-direction: column; align-items: center; gap: 30px; }
        .led-button { width: 100px; height: 100px; border-radius: 50%; border: 2px solid rgba(255,255,255,0.1); cursor: pointer; display: flex; align-items: center; justify-content: center; transition: all 0.3s; }
        .led-button svg { width: 50px; height: 50px; stroke: currentColor; }
        .lampada-off { color: #084e55; }
        .lampada-on { color: var(--cor-primaria); filter: drop-shadow(0 0 12px var(--cor-primaria)); }
        .dimmer-container { display: flex; flex-direction: column; align-items: center; gap: 10px; width: 250px; }
        .gauge-wrapper { display: flex; flex-direction: column; align-items: center; }
        .gauge { position: relative; width: 150px; height: 150px; }
        .gauge svg { width: 100%; height: 100%; transform: rotate(-90deg); }
        .gauge .circle-bg { fill: none; stroke: #084e55; stroke-width: 4; }
        .gauge .circle { fill: none; stroke: var(--cor-primaria); stroke-width: 4; stroke-linecap: round; transition: stroke-dasharray 0.6s ease; }
        .gauge .gauge-value { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); font-size: 1.8rem; font-weight: 700; }
        .gauge-wrapper .gauge-label { margin-top: 10px; font-weight: 700; }
    </style>
</head>
<body>
    <div class="container">
        <header><h1>Painel de Controle IoT</h1></header>
        <nav>
            <button onclick="showTab(0)" class="active">Iluminação</button>
            <button onclick="showTab(1)">Monitor Ambiental</button>
        </nav>
        <main>
            <div id="tab0" class="card active">
                <h2>Controle de Iluminação</h2>
                <div class="led-container">
                    <button id="lampada" class="led-button lampada-off" onclick="toggleLampada()">
                        <svg viewBox="0 0 24 24" fill="none" stroke-width="2"><path d="M18.36 6.64a9 9 0 1 1-12.73 0"></path><line x1="12" y1="2" x2="12" y2="12"></line></svg>
                    </button>
                    <div class="dimmer-container">
                        <label for="dimmerSlider">Brilho: <span id="brilhoValue">0</span>%</label>
                        <input type="range" id="dimmerSlider" min="0" max="100" value="0" oninput="updateBrilhoValue(this.value)" onchange="setBrilho(this.value)">
                    </div>
                </div>
            </div>
            <div id="tab1" class="card">
                <h2>Monitor Ambiental (<span id="monitorStatus">--</span>)</h2>
                <div class="content-grid">
                    <div class="gauge-wrapper">
                        <div class="gauge"><svg viewBox="0 0 36 36"><path class="circle-bg" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/><path class="circle" id="circleTemp" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/></svg><div class="gauge-value" id="tempValue">--°C</div></div><div class="gauge-label">Temperatura</div>
                    </div>
                    <div class="gauge-wrapper">
                        <div class="gauge"><svg viewBox="0 0 36 36"><path class="circle-bg" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/><path class="circle" id="circleUmid" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/></svg><div class="gauge-value" id="umidValue">--%</div></div><div class="gauge-label">Umidade</div>
                    </div>
                    <div class="gauge-wrapper">
                        <div class="gauge"><svg viewBox="0 0 36 36"><path class="circle-bg" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/><path class="circle" id="circleLuz" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/></svg><div class="gauge-value" id="luzValue">--%</div></div><div class="gauge-label">Luminosidade</div>
                    </div>
                </div>
            </div>
        </main>
    </div>
    <script>
        function showTab(index) {
            document.querySelectorAll(".card").forEach((c, i) => c.classList.toggle("active", i === index));
            document.querySelectorAll("nav button").forEach((b, i) => b.classList.toggle("active", i === index));
        }
        function toggleLampada() { fetch('/toggle_led'); }
        function updateBrilhoValue(value) { document.getElementById('brilhoValue').textContent = value; }
        function setBrilho(value) { fetch(`/set_brilho?valor=${value}`); }
        
        setInterval(async () => {
            try {
                const res = await fetch('/data');
                const data = await res.json();
                
                document.getElementById('lampada').classList.toggle('lampada-on', data.dimmer_ligado);
                document.getElementById('lampada').classList.toggle('lampada-off', !data.dimmer_ligado);
                document.getElementById('dimmerSlider').value = data.dimmer_brilho;
                document.getElementById('brilhoValue').textContent = data.dimmer_brilho;

                document.getElementById('monitorStatus').textContent = data.monitor_ligado ? 'ATIVO' : 'PAUSADO';
                document.getElementById('tempValue').textContent = data.temperatura.toFixed(1) + '°C';
                document.getElementById('circleTemp').setAttribute('stroke-dasharray', (data.temperatura / 50 * 100) + ', 100');
                document.getElementById('umidValue').textContent = data.umidade.toFixed(1) + '%';
                document.getElementById('circleUmid').setAttribute('stroke-dasharray', data.umidade + ', 100');
                document.getElementById('luzValue').textContent = data.luminosidade + '%';
                document.getElementById('circleLuz').setAttribute('stroke-dasharray', data.luminosidade + ', 100');
            } catch (e) { console.error('Falha ao buscar dados:', e); }
        }, 2000);
        
        document.addEventListener('DOMContentLoaded', () => { showTab(0); });
    </script>
</body>
</html>
)rawliteral";

// --- Funções de Apoio (do código do Monitor) ---
String dateTimeStr(time_t t, const int8_t tz = 0, const bool flBr = true);
void log(const String &s) { Serial.println(dateTimeStr(time(NULL), DATAHORA_FUSO) + " " + s); }

// --- Funções de Callback (Sinric Pro) ---
bool onMonitorPowerState(const String &deviceId, bool &state) { log("Comando de energia para o MONITOR: " + String(state ? "LIGADO" : "DESLIGADO")); monitorLigado = state; return true; }
bool onDimmerPowerState(const String &deviceId, bool &state) {
    Serial.printf("Comando de energia para o DIMMER (Sinric): %s\n", state ? "LIGADO" : "DESLIGADO");
    dimmerState.powerState = state;
    if (state) {
        if (dimmerState.powerLevel == 0) dimmerState.powerLevel = 100;
        analogWrite(LED_PIN, map(dimmerState.powerLevel, 0, 100, 0, 255));
    } else {
        analogWrite(LED_PIN, 0);
    }
    return true;
}
bool onPowerLevel(const String &deviceId, int &powerLevel) {
    dimmerState.powerLevel = powerLevel;
    dimmerState.powerState = (powerLevel > 0);
    Serial.printf("Nível de brilho do Dimmer (Sinric) alterado para %d\n", dimmerState.powerLevel);
    analogWrite(LED_PIN, map(dimmerState.powerLevel, 0, 100, 0, 255));
    return true;
}
bool onAdjustPowerLevel(const String &deviceId, int &levelDelta) {
    dimmerState.powerLevel += levelDelta;
    if (dimmerState.powerLevel < 0) dimmerState.powerLevel = 0;
    if (dimmerState.powerLevel > 100) dimmerState.powerLevel = 100;
    dimmerState.powerState = (dimmerState.powerLevel > 0);
    onPowerLevel(deviceId, dimmerState.powerLevel);
    levelDelta = dimmerState.powerLevel;
    return true;
}

// --- NOVO: Funções de Controle para o Servidor Web ---
void handleRoot() { server.send(200, "text/html", index_html); }

void handleData() {
    String json = "{";
    json += "\"dimmer_ligado\":" + String(dimmerState.powerState ? "true" : "false") + ",";
    json += "\"dimmer_brilho\":" + String(dimmerState.powerLevel) + ",";
    json += "\"monitor_ligado\":" + String(monitorLigado ? "true" : "false") + ",";
    json += "\"temperatura\":" + String(temperatura) + ",";
    json += "\"umidade\":" + String(umidade) + ",";
    json += "\"luminosidade\":" + String(luminosidade);
    json += "}";
    server.send(200, "application/json", json);
}

void handleToggleLED() {
    // ## CORREÇÃO APLICADA AQUI ##
    bool novoEstado = !dimmerState.powerState; // Cria uma variável para o novo estado
    onDimmerPowerState(DIMSWITCH_ID, novoEstado); // Passa a variável para a função
    server.send(200, "text/plain", "OK");
}

void handleSetBrilho() {
    if (server.hasArg("valor")) {
        int brilho = server.arg("valor").toInt();
        onPowerLevel(DIMSWITCH_ID, brilho);
    }
    server.send(200, "text/plain", "OK");
}

void handleNotFound() { server.send(404, "text/plain", "Nao encontrado"); }

// --- Funções de Configuração ---
void setupWiFi() {
    Serial.print("\nConectando WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.print("\nWiFi conectado, IP "); Serial.println(WiFi.localIP());
}

void setupSinricPro() {
    TemperaturaUmidadeLuminosidade &meuMonitor = SinricPro[MONITOR_ID];
    meuMonitor.onPowerState(onMonitorPowerState);
    SinricProDimSwitch &meuDimmer = SinricPro[DIMSWITCH_ID];
    meuDimmer.onPowerState(onDimmerPowerState);
    meuDimmer.onPowerLevel(onPowerLevel);
    meuDimmer.onAdjustPowerLevel(onAdjustPowerLevel);
    SinricPro.onConnected([](){ Serial.println("Sinric Pro conectado"); }); 
    SinricPro.onDisconnected([](){ Serial.println("Sinric Pro desconectado"); });
    SinricPro.restoreDeviceStates(true);
    SinricPro.begin(APP_KEY, APP_SECRET);
}

// --- Função Principal de SETUP ---
void setup() {
    Serial.begin(BAUD_RATE);
    Serial.println("\n\n*** PROJETO MESCLADO COM PAINEL WEB ***");

    pinMode(LED_PIN, OUTPUT); digitalWrite(LED_PIN, LOW);
    pinMode(BOTAO_PIN, INPUT_PULLUP);
    dht.begin();
    configTime(DATAHORA_FUSO * 3600, 0, NTP_SERVER);
    
    setupWiFi();
    setupSinricPro();

    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/toggle_led", handleToggleLED);
    server.on("/set_brilho", handleSetBrilho);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.printf("Servidor Web iniciado! Acesse http://%s\n", WiFi.localIP().toString().c_str());
}

// --- Função Principal de LOOP ---
void loop() {
    SinricPro.handle();
    server.handleClient();

    if (time(NULL) > proxAtualizacao) {
        if (monitorLigado) {
            temperatura = dht.readTemperature();
            umidade = dht.readHumidity();
            if (isnan(temperatura) || isnan(umidade)) { log("Falha lendo DHT22"); }
            else {
                luminosidade = map(analogRead(LDR_PIN), 0, 4095, 0, 100);
                TemperaturaUmidadeLuminosidade &meuMonitor = SinricPro[MONITOR_ID];
                meuMonitor.sendTemperatureEvent(temperatura, umidade);
                meuMonitor.sendRangeValueEvent("luminosidade", luminosidade);
                log("Dados atualizados: t=" + String(temperatura, 1) + "C u=" + String(umidade, 1) + "% l=" + String(luminosidade) + "%");
            }
        }
        proxAtualizacao = time(NULL) + INTERVALO_SENSOR;
    }

    if (!digitalRead(BOTAO_PIN)) {
        delay(100);
        while (!digitalRead(BOTAO_PIN)) { yield(); }
        monitorLigado = !monitorLigado;
        log("Botao pressionado. Estado do monitor: " + String(monitorLigado ? "LIGADO" : "DESLIGADO"));
        TemperaturaUmidadeLuminosidade &meuMonitor = SinricPro[MONITOR_ID];
        meuMonitor.sendPowerStateEvent(monitorLigado);
    }
}

// --- Implementação da Função de Apoio ---
String dateTimeStr(time_t t, const int8_t tz, const bool flBr) {
    if (t == 0) return F("N/A");
    t += tz * 3600;
    char buff[20];
    if (flBr) { sprintf(buff, "%02d/%02d/%d %02d:%02d:%02d", day(t), month(t), year(t), hour(t), minute(t), second(t)); }
    else { sprintf(buff, "%d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t)); }
    return String(buff);
}
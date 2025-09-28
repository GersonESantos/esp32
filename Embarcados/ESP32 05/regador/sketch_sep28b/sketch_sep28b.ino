
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// =================================================================
// --- 1. CONFIGURAÇÕES GERAIS E PINOS ---
// =================================================================

// --- Rede Wi-Fi ---
const char* ssid = "BRUGER_2G";
const char* password = "Gersones68";

// --- Pinos dos Dispositivos 
#define LAMPADA_PIN 13
#define BOMBA_RELE_PIN 12
#define DHT_PIN 4
#define SOLO_SENSOR_PIN 34

// --- Calibração do Sensor de Solo ---
const int SENSOR_SECO = 4095;
const int SENSOR_MOLHADO = 1800;

// =================================================================
// --- 2. OBJETOS E VARIÁVEIS GLOBAIS ---
// =================================================================

WebServer server(80);
DHT dht(DHT_PIN, DHT11);

// --- Variáveis de Estado Globais ---
bool estadoLampada = false;
float temperaturaAtual = 0.0;
float umidadeArAtual = 0.0;
float umidadeMinimaIrrigacao = 40.0;
int umidadeSoloAtual = 0;
bool estadoReleBomba = false;

// --- Buffers e Variáveis para Médias (RESTAURADO) ---
const int N_LEITURAS_CLIMA = 10;
float tempBuffer[N_LEITURAS_CLIMA] = {0};
float umidArBuffer[N_LEITURAS_CLIMA] = {0};
int bufferClimaIndex = 0;
int totalLeiturasClima = 0;
float tempMediaAtual = 0.0;
float umidArMediaAtual = 0.0;

// --- Temporizador Principal ---
unsigned long previousMillis = 0;
const long intervalo = 1000;

// =================================================================
// --- 3. PÁGINA WEB (HTML, CSS, JAVASCRIPT) ---
// =================================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
    
<head>
  <meta charset="UTF-8" /><meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Central de Controle IoT</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Roboto:wght@400;700&display=swap');
    :root { --cor-primaria: #0b9dac; --cor-fundo: #001f21; --cor-card: #012526; --cor-texto: #e4e4e4; }
    * { margin: 0; padding: 0; box-sizing: border-box; user-select: none; }
    body {
      font-family: 'Roboto', sans-serif; background: var(--cor-fundo); color: var(--cor-texto);
      display: flex; flex-direction: column; align-items: center; padding: 20px; min-height: 100vh; text-align: center;
    }
    .container { max-width: 800px; width: 100%; }
    header { text-align: center; margin-bottom: 20px; }
    nav { display: flex; margin-bottom: 20px; background: var(--cor-card); border-radius: 8px; padding: 5px; }
    nav button {
      flex: 1; background: transparent; color: #9ca3af; border: none; padding: 12px 10px; cursor: pointer;
      border-radius: 6px; font-size: 0.9rem; display: flex; align-items: center; justify-content: center; gap: 8px;
      transition: all 0.3s;
    }
    nav button:hover { background: rgba(255,255,255,0.05); }
    nav button.active { background: var(--cor-primaria); color: white; font-weight: 700; }
    .card {
      background: var(--cor-card); padding: 25px; border-radius: 8px; border-top: 3px solid var(--cor-primaria);
      box-shadow: 0 4px 20px rgba(0,0,0,0.2); display: none; animation: fadeInUp 0.5s ease-out both;
      flex-direction: column; align-items: center; justify-content: center; /* CENTRALIZA O CONTEÚDO DO CARD */
    }
    .card.active { display: flex; } /* MUDA PARA FLEX QUANDO ATIVO */
    @keyframes fadeInUp { from {opacity:0; transform:translateY(20px);} to {opacity:1; transform:translateY(0);} }
    .card > h2 { width:100%; margin-bottom: 20px; font-weight: 400; border-bottom: 1px solid rgba(255,255,255,0.1); padding-bottom: 10px;}
    .content-grid { display: flex; flex-wrap: wrap; justify-content: center; align-items: center; gap: 40px; }
    
    /* --- ESTILOS EXATOS DO SEU EXEMPLO PARA A LÂMPADA --- */
    .title-container { margin-bottom: 50px; }
    .title-container h1 { font-size: clamp(2rem, 6vw, 2.8rem); margin-bottom: 10px; color: #ffffff; text-shadow: 0 0 10px rgba(255,255,255,0.2), 0 0 20px rgba(255,100,200,0.2); letter-spacing: 2px; }
    .button-grid { display: flex; justify-content: center; width: 100%; max-width: 500px; }
    .led-wrapper { display: flex; flex-direction: column; align-items: center; }
    .led-button {
      width: 100px; height: 100px; border-radius: 50%; border: 2px solid rgba(255, 255, 255, 0.1); cursor: pointer;
      background: linear-gradient(145deg, rgba(255,255,255,0.05), rgba(255,255,255,0));
      box-shadow: 0 10px 30px rgba(0,0,0,0.5), inset 0 3px 2px rgba(255,255,255,0.05);
      display: flex; align-items: center; justify-content: center;
      transition: transform 0.3s ease, box-shadow 0.3s ease;
      -webkit-tap-highlight-color: transparent; touch-action: manipulation;
    }
    .led-button:hover { transform: scale(1.05) translateY(-3px); box-shadow: 0 15px 40px rgba(0,0,0,0.6), inset 0 3px 2px rgba(255,255,255,0.1); }
    .led-button:active { transform: scale(0.98); transition-duration: 0.1s; }
    .led-button svg { width: 50px; height: 50px; stroke: currentColor; pointer-events: none; }
    .lampada-off { color: #084e55; }
    .lampada-on { color: var(--cor-primaria); }
    .lampada-on svg { filter: drop-shadow(0 0 12px var(--cor-primaria)) drop-shadow(0 0 25px var(--cor-primaria)); }
    .led-button.lampada-on { background: radial-gradient(circle, rgba(11, 157, 172, 0.15), transparent 75%); box-shadow: 0 0 40px rgba(11, 157, 172, 0.7); }
    .led-label { margin-top: 10px; font-weight: 700; font-size: 1rem; color: #e4e4e4; }
    
    /* --- ESTILOS DOS GAUGES E MÉDIAS --- */
    .gauge-wrapper { display: flex; flex-direction: column; align-items: center; }
    .gauge { position: relative; width: 150px; height: 150px; }
    .gauge svg { width: 100%; height: 100%; transform: rotate(-90deg); }
    .gauge .circle-bg { fill: none; stroke: #084e55; stroke-width: 4; }
    .gauge .circle { fill: none; stroke: var(--cor-primaria); stroke-width: 4; stroke-linecap: round; transition: stroke-dasharray 0.6s ease; }
    .gauge .gauge-value { position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); font-size: 1.8rem; font-weight: 700; color: var(--cor-primaria); }
    .gauge-wrapper .gauge-label { text-align: center; margin-top: 10px; font-size: 1rem; font-weight: 700; }
    .medias { margin-top: 40px; display: flex; flex-direction:column; gap: 15px; } /* MÉDIAS RESTAURADAS */
    .media-item { font-size: 1.2rem; font-weight: 400; }
    .media-item .label { color: var(--cor-primaria); font-weight: 700; }
    
    /* --- ESTILOS CONTROLES IRRIGAÇÃO --- */
    .controles { display: flex; flex-direction: column; gap: 15px; align-items: center; }
    .controle-item { display: flex; align-items: center; gap: 10px; font-size: 1rem;}
    .controle-item label { font-weight: 700; }
    .controle-item input { font-size: 1rem; padding: 8px; width: 70px; background-color: #003133; border: 1px solid #084e55; color: white; border-radius: 5px; text-align: center; }
    .controle-item button { font-size: 0.9rem; padding: 8px 15px; background-color: var(--cor-primaria); color: white; border: none; border-radius: 5px; cursor: pointer; }
    #statusRele.ligado { color: #23d18b; } #statusRele.desligado { color: #ff5252; }
    /* --- ESTILO DO RODAPÉ ADICIONADO AQUI --- */
    footer {
      margin-top: 40px;
      font-size: 0.9rem;
      color: #9ca3af;
      opacity: 0.7;
    }
  </style>
</head>
<body>
  <div class="container">
    <header><h1 id="header-title">Central de Controle IoT</h1></header>
    <nav>
      <button onclick="showTab(0)" class="active">Controle Lâmpada</button>
      <button onclick="showTab(1)">Monitor de Clima</button>
      <button onclick="showTab(2)">Controle de Irrigação</button>
    </nav>
    <main>
      <div id="tab0" class="card active">
          <div class="title-container">
            <h1>Controle de Dispositivos</h1>
          </div>
          <div class="button-grid">
            <div class="led-wrapper">
              <button id="lampada" class="led-button lampada-off">
                <svg viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                  <path d="M18.36 6.64a9 9 0 1 1-12.73 0"></path><line x1="12" y1="2" x2="12" y2="12"></line>
                </svg>
              </button>
              <div class="led-label">Lâmpada</div>
            </div>
          </div>
      </div>
      <div id="tab1" class="card">
        <h2>Monitor de Clima Ambiente</h2>
        <div class="content-grid">
          <div class="gauge-wrapper">
            <div class="gauge">
              <svg viewBox="0 0 36 36"><path class="circle-bg" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/><path class="circle" id="circleTemp" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/></svg>
              <div class="gauge-value" id="tempValue">--°C</div>
            </div><div class="gauge-label">Temperatura</div>
          </div>
          <div class="gauge-wrapper">
            <div class="gauge">
              <svg viewBox="0 0 36 36"><path class="circle-bg" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/><path class="circle" id="circleUmidAr" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/></svg>
              <div class="gauge-value" id="umidArValue">--%</div>
            </div><div class="gauge-label">Umidade do Ar</div>
          </div>
        </div>
        <div class="medias">
          <div class="media-item"><span class="label">Temperatura Média: </span><span id="tempMediaValue">--</span>°C</div>
          <div class="media-item"><span class="label">Umidade Média: </span><span id="umidArMediaValue">--</span>%</div>
        </div>
      </div>
      <div id="tab2" class="card">
        <h2>Controle Autônomo de Irrigação</h2>
        <div class="content-grid">
          <div class="gauge-wrapper">
             <div class="gauge">
                <svg viewBox="0 0 36 36"><path class="circle-bg" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/><path class="circle" id="circleUmidSolo" d="M18 2.08a15.92 15.92 0 0 1 0 31.84a15.92 15.92 0 0 1 0-31.84"/></svg>
                <div class="gauge-value" id="umidSoloValue">--%</div>
              </div><div class="gauge-label">Umidade do Solo</div>
          </div>
          <div class="controles">
            <div class="controle-item"><label for="umidMinInput">Umid. Mínima (%):</label><input type="number" id="umidMinInput" step="1" value="40"><button onclick="definirUmidadeMin()">Definir</button></div>
            <div class="controle-item"><label>Status Bomba:</label><span id="statusRele" class="status-rele desligado">PARADA</span></div>
          </div>
        </div>
      </div>
    </main>
    <footer>Eletrônica Omega | E-book Iot com ESP 32</footer>
  </div>
  <script>
    let activeTab = 0; let updateInterval;
    function showTab(index) {
      activeTab = index;
      document.querySelectorAll(".card").forEach((c, i) => c.classList.toggle("active", i === index));
      document.querySelectorAll("nav button").forEach((b, i) => b.classList.toggle("active", i === index));
      runUpdater();
    }
    function runUpdater() { clearInterval(updateInterval); updateData(); updateInterval = setInterval(updateData, 2000); }
    function updateData() {
      if (activeTab === 0) atualizarLampada();
      if (activeTab === 1) atualizarClima();
      if (activeTab === 2) atualizarIrrigacao();
    }
    
    // --- JAVASCRIPT CORRIGIDO PARA O BOTÃO ---
    async function toggleLampada() {
      const botao = document.getElementById('lampada');
      const isLigado = botao.classList.contains('lampada-on');
      // Dá o feedback visual instantaneamente
      botao.classList.toggle('lampada-on', !isLigado);
      botao.classList.toggle('lampada-off', isLigado);
      // Envia o comando para o ESP32
      await fetch(!isLigado ? '/luz_on' : '/luz_off');
      // Re-sincroniza com o estado real do servidor por segurança
      setTimeout(atualizarLampada, 300);
    }
    async function atualizarLampada() {
      const res = await fetch('/estado_luz'); const data = await res.json();
      const botao = document.getElementById('lampada');
      botao.classList.toggle('lampada-on', data.estado === 1);
      botao.classList.toggle('lampada-off', data.estado !== 1);
    }
    
    // --- Funções das outras abas (com médias restauradas) ---
    async function atualizarClima() {
      const res = await fetch('/dados_clima'); const data = await res.json();
      document.getElementById('tempValue').textContent = data.temperatura.toFixed(1) + '°C';
      document.getElementById('circleTemp').setAttribute('stroke-dasharray', (data.temperatura / 50 * 100) + ', 100');
      document.getElementById('umidArValue').textContent = data.umidade.toFixed(1) + '%';
      document.getElementById('circleUmidAr').setAttribute('stroke-dasharray', data.umidade + ', 100');
      document.getElementById('tempMediaValue').textContent = data.tempMedia.toFixed(1);
      document.getElementById('umidArMediaValue').textContent = data.umidArMedia.toFixed(1);
    }
    async function atualizarIrrigacao() {
      const res = await fetch('/dados_irrigacao'); const data = await res.json();
      const umidSolo = parseFloat(data.umidadeSolo);
      document.getElementById('umidSoloValue').textContent = umidSolo.toFixed(0) + '%';
      document.getElementById('circleUmidSolo').setAttribute('stroke-dasharray', umidSolo + ', 100');
      const statusReleEl = document.getElementById('statusRele');
      if (data.releBomba === 1) { statusReleEl.textContent = 'LIGADA'; statusReleEl.className = 'status-rele ligado'; }
      else { statusReleEl.textContent = 'PARADA'; statusReleEl.className = 'status-rele desligado'; }
    }
    async function definirUmidadeMin() {
      const umidMin = document.getElementById('umidMinInput').value;
      await fetch(`/set_umidade?valor=${umidMin}`);
      alert('Umidade mínima definida!');
    }
    
    // Adiciona o evento de clique ao botão da lâmpada assim que a página carrega
    document.addEventListener('DOMContentLoaded', () => {
      document.getElementById('lampada').addEventListener('click', toggleLampada);
      showTab(0);
    });
  </script>
</body>
</html>
)rawliteral";


// =================================================================
// --- 4. FUNÇÕES DE LÓGICA E CONTROLE ---
// =================================================================

void logicaControlePrincipal() {
  // Lógica do Monitor de Clima (com cálculo de média restaurado)
  temperaturaAtual = dht.readTemperature();
  umidadeArAtual = dht.readHumidity();
  if (isnan(temperaturaAtual) || isnan(umidadeArAtual)) {
    temperaturaAtual = 0; umidadeArAtual = 0;
  }
  tempBuffer[bufferClimaIndex] = temperaturaAtual;
  umidArBuffer[bufferClimaIndex] = umidadeArAtual;
  bufferClimaIndex = (bufferClimaIndex + 1) % N_LEITURAS_CLIMA;
  if (totalLeiturasClima < N_LEITURAS_CLIMA) totalLeiturasClima++;
  float somaTemp = 0.0, somaUmidAr = 0.0;
  for (int i = 0; i < totalLeiturasClima; i++) {
    somaTemp += tempBuffer[i];
    somaUmidAr += umidArBuffer[i];
  }
  tempMediaAtual = (totalLeiturasClima > 0) ? somaTemp / totalLeiturasClima : 0;
  umidArMediaAtual = (totalLeiturasClima > 0) ? somaUmidAr / totalLeiturasClima : 0;

  // Lógica do Controle de Irrigação
  int valorAnalogico = analogRead(SOLO_SENSOR_PIN);
  umidadeSoloAtual = map(valorAnalogico, SENSOR_SECO, SENSOR_MOLHADO, 0, 100);
  umidadeSoloAtual = constrain(umidadeSoloAtual, 0, 100);
  
  if (umidadeSoloAtual <= umidadeMinimaIrrigacao) {
    digitalWrite(BOMBA_RELE_PIN, LOW); estadoReleBomba = true;
  } else {
    digitalWrite(BOMBA_RELE_PIN, HIGH); estadoReleBomba = false;
  }
}

// =================================================================
// --- 5. FUNÇÕES DO SERVIDOR WEB (HANDLERS) ---
// =================================================================

void handleRoot() { server.send_P(200, "text/html", index_html); }

void handleEstadoLuz() { server.send(200, "application/json", "{\"estado\":" + String(estadoLampada ? 1 : 0) + "}"); }
void handleLuzOn() { digitalWrite(LAMPADA_PIN, LOW); estadoLampada = true; server.send(200, "text/plain", "OK"); }
void handleLuzOff() { digitalWrite(LAMPADA_PIN, HIGH); estadoLampada = false; server.send(200, "text/plain", "OK"); }

void handleDadosClima() { // JSON das médias restaurado
  String json = "{";
  json += "\"temperatura\":" + String(temperaturaAtual, 1) + ",";
  json += "\"umidade\":" + String(umidadeArAtual, 1) + ",";
  json += "\"tempMedia\":" + String(tempMediaAtual, 1) + ",";
  json += "\"umidArMedia\":" + String(umidArMediaAtual, 1);
  json += "}";
  server.send(200, "application/json", json);
}

void handleDadosIrrigacao() {
  String json = "{\"umidadeSolo\":" + String(umidadeSoloAtual) + ",\"releBomba\":" + String(estadoReleBomba ? 1 : 0) + "}";
  server.send(200, "application/json", json);
}
void handleSetUmidade() {
  if (server.hasArg("valor")) { umidadeMinimaIrrigacao = server.arg("valor").toFloat(); }
  server.send(200, "text/plain", "OK");
}

// =================================================================
// --- 6. SETUP E 7. LOOP (sem alterações) ---
// =================================================================
void setup() {
  Serial.begin(115200);
  pinMode(LAMPADA_PIN, OUTPUT); pinMode(BOMBA_RELE_PIN, OUTPUT);
  digitalWrite(LAMPADA_PIN, HIGH); digitalWrite(BOMBA_RELE_PIN, HIGH);
  dht.begin();
  WiFi.mode(WIFI_STA); WiFi.begin(ssid, password);
  Serial.print("\nTentando conectar ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConectado! Endereço IP: " + WiFi.localIP().toString());
  server.on("/", handleRoot); server.on("/estado_luz", handleEstadoLuz);
  server.on("/luz_on", handleLuzOn); server.on("/luz_off", handleLuzOff);
  server.on("/dados_clima", handleDadosClima); server.on("/dados_irrigacao", handleDadosIrrigacao);
  server.on("/set_umidade", handleSetUmidade);
  server.begin(); Serial.println("Servidor web iniciado.");
}
void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo) { previousMillis = currentMillis; logicaControlePrincipal(); }
}

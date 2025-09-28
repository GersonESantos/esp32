#include <WiFi.h>
#include <WebServer.h>

#define SENSOR_PIN 34
#define RELAY_PIN 13

const char* ssid = "BRUGER_2G";
const char* password = "Gersones68";

WebServer server(80);

int umidade = 0;
int nivelMinimo = 30;
bool bombaLigada = false;
bool modoManual = false;

unsigned long ultimaLeitura = 0;
const unsigned long intervaloLeitura = 1000; // 1 segundo

String gerarPagina() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; text-align: center; background: #f2f2f2; }
        .container { padding: 20px; margin: auto; max-width: 400px; background: white; border-radius: 10px; box-shadow: 0 0 10px #ccc; }
        h2 { color: #333; }
        input[type=range] { width: 100%; }
        button { padding: 10px 20px; font-size: 16px; margin-top: 10px; }
        .status { font-size: 18px; margin-top: 10px; }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Monitor de Umidade</h2>
        <p><strong>Umidade do solo:</strong> <span id="umidade">--</span>%</p>
        <label for="nivel">Nivel minimo para ligar bomba:</label><br>
        <input type="range" id="nivel" min="0" max="100" value="%NIVEL%">
        <p><span id="nivelVal">%NIVEL%</span>%</p>
        <button onclick="toggleManual()"><span id="modo">Modo: Automatico</span></button>
        <button onclick="toggleBomba()">Bomba <span id='estado'>DESLIGADA</span></button>
        <p class="status" id="statusText">Ultima leitura: agora</p>
      </div>

      <script>
        setInterval(() => {
          fetch('/data').then(r => r.json()).then(data => {
            document.getElementById('umidade').innerText = data.umidade;
            document.getElementById('estado').innerText = data.bomba ? 'LIGADA' : 'DESLIGADA';
            document.getElementById('modo').innerText = 'Modo: ' + (data.manual ? 'Manual' : 'Automatico');
            document.getElementById('statusText').innerText = 'Ultima leitura: agora';
          });
        }, 1000);

        document.getElementById("nivel").addEventListener("input", function() {
          document.getElementById("nivelVal").innerText = this.value;
          fetch('/setnivel?valor=' + this.value);
        });

        function toggleManual() {
          fetch('/manual');
        }

        function toggleBomba() {
          fetch('/toggle');
        }
      </script>
    </body>
    </html>
  )rawliteral";

  html.replace("%NIVEL%", String(nivelMinimo));
  return html;
}

void handleRoot() {
  server.send(200, "text/html", gerarPagina());
}

void handleData() {
  String json = "{";
  json += "\"umidade\":" + String(umidade) + ",";
  json += "\"bomba\":" + String(bombaLigada ? "true" : "false") + ",";
  json += "\"manual\":" + String(modoManual ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleSetNivel() {
  if (server.hasArg("valor")) {
    nivelMinimo = server.arg("valor").toInt();
    Serial.print("Nivel minimo ajustado para: ");
    Serial.println(nivelMinimo);
  }
  server.send(200, "text/plain", "OK");
}

void handleManual() {
  modoManual = !modoManual;
  Serial.print("Modo manual: ");
  Serial.println(modoManual ? "Ativado" : "Desativado");
  server.send(200, "text/plain", "OK");
}

void handleToggle() {
  bombaLigada = !bombaLigada;
  digitalWrite(RELAY_PIN, bombaLigada ? HIGH : LOW);
  Serial.print("Bomba ");
  Serial.println(bombaLigada ? "LIGADA (manual)" : "DESLIGADA (manual)");
  server.send(200, "text/plain", "OK");
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/setnivel", handleSetNivel);
  server.on("/manual", handleManual);
  server.on("/toggle", handleToggle);
  server.begin();
}

void loop() {
  server.handleClient();

  // Leitura da umidade a cada 1 segundo
  if (millis() - ultimaLeitura > intervaloLeitura) {
    int leituraBruta = analogRead(SENSOR_PIN);
    umidade = map(leituraBruta, 0, 4095, 100, 0);
    ultimaLeitura = millis();
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println(" %");
  }

  // Controle da bomba em tempo real
  if (!modoManual) {
    bool novaBomba = (umidade < nivelMinimo);
    if (novaBomba != bombaLigada) {
      bombaLigada = novaBomba;
      digitalWrite(RELAY_PIN, bombaLigada ? HIGH : LOW);
      Serial.println(bombaLigada ? "Bomba LIGADA (automatico)" : "Bomba DESLIGADA (automatico)");
    }
  }
}

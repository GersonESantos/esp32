#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Bibliotecas e Configurações para o Sensor DHT22 ---
#include <DHT.h>       // Biblioteca principal do sensor DHT
#include <DHT_U.h>     // Biblioteca de utilitários DHT da Adafruit (geralmente usada junto com DHT.h)

#define DHTPIN 26      // Pino GPIO do ESP32 onde o pino de dados do DHT22 está conectado
#define DHTTYPE DHT22  // Define o tipo de sensor DHT que você está usando (DHT11, DHT22, DHT21)

DHT_Unified dht(DHTPIN, DHTTYPE); // Cria uma instância do sensor DHT



#define OLED_SDA 21 // Pino SDA do ESP32 (data)
#define OLED_SCL 22 // Pino SCL do ESP32 (clock)
#define OLED_RESET -1 // Pino de reset do OLED (use -1 para não usar ou conectar ao RST)

Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET); 

// Credenciais do WiFi - Substitua pelos seus dados
const char* ssid = "BRUGER_2G";       // Nome da sua rede WiFi
const char* password = "  
"; // Senha da sua rede WiFi

// Define o pino da lâmpada
const int lampadaPin = 13;

// Cria o objeto do servidor na porta 80
WiFiServer server(80);

// Variável para armazenar a requisição HTTP
String header;

// Estado atual da lâmpada
String estadoLampada = "off";

// Variáveis para armazenar as leituras do sensor DHT
float temperature = 0.0;
float humidity = 0.0;

void setup() {
  Serial.begin(115200);

  // --- Inicialização do OLED ---
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha na inicialização do display OLED!"));
    while (true);
  }
  display.display();
  delay(2000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // --- Inicialização do Sensor DHT22 ---
  dht.begin(); // Inicia o sensor DHT
  Serial.println(F("Sensor DHT22 inicializado."));
  display.setCursor(0, 50); // Mova o cursor para o final da tela OLED para uma mensagem adicional
  display.println("DHT OK!");
  display.display();
  delay(1000); // Pequeno atraso para ver a mensagem do DHT

  // Configura o pino da lâmpada como saída
  pinMode(lampadaPin, OUTPUT);
  digitalWrite(lampadaPin, LOW);
  estadoLampada = "off";

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Conectando WiFi...");
  display.display();

  // Conecta ao WiFi
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
    tentativas++;
  }

  Serial.println("");
  display.clearDisplay();
  display.setCursor(0,0);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi conectado.");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());

    display.println("WiFi conectado!");
    display.print("IP: ");
    display.println(WiFi.localIP());
  } else {
    Serial.println("Falha na conexão WiFi.");
    display.println("Falha WiFi!");
    display.println("Verifique credenciais.");
  }
  display.display();
  delay(2000);

  server.begin();
  Serial.println("Servidor HTTP iniciado na porta 80.");

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Servidor OK.");
  display.println("Lampada: " + estadoLampada);
  display.display();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    String currentLine = "";
    header = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Cabeçalho HTTP
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Verifica as requisições GET
            if (header.indexOf("GET /lampada/on") >= 0) {
              estadoLampada = "on";
              digitalWrite(lampadaPin, HIGH);
              Serial.println("Lâmpada LIGADA");
            } else if (header.indexOf("GET /lampada/off") >= 0) {
              estadoLampada = "off";
              digitalWrite(lampadaPin, LOW);
              Serial.println("Lâmpada DESLIGADA");
            }

            // --- Leitura do Sensor DHT22 ---
            sensors_event_t event; // Objeto para armazenar os dados do sensor
            dht.temperature().getEvent(&event);
            if (isnan(event.temperature)) {
              Serial.println(F("Erro ao ler temperatura!"));
              temperature = -999.0; // Valor de erro
            } else {
              temperature = event.temperature;
            }

            dht.humidity().getEvent(&event);
            if (isnan(event.relative_humidity)) {
              Serial.println(F("Erro ao ler umidade!"));
              humidity = -999.0; // Valor de erro
            } else {
              humidity = event.relative_humidity;
            }

            Serial.print(F("Temperatura: "));
            Serial.print(temperature);
            Serial.println(F(" *C"));
            Serial.print(F("Umidade: "));
            Serial.print(humidity);
            Serial.println(F(" %"));


            // --- Atualiza o OLED com o estado da lâmpada e leituras do DHT ---
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("IP: ");
            display.println(WiFi.localIP());
            display.println("-----------------");
            display.print("Lampada: ");
            display.println(estadoLampada);
            display.print("Temp: ");
            display.print(temperature);
            display.println(" C");
            display.print("Umid: ");
            display.print(humidity);
            display.println(" %");
            display.display();

            // Página HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>html { font-family: Helvetica; text-align: center;}");
            client.println(".button { padding: 15px 30px; font-size: 24px; margin: 10px; text-decoration: none; border-radius: 5px;}");
            client.println(".button-on { background-color: #4CAF50; color: white; }");
            client.println(".button-off { background-color: #f44336; color: white; }");
            client.println(".sensor-data { font-size: 20px; margin-top: 20px; color: #333; }</style></head>"); // Novo estilo para dados do sensor
            
            client.println("<body><h1>Controle da Lampada</h1>");
            client.println("<p>Estado: " + estadoLampada + "</p>");
            // --- ADICIONA AS LEITURAS DO SENSOR NO HTML ---
            client.println("<p class=\"sensor-data\">Temperatura: " + String(temperature, 1) + " &deg;C</p>"); // String(float, decimal_places)
            client.println("<p class=\"sensor-data\">Umidade: " + String(humidity, 1) + " %</p>");
            // ------------------------------------------------
            client.println("<p><a href=\"/lampada/on\"><button class=\"button button-on\">LIGAR</button></a>");
            client.println("<a href=\"/lampada/off\"><button class=\"button button-off\">DESLIGAR</button></a></p>");
            client.println("</body></html>");

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
  }
  // Pequeno atraso para evitar leituras DHT muito rápidas e consecutivas
  // O DHT tem um tempo mínimo entre leituras (cerca de 2 segundos para o DHT22)
  delay(100); 
}
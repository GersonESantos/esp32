#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h> // <--- MUDANÇA AQUI: Inclua a biblioteca SSD1306

// --- Configurações do OLED ---
// Pinos I2C para ESP32 são geralmente SDA: GPIO21, SCL: GPIO22
#define OLED_SDA 21 // Pino SDA do ESP32 (data)
#define OLED_SCL 22 // Pino SCL do ESP32 (clock)
#define OLED_RESET -1 // Pino de reset do OLED (use -1 para não usar ou conectar ao RST)

// CORREÇÃO AQUI: Use Adafruit_SSD1306
// Para displays de 128x64 pixels (mais comum)
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET); 
// Se seu display for 128x32 pixels, use:
// Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET);

// Credenciais do WiFi - Substitua pelos seus dados
const char* ssid = "BRUGER_2G";       // Nome da sua rede WiFi
const char* password = "Gersones68"; // Senha da sua rede WiFi

// Define o pino da lâmpada
const int lampadaPin = 13;

// Cria o objeto do servidor na porta 80
WiFiServer server(80);

// Variável para armazenar a requisição HTTP
String header;

// Estado atual da lâmpada
String estadoLampada = "off";

void setup() {
  // Inicia a comunicação serial
  Serial.begin(115200);

  // --- Inicialização do OLED ---
  // CORREÇÃO AQUI: Use SSD1306_SWITCHCAPVCC
  // O 0x3C é o endereço I2C mais comum para displays OLED 128x64.
  // Se não funcionar, tente 0x3D
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // <--- MUDANÇA AQUI: SSD1306_SWITCHCAPVCC
    Serial.println(F("Falha na inicialização do display OLED!"));
    while (true); // Trava aqui se o display não for encontrado
  }

  // Limpa o buffer do display e exibe o logo da Adafruit
  display.display();
  delay(2000); // Mostra por 2 segundos

  // Limpa o display
  display.clearDisplay();
  display.setTextSize(1);      // Tamanho do texto 1
  display.setTextColor(WHITE); // Cor do texto branco (definido pela Adafruit_GFX)

  // Configura o pino da lâmpada como saída
  pinMode(lampadaPin, OUTPUT);
  digitalWrite(lampadaPin, LOW); // Começa com a lâmpada apagada
  estadoLampada = "off"; // Garante que o estado inicial reflita o pino

  // Exibe mensagem no OLED
  display.setCursor(0,0);
  display.println("Conectando WiFi...");
  display.display();


  // Conecta ao WiFi
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) { // Limita tentativas para não travar muito
    delay(500);
    Serial.print(".");
    display.print("."); // Mostra pontos no OLED também
    display.display();
    tentativas++;
  }

  // Mostra o IP quando conectado
  Serial.println("");

  // Limpa o display para mostrar o status da conexão
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
  display.display(); // Atualiza o OLED
  delay(2000); // Deixa a mensagem de IP visível por um tempo

  // Inicia o servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado na porta 80.");

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Servidor OK.");
  display.println("Lampada: " + estadoLampada);
  display.display();
}

void loop() {
  WiFiClient client = server.available(); // Verifica se há clientes conectados

  if (client) { // Se um novo cliente se conectar
    String currentLine = "";              // String para armazenar dados do cliente
    header = ""; // Limpa o header para cada nova requisição
    while (client.connected()) {          // Enquanto o cliente estiver conectado
      if (client.available()) {           // Se houver bytes para ler
        char c = client.read();           // Lê um byte
        header += c;

        if (c == '\n') { // Se o byte for uma nova linha
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
              Serial.println("Lâmpada LIGADA"); // Feedback na serial
            } else if (header.indexOf("GET /lampada/off") >= 0) {
              estadoLampada = "off";
              digitalWrite(lampadaPin, LOW);
              Serial.println("Lâmpada DESLIGADA"); // Feedback na serial
            }

            // --- Atualiza o OLED com o estado da lâmpada ---
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("IP: ");
            display.println(WiFi.localIP());
            display.println("-----------------");
            display.print("Lampada: ");
            display.println(estadoLampada);
            display.display();

            // Página HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>html { font-family: Helvetica; text-align: center;}");
            client.println(".button { padding: 15px 30px; font-size: 24px; margin: 10px; text-decoration: none; border-radius: 5px;}");
            client.println(".button-on { background-color: #4CAF50; color: white; }");
            client.println(".button-off { background-color: #f44336; color: white; }</style></head>");

            client.println("<body><h1>Controle da Lampada</h1>");
            client.println("<p>Estado: " + estadoLampada + "</p>");
            client.println("<p><a href=\"/lampada/on\"><button class=\"button button-on\">LIGAR</button></a>");
            client.println("<a href=\"/lampada/off\"><button class=\"button button-off\">DESLIGAR</button></a></p>");
            client.println("</body></html>");

            break; // Sai do loop 'while (client.connected())'
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    // Fecha a conexão
    client.stop();
  }
}
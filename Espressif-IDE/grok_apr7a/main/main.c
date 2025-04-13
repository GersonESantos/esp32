//#include <WiFi.h>

// Credenciais do WiFi - Substitua pelos seus dados
const char* ssid = "BRUGER_2G";      // Nome da sua rede WiFi
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
  
  // Configura o pino da lâmpada como saída
  pinMode(lampadaPin, OUTPUT);
  digitalWrite(lampadaPin, LOW); // Começa com a lâmpada apagada

  // Conecta ao WiFi
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Mostra o IP quando conectado
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
  
  // Inicia o servidor
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Verifica se há clientes conectados

  if (client) {                             // Se um novo cliente se conectar
    String currentLine = "";                // String para armazenar dados do cliente
    while (client.connected()) {            // Enquanto o cliente estiver conectado
      if (client.available()) {             // Se houver bytes para ler
        char c = client.read();             // Lê um byte
        header += c;
        
        if (c == '\n') {                    // Se o byte for uma nova linha
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
            } else if (header.indexOf("GET /lampada/off") >= 0) {
              estadoLampada = "off";
              digitalWrite(lampadaPin, LOW);
            }
            
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
            
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    // Limpa o header
    header = "";
    // Fecha a conexão
    client.stop();
  }
}
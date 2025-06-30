// Projeto: Controle de LED via MQTT com ESP32
// LED no pino 13 controlado por mensagens MQTT

#include <WiFi.h>
#include <PubSubClient.h>

// Dados da rede Wi-Fi
const char* ssid = "BRUGER_2G";
const char* password = "Gersones68";

// Endereço do broker MQTT
const char* mqtt_server = "broker.emqx.io";

// Objeto WiFi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Pino do LED
#define LED_PIN 13

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando ao WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado com sucesso!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String comando = "";
  for (int i = 0; i < length; i++) {
    comando += (char)payload[i];
  }

  Serial.print("Comando recebido: ");
  Serial.println(comando);

  if (comando == "ON") {
    digitalWrite(LED_PIN, HIGH);
    client.publish("casa/status_led", "Ligado");
    Serial.println("Status do LED: Ligado");

  } else if (comando == "OFF") {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Status do LED: Desligado");

    client.publish("casa/status_led", "Desligado");
  }
}



void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao broker MQTT...");
    if (client.connect("esp32_led_controller")) {
      Serial.println("Conectado!");
      client.subscribe("casa/led13");
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

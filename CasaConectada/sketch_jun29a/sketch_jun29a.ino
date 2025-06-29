#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "SEU_WIFI";
const char* password = "SENHA_WIFI";
const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);

#define LED_SALA 5
#define LED_QUARTO 18
#define LED_COZINHA 19

unsigned long tempoSensor = 0;
char msg[50];

void setup_wifi() {
  delay(10);
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String comando = String((char)payload[0]);
  if (comando == "S") digitalWrite(LED_SALA, HIGH);
  if (comando == "s") digitalWrite(LED_SALA, LOW);
  if (comando == "Q") digitalWrite(LED_QUARTO, HIGH);
  if (comando == "q") digitalWrite(LED_QUARTO, LOW);
  if (comando == "C") digitalWrite(LED_COZINHA, HIGH);
  if (comando == "c") digitalWrite(LED_COZINHA, LOW);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado!");
      client.subscribe("casa/sala");
      client.subscribe("casa/quarto");
      client.subscribe("casa/cozinha");
    } else {
      Serial.print("Falhou. Tentando em 5s...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LED_SALA, OUTPUT);
  pinMode(LED_QUARTO, OUTPUT);
  pinMode(LED_COZINHA, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  if (millis() - tempoSensor > 10000) {
    tempoSensor = millis();
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      snprintf(msg, 50, "Temperatura: %.1f C", t);
      client.publish("casa/temperatura", msg);
      snprintf(msg, 50, "Umidade: %.1f %%", h);
      client.publish("casa/umidade", msg);
    }
  }
}

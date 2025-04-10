// Define o pino do LED embutido (geralmente GPIO 2 na ESP32-WROOM-32 DevKit)
const int ledPin = 2;

void setup() {
  // Configura o pino do LED como saída
  pinMode(ledPin, OUTPUT);
  
  // Inicia a comunicação serial para monitoramento (opcional)
  Serial.begin(115200);
  Serial.println("ESP32 LED Blink iniciado!");
}

void loop() {
  // Acende o LED
  digitalWrite(ledPin, HIGH);  // HIGH acende o LED na maioria das placas
  Serial.println("LED ligado");
  delay(5000);                 // Espera 1 segundo
  
  // Apaga o LED
  digitalWrite(ledPin, LOW);   // LOW apaga o LED
  Serial.println("LED desligado");
  delay(5000);                 // Espera 1 segundo
}
// Array com os pinos GPIO a serem testados (saídas seguras)
int gpios[] = {2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
// Número de GPIOs no array
int numGpios = sizeof(gpios) / sizeof(gpios[0]);

void setup() {
  // Inicia a comunicação serial
  Serial.begin(115200);
  Serial.println("Teste de GPIOs da ESP32 iniciado!");

  // Configura todos os pinos como saída
  for (int i = 0; i < numGpios; i++) {
    pinMode(gpios[i], OUTPUT);
    digitalWrite(gpios[i], LOW); // Começa com todos desligados
  }
}

void loop() {
  // Testa cada GPIO, acendendo e apagando
  for (int i = 0; i < numGpios; i++) {
    Serial.print("Testando GPIO ");
    Serial.println(gpios[i]);
    
    digitalWrite(gpios[i], HIGH); // Acende o LED
    delay(500);                   // Espera 500ms
    digitalWrite(gpios[i], LOW);  // Apaga o LED
    delay(500);                   // Espera 500ms
  }
  
  Serial.println("Ciclo completo! Reiniciando...");
  delay(2000); // Pausa antes de reiniciar o ciclo
}
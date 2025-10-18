// Programa para ESP32-S3 DevKitC-1
// Acende GPIOs seguros (somente OUTPUT) um de cada vez por 100ms em sequência
// GPIO45 (input-only) foi removido automaticamente

const int gpio_list[] = {
  4,
  5,
  6,
  7,
  15,
  16,
  17,
  18,
  8,
  3,
  9,
  10,
  11,
  12,
  13,
  14,
  21,
  2,
  42,
  41,
  40,
  39,
  38,
  37,
  36,
  35,
  48,
  47,
  21,
};
const int num_gpio = sizeof(gpio_list) / sizeof(gpio_list[0]);

void setup() {
  // Inicializa cada GPIO como saída digital e desliga
  for (int i = 0; i < num_gpio; i++) {
    pinMode(gpio_list[i], OUTPUT);        // Configura o pino como saída
    digitalWrite(gpio_list[i], LOW);      // Garante que inicie desligado
  }
}

void loop() {
  // Loop contínuo que acende um pino por vez
  for (int i = 0; i < num_gpio; i++) {
    digitalWrite(gpio_list[i], HIGH);     // Liga o pino atual
    delay(100);                            // Aguarda 100ms
    digitalWrite(gpio_list[i], LOW);      // Desliga o pino atual
  }
}

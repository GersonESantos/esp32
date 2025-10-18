// FERNANDO KOYANAGI contato@fernandok.com
// Programa para ESP32-S3 DevKitC-1
// Acende GPIOs seguros um de cada vez por 100ms em sequência
// Feito para missão crítica - base em GPIOs confirmados pelas tabelas fornecidas
// 29 GPIOs diferentes estão sendo testados.
// Lista com os GPIOs seguros selecionados para teste sequencial

// -------------------------------------------------------------------------------------------------
// ************* SEGUROS para uso geral (sem risco no boot)
// Esses podem ser usados livremente como entrada/saída digital, PWM, periféricos etc:
// GPIO3, 4, 5, 6, 7
// GPIO8, 9, 10, 11, 12, 13, 14
// GPIO17, 18, 21
// GPIO37, 38, 39, 40, 41, 42
// GPIO45, 47, 48
// -------------------------------------------------------------------------------------------------
// Pinos com RESTRIÇÕES/ risco de uso
// GPIO2 → Bootstrap crítico
// Para boot normal pela Flash SPI, o GPIO2 deve estar em LOW (0) na inicialização.
//  Se for usado com carga externa que mantenha HIGH, o chip pode falhar no boot.
//  Pode ser usado, mas com muito cuidado (ex.: evitar LEDs com resistor para VCC, evitar circuitos que sustentem alto na energização).
//  GPIO15 (U0RTS, XTAL_32K_P) → Bootstrap
//  Está ligado à seleção de boot em alguns modos de inicialização.
//  Recomendado não forçar nível elétrico durante reset.
//  Também opcionalmente pode estar associado ao cristal de 32 kHz.
//  GPIO16 (U0CTS, XTAL_32K_N) → Bootstrap
//  Assim como o 15, usado em bootstrapping e opcionalmente para cristal de 32 kHz.
//  Melhor evitar cargas pesadas que puxem o nível na energização.
//  GPIO35, 36 → Atenção nos módulos
//  No ESP32‑S3‑WROOM‑1 (usado no DevKitC‑1 básico), são livres.
//  Mas no ESP32‑S3‑WROOM‑2 ou WROVER, podem estar conectados internamente à Flash/PSRAM OCTAL.
//  Se estiver usando o DevKitC‑1 comum (WROOM‑1) → são seguros, mas em designs futuros tenha cautela.
// -------------------------------------------------------------------------------------------------
//  RESTRIÇÕES FORTES (não usar para GPIO comum em muitos casos)
//  GPIO19 / GPIO20 (USB D− e USB D+) → você não listou, mas é importante lembrar.
//  Esses são usados pela interface USB nativa. Se tentar usar como GPIO em conjunto com USB, pode travar a comunicação.

const int gpio_list[] = {
  4,   // GPIO4
  5,   // GPIO5
  6,   // GPIO6
  7,   // GPIO7
  15,  // GPIO15  (U0RTS, XTAL_32K_P) → Bootstrap
  16,  // GPIO16  (U0CTS, XTAL_32K_N) → Bootstrap
  17,  // GPIO17
  18,  // GPIO18
  8,   // GPIO8
  3,   // GPIO3
  9,   // GPIO9
  10,  // GPIO10
  11,  // GPIO11
  12,  // GPIO12
  13,  // GPIO13
  14,  // GPIO14
  21,  // GPIO21
  2,   // GPIO2    // cuidado GPIO2 bootstrap deve estar em nível baixo (0) para permitir boot normal a partir da Flash SPI.   
  42,  // GPIO42
  41,  // GPIO41
  40,  // GPIO40
  39,  // GPIO39
  38,  // GPIO38
  37,  // GPIO37
  36,  // GPIO36  no ESP32‑S3‑WROOM‑2 ou WROVER, podem estar conectados internamente à Flash/PSRAM OCTAL.
  35,  // GPIO35  no ESP32‑S3‑WROOM‑2 ou WROVER, podem estar conectados internamente à Flash/PSRAM OCTAL.
  45,  // GPIO45 
  48,  // GPIO48  LED 
  47,  // GPIO47
  21   // GPIO21 
};

// Calcula quantos elementos existem no array (total de GPIOs a testar)
const int num_gpio = sizeof(gpio_list) / sizeof(gpio_list[0]);

void setup() {
  // Inicializa cada GPIO como saída digital e coloca em nível baixo (desligado)
  for (int i = 0; i < num_gpio; i++) {
    pinMode(gpio_list[i], OUTPUT);        // Configura o GPIO atual como saída
    digitalWrite(gpio_list[i], LOW);      // Garante que ele comece desligado
  }
}

void loop() {
  // Loop contínuo para testar GPIOs um por um
  for (int i = 0; i < num_gpio; i++) {
    digitalWrite(gpio_list[i], HIGH);     // Liga o GPIO atual
    delay(100);                            // Aguarda 100 milissegundos
    digitalWrite(gpio_list[i], LOW);      // Desliga o GPIO atual
  }
}

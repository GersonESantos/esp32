/*
 * Teste do módulo PCM5102A com ESP32-S3 DevKitC-1
 * Gera um tom senoidal de 440Hz (Lá musical) para testar o áudio
 * 
 * Conexões:
 * PCM5102A -> ESP32-S3
 * VCC -> 3V3
 * GND -> GND  
 * BCK -> GPIO4
 * DIN -> GPIO5
 * LCK -> GPIO6
 * SCK -> GND
 * FLT -> 3V3
 * FMT -> GND
 * XSMT -> 3V3
 */

#include "driver/i2s.h"
#include <math.h>

// Definição dos pinos I2S (seguros para ESP32-S3)
#define I2S_BCLK_PIN    4    // Bit Clock
#define I2S_DOUT_PIN    5    // Data Output  
#define I2S_LRC_PIN     6    // Left/Right Clock (Word Select)

// Configurações de áudio
#define SAMPLE_RATE     44100    // Taxa de amostragem em Hz
#define BITS_PER_SAMPLE 16       // Bits por amostra
#define CHANNELS        2        // Estéreo (2 canais)
#define BUFFER_SIZE     1024     // Tamanho do buffer de áudio

// Parâmetros do tom de teste
#define TEST_FREQUENCY  440.0    // Frequência do tom (Lá musical)
#define AMPLITUDE       8000     // Amplitude do sinal (0-32767 para 16 bits)

// Variáveis globais
float phase = 0.0;               // Fase atual da onda senoidal
float phase_increment;           // Incremento de fase por amostra

void setup() {
  // Inicializa comunicação serial para debug
  Serial.begin(115200);
  Serial.println("Iniciando teste PCM5102A...");
  
  // Calcula o incremento de fase para gerar a frequência desejada
  // phase_increment = 2π × frequência / sample_rate
  phase_increment = 2.0 * PI * TEST_FREQUENCY / SAMPLE_RATE;
  
  // Configura os parâmetros do I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),  // Modo mestre, apenas transmissão
    .sample_rate = SAMPLE_RATE,                           // Taxa de amostragem
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,        // 16 bits por amostra
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,        // Formato estéreo
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,   // Formato I2S padrão
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,            // Prioridade da interrupção
    .dma_buf_count = 8,                                   // Número de buffers DMA
    .dma_buf_len = BUFFER_SIZE,                          // Tamanho de cada buffer DMA
    .use_apll = false,                                   // Não usar APLL (Audio PLL)
    .tx_desc_auto_clear = true,                          // Limpar automaticamente descritores TX
    .fixed_mclk = 0                                      // MCLK automático
  };
  
  // Configura os pinos do I2S
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK_PIN,      // Pino do Bit Clock
    .ws_io_num = I2S_LRC_PIN,        // Pino do Word Select (LR Clock)
    .data_out_num = I2S_DOUT_PIN,    // Pino de saída de dados
    .data_in_num = I2S_PIN_NO_CHANGE // Não usar entrada de dados
  };
  
  // Instala o driver I2S
  esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Erro ao instalar driver I2S: %s\n", esp_err_to_name(err));
    return;
  }
  
  // Configura os pinos I2S
  err = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Erro ao configurar pinos I2S: %s\n", esp_err_to_name(err));
    return;
  }
  
  // Inicia o I2S
  err = i2s_start(I2S_NUM_0);
  if (err != ESP_OK) {
    Serial.printf("Erro ao iniciar I2S: %s\n", esp_err_to_name(err));
    return;
  }
  
  Serial.println("PCM5102A inicializado com sucesso!");
  Serial.printf("Gerando tom de %.1f Hz\n", TEST_FREQUENCY);
  Serial.println("Conecte fones de ouvido ou alto-falantes na saída do PCM5102A");
}

void loop() {
  // Buffer para armazenar as amostras de áudio
  int16_t samples[BUFFER_SIZE * 2]; // *2 porque são 2 canais (estéreo)
  
  // Gera as amostras de áudio (onda senoidal)
  for (int i = 0; i < BUFFER_SIZE; i++) {
    // Calcula o valor da onda senoidal
    int16_t sample = (int16_t)(sin(phase) * AMPLITUDE);
    
    // Atualiza a fase para a próxima amostra
    phase += phase_increment;
    
    // Mantém a fase no intervalo [0, 2π] para evitar overflow
    if (phase >= 2.0 * PI) {
      phase -= 2.0 * PI;
    }
    
    // Coloca a mesma amostra nos dois canais (mono em estéreo)
    samples[i * 2] = sample;     // Canal esquerdo
    samples[i * 2 + 1] = sample; // Canal direito
  }
  
  // Envia as amostras para o I2S
  size_t bytes_written;
  esp_err_t err = i2s_write(I2S_NUM_0, samples, sizeof(samples), &bytes_written, portMAX_DELAY);
  
  // Verifica se houve erro na transmissão
  if (err != ESP_OK) {
    Serial.printf("Erro ao escrever dados I2S: %s\n", esp_err_to_name(err));
  }
  
  // Verifica se todos os bytes foram escritos
  if (bytes_written != sizeof(samples)) {
    Serial.printf("Aviso: Apenas %d de %d bytes foram escritos\n", bytes_written, sizeof(samples));
  }
  
  // Pequeno delay para debug (opcional)
  // delay(1); // Descomente se quiser ver mensagens de debug mais devagar
}

/*
 * Funções auxiliares para diferentes tipos de teste
 */

// Função para gerar tom de frequência específica
void generateTone(float frequency) {
  phase_increment = 2.0 * PI * frequency / SAMPLE_RATE;
  Serial.printf("Mudando para frequência: %.1f Hz\n", frequency);
}

// Função para testar diferentes frequências sequencialmente
void testFrequencies() {
  static unsigned long lastChange = 0;
  static int freqIndex = 0;
  float testFreqs[] = {220, 440, 880, 1760}; // Lá em diferentes oitavas
  int numFreqs = sizeof(testFreqs) / sizeof(testFreqs[0]);
  
  // Muda frequência a cada 3 segundos
  if (millis() - lastChange > 3000) {
    generateTone(testFreqs[freqIndex]);
    freqIndex = (freqIndex + 1) % numFreqs;
    lastChange = millis();
  }
}

/*
 * Para usar o teste de frequências, substitua o conteúdo do loop() por:
 * testFrequencies();
 * 
 * Para testar uma frequência específica, chame no setup():
 * generateTone(1000); // Para 1kHz, por exemplo
 */
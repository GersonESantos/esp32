#include <Adafruit_NeoPixel.h>

// Define o pino do LED RGB (NeoPixel)
#define LED_PIN 48   // GPIO48 na versão 1.0 do DevKitC-1
#define NUMPIXELS 1  // Apenas 1 LED onboard

// Cria o objeto NeoPixel
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Tempo de troca
unsigned long delay_ms = 500;

void setup() {
  pixels.begin();   // Inicializa o NeoPixel
}

void loop() {
  // Vermelho
  pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  pixels.show();
  delay(delay_ms);

  // Verde
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();
  delay(delay_ms);

  // Azul
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show();
  delay(delay_ms);
}

# esp32
 
# Projeto: Automacao Residencial com ESP32 + MQTT

## 🧰Etapa 1 - Montagem do Circuito e Código do ESP32

### Objetivo

Controlar três cômodos (Sala, Quarto e Cozinha) remotamente via MQTT, usando a placa ESP32. Opcionalmente, monitorar temperatura e umidade com o sensor DHT22.

---

### Materiais Necessários

* 1x ESP32
* 3x LEDs (ou módulos relê, caso for controlar lâmpadas reais)
* 3x Resistores de 220 Ω (se usar LEDs)
* 1x Sensor DHT22 (opcional)
* Jumpers e Protoboard
* Fonte 5V USB

---

### Ligação do Circuito

| Componente   | Pino do ESP32 |
| ------------ | ------------- |
| LED Sala     | GPIO 5        |
| LED Quarto   | GPIO 18       |
| LED Cozinha  | GPIO 19       |
| DHT22 (DATA) | GPIO 2        |

**Observação:** Ligar o VCC e GND do DHT22 corretamente. Se usar relê, ligar também os 5V e GND.

---

### Configurações Iniciais

1. Instale as bibliotecas na IDE do Arduino:

   * `PubSubClient` (autor: Nick O'Leary)
   * `DHT sensor library` (autor: Adafruit)
   * `Adafruit Unified Sensor` (dependência do DHT)

2. Configure a IDE para ESP32:

   * Arquivo > Preferências > URL adicional:

     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   * Ferramentas > Placa > Gerenciador de Placas > Instale "ESP32 by Espressif Systems"

---

### Parâmetros do Código

* **SSID e senha do WiFi:**

  ```cpp
  const char* ssid = "SEU_WIFI";
  const char* password = "SENHA_WIFI";
  ```
* **Broker MQTT:**

  ```cpp
  const char* mqtt_server = "broker.emqx.io";
  ```

---

### Funcionalidades do Código

* Conecta no WiFi
* Conecta ao broker MQTT e assina os tópicos:

  * `casa/sala`, `casa/quarto`, `casa/cozinha`
* Espera comandos para ligar/desligar os LEDs (ou relês):

  * "S" / "s" = Sala ON/OFF
  * "Q" / "q" = Quarto ON/OFF
  * "C" / "c" = Cozinha ON/OFF
* (Opcional) Publica temperatura e umidade a cada 10 segundos:

  * `casa/temperatura`
  * `casa/umidade`

---

### Próxima Etapa

Criar o aplicativo no **MIT App Inventor** com suporte a MQTT para enviar e receber esses comandos. (Etapa 2)

---

Feito com ❤️ por Eng. Luan | IoT e Automação
[http://youtube.com/engeasier](http://youtube.com/engeasier)


✅ instalar a biblioteca PubSubClient
Segue o passo a passo:

Abra a IDE do Arduino.

Vá em Sketch > Incluir Biblioteca > Gerenciar Bibliotecas...

Na aba que abrir, no campo de busca digite: PubSubClient

Selecione a biblioteca do Nick O'Leary.

Clique em Instalar.
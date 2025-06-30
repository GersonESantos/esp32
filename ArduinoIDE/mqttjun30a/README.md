# Projeto: Controle de LED com ESP32 via MQTT

## 💡 Objetivo

Controlar um LED conectado ao pino 13 da placa ESP32 usando mensagens enviadas por MQTT, através de um app Android como o IoT MQTT Panel.

---

## 🤖 Componentes Necessários

* 1x ESP32
* 1x LED
* 1x Resistor de 220 Ω (opcional)
* Jumpers e Protoboard
* Smartphone Android com o app **IoT MQTT Panel** instalado

---

## ⚖️ Ligação do Circuito

| Componente   | Pino ESP32 |
| ------------ | ---------- |
| LED (anodo)  | GPIO 13    |
| LED (catodo) | GND        |

**Dica:** Use resistor em série com o LED (220 Ω) para limitar corrente.

---

## 💻 Código-Fonte (Resumo)

* Conecta no Wi-Fi `BRUGER_2G`
* Conecta no broker MQTT público `broker.emqx.io`
* Assina o tópico `casa/led13`
* Quando recebe:

  * `ON` → Acende o LED
  * `OFF` → Apaga o LED

**Velocidade da Serial:** `115200`

---

## 🛶 Como usar o IoT MQTT Panel

1. Baixe o app na Play Store: **IoT MQTT Panel**
2. Crie uma nova conexão (Add Panel)

   * **Name:** LED ESP32
   * **Broker URL:** `tcp://broker.emqx.io:1883`
   * **Client ID:** algo como `mqtt_android_led`
3. Dentro do painel, toque em “+” > **Switch**

   * **Name:** Luz da Sala
   * **Topic:** `casa/led13`
   * **Payload ON:** `ON`
   * **Payload OFF:** `OFF`
   * Salve e teste

---

## 📊 Teste e Monitoramento

No **Serial Monitor da IDE Arduino**, você deve ver:

```
Conectando ao WiFi: BRUGER_2G
WiFi conectado com sucesso!
IP: 192.168.0.116
Tentando conectar ao broker MQTT...Conectado!
Comando recebido: ON
Comando recebido: OFF
```

E o LED deve ligar e desligar conforme os comandos enviados do app.

---

## 🌟 Possíveis Expansões

* Adicionar feedback do estado do LED publicando em `casa/status_led`
* Controle de vários LEDs ou relês
* Integração com App Inventor ou Node-RED
* Acesso remoto via DDNS

---



---

# Projeto: Controle de LED com ESP32 via MQTT e Interface Web


>
> * Um dispositivo físico (ESP32) que pode ser controlado tanto por um botão local quanto por comandos remotos.
> * Uma interface web que funciona como um painel de controle, enviando comandos e mostrando o status do dispositivo em tempo real.
> * Tudo isso se comunicando de forma eficiente e sincronizada através de um broker MQTT.

Este projeto demonstra como controlar um LED conectado a um ESP32 através de uma página da web usando o protocolo MQTT para comunicação em tempo real.

## Como Funciona

A arquitetura do projeto é simples:

1.  O **ESP32** se conecta à rede Wi-Fi e a um broker MQTT público. Ele se inscreve em um tópico para receber comandos (`ON`/`OFF`) e publica seu estado atual em outro tópico.
2.  A **Interface Web (`index.html`)** também se conecta ao mesmo broker MQTT. Ela possui botões para enviar comandos e uma área de status que se inscreve no tópico de estado do ESP32 para exibir se o LED está "Ligado" ou "Desligado".
3.  O **Broker MQTT** (`broker.emqx.io`) atua como um intermediário, recebendo as mensagens da página web e as entregando ao ESP32, e vice-versa.

## Componentes do Projeto

### 1. Código do ESP32 (`esp32_code.ino`)

Este código é responsável por conectar o ESP32 à rede, gerenciar a comunicação MQTT e controlar o pino do LED. O código-fonte completo pode ser encontrado no arquivo `esp32_code.ino`.

### 2. Interface Web de Controle (`index.html`)

Este arquivo cria uma página web com botões para enviar comandos "Ligar" e "Desligar" e uma área para exibir o status atual do LED. O código-fonte completo pode ser encontrado no arquivo `index.html`.

## Como Usar

1.  **Hardware**: Monte um circuito simples com um LED conectado ao pino 13 do ESP32 e ao GND (com um resistor apropriado, ex: 220Ω).
2.  **Configuração do ESP32**:
    * Abra o arquivo `esp32_code.ino` na Arduino IDE.
    * Altere as variáveis `ssid` e `password` com os dados da sua rede Wi-Fi.
    * Faça o upload do código para o seu ESP32.
    * Abra o Monitor Serial (baud rate 115200) para verificar se ele conectou com sucesso ao Wi-Fi e ao broker MQTT.
3.  **Interface Web**:
    * Abra o arquivo `index.html` em qualquer navegador de internet moderno (Chrome, Firefox, etc.).

Agora você pode clicar nos botões "Ligar LED" e "Desligar LED" na página para controlar o LED no seu ESP32, e o status será atualizado na tela em tempo real.

## Testando com o MQTTX Web Client

Como alternativa à interface web, ou para fins de depuração, você pode usar um cliente MQTT como o [MQTTX Web Client](https://mqttx.app/web-client).

1.  **Conexão**:
    * **Host**: `wss` e `broker.emqx.io`
    * **Port**: `8084`
    * **Client ID**: Pode ser qualquer um (ex: `meu_teste_web`)
2.  **Para enviar comandos**:
    * Publique no tópico: `casa/led13`
    * Com a mensagem (Payload): `ON` ou `OFF`
3.  **Para ver o status**:
    * Inscreva-se (Subscribe) no tópico: `casa/status_led`

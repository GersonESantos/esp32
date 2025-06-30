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
IP: 192.168.X.X
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



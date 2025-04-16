#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <ArduinoJson.h>

// Configurações Wi-Fi
const char* ssid = "YOUR_SSID";         // Substitua pelo nome da sua rede Wi-Fi
const char* password = "YOUR_PASSWORD"; // Substitua pela senha da sua rede Wi-Fi

// URL dos endpoints do servidor local (ajuste o endereço IP conforme sua rede)
const char* updateUrl = "http://192.168.X.Y:5000/update";   // Envio de status
const char* controlUrl = "http://192.168.X.Y:5000/control"; // Recebe comando manual

Adafruit_AMG88xx amg;  // Objeto para o sensor termográfico

// Definição dos pinos para os LEDs que representam os quadrantes
const int ledQ1 = 25;  // Quadrante Superior Esquerdo
const int ledQ2 = 26;  // Quadrante Superior Direito
const int ledQ3 = 27;  // Quadrante Inferior Esquerdo
const int ledQ4 = 14;  // Quadrante Inferior Direito

// Pino para simular o controle do ar condicionado (LED ou relé)
const int acPin = 32;

// Temperaturas de referência
const float tempLimite = 36.0;   // Para acionar cada quadrante
const float acThreshold = 33.0;  // Média para ligar o ar condicionado automaticamente

// Vetor para armazenar os 64 pixels (matriz 8x8)
float pixels[64];

void setup() {
  Serial.begin(115200);
  
  // Conecta ao Wi-Fi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao WiFi!");

  // Inicializa comunicação I2C (pinos padrão do ESP32: SDA = GPIO21, SCL = GPIO22)
  Wire.begin(21, 22);
  
  // Configura os pinos dos LEDs e do ar condicionado como saída
  pinMode(ledQ1, OUTPUT);
  pinMode(ledQ2, OUTPUT);
  pinMode(ledQ3, OUTPUT);
  pinMode(ledQ4, OUTPUT);
  pinMode(acPin, OUTPUT);
  
  // Inicializa o sensor AMG8833
  if (!amg.begin()) {
    Serial.println("Falha na inicializacao do AMG8833. Verifique as conexoes.");
    while (1);
  }
  delay(100);  // Tempo para estabilizacao do sensor
}

void loop() {
  // Lê os 64 pixels do sensor
  amg.readPixels(pixels);
  
  // Estados automáticos calculados
  bool autoQ1 = false, autoQ2 = false, autoQ3 = false, autoQ4 = false;
  bool autoIluminacao = false;  // Se qualquer quadrante tiver pixel acima do limite
  bool autoAC = false;          // Para o ar condicionado
  float soma = 0;
  
  // Calcula a temperatura média
  for (int i = 0; i < 64; i++) {
    soma += pixels[i];
  }
  float media = soma / 64.0;
  
  // Determina o acionamento automático para cada quadrante
  for (int linha = 0; linha < 8; linha++) {
    for (int coluna = 0; coluna < 8; coluna++) {
      int idx = linha * 8 + coluna;
      float t = pixels[idx];
      if (t > tempLimite) {
        if (linha < 4 && coluna < 4) autoQ1 = true;
        else if (linha < 4 && coluna >= 4) autoQ2 = true;
        else if (linha >= 4 && coluna < 4) autoQ3 = true;
        else if (linha >= 4 && coluna >= 4) autoQ4 = true;
      }
    }
  }
  autoIluminacao = autoQ1 || autoQ2 || autoQ3 || autoQ4;
  autoAC = (media > acThreshold);

  // Variáveis de controle final que podem ser sobrescritas por comando manual
  bool finalIluminacao = autoIluminacao;
  bool finalAC = autoAC;

  // Consulta o servidor para verificar comandos manuais
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(controlUrl);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      // Exemplo de JSON esperado: {"iluminacao": "null" ou "ligado"/"desligado", "ar_condicionado": "null" ou "ligado"/"desligado"}
      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        const char* manualIlum = doc["iluminacao"]; 
        const char* manualAC = doc["ar_condicionado"];
        if (String(manualIlum) != "null") {
          finalIluminacao = (String(manualIlum) == "ligado");
        }
        if (String(manualAC) != "null") {
          finalAC = (String(manualAC) == "ligado");
        }
      } else {
        Serial.println("Erro ao parsear JSON do controle.");
      }
    } else {
      Serial.println("Erro ao acessar /control: " + String(httpCode));
    }
    http.end();
  } else {
    Serial.println("WiFi desconectado!");
  }
  
  // Atualiza os LEDs dos quadrantes conforme o controle final
  digitalWrite(ledQ1, finalIluminacao ? HIGH : LOW);
  digitalWrite(ledQ2, finalIluminacao ? HIGH : LOW);
  digitalWrite(ledQ3, finalIluminacao ? HIGH : LOW);
  digitalWrite(ledQ4, finalIluminacao ? HIGH : LOW);
  
  // Atualiza o status do ar condicionado
  digitalWrite(acPin, finalAC ? HIGH : LOW);

  // Envia os dados de status para o servidor via HTTP POST
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http2;
    http2.begin(updateUrl);
    http2.addHeader("Content-Type", "application/json");

    String jsonPayload = "{";
    jsonPayload += "\"iluminacao\": \"" + String(finalIluminacao ? "ligado" : "desligado") + "\", ";
    jsonPayload += "\"ar_condicionado\": \"" + String(finalAC ? "ligado" : "desligado") + "\", ";
    jsonPayload += "\"temperatura_media\": " + String(media, 1);
    jsonPayload += "}";

    int httpResponseCode = http2.POST(jsonPayload);
    if (httpResponseCode > 0) {
      String response = http2.getString();
      Serial.println("Resposta do servidor: " + response);
    } else {
      Serial.println("Erro no HTTP POST: " + String(httpResponseCode));
    }
    http2.end();
  }
  
  // Exibe a matriz de temperaturas para depuração
  Serial.println("Matriz de temperaturas:");
  for (int linha = 0; linha < 8; linha++) {
    String linhaTemp = "";
    for (int coluna = 0; coluna < 8; coluna++) {
      int idx = linha * 8 + coluna;
      linhaTemp += String(pixels[idx], 1) + "\t";
    }
    Serial.println(linhaTemp);
  }
  Serial.println("-------------------------");

  delay(1000);  // Aguarda 1 segundo antes da próxima iteração
}

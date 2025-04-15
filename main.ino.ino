#include <Wire.h>
#include <Adafruit_AMG88xx.h>

Adafruit_AMG88xx amg;  // Cria objeto para o sensor termográfico

// Defina os pinos dos LEDs para o ESP32:
const int ledQ1 = 25;  // Quadrante Superior Esquerdo
const int ledQ2 = 26;  // Quadrante Superior Direito
const int ledQ3 = 27;  // Quadrante Inferior Esquerdo
const int ledQ4 = 14;  // Quadrante Inferior Direito

// Temperatura limite para ligar o LED
const float tempLimite = 36.0;

// Array para armazenar os dados do sensor (matriz 8x8 = 64 pixels)
float pixels[64];

void setup() {
  Serial.begin(115200);
  
  // Inicializa a comunicação I2C com os pinos padrão do ESP32:
  Wire.begin(21, 22);  // SDA: GPIO21, SCL: GPIO22

  // Configura os pinos dos LEDs como saída
  pinMode(ledQ1, OUTPUT);
  pinMode(ledQ2, OUTPUT);
  pinMode(ledQ3, OUTPUT);
  pinMode(ledQ4, OUTPUT);

  // Inicializa o sensor AMG8833
  if (!amg.begin()) {
    Serial.println("Falha na inicializacao do AMG8833. Verifique as conexoes.");
    while (1);  // Para execução caso haja erro
  }
  
  // Aguarda um tempo para estabilizar o sensor
  delay(100);
}

void loop() {
  // Lê os 64 pixels do sensor
  amg.readPixels(pixels);

  // Variáveis para armazenar o estado de cada quadrante
  bool q1Acende = false, q2Acende = false, q3Acende = false, q4Acende = false;
  
  // Percorre a matriz do sensor (8 linhas x 8 colunas)
  for (int linha = 0; linha < 8; linha++) {
    for (int coluna = 0; coluna < 8; coluna++) {
      int idx = linha * 8 + coluna;  // Calcula o índice no array
      float temperatura = pixels[idx];

      // Se a temperatura estiver acima do limite, aciona o LED correspondente
      if (temperatura > tempLimite) {
        if (linha < 4 && coluna < 4) {
          q1Acende = true;  // Quadrante Superior Esquerdo
        } else if (linha < 4 && coluna >= 4) {
          q2Acende = true;  // Quadrante Superior Direito
        } else if (linha >= 4 && coluna < 4) {
          q3Acende = true;  // Quadrante Inferior Esquerdo
        } else if (linha >= 4 && coluna >= 4) {
          q4Acende = true;  // Quadrante Inferior Direito
        }
      }
    }
  }

  // Atualiza os LEDs de acordo com a detecção
  digitalWrite(ledQ1, q1Acende ? HIGH : LOW);
  digitalWrite(ledQ2, q2Acende ? HIGH : LOW);
  digitalWrite(ledQ3, q3Acende ? HIGH : LOW);
  digitalWrite(ledQ4, q4Acende ? HIGH : LOW);

  // Exibe os dados do sensor no Serial Monitor para depuração:
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

  // Aguarda 1 segundo antes da próxima leitura
  delay(1000);
}

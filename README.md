# Sistema de Iluminação com Sensor Termográfico AMG8833 e ESP32

Este projeto utiliza um sensor termográfico **AMG8833** juntamente com um ESP32 para criar um sistema que acende LEDs (ou, futuramente, pode acionar relés para controlar luzes reais) quando detecta a presença de uma pessoa em um dos quatro quadrantes de uma área monitorada. Cada quadrante é definido com base na matriz 8x8 obtida do sensor. Se a temperatura em algum quadrante ultrapassar 36°C, o LED correspondente será acionado.

## Sumário

- [Materiais Necessários](#materiais-necess%C3%A1rios)
- [Esquemático e Conexões](#esquem%C3%A1tico-e-conex%C3%B5es)
- [Configuração do Ambiente](#configura%C3%A7%C3%A3o-do-ambiente)
- [Código do Projeto](#c%C3%B3digo-do-projeto)
- [Testes e Depuração](#testes-e-depura%C3%A7%C3%A3o)
- [Considerações Finais](#considera%C3%A7%C3%B5es-finais)

## Materiais Necessários

- **ESP32** (qualquer modelo compatível)
- **Sensor Termográfico AMG8833 (Grid-EYE)**
- **4 LEDs** (um para cada quadrante)
- **4 resistores de 220 Ω** (para limitar a corrente dos LEDs)
- **Protoboard e fios jumpers**

## Esquemático e Conexões

### Conexão do Sensor AMG8833 (I²C)

- **VCC do AMG8833** → Pino **3.3V** do ESP32  
- **GND do AMG8833** → Pino **GND** do ESP32  
- **SDA do AMG8833** → Pino **GPIO21** (usado para SDA)  
- **SCL do AMG8833** → Pino **GPIO22** (usado para SCL)  

> **Observação:** Verifique se seu módulo já possui resistores pull-up nos pinos SDA e SCL. Caso contrário, adicione resistores de 4.7 kΩ entre cada pino (SDA/SCL) e o 3.3V.

### Conexão dos LEDs

Cada LED representa um quadrante da área monitorada, conforme a divisão abaixo:

- **Quadrante Superior Esquerdo (Q1):**  
  - Conecte o ânodo (com resistor de 220 Ω) ao pino **GPIO25**  
  - Conecte o cátodo ao **GND**

- **Quadrante Superior Direito (Q2):**  
  - Conecte o ânodo (com resistor de 220 Ω) ao pino **GPIO26**  
  - Conecte o cátodo ao **GND**

- **Quadrante Inferior Esquerdo (Q3):**  
  - Conecte o ânodo (com resistor de 220 Ω) ao pino **GPIO27**  
  - Conecte o cátodo ao **GND**

- **Quadrante Inferior Direito (Q4):**  
  - Conecte o ânodo (com resistor de 220 Ω) ao pino **GPIO14**  
  - Conecte o cátodo ao **GND**

## Configuração do Ambiente

1. **Instale a IDE do Arduino:**  
   Faça o download e instale a [IDE do Arduino](https://www.arduino.cc/en/software).

2. **Instale a Biblioteca AMG8833:**  
   Abra a IDE do Arduino e navegue até **Sketch > Incluir Biblioteca > Gerenciar Bibliotecas...**  
   Procure por “Adafruit AMG88xx” e instale a biblioteca compatível com o seu sensor.

## Código do Projeto

O código do projeto se encontra no arquivo `main.ino` e realiza as seguintes funções:

- **Leitura do sensor:**  
  Coleta os 64 valores da matriz de temperatura do sensor AMG8833.

- **Divisão da matriz em 4 quadrantes:**  
  - **Quadrante Superior Esquerdo:** Linhas 0 a 3 e Colunas 0 a 3  
  - **Quadrante Superior Direito:** Linhas 0 a 3 e Colunas 4 a 7  
  - **Quadrante Inferior Esquerdo:** Linhas 4 a 7 e Colunas 0 a 3  
  - **Quadrante Inferior Direito:** Linhas 4 a 7 e Colunas 4 a 7

- **Condição para acionar o LED:**  
  Se pelo menos um pixel em um quadrante apresentar temperatura superior a 36°C, o LED correspondente é acionado.

Consulte o arquivo `main.ino` para o código completo.

## Testes e Depuração

1. **Verifique as Conexões:**  
   Certifique-se de que todas as conexões estão corretas, principalmente a alimentação do sensor e os pinos I²C do ESP32.

2. **Carregue o Código:**  
   Conecte o ESP32 ao computador e faça o upload do código utilizando a IDE do Arduino.

3. **Monitor Serial:**  
   Utilize o Serial Monitor para visualizar os valores de temperatura e confirmar o acionamento dos LEDs conforme os quadrantes.

4. **Ajuste de Parâmetros:**  
   Se necessário, ajuste o limite de temperatura (atualmente definido como 36°C) ou refine a lógica de detecção no código.

## Considerações Finais

- **Escalabilidade:**  
  O sistema pode ser expandido para acionar relés que controlem luzes reais ou integrar a conectividade Wi-Fi/Bluetooth para monitoramento remoto.

- **Calibração:**  
  Execute testes e calibrações em ambiente real para garantir a precisão nas medições do sensor.

- **Aplicações:**  
  Além do controle de iluminação, este sistema pode ser adaptado para aplicações de monitoramento e segurança em ambientes diversos.

---

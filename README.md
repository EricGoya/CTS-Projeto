# Sistema Integrado de Iluminação, Climatização e Monitoramento com Comando Manual

Este projeto utiliza o sensor termográfico **AMG8833** com um **ESP32** para monitorar a presença (através de leituras térmicas) e controlar sistemas de iluminação e ar condicionado, de modo automático e manual.  
Caso a temperatura em algum quadrante (da matriz 8x8) ultrapasse 36°C, os LEDs correspondentes serão acionados automaticamente; da mesma forma, uma temperatura média acima de 33°C aciona o ar condicionado automaticamente.  
Porém, o sistema também permite que um usuário sobrescreva o comportamento automático via interface web, definindo manualmente os estados dos dispositivos.

## Sumário

- [Materiais Necessários](#materiais-necess%C3%A1rios)
- [Esquemático e Conexões](#esquem%C3%A1tico-e-conex%C3%B5es)
- [Configuração do Ambiente e Servidor Local](#configura%C3%A7%C3%A3o-do-ambiente-e-servidor-local)
- [Código do Projeto](#c%C3%B3digo-do-projeto)
- [Frontend e Controle Manual](#frontend-e-controle-manual)
- [Ferramentas Explicadas](#ferramentas-explicadas)
- [Testes e Depuração](#testes-e-depura%C3%A7%C3%A3o)
- [Considerações Finais](#considera%C3%A7%C3%B5es-finais)

## Materiais Necessários

- **ESP32** (qualquer modelo compatível)  
- **Sensor Termográfico AMG8833 (Grid-EYE)**  
- **4 LEDs** (para representar cada quadrante – iluminação)  
- **1 LED adicional** (para simular o ar condicionado)  
- **5 resistores de 220 Ω**  
- **Protoboard e fios jumpers**

## Esquemático e Conexões

### Conexão do Sensor AMG8833 (I²C)
- **VCC** do AMG8833 → **3.3V** do ESP32  
- **GND** do AMG8833 → **GND** do ESP32  
- **SDA** do AMG8833 → **GPIO21**  
- **SCL** do AMG8833 → **GPIO22**  

> **Observação:** Se o módulo não possuir resistores pull-up, conecte resistores de 4.7 kΩ entre SDA/SCL e 3.3V.

### Conexões dos LEDs

- **LEDs dos Quadrantes (Iluminação):**  
  - **Q1 (Superior Esquerdo):**  
    - Ânodo (via resistor 220 Ω) → **GPIO25**  
    - Cátodo → **GND**
  - **Q2 (Superior Direito):**  
    - Ânodo (via resistor 220 Ω) → **GPIO26**  
    - Cátodo → **GND**
  - **Q3 (Inferior Esquerdo):**  
    - Ânodo (via resistor 220 Ω) → **GPIO27**  
    - Cátodo → **GND**
  - **Q4 (Inferior Direito):**  
    - Ânodo (via resistor 220 Ω) → **GPIO14**  
    - Cátodo → **GND**

- **Simulação do Ar Condicionado:**  
  - Utilize um LED (ou relé) para representar seu status.  
  - Conecte o ânodo (via resistor 220 Ω) → **GPIO32**  
  - Conecte o cátodo → **GND**

## Configuração do Ambiente e Servidor Local

### Ambiente de Desenvolvimento do ESP32

1. Instale a [IDE do Arduino](https://www.arduino.cc/en/software).  
2. Instale as bibliotecas necessárias:
   - **Adafruit AMG88xx** (via Sketch > Incluir Biblioteca > Gerenciar Bibliotecas...)
   - **ArduinoJson** (utilizada para fazer parse dos JSONs)

### Configuração do Servidor Local

Utilizaremos **Flask** para criar um servidor web simples que:
- Recebe os status do ESP32 pelo endpoint `/update`.
- Permite que o usuário envie comandos manuais via o endpoint `/control`.

#### Arquivos do Servidor

1. **app.py**

   ```python
   from flask import Flask, request, jsonify, render_template
   app = Flask(__name__)

   # Armazena os status do sistema enviados pelo ESP32
   status_data = {
       "iluminacao": "desligado",
       "ar_condicionado": "desligado",
       "temperatura_media": 0.0
   }

   # Variáveis de controle manual (inicialmente nulas)
   control_data = {
       "iluminacao": "null",
       "ar_condicionado": "null"
   }

   @app.route('/update', methods=['POST'])
   def update_status():
       global status_data
       data = request.get_json()
       if data:
           status_data = data
           return jsonify({"status": "sucesso"}), 200
       return jsonify({"status": "erro", "mensagem": "dados inválidos"}), 400

   @app.route('/control', methods=['GET', 'POST'])
   def control():
       global control_data
       if request.method == 'POST':
           data = request.get_json()
           if data:
               if "iluminacao" in data:
                   control_data["iluminacao"] = data["iluminacao"]
               if "ar_condicionado" in data:
                   control_data["ar_condicionado"] = data["ar_condicionado"]
               return jsonify({"status": "sucesso"}), 200
           return jsonify({"status": "erro", "mensagem": "dados inválidos"}), 400
       else:
           return jsonify(control_data)

   @app.route('/')
   def index():
       return render_template('index.html', status=status_data, control=control_data)

   if __name__ == '__main__':
       app.run(host='0.0.0.0', port=5000)```


2. **templates/index.html**

```html
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <title>Status e Controle do Sistema</title>
    <link rel="stylesheet" href="/static/style.css">
    <script>
        async function enviarComando() {
            const iluminacao = document.querySelector('input[name="iluminacao"]:checked').value;
            const arCondicionado = document.querySelector('input[name="ar_condicionado"]:checked').value;
            
            const response = await fetch('/control', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ iluminacao: iluminacao, ar_condicionado: arCondicionado })
            });
            const result = await response.json();
            alert("Comando enviado: " + JSON.stringify(result));
        }
    </script>
</head>
<body>
    <h1>Status do Sistema</h1>
    <ul>
        <li>Iluminação da Sala: {{ status.iluminacao }}</li>
        <li>Ar Condicionado: {{ status.ar_condicionado }}</li>
        <li>Temperatura Média: {{ status.temperatura_media }} °C</li>
    </ul>
    
    <h2>Controle Manual</h2>
    <form onsubmit="event.preventDefault(); enviarComando();">
        <fieldset>
            <legend>Iluminação</legend>
            <input type="radio" id="luzLigado" name="iluminacao" value="ligado">
            <label for="luzLigado">Ligado</label>
            <input type="radio" id="luzDesligado" name="iluminacao" value="desligado" checked>
            <label for="luzDesligado">Desligado</label>
        </fieldset>
        <fieldset>
            <legend>Ar Condicionado</legend>
            <input type="radio" id="acLigado" name="ar_condicionado" value="ligado">
            <label for="acLigado">Ligado</label>
            <input type="radio" id="acDesligado" name="ar_condicionado" value="desligado" checked>
            <label for="acDesligado">Desligado</label>
        </fieldset>
        <button type="submit">Enviar Comando</button>
    </form>
    
    <p>Atualize a página para ver os status atualizados.</p>
</body>
</html>
```

3. **static/style.css**
```css
body {
    font-family: Arial, sans-serif;
    margin: 20px;
    background: #f4f4f4;
}
h1, h2 {
    color: #333;
}
ul {
    list-style-type: none;
    padding: 0;
}
li {
    background: #fff;
    padding: 10px;
    margin-bottom: 5px;
    border: 1px solid #ddd;
}
fieldset {
    border: 1px solid #ccc;
    padding: 10px;
    margin-bottom: 10px;
    background: #fff;
}
button {
    padding: 10px 15px;
    background: #007BFF;
    color: #fff;
    border: none;
    cursor: pointer;
}
button:hover {
    background: #0056b3;
}

```

4. **Dockerfile para empacotar a aplicação Flask**
```dockerfile
FROM python:3.9-slim

WORKDIR /app
COPY app.py /app/app.py
COPY templates /app/templates
COPY static /app/static

RUN pip install flask

EXPOSE 5000

CMD ["python", "app.py"]

```

5. **Intruções para executar dentro do terminal do Docker**
``` bash
docker build -t flask-control .
docker run -d -p 5000:5000 flask-control

```
A aplicação ficará disponível em http://localhost:5000.

## Código do projeto

 - O arquivo main.ino integra a lógica de leitura do sensor, controle automático, verificação de comandos manuais e comunicação com o servidor.

 - A cada iteração, o ESP32 realiza um GET no endpoint /control para verificar se há comandos manuais. Se houver, esses comandos sobrescrevem a lógica automática.

## FrontEnd e Controle Manual

- O frontend exibe o status atual dos dispositivos e disponibiliza botões (através de formulários) para que o usuário possa enviar comandos manuais.

- Ao submeter o formulário, um script JavaScript envia os comandos via POST para o endpoint `/control`.

- O ESP32 utiliza estes comandos (caso não sejam "null") para controlar os LEDs e simular o ar condicionado.

## Ferramentas Explicadas

### Docker

- Ferramenta que cria ambientes isolados (contêineres). Aqui, usamos para empacotar e rodar a aplicação Flask de forma simples e portátil.

### Flask

- Micro-framework em Python para desenvolver aplicações web. Usado neste projeto para criar os endpoints de atualização e controle, e para renderizar a interface frontend.

### ArduinoJson:

- Biblioteca para Arduino que facilita a leitura, escrita e manipulação de dados no formato JSON. No projeto, ela é usada para interpretar os comandos manuais recebidos do servidor.

## Testes e Depuração

1. **Hardware:**  
   Verifique todas as conexões dos sensores e LEDs.

2. **Rede:**  
esteja conectado ao Wi-Fi e que os endereços dos endpoints estejam corretos.

3. **Monitor Serial:**  
   Utilize o Serial Monitor para visualizar os valores de temperatura e confirmar o acionamento dos LEDs conforme os quadrantes.

4. **Ajuste de Parâmetros:**  
   Se necessário, ajuste o limite de temperatura (atualmente definido como 36°C) ou refine a lógica de detecção no código.

5. **Servidor e Frontend**
   Inicie o contêiner Docker e acesse http://localhost:5000 para testar a interface e enviar comandos manuais.

## Considerações Finais

- **Escalabilidade:**  
  O sistema integrado pode ser ampliado para controlar dispositivos reais (através de relés, por exemplo) e integrado com plataformas IoT mais robustas.

- **Calibração:**  
  Realize testes em ambiente real e ajuste os thresholds de temperatura conforme necessário..

- **Integração Iot:**  
  A utilização de endpoints HTTP possibilita a comunicação bidirecional, permitindo futuras integrações com outras plataformas.

## Custos Estimados

| Despesa                        | Faixa de valor (R$)     |
|--------------------------------|-------------------------|
| Componentes físicos do projeto | R$ 210 – R$ 365         |
| Energia com PC ligado (100 W)  | ~R$ 50/mês              |
| Raspberry Pi (5 W) – energia   | ~R$ 2/mês (após compra) |
| Hospedagem Flask na nuvem      | Gratuito (planos free)  |

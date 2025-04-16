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
    app.run(host='0.0.0.0', port=5000)
from flask import Flask, request, send_from_directory, jsonify
import sqlite3
import datetime
import ssl
import os

app = Flask(__name__)

DB_NAME = "iot_data.db"
UPLOAD_FOLDER = '.'

def init_db():
    conn = sqlite3.connect(DB_NAME)
    c = conn.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS temperature 
                 (id INTEGER PRIMARY KEY AUTOINCREMENT, 
                  valeur REAL, 
                  date_enreg TIMESTAMP DEFAULT CURRENT_TIMESTAMP)''')
    c.execute('''CREATE TABLE IF NOT EXISTS humidity 
                 (id INTEGER PRIMARY KEY AUTOINCREMENT, 
                  valeur REAL, 
                  date_enreg TIMESTAMP DEFAULT CURRENT_TIMESTAMP)''')
    conn.commit()
    conn.close()
    print("Base de données initialisée.")

@app.route('/api/data', methods=['POST'])
def receive_data():
    try:
        data = request.json
        conn = sqlite3.connect(DB_NAME)
        c = conn.cursor()
        
        if 'temperature' in data:
            val = data['temperature']
            c.execute("INSERT INTO temperature (valeur) VALUES (?)", (val,))
            print(f"🌡️  Temperature reçue: {val}°C")
            
        elif 'humidity' in data:
            val = data['humidity']
            c.execute("INSERT INTO humidity (valeur) VALUES (?)", (val,))
            print(f"💧 Humidité reçue: {val}%")
            
        conn.commit()
        conn.close()
        return jsonify({"status": "success"}), 200
    except Exception as e:
        print("Erreur:", e)
        return jsonify({"status": "error"}), 500

@app.route('/version.txt')
def get_version():
    return send_from_directory(UPLOAD_FOLDER, 'version.txt')

@app.route('/firmware.bin')
def get_firmware():
    return send_from_directory(UPLOAD_FOLDER, 'firmware.bin')

if __name__ == '__main__':
    init_db()
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain('cert.pem', 'key.pem')
    
    print("🚀 Serveur IoT HTTPS démarré sur le port 8000...")
    app.run(host='0.0.0.0', port=8000, ssl_context=context)
import sqlite3
import os

# Le nom défini dans votre code serveur Flask
db_file = 'iot_data.db' 

def lire_donnees_iot():
    if not os.path.exists(db_file):
        # Si le fichier .db n'existe pas, on essaie sans l'extension au cas où
        if os.path.exists('iot_data'):
            print("⚠️ Attention : Le fichier 'iot_data.db' est introuvable, mais 'iot_data' existe.")
            print("   Utilisation de 'iot_data' à la place.\n")
            nom_fichier = 'iot_data'
        else:
            print(f"❌ Erreur : Le fichier de base de données '{db_file}' est introuvable.")
            return
    else:
        nom_fichier = db_file

    try:
        conn = sqlite3.connect(nom_fichier)
        cursor = conn.cursor()
        print(f"--- Lecture de la base : {nom_fichier} ---\n")

        # --- 1. Lire la table TEMPERATURE ---
        print("🌡️  TABLE : TEMPERATURE")
        cursor.execute("SELECT id, valeur, date_enreg FROM temperature ORDER BY id DESC")
        lignes_temp = cursor.fetchall()

        if not lignes_temp:
            print("   (Aucune donnée)")
        else:
            print(f"   {'ID':<5} | {'Valeur':<10} | {'Date'}")
            print("   " + "-"*40)
            for row in lignes_temp:
                # row[0]=id, row[1]=valeur, row[2]=date
                print(f"   {row[0]:<5} | {str(row[1]) + '°C':<10} | {row[2]}")
        
        print("\n" + "="*40 + "\n")

        # --- 2. Lire la table HUMIDITY ---
        print("💧 TABLE : HUMIDITY")
        cursor.execute("SELECT id, valeur, date_enreg FROM humidity ORDER BY id DESC")
        lignes_hum = cursor.fetchall()

        if not lignes_hum:
            print("   (Aucune donnée)")
        else:
            print(f"   {'ID':<5} | {'Valeur':<10} | {'Date'}")
            print("   " + "-"*40)
            for row in lignes_hum:
                print(f"   {row[0]:<5} | {str(row[1]) + '%':<10} | {row[2]}")

    except sqlite3.Error as e:
        print(f"❌ Erreur SQLite : {e}")
    finally:
        if conn:
            conn.close()

if __name__ == "__main__":
    lire_donnees_iot()
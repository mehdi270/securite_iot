#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266httpUpdate.h>
#include "DHT.h"

#define FW_VERSION "3.0"

#define DHTTYPE DHT11
const int DHTPin = 5; // Pin D1 (GPIO 5)

const char* ssid = "myNetwork";
const char* password = "karkadan2";

const char* host = "172.20.146.179";
const int port = 8000;
const char* fingerprint = "F9:FE:F7:70:F9:18:1B:06:3F:D0:3D:80:26:9E:C4:7F:E2:DE:DF:8B";

IPAddress local_IP(172, 20, 146, 175);
IPAddress gateway(172, 20, 146, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

// Temps de sommeil (60 secondes * 1 million de microsecondes)
const uint64_t SLEEP_TIME = 60e6; 

DHT dht(DHTPin, DHTTYPE);

void checkAndPerformUpdate(WiFiClientSecure& client);

void setup() {
  // 1. Initialisation Hardware
  WiFi.mode(WIFI_OFF); // On coupe le wifi pour configurer
  Serial.begin(115200);
  Serial.println("\n\n--- DEMARRAGE V3.0 (Temp + Humidite) ---");
  dht.begin();

  // 2. Connexion WiFi Rapide
  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet, dns);
  WiFi.begin(ssid, password);

  Serial.print("Connexion WiFi");
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    timeout++;
    if (timeout > 100) { // 10 secondes max
      Serial.println("\nEchec WiFi. Dodo.");
      ESP.deepSleep(SLEEP_TIME);
    }
  }
  Serial.println("\nConnecté !");

  // 3. LECTURE DES CAPTEURS
  // On attend 2 secondes pour que le DHT soit stable au réveil
  delay(2000); 
  
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Gestion des erreurs de lecture
  if (isnan(t)) { Serial.println("Erreur lecture Temp !"); t = 0.0; }
  if (isnan(h)) { Serial.println("Erreur lecture Hum !"); h = 0.0; }

  Serial.printf("Données lues -> Temp: %.1f C, Hum: %.1f %%\n", t, h);

  // 4. ENVOI DES DONNEES (HTTPS POST)
  WiFiClientSecure client;
  client.setFingerprint(fingerprint);
  client.setTimeout(5000); // Timeout court pour ne pas bloquer

  HTTPClient http;
  String urlData = String("https://") + host + ":" + port + "/api/data";

  Serial.println("Envoi des données...");
  
  if (http.begin(client, urlData)) {
    http.addHeader("Content-Type", "application/json");
    
    // Création du JSON contenant les DEUX valeurs
    String json = "{\"temperature\": " + String(t) + ", \"humidity\": " + String(h) + "}";
    
    int httpCode = http.POST(json);
    
    if (httpCode > 0) {
      Serial.printf("Succès envoi (Code %d)\n", httpCode);
    } else {
      Serial.printf("Echec envoi (Erreur: %s)\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("Impossible de connecter au serveur pour l'API");
  }

  // 5. VERIFICATION OTA (Mise à jour)
  // Même si on est en V2, on vérifie si une V3 existe
  checkAndPerformUpdate(client);

  // 6. MISE EN VEILLE
  Serial.println("Fin des tâches. Mise en veille (Deep Sleep)...");
  Serial.flush();
  ESP.deepSleep(SLEEP_TIME);
}

void loop() {
  // Vide : on ne revient jamais ici après le Deep Sleep
}

// Fonction de vérification de mise à jour
void checkAndPerformUpdate(WiFiClientSecure& client) {
  HTTPClient http;
  String versionUrl = String("https://") + host + ":" + port + "/version.txt";
  
  Serial.println("Vérification MAJ...");
  
  if (http.begin(client, versionUrl)) {
    int httpCode = http.GET();
    if (httpCode == 200) {
      String newVersion = http.getString();
      newVersion.trim();
      
      Serial.printf("Version actuelle: %s | Serveur: %s\n", FW_VERSION, newVersion.c_str());

      if (newVersion.toFloat() > String(FW_VERSION).toFloat()) {
        Serial.println(">>> Nouvelle version disponible ! Lancement MAJ...");
        http.end();
        
        String firmwareUrl = String("https://") + host + ":" + port + "/firmware.bin";
        t_httpUpdate_return ret = ESPhttpUpdate.update(client, firmwareUrl);
        
        switch(ret) {
          case HTTP_UPDATE_FAILED: Serial.println("Echec MAJ"); break;
          case HTTP_UPDATE_OK: Serial.println("MAJ OK ! Reboot..."); break;
        }
      } else {
        Serial.println("Appareil à jour.");
      }
    }
    http.end();
  } else {
    Serial.println("Erreur connexion version.txt");
  }
}
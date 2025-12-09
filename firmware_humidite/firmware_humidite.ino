#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266httpUpdate.h>
#include "DHT.h"

// --- CONFIG VERSION 1 ---
#define FW_VERSION "1.0"
#define DHTTYPE DHT11
const int DHTPin = 5; // D1 sur NodeMCU

// --- RESEAU & SERVEUR ---
const char* ssid = "myNetwork";
const char* password = "karkadan2";
const char* host = "172.20.146.179"; // TON IP
const int port = 8000;
const char* fingerprint = "F9:FE:F7:70:F9:18:1B:06:3F:D0:3D:80:26:9E:C4:7F:E2:DE:DF:8B"; // TON FINGERPRINT

// IP STATIQUE (Pour batterie)
IPAddress local_IP(172, 20, 146, 175);
IPAddress gateway(172, 20, 146, 1);
IPAddress subnet(255, 255, 255, 0);

DHT dht(DHTPin, DHTTYPE);

void setup() {
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);
  dht.begin();
  
  // Connexion
  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 100) {
    delay(100); timeout++;
  }
  if(WiFi.status() != WL_CONNECTED) ESP.deepSleep(10e6); // Retry later

  Serial.println("\n--- V1.0 CONNECTE ---");

  // 1. LIRE HUMIDITE
  float h = dht.readHumidity(); // <--- Changement ici
  if (isnan(h)) h = 0.0;

  // 2. ENVOYER AU SERVEUR
  WiFiClientSecure client;
  client.setFingerprint(fingerprint);
  HTTPClient http;
  
  String urlData = String("https://") + host + ":" + port + "/api/data";
  
  Serial.print("Envoi Humidité: "); Serial.println(h);
  
  http.begin(client, urlData);
  http.addHeader("Content-Type", "application/json");
  // JSON modifiée pour envoyer l'humidité
  String json = "{\"humidity\": " + String(h) + "}"; // <--- Clé JSON différente
  
  int httpResponseCode = http.POST(json);

  // 3. CHECK MISE A JOUR (OTA)
  checkForUpdate(client);

  // 4. DODO (1 minute)
  Serial.println("Dodo...");
  ESP.deepSleep(60e6); 
}

void loop() {}

void checkForUpdate(WiFiClientSecure& client) {
  HTTPClient http;
  String urlVersion = String("https://") + host + ":" + port + "/version.txt";
  
  if (http.begin(client, urlVersion)) {
    if (http.GET() == 200) {
      String newVer = http.getString();
      newVer.trim();
      if (newVer.toFloat() > String(FW_VERSION).toFloat()) {
        Serial.println("MAJ Trouvée !");
        t_httpUpdate_return ret = ESPhttpUpdate.update(client, String("https://") + host + ":" + port + "/firmware.bin");
      }
    }
    http.end();
  }
}
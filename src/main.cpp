#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WiFi.h>      
#include <WebServer.h> 
#include "page_html.h" // Interface graphique brute

// =================================================================
// ⚙️ CONFIGURATION DIRECTE (Évite les erreurs de fichiers manquants)
// =================================================================
const int ETH_RST_PIN  = 9;   
const int ETH_CS_PIN   = 14;  
const int ETH_SCLK_PIN = 13;  
const int ETH_MISO_PIN = 12;  
const int ETH_MOSI_PIN = 11;  

const IPAddress IP_ESP32_ETH(192, 168, 1, 50);
const IPAddress IP_ADAM(192, 168, 1, 1);
const IPAddress PASSERELLE_ETH(192, 168, 1, 1);
const IPAddress MASQUE_ETH(255, 255, 255, 0);

const String NOM_BARRAGE = "Barrage_AYLMER";
const String MOT_DE_PASSE_WIFI = "Aylmer2026";

const String NOMS_CANAUX[8] = {
    "Capteur Temperature Cuve",
    "Exemple:Capteur Pression Vapeur",
    "Exemple:Debitmetre Entree",
    "Exemple:Niveau Cuve Eau",
    "Canal Non Utilise",  
    "Canal Non Utilise",  
    "Exemple:Vitesse Turbine",
    "Exemple:Mesure Secours"
};

// =================================================================
// 📊 VARIABLES GLOBALES DU SYSTÈME
// =================================================================
uint16_t valeursBrutesADAM[8] = {0};
float valeursTensions[8] = {0.0};
int estVide[8] = {0};
bool adamEnLigne = false;

byte mac[6];
EthernetClient modbusClient;
WebServer server(80); 

// =================================================================
// 🌐 GESTION DU SERVEUR WEB (WIFI)
// =================================================================
void handleRoot() {
    String html = String(INDEX_HTML);
    for(int i = 0; i < 8; i++) {
        html.replace("%NOM" + String(i) + "%", NOMS_CANAUX[i]);
    }
    server.send(200, "text/html", html);
}

void handleData() {
    String json = "{";
    json += "\"adam_en_ligne\":" + String(adamEnLigne ? "true" : "false") + ",";
    json += "\"nom_barrage\":\"" + NOM_BARRAGE + "\",";
    json += "\"brutes\":[" + String(valeursBrutesADAM[0]) + "," + String(valeursBrutesADAM[1]) + "," + String(valeursBrutesADAM[2]) + "," + String(valeursBrutesADAM[3]) + "," + String(valeursBrutesADAM[4]) + "," + String(valeursBrutesADAM[5]) + "," + String(valeursBrutesADAM[6]) + "," + String(valeursBrutesADAM[7]) + "],";
    json += "\"tensions\":[" + String(valeursTensions[0],2) + "," + String(valeursTensions[1],2) + "," + String(valeursTensions[2],2) + "," + String(valeursTensions[3],2) + "," + String(valeursTensions[4],2) + "," + String(valeursTensions[5],2) + "," + String(valeursTensions[6],2) + "," + String(valeursTensions[7],2) + "],";
    json += "\"est_vide\":[" + String(estVide[0]) + "," + String(estVide[1]) + "," + String(estVide[2]) + "," + String(estVide[3]) + "," + String(estVide[4]) + "," + String(estVide[5]) + "," + String(estVide[6]) + "," + String(estVide[7]) + "]";
    json += "}";
    server.send(200, "application/json", json);
}

// =================================================================
// 📥 LIAISON MODBUS TCP (ETHERNET CORRIGÉE POUR PLAGE +/-10V)
// =================================================================
void requeteLectureADAM() {
    if (!modbusClient.connected()) {
        if (!modbusClient.connect(IP_ADAM, 502)) {
            adamEnLigne = false;
            for (int i = 0; i < 8; i++) {
                valeursBrutesADAM[i] = 0; valeursTensions[i] = 0.0; estVide[i] = 0;
            }
            return;
        }
    }

    uint8_t requeteLireAI[12] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01, 0x04, 0x00, 0x00, 0x00, 0x08};
    modbusClient.write(requeteLireAI, 12);

    unsigned long timeout = millis();
    while (modbusClient.available() < 25) {
        if (millis() - timeout > 300) {
            adamEnLigne = false; modbusClient.stop(); return;
        }
    }

    uint8_t reponse[25];
    modbusClient.read(reponse, 25);
    adamEnLigne = true;

    int indexOctet = 9;
    for (int i = 0; i < 8; i++) {
        uint8_t highByte = reponse[indexOctet];
        uint8_t lowByte  = reponse[indexOctet + 1];
        valeursBrutesADAM[i] = (highByte << 8) | lowByte;
        
        // 1. VRAIE formule pour l'ADAM configuré d'usine en +/- 10V
        valeursTensions[i] = ((float)valeursBrutesADAM[i] / 65535.0) * 20.0 - 10.0;
        
        // Seuil d'atténuation : force à 0.0V si la tension oscille juste au niveau du point mort
        if (abs(valeursTensions[i]) < 0.06) {
            valeursTensions[i] = 0.0;
        }

        // 2. Filtrage intelligent du bruit électromagnétique (canal flottant en l'air)
        if (valeursBrutesADAM[i] >= 32740 && valeursBrutesADAM[i] <= 32785) {
            estVide[i] = 1;
            valeursTensions[i] = 0.0;
        } else {
            estVide[i] = 0;
        }
        
        indexOctet += 2;
    }
}

// =================================================================
// 🚀 DÉMARRAGE DU MATÉRIEL
// =================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    // 1. Initialisation du Wi-Fi Point d'Accès de Maintenance (192.168.4.1)
    String ssId = NOM_BARRAGE; ssId.replace(" ", "_");
    WiFi.softAP(ssId.c_str(), MOT_DE_PASSE_WIFI.c_str());
    Serial.println("\n[Wi-Fi] Point d'acces active : " + ssId);
    Serial.print("[Wi-Fi] Dashboard disponible sur : http://");
    Serial.println(WiFi.softAPIP()); 

    // 2. Initialisation Physique de l'Ethernet RJ45
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    pinMode(ETH_RST_PIN, OUTPUT);
    digitalWrite(ETH_RST_PIN, LOW); delay(50);
    digitalWrite(ETH_RST_PIN, HIGH); delay(300);

    SPI.begin(ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);
    Ethernet.init(ETH_CS_PIN);
    Ethernet.begin(mac, IP_ESP32_ETH, IP_ADAM, PASSERELLE_ETH, MASQUE_ETH);
    Serial.println("[Ethernet] Liaison RJ45 configuree vers l'ADAM.");

    // 3. Activation des routes du Serveur Web d'infrastructure
    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
}

void loop() {
    server.handleClient(); // Écoute le réseau Wi-Fi de ton ordinateur/téléphone

    static unsigned long precedentMillisModbus = 0;
    if (millis() - precedentMillisModbus > 1000) {
        precedentMillisModbus = millis();
        requeteLectureADAM(); // Sonde l'ADAM par le câble réseau
    }
}
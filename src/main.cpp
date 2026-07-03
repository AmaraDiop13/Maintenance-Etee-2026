#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

// =================================================================
// ⚙️ MAPPING MATÉRIEL EXACT DE LA CARTE WAVESHARE ESP32-S3-ETH
// =================================================================
const int ETH_RST_PIN  = 9;   // Reset du W5500
const int ETH_CS_PIN   = 14;  // Chip Select (SCSn)
const int ETH_SCLK_PIN = 13;  // Horloge SPI
const int ETH_MISO_PIN = 12;  // Master Input Slave Output
const int ETH_MOSI_PIN = 11;  // Master Output Slave Input

// Adresse MAC unique fictive pour ta carte
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// =================================================================
// 🚀 INITIALISATION
// =================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==================================================");
    Serial.println("INITIALISATION DU PORT RJ45 WAVESHARE (W5500)");
    Serial.println("==================================================");

    // Étape 1 : Réinitialiser physiquement la puce Ethernet soudée sur la carte
    pinMode(ETH_RST_PIN, OUTPUT);
    digitalWrite(ETH_RST_PIN, LOW);
    delay(50);
    digitalWrite(ETH_RST_PIN, HIGH); // Réveille le contrôleur W5500
    delay(300);

    // Étape 2 : Forcer le bus SPI sur les broches spécifiques de Waveshare
    SPI.begin(ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);

    // Étape 3 : Assigner la broche Chip Select à la bibliothèque Ethernet
    Ethernet.init(ETH_CS_PIN);

    // Étape 4 : Demander une adresse IP automatique (DHCP)
    Serial.println("Connexion au réseau... Attente du DHCP...");
    
    if (Ethernet.begin(mac) == 0) {
        Serial.println("❌ Échec DHCP : Aucun routeur n'a donné d'IP.");
        Serial.println("👉 Bascule automatique sur IP Fixe pour test direct PC ↔ ESP32");
        
        // IP de secours si tu branches l'ESP32 DIRECTEMENT à ton ordi avec un câble
        IPAddress ipManuel(10, 0, 0, 50); 
        IPAddress dnsManuel(10, 0, 0, 1);
        IPAddress gatewayManuel(10, 0, 0, 1);
        IPAddress subnetManuel(255, 255, 255, 0);
        Ethernet.begin(mac, ipManuel, dnsManuel, gatewayManuel, subnetManuel);
    }

    // Étape 5 : Affichage du succès et de l'IP obtenue
    Serial.println("==================================================");
    Serial.println("--- ✅ PORT ETHERNET WAVESHARE CONFIGURÉ ! ---");
    Serial.print("👉 TAPE DANS TON INVITE DE COMMANDE : ping ");
    Serial.println(Ethernet.localIP());
    Serial.println("==================================================");
}

// =================================================================
// 🔄 BOUCLE PRINCIPALE
// =================================================================
void loop() {
    auto statutLien = Ethernet.linkStatus();
    
    static unsigned long precedentMillis = 0;
    if (millis() - precedentMillis > 4000) {
        precedentMillis = millis();
        
        if (statutLien == LinkON) {
            Serial.print("[Statut] Réseau branché. Prêt à recevoir un Ping sur l'IP : ");
            Serial.println(Ethernet.localIP());
        } else if (statutLien == LinkOFF) {
            Serial.println("🚨 Alerte : Câble RJ45 débranché de la carte Waveshare !");
        }
    }
    delay(10);
}
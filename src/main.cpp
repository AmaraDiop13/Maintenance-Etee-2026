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

// Tableau dynamique qui va recevoir la VRAIE adresse MAC physique de l'ESP32
byte mac[6]; 

// =================================================================
// 🚀 INITIALISATION
// =================================================================
void setup() {
    Serial.begin(115200);
    delay(2000); // Laisse le temps au moniteur série de s'ouvrir proprement
    
    Serial.println("\n==================================================");
    Serial.println("🌐 PORT ETHERNET WAVESHARE (CLASSE C - RFC 1918)");
    Serial.println("==================================================");

    // 🔒 Étape 1 : Récupérer de force la VRAIE adresse MAC gravée en usine (eFuse)
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    
    Serial.print("🔍 Vraie adresse MAC détectée sur le silicium : ");
    for (int i = 0; i < 6; i++) {
        if (mac[i] < 0x10) Serial.print("0"); // Ajoute un zéro de formatage si nécessaire
        Serial.print(mac[i], HEX);
        if (i < 5) Serial.print(":");
    }
    Serial.println();

    // Étape 2 : Réinitialiser physiquement la puce Ethernet soudée sur la carte
    pinMode(ETH_RST_PIN, OUTPUT);
    digitalWrite(ETH_RST_PIN, LOW);
    delay(50);
    digitalWrite(ETH_RST_PIN, HIGH); // Réveille le contrôleur W5500
    delay(300);

    // Étape 3 : Forcer le bus SPI sur les broches spécifiques de Waveshare
    SPI.begin(ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);

    // Étape 4 : Assigner la broche Chip Select à la bibliothèque Ethernet
    Ethernet.init(ETH_CS_PIN);

    // Étape 5 : Demander une adresse IP automatique (DHCP)
    Serial.println("Connexion au réseau... Attente du DHCP...");
    
    if (Ethernet.begin(mac) == 0) {
        Serial.println("⚠️ Aucun serveur DHCP détecté (Liaison par câble direct PC/ADAM).");
        Serial.println("🚀 Application de l'IP Fixe réglementaire de Classe C...");
        
        // Configuration fixe pour ton infrastructure locale
        IPAddress ipManuel(192, 168, 1, 50); 
        IPAddress dnsManuel(192, 168, 1, 1);
        IPAddress gatewayManuel(192, 168, 1, 1);
        IPAddress subnetManuel(255, 255, 255, 0);
        
        // On initialise la puce avec la vraie MAC et l'IP de Classe C
        Ethernet.begin(mac, ipManuel, dnsManuel, gatewayManuel, subnetManuel);
    }

    // Étape 6 : Affichage du succès final
    Serial.println("==================================================");
    Serial.println("--- ✅ INFRASTRUCTURE FILAIRE ACTIVED ! ---");
    Serial.print("👉 LANCE LA COMMANDE DANS TON PC : ping ");
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
            Serial.print("[Statut OK] Prêt à recevoir un Ping sur l'IP : ");
            Serial.println(Ethernet.localIP());
        } else if (statutLien == LinkOFF) {
            Serial.println("🚨 Alerte : Câble RJ45 débranché de la carte Waveshare !");
        }
    }
    delay(10);
}
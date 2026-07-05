#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

// Broches Matérielles (Carte d'extension W5500 / Waveshare)
const int ETH_RST_PIN  = 9;   
const int ETH_CS_PIN   = 14;  
const int ETH_SCLK_PIN = 13;  
const int ETH_MISO_PIN = 12;  
const int ETH_MOSI_PIN = 11;  

// Adresses IP Statiques de ton Réseau Laboratoire
const IPAddress IP_ESP32_ETH(192, 168, 1, 50);
const IPAddress IP_ADAM(192, 168, 1, 1);
const IPAddress PASSERELLE_ETH(192, 168, 1, 1);
const IPAddress MASQUE_ETH(255, 255, 255, 0);

// Point d'accès Wi-Fi de Maintenance (Généré par l'ESP32)
const String NOM_BARRAGE = "Barrage_AYLMER";
const String MOT_DE_PASSE_WIFI = "Aylmer2026";

// Étiquettes textuelles des Canaux de Supervision
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

#endif
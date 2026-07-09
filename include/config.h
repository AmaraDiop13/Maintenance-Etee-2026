#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

// Broches Matérielles (Carte d'extension W5500 et Lecteur SD)
const int ETH_RST_PIN  = 9;   
const int ETH_CS_PIN   = 14;  
const int ETH_SCLK_PIN = 13;  
const int ETH_MISO_PIN = 12;  
const int ETH_MOSI_PIN = 11;  
const int SD_CS_PIN    = 4;     // Broche CS dédiée pour la carte MicroSD

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
    "Courant Turbine (Dwyer CCT60)", // Canal 0 étalonné en Ampères
    "Capteur Pression Vapeur",
    "Débitmètre Entrée",
    "Niveau Cuve Eau",
    "Canal Non Utilisé",  
    "Canal Non Utilisé",  
    "Vitesse Turbine",
    "Mesure Sécurité Secours"
};

#endif
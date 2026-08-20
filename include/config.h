#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Preferences.h> // Ajout de la bibliothèque de mémoire flash

// On déclare l'outil de sauvegarde pour qu'il soit accessible partout
extern Preferences preferences;

// Broches Matérielles (Carte d'extension W5500 et Lecteur SD)
const int ETH_RST_PIN  = 9;   
const int ETH_CS_PIN   = 14;  
const int ETH_SCLK_PIN = 13;  
const int ETH_MISO_PIN = 12;  
const int ETH_MOSI_PIN = 11;  
const int SD_CS_PIN    = 4;     

// Adresses IP Statiques de ton Réseau Laboratoire
const IPAddress IP_ESP32_ETH(192, 168, 1, 50);
const IPAddress IP_ADAM(192, 168, 1, 1);
const IPAddress PASSERELLE_ETH(192, 168, 1, 1);
const IPAddress MASQUE_ETH(255, 255, 255, 0);

// --- VARIABLES DYNAMIQUES (Modifiables via le Portail Admin) ---
// On enlève "const" et on met "extern" pour dire qu'elles vont changer
extern String NOM_BARRAGE;
extern String MOT_DE_PASSE_WIFI;
extern String NOMS_CANAUX[8];

#endif
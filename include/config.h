#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Preferences.h>

extern Preferences preferences;

const int ETH_RST_PIN  = 9;   
const int ETH_CS_PIN   = 14;  
const int ETH_SCLK_PIN = 13;  
const int ETH_MISO_PIN = 12;  
const int ETH_MOSI_PIN = 11;  
const int SD_CS_PIN    = 4;     

const IPAddress IP_ESP32_ETH(192, 168, 1, 50);
const IPAddress IP_ADAM(192, 168, 1, 1);
const IPAddress PASSERELLE_ETH(192, 168, 1, 1);
const IPAddress MASQUE_ETH(255, 255, 255, 0);

// Variables dynamiques modifiables via la page Admin
extern String NOM_BARRAGE;
extern String MOT_DE_PASSE_WIFI;
extern String NOMS_CANAUX[8];

// NOUVEAUX PARAMÈTRES PAR CANAL : y = ax + b, Min et Max
extern float CANAL_A[8];
extern float CANAL_B[8];
extern float CANAL_MIN[8];
extern float CANAL_MAX[8];

#endif
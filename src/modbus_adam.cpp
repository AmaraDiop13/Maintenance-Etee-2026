#include "modbus_adam.h"
#include "config.h"
#include <Ethernet.h>

uint16_t valeursBrutesADAM[8] = {0};
float valeursConverties[8] = {0.0};
int estVide[8] = {0};
bool adamEnLigne = false;

// Mise à jour de l'affichage de configuration pour refléter le 4-20 mA
uint16_t codesConfigurationADAM[8] = {0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180};
String unitesCanaux[8] = {"A", "mA", "mA", "mA", "mA", "mA", "mA", "mA"}; // Canal 0 en Ampères, le reste en mA
String typesEntreesDetectes[8] = {"4~20 mA", "4~20 mA", "4~20 mA", "4~20 mA", "4~20 mA", "4~20 mA", "4~20 mA", "4~20 mA"};

EthernetClient modbusClient;

void scannerConfigurationADAM() {
    Serial.println("[MODBUS] Configuration figée d'usine sur la plage industrielle (4~20 mA).");
    for (int i = 0; i < 8; i++) {
        codesConfigurationADAM[i] = 0x0180; 
        typesEntreesDetectes[i] = "4~20 mA";
        unitesCanaux[i] = (i == 0) ? "A" : "mA"; 
    }
}

void requeteLectureADAM() {
    if (!modbusClient.connected()) {
        if (!modbusClient.connect(IP_ADAM, 502)) {
            adamEnLigne = false;
            for (int i = 0; i < 8; i++) {
                valeursBrutesADAM[i] = 0; valeursConverties[i] = 0.0; estVide[i] = 1;
            }
            return;
        }
    }

    uint8_t requeteLireAI[12] = {0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x01, 0x04, 0x00, 0x00, 0x00, 0x08};
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
        uint16_t brut = valeursBrutesADAM[i];

        // --- NOUVELLE LOGIQUE 4-20 mA ---
        
        // 1. Calcul du courant brut reçu par l'ADAM en milliampères (mA)
        // L'ADAM lit 0 brut à 4 mA, et 65535 brut à 20 mA.
        float courant_mA = 4.0 + ((float)brut / 65535.0) * 16.0;

        // 2. Le Super Filtre anti-fil coupé / canal vide (Solution B)
        if (courant_mA < 3.8) {
            // Le courant est anormalement bas (< 4mA) = fil débranché
            estVide[i] = 1;
            valeursConverties[i] = 0.0;
        } else {
            estVide[i] = 0;
            
            // 3. Étalonnage selon les capteurs branchés
            if (i == 0) {
                // Canal 0 : Capteur Dwyer (4-20 mA correspond à 0-20 A physiques du barrage)
                float courant_Amperes = ((float)brut / 65535.0) * 20.0;
                valeursConverties[i] = courant_Amperes; 
            } else {
                // Canaux standards restants : on affiche directement les milliampères (mA)
                valeursConverties[i] = courant_mA;
            }
        }
        indexOctet += 2;
    }
}
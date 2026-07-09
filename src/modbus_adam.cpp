#include "modbus_adam.h"
#include "config.h"
#include <Ethernet.h>

uint16_t valeursBrutesADAM[8] = {0};
float valeursConverties[8] = {0.0};
int estVide[8] = {0};
bool adamEnLigne = false;

uint16_t codesConfigurationADAM[8] = {0x0323, 0x0323, 0x0323, 0x0323, 0x0323, 0x0323, 0x0323, 0x0323};
String unitesCanaux[8] = {"A", "V", "V", "V", "V", "V", "V", "V"}; // Canal 0 forcé en A (Ampères)
String typesEntreesDetectes[8] = {"+/- 10 V", "+/- 10 V", "+/- 10 V", "+/- 10 V", "+/- 10 V", "+/- 10 V", "+/- 10 V", "+/- 10 V"};

EthernetClient modbusClient;

void scannerConfigurationADAM() {
    Serial.println("[MODBUS] Configuration figée d'usine sur la plage du labo (+/- 10V).");
    for (int i = 0; i < 8; i++) {
        codesConfigurationADAM[i] = 0x0323; 
        typesEntreesDetectes[i] = "+/- 10 V";
        unitesCanaux[i] = (i == 0) ? "A" : "V"; 
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

        // Équation d'échelle globale de l'ADAM pour du +/- 10V
        float tensionBrute = ((float)brut / 65535.0) * 20.0 - 10.0;

        // Détection de fil flottant/vide
        if (brut >= 32740 && brut <= 32785) {
            estVide[i] = 1;
            valeursConverties[i] = 0.0;
        } else {
            estVide[i] = 0;
            
            // Étalonnage industriel spécifique
            if (i == 0) {
                // Étalonnage Dwyer CCT60 (0-10V -> 0-20A). On ne garde que la partie positive
                if (tensionBrute < 0.05) tensionBrute = 0.0;
                valeursConverties[i] = tensionBrute * 2.0; 
            } else {
                // Canaux standards restants (Volts)
                if (abs(tensionBrute) < 0.06) tensionBrute = 0.0;
                valeursConverties[i] = tensionBrute;
            }
        }
        indexOctet += 2;
    }
}
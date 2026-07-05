#include "modbus_adam.h"
#include "config.h" // Appelle directement ton fichier dans include/
#include <Ethernet.h>

uint16_t valeursBrutesADAM[8] = {0};
float valeursConverties[8] = {0.0};
int estVide[8] = {0};
bool adamEnLigne = false;

// 🛠️ FORÇAGE MANUEL INTÉGRIAL : On fige tous les canaux sur le mode +/-10V de ton labo
uint16_t codesConfigurationADAM[8] = {0x0323, 0x0323, 0x0323, 0x0323, 0x0323, 0x0323, 0x0323, 0x0323};
String unitesCanaux[8] = {"V", "V", "V", "V", "V", "V", "V", "V"};
String typesEntreesDetectes[8] = {"+/- 10 V", "+/- 10 V", "+/- 10 V", "+/- 10 V", "+/- 10 V", "+/- 10 V", "+/- 10 V", "+/- 10 V"};

EthernetClient modbusClient;

// Cette fonction ne fait plus de requête réseau risquée, elle initialise proprement les textes
void scannerConfigurationADAM() {
    Serial.println("[MODBUS Scan] Forçage manuel de la configuration en +/- 10 V.");
    for (int i = 0; i < 8; i++) {
        codesConfigurationADAM[i] = 0x0323; 
        typesEntreesDetectes[i] = "+/- 10 V";
        unitesCanaux[i] = "V";
    }
}

void requeteLectureADAM() {
    if (!modbusClient.connected()) {
        if (!modbusClient.connect(IP_ADAM, 502)) {
            adamEnLigne = false;
            for (int i = 0; i < 8; i++) {
                valeursBrutesADAM[i] = 0; valeursConverties[i] = 0.0; estVide[i] = 0;
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

        // On utilise directement la formule bipolaire +/-10V validée sur l'ADAM
        valeursConverties[i] = ((float)brut / 65535.0) * 20.0 - 10.0;
        
        // Filtrage du canal vide (fil débranché ou flottant)
        if (brut >= 32740 && brut <= 32785) {
            estVide[i] = 1;
            valeursConverties[i] = 0.0;
        } else {
            estVide[i] = 0;
        }

        // Nettoyage des petites oscillations résiduelles autour de 0.0V
        if (!estVide[i] && abs(valeursConverties[i]) < 0.06) {
            valeursConverties[i] = 0.0;
        }

        if (estVide[i]) valeursConverties[i] = 0.0;
        
        indexOctet += 2;
    }
}
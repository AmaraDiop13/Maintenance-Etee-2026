#include <Arduino.h>
#include "reseau_local.h"
#include "modbus_adam.h"

void setup() {
    Serial.begin(115200);
    delay(1500); 
    Serial.println("[BARRAGE AYLMER] Démarrage de la passerelle...");

    initialiserReseaux();
    scannerConfigurationADAM();

    Serial.println("[SYSTEME] Système auto-adaptatif opérationnel !");
}

void loop() {
    server.handleClient(); 

    static unsigned long precedentMillisModbus = 0;
    if (millis() - precedentMillisModbus > 1000) {
        precedentMillisModbus = millis();
        requeteLectureADAM(); 
    }
}
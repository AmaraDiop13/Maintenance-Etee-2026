#include <Arduino.h>
#include "reseau_local.h"
#include "modbus_adam.h"
#include "config.h"

Preferences preferences;

// --- VALEURS D'USINE UNIQUES (Déclaration réelle) ---
String NOM_BARRAGE = "Barrage_AYLMER";
String MOT_DE_PASSE_WIFI = "Aylmer2026";
String ADMIN_USER = "admin";
String ADMIN_PASS = "12345";

String NOMS_CANAUX[8] = {
    "NEANT", "NEANT", "MOTEUR", "L1 ", "L2", "L3", "NEANT", "NEANT"
};

float CANAL_A[8]   = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
float CANAL_B[8]   = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float CANAL_MIN[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float CANAL_MAX[8] = {100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0};

void setup() {
    Serial.begin(115200);
    delay(1500); 

    // Chargement de la mémoire flash NVS
    preferences.begin("param_barrage", false);
    NOM_BARRAGE       = preferences.getString("nom", NOM_BARRAGE);
    MOT_DE_PASSE_WIFI = preferences.getString("mdp", MOT_DE_PASSE_WIFI);
    ADMIN_USER        = preferences.getString("admin_user", ADMIN_USER);
    ADMIN_PASS        = preferences.getString("admin_pass", ADMIN_PASS);
    
    for (int i = 0; i < 8; i++) {
        NOMS_CANAUX[i] = preferences.getString(("ch_n_" + String(i)).c_str(), NOMS_CANAUX[i]);
        CANAL_A[i]     = preferences.getFloat(("ch_a_" + String(i)).c_str(), CANAL_A[i]);
        CANAL_B[i]     = preferences.getFloat(("ch_b_" + String(i)).c_str(), CANAL_B[i]);
        CANAL_MIN[i]   = preferences.getFloat(("ch_min_" + String(i)).c_str(), CANAL_MIN[i]);
        CANAL_MAX[i]   = preferences.getFloat(("ch_max_" + String(i)).c_str(), CANAL_MAX[i]);
    }

    initialiserReseaux();
    scannerConfigurationADAM();
    
    Serial.println("[SYSTEME] Initialisation complete. Pret.");
}

void loop() {
    server.handleClient(); 

    static unsigned long precedentMillisModbus = 0;
    if (millis() - precedentMillisModbus > 1000) {
        precedentMillisModbus = millis();
        requeteLectureADAM(); 
    }
}
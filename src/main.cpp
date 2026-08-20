#include <Arduino.h>
#include "reseau_local.h"
#include "modbus_adam.h"
#include "config.h"
#include <SD.h>
#include <time.h>

Preferences preferences;

String NOM_BARRAGE = "Barrage_AYLMER";
String MOT_DE_PASSE_WIFI = "Aylmer2026";
String NOMS_CANAUX[8] = {
    "NEANT", "NEANT", "MOTEUR", "L1 ", "L2", "L3", "NEANT", "NEANT"
};

// Valeurs par défaut de l'équation y = 1x + 0 et des limites [0 à 100]
float CANAL_A[8] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
float CANAL_B[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float CANAL_MIN[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float CANAL_MAX[8] = {100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0};

String fichierLogActuel = "";

void gererAutodestructionEtNomFichier(struct tm* timeinfo) {
    char nomFichierMois[30];
    snprintf(nomFichierMois, sizeof(nomFichierMois), "/log_%04d_%02d.txt", 
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1);
    
    String nouveauFichier = String(nomFichierMois);

    if (nouveauFichier != fichierLogActuel) {
        fichierLogActuel = nouveauFichier;
        int moisAncien = timeinfo->tm_mon + 1 - 4; 
        int anneeAncienne = timeinfo->tm_year + 1900;
        
        if (moisAncien <= 0) {
            moisAncien += 12;
            anneeAncienne -= 1;
        }

        char fichierASupprimer[30];
        snprintf(fichierASupprimer, sizeof(fichierASupprimer), "/log_%04d_%02d.txt", anneeAncienne, moisAncien);
        
        if (SD.exists(fichierASupprimer)) {
            SD.remove(fichierASupprimer);
        }
    }
}

void ecrireLogSysteme() {
    if (!sdDisponible) return; 
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    gererAutodestructionEtNomFichier(&timeinfo);
    int heure = timeinfo.tm_hour;

    if ((heure >= 9 && heure < 12) || (heure >= 14 && heure < 17)) {
        File fichier = SD.open(fichierLogActuel.c_str(), FILE_APPEND);
        if (fichier) {
            char horodatage[30];
            strftime(horodatage, sizeof(horodatage), "%Y-%m-%d %H:%M:%S", &timeinfo);
            
            fichier.print(horodatage);
            fichier.print(" | ");
            fichier.print(NOM_BARRAGE);
            fichier.print(" | ");
            
            for (int i = 0; i < 8; i++) {
                // Application de l'équation y = ax + b dans les logs
                float valeurFinale = (valeursConverties[i] * CANAL_A[i]) + CANAL_B[i];
                fichier.printf("AI%d: %.2f %s ", i, valeurFinale, unitesCanaux[i].c_str());
                if (i < 7) fichier.print(" - ");
            }
            fichier.println();
            fichier.close();
        } else {
            sdDisponible = false; 
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1500); 

    // --- CHARGEMENT DES PARAMÈTRES DEPUIS LA MÉMOIRE FLASH ---
    preferences.begin("param_barrage", false);
    
    NOM_BARRAGE = preferences.getString("nom", NOM_BARRAGE);
    MOT_DE_PASSE_WIFI = preferences.getString("mdp", MOT_DE_PASSE_WIFI);
    
    for (int i = 0; i < 8; i++) {
        NOMS_CANAUX[i] = preferences.getString(("ch_n_" + String(i)).c_str(), NOMS_CANAUX[i]);
        CANAL_A[i]     = preferences.getFloat(("ch_a_" + String(i)).c_str(), CANAL_A[i]);
        CANAL_B[i]     = preferences.getFloat(("ch_b_" + String(i)).c_str(), CANAL_B[i]);
        CANAL_MIN[i]   = preferences.getFloat(("ch_min_" + String(i)).c_str(), CANAL_MIN[i]);
        CANAL_MAX[i]   = preferences.getFloat(("ch_max_" + String(i)).c_str(), CANAL_MAX[i]);
    }

    initialiserReseaux();
    scannerConfigurationADAM();

    configTime(-5 * 3600, 3600, "pool.ntp.org", "time.nist.gov");
    Serial.println("[SYSTEME] Initialisation complete. Pret.");
}

void loop() {
    server.handleClient(); 

    static unsigned long precedentMillisModbus = 0;
    if (millis() - precedentMillisModbus > 1000) {
        precedentMillisModbus = millis();
        requeteLectureADAM(); 
    }

    static unsigned long precedentMillisLog = 0;
    if (millis() - precedentMillisLog > 900000) { 
        precedentMillisLog = millis();
        ecrireLogSysteme();
    }
}
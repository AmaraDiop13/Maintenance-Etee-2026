#include <Arduino.h>
#include "reseau_local.h"
#include "modbus_adam.h"
#include "config.h"
#include <SD.h>
#include <time.h>

String fichierLogActuel = "";

void gererAutodestructionEtNomFichier(struct tm* timeinfo) {
    char nomFichierMois[30];
    snprintf(nomFichierMois, sizeof(nomFichierMois), "/log_%04d_%02d.txt", 
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1);
    
    String nouveauFichier = String(nomFichierMois);

    if (nouveauFichier != fichierLogActuel) {
        fichierLogActuel = nouveauFichier;
        Serial.println("[📊 MOIS ACTIVE] Nouveau fichier : " + fichierLogActuel);

        // 💣 AUTODESTRUCTION GLISSANTE : Rétention stricte de 3 mois
        // Pour supprimer ce qui a dépassé 3 mois, on cible le fichier d'il y a 4 mois
        int moisAncien = timeinfo->tm_mon + 1 - 4; 
        int anneeAncienne = timeinfo->tm_year + 1900;
        
        if (moisAncien <= 0) {
            moisAncien += 12;
            anneeAncienne -= 1;
        }

        char fichierASupprimer[30];
        snprintf(fichierASupprimer, sizeof(fichierASupprimer), "/log_%04d_%02d.txt", anneeAncienne, moisAncien);
        
        if (SD.exists(fichierASupprimer)) {
            Serial.printf("[💣 PURGE] Suppression du fichier expire : %s\n", fichierASupprimer);
            SD.remove(fichierASupprimer);
        }
    }
}

void ecrireLogSysteme() {
    if (!sdDisponible) return; 

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("[❌ HORLOGE] Synchro temporelle manquante pour le log.");
        return;
    }

    gererAutodestructionEtNomFichier(&timeinfo);

    int heure = timeinfo.tm_hour;

    // Plages horaires de 3h réglementaires : Matin (9h à 12h) et Après-midi (14h à 17h)
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
                fichier.printf("AI%d: %.2f %s ", i, valeursConverties[i], unitesCanaux[i].c_str());
                if (i < 7) fichier.print(" - ");
            }
            fichier.println();
            fichier.close();
            Serial.println("[💾 LOGGER] Enregistrement 15-min valide effectue.");
        } else {
            Serial.println("[❌ LOGGER] Erreur critique d'ecriture.");
            sdDisponible = false; 
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1500); 

    initialiserReseaux();
    scannerConfigurationADAM();

    // Configuration de l'heure du Québec (NTP)
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

    // ⏱️ CADENCE INDUSTRIELLE : Enregistrement toutes les 15 minutes (900 000 ms)
    static unsigned long precedentMillisLog = 0;
    if (millis() - precedentMillisLog > 900000) { 
        precedentMillisLog = millis();
        ecrireLogSysteme();
    }
}
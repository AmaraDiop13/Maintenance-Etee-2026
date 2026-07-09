#include "reseau_local.h"
#include "config.h" 
#include "modbus_adam.h"
#include "page_html.h"
#include <WiFi.h>
#include <Ethernet.h>
#include <SD.h>

WebServer server(80);
byte mac[6];
bool sdDisponible = false;

extern String fichierLogActuel; // Récupère le nom du fichier généré par main.cpp

void handleDownload() {
    if (!sdDisponible) {
        server.send(503, "text/plain", "Erreur : Lecteur MicroSD non disponible.");
        return;
    }
    if (fichierLogActuel == "") {
        server.send(503, "text/plain", "Horloge systeme non synchronisee. Reessayez.");
        return;
    }
    if (!SD.exists(fichierLogActuel.c_str())) {
        server.send(404, "text/plain", "Aucun log enregistre pour ce mois-ci.");
        return;
    }

    File fichierLog = SD.open(fichierLogActuel.c_str(), FILE_READ);
    if (!fichierLog) {
        server.send(500, "text/plain", "Erreur d'ouverture du journal.");
        return;
    }

    // Force le navigateur à lire le fichier comme du texte brut universel (.txt)
    server.streamFile(fichierLog, "text/plain");
    fichierLog.close();
}

void handleRoot() {
    String html = String(INDEX_HTML);
    for(int i = 0; i < 8; i++) {
        html.replace("%NOM" + String(i) + "%", NOMS_CANAUX[i]);
    }
    server.send(200, "text/html", html);
}

void handleData() {
    String json = "{";
    json += "\"adam_en_ligne\":" + String(adamEnLigne ? "true" : "false") + ",";
    json += "\"sd_present\":" + String(sdDisponible ? "true" : "false") + ",";
    json += "\"nom_barrage\":\"" + NOM_BARRAGE + "\",";
    
    json += "\"brutes\":[" + String(valeursBrutesADAM[0]) + "," + String(valeursBrutesADAM[1]) + "," + String(valeursBrutesADAM[2]) + "," + String(valeursBrutesADAM[3]) + "," + String(valeursBrutesADAM[4]) + "," + String(valeursBrutesADAM[5]) + "," + String(valeursBrutesADAM[6]) + "," + String(valeursBrutesADAM[7]) + "],";
    json += "\"tensions\":[" + String(valeursConverties[0],2) + "," + String(valeursConverties[1],2) + "," + String(valeursConverties[2],2) + "," + String(valeursConverties[3],2) + "," + String(valeursConverties[4],2) + "," + String(valeursConverties[5],2) + "," + String(valeursConverties[6],2) + "," + String(valeursConverties[7],2) + "],";
    json += "\"est_vide\":[" + String(estVide[0]) + "," + String(estVide[1]) + "," + String(estVide[2]) + "," + String(estVide[3]) + "," + String(estVide[4]) + "," + String(estVide[5]) + "," + String(estVide[6]) + "," + String(estVide[7]) + "],";
    
    json += "\"unites\":[";
    for(int i = 0; i < 8; i++) {
        json += "\"" + unitesCanaux[i] + "\"";
        if(i < 7) json += ",";
    }
    json += "],";

    json += "\"config_modes\":[";
    for(int i = 0; i < 8; i++) {
        json += "\"" + typesEntreesDetectes[i] + "\"";
        if(i < 7) json += ",";
    }
    json += "]";
    json += "}";
    server.send(200, "application/json", json);
}

void initialiserReseaux() {
    String ssId = NOM_BARRAGE; ssId.replace(" ", "_");
    WiFi.softAP(ssId.c_str(), MOT_DE_PASSE_WIFI.c_str());

    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    pinMode(ETH_RST_PIN, OUTPUT);
    digitalWrite(ETH_RST_PIN, LOW); delay(50);
    digitalWrite(ETH_RST_PIN, HIGH); delay(300);

    SPI.begin(ETH_SCLK_PIN, ETH_MISO_PIN, ETH_MOSI_PIN, ETH_CS_PIN);
    Ethernet.init(ETH_CS_PIN);
    Ethernet.begin(mac, IP_ESP32_ETH, IP_ADAM, PASSERELLE_ETH, MASQUE_ETH);

    // Initialisation sécurisée du lecteur de carte MicroSD
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("[⚠️ ALERTE SD] Carte mémoire non détectée !");
        sdDisponible = false;
    } else {
        Serial.println("[💾 SD] Lecteur de carte MicroSD prêt.");
        sdDisponible = true;
    }

    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/telecharger", handleDownload); 
    server.begin();
}
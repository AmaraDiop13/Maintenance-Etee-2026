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
File uploadFile;

extern String fichierLogActuel; 

const char* nomUtilisateur = "admin";
const char* motDePasseAdmin = "12345";

void handleDownload() {
    if (!sdDisponible) { server.send(503, "text/plain", "Erreur : Lecteur MicroSD non disponible."); return; }
    if (fichierLogActuel == "") { server.send(503, "text/plain", "Horloge systeme non synchronisee."); return; }
    if (!SD.exists(fichierLogActuel.c_str())) { server.send(404, "text/plain", "Aucun log enregistre pour ce mois."); return; }

    File fichierLog = SD.open(fichierLogActuel.c_str(), FILE_READ);
    if (!fichierLog) { server.send(500, "text/plain", "Erreur d'ouverture du journal."); return; }
    server.streamFile(fichierLog, "text/plain");
    fichierLog.close();
}

void handleRoot() {
    if (sdDisponible && SD.exists("/index.html")) {
        File file = SD.open("/index.html", FILE_READ);
        server.streamFile(file, "text/html");
        file.close();
    } else {
        String html = String(INDEX_HTML);
        for(int i = 0; i < 8; i++) {
            html.replace("%NOM" + String(i) + "%", NOMS_CANAUX[i]);
        }
        server.send(200, "text/html", html);
    }
}

// --- PORTAIL ADMINISTRATEUR PARAMÉTRABLE ---
void handleAdmin() {
    if (!server.authenticate(nomUtilisateur, motDePasseAdmin)) {
        return server.requestAuthentication();
    }

    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Admin - Supervision</title>";
    html += "<style>body{font-family:'Segoe UI',sans-serif;background:#1e1e2f;color:white;text-align:center;padding:20px;}";
    html += ".box{background:#2a2a40;padding:25px;border-radius:12px;display:inline-block;margin:15px;vertical-align:top;border-top:5px solid #00ffcc;max-width:450px;text-align:left;}";
    html += "input[type=text]{margin:5px 0 10px 0; padding:6px; width:95%; border-radius:4px; border:none;}";
    html += "input[type=submit]{background:#ff6600;color:white;border:none;padding:12px 24px;border-radius:5px;cursor:pointer;font-weight:bold;width:100%;margin-top:15px;}";
    html += "input[type=submit]:hover{background:#e65c00;}";
    html += "a{color:#8a8ab0;text-decoration:none;display:block;margin-top:20px;text-align:center;}</style></head><body>";
    
    html += "<h2>⚙️ Configuration Avancée du Barrage</h2>";

    html += "<form method='POST' action='/save_config'>";
    
    // Paramètres généraux
    html += "<div class='box'><h3>🌐 Réseau & Système</h3>";
    html += "Nom du Barrage :<br><input type='text' name='nom_barrage' value='" + NOM_BARRAGE + "'>";
    html += "Mot de passe Wi-Fi :<br><input type='text' name='mdp_wifi' value='" + MOT_DE_PASSE_WIFI + "'>";
    html += "</div>";

    // Configuration par canal (Nom, Min, Max, a, b)
    for(int i=0; i<8; i++){
        html += "<div class='box'><h3 style='color:#ff6600;'>Canal AI" + String(i) + "</h3>";
        html += "Nom du Canal :<br><input type='text' name='ch_n_" + String(i) + "' value='" + NOMS_CANAUX[i] + "'>";
        html += "<div style='display:flex; gap:10px;'><div>Min:<br><input type='text' name='ch_min_" + String(i) + "' value='" + String(CANAL_MIN[i]) + "'></div>";
        html += "<div>Max:<br><input type='text' name='ch_max_" + String(i) + "' value='" + String(CANAL_MAX[i]) + "'></div></div>";
        html += "<div style='display:flex; gap:10px;'><div>Paramètre a (y=ax+b):<br><input type='text' name='ch_a_" + String(i) + "' value='" + String(CANAL_A[i]) + "'></div>";
        html += "<div>Paramètre b:<br><input type='text' name='ch_b_" + String(i) + "' value='" + String(CANAL_B[i]) + "'></div></div>";
        html += "</div>";
    }

    html += "<div style='width:100%; text-align:center; margin-top:20px;'>";
    html += "<input type='submit' value='💾 Sauvegarder tous les réglages & Redémarrer' style='max-width:500px;'>";
    html += "</div></form>";

    html += "<a href='/'>← Retour à la supervision</a></body></html>";
    
    server.send(200, "text/html", html);
}

// --- SAUVEGARDE DANS LA MÉMOIRE FLASH ---
void handleSaveConfig() {
    if (!server.authenticate(nomUtilisateur, motDePasseAdmin)) return server.requestAuthentication();

    if(server.hasArg("nom_barrage")) preferences.putString("nom", server.arg("nom_barrage"));
    if(server.hasArg("mdp_wifi")) preferences.putString("mdp", server.arg("mdp_wifi"));
    
    for(int i=0; i<8; i++){
        if(server.hasArg("ch_n_" + String(i))) preferences.putString(("ch_n_" + String(i)).c_str(), server.arg("ch_n_" + String(i)));
        if(server.hasArg("ch_a_" + String(i))) preferences.putFloat(("ch_a_" + String(i)).c_str(), server.arg("ch_a_" + String(i)).toFloat());
        if(server.hasArg("ch_b_" + String(i))) preferences.putFloat(("ch_b_" + String(i)).c_str(), server.arg("ch_b_" + String(i)).toFloat());
        if(server.hasArg("ch_min_" + String(i))) preferences.putFloat(("ch_min_" + String(i)).c_str(), server.arg("ch_min_" + String(i)).toFloat());
        if(server.hasArg("ch_max_" + String(i))) preferences.putFloat(("ch_max_" + String(i)).c_str(), server.arg("ch_max_" + String(i)).toFloat());
    }

    String html = "<!DOCTYPE html><html><body style='background:#1e1e2f;color:white;text-align:center;font-family:sans-serif;padding:50px;'>";
    html += "<h2 style='color:#00ffcc;'>✅ Paramètres mis à jour avec succès !</h2>";
    html += "<p>Redémarrage en cours...</p>";
    html += "<script>setTimeout(function(){window.location.href='/';}, 5000);</script></body></html>";
    
    server.send(200, "text/html", html);
    delay(1500);
    ESP.restart();
}

void handleFileUpload() {
    if (!server.authenticate(nomUtilisateur, motDePasseAdmin)) return server.requestAuthentication();
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        if (!filename.startsWith("/")) filename = "/" + filename;
        if (SD.exists(filename)) { SD.remove(filename); }
        uploadFile = SD.open(filename, FILE_WRITE);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) { uploadFile.write(upload.buf, upload.currentSize); }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (uploadFile) { uploadFile.close(); }
        server.sendHeader("Location", "/");
        server.send(303);
    }
}

// --- FLUX JSON DE CALCUL (Applique y = ax + b en temps réel) ---
void handleData() {
    String json = "{";
    json += "\"adam_en_ligne\":" + String(adamEnLigne ? "true" : "false") + ",";
    json += "\"sd_present\":" + String(sdDisponible ? "true" : "false") + ",";
    json += "\"nom_barrage\":\"" + NOM_BARRAGE + "\",";
    
    json += "\"brutes\":[";
    for(int i = 0; i < 8; i++) {
        json += String(valeursBrutesADAM[i]);
        if(i < 7) json += ",";
    }
    json += "],";

    json += "\"tensions\":[";
    for(int i = 0; i < 8; i++) {
        // CALCUL AUTOMATIQUE DE L'ÉQUATION y = ax + b
        float y = (valeursConverties[i] * CANAL_A[i]) + CANAL_B[i];
        json += String(y, 2);
        if(i < 7) json += ",";
    }
    json += "],";

    json += "\"est_vide\":[";
    for(int i = 0; i < 8; i++) {
        json += String(estVide[i]);
        if(i < 7) json += ",";
    }
    json += "],";
    
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

    if (!SD.begin(SD_CS_PIN)) {
        sdDisponible = false;
    } else {
        sdDisponible = true;
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.on("/telecharger", HTTP_GET, handleDownload); 
    server.on("/admin", HTTP_GET, handleAdmin);
    server.on("/save_config", HTTP_POST, handleSaveConfig);
    server.on("/upload", HTTP_POST, []() { server.send(200, "text/plain", "Upload OK"); }, handleFileUpload);

    server.begin();
}
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

// --- IDENTIFIANTS ADMINISTRATEUR ---
const char* nomUtilisateur = "admin";
const char* motDePasseAdmin = "12345"; // Tu peux changer ce mot de passe !

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

// --- PORTAIL ADMINISTRATEUR SÉCURISÉ ---
void handleAdmin() {
    // Vérification du mot de passe
    if (!server.authenticate(nomUtilisateur, motDePasseAdmin)) {
        return server.requestAuthentication(); // Fait pop-up la demande de mot de passe !
    }

    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Admin - Supervision</title>";
    html += "<style>body{font-family:'Segoe UI',sans-serif;background:#1e1e2f;color:white;text-align:center;padding:20px;}";
    html += ".box{background:#2a2a40;padding:30px;border-radius:12px;display:inline-block;margin:15px;vertical-align:top;border-top:5px solid #00ffcc;}";
    html += "input[type=text]{margin:5px 0 15px 0; padding:8px; width:90%; border-radius:5px; border:none;}";
    html += "input[type=file]{margin:20px 0; color:white;}";
    html += "input[type=submit]{background:#ff6600;color:white;border:none;padding:12px 24px;border-radius:5px;cursor:pointer;font-weight:bold;}";
    html += "input[type=submit]:hover{background:#e65c00;}";
    html += "a{color:#8a8ab0;text-decoration:none;display:block;margin-top:20px;}</style></head><body>";
    
    html += "<h2>⚙️ Espace d'Administration</h2>";

    // Formulaire de configuration des noms
    html += "<div class='box' style='width:350px;'><h3>📝 Paramètres du Barrage</h3>";
    html += "<form method='POST' action='/save_config'>";
    html += "<div style='text-align:left; font-size:0.9em;'>Nom du Barrage (Réseau Wi-Fi) :</div>";
    html += "<input type='text' name='nom_barrage' value='" + NOM_BARRAGE + "'>";
    html += "<div style='text-align:left; font-size:0.9em;'>Mot de passe Wi-Fi :</div>";
    html += "<input type='text' name='mdp_wifi' value='" + MOT_DE_PASSE_WIFI + "'><hr style='border:1px solid #555566; margin:20px 0;'>";
    
    for(int i=0; i<8; i++){
        html += "<div style='text-align:left; font-size:0.9em; color:#00ffcc;'>Canal AI" + String(i) + " :</div>";
        html += "<input type='text' name='ch_" + String(i) + "' value='" + NOMS_CANAUX[i] + "'>";
    }
    html += "<br><input type='submit' value='💾 Sauvegarder & Redémarrer'>";
    html += "</form></div>";

    // Formulaire d'upload de carte SD (que tu avais déjà)
    html += "<div class='box' style='width:350px;'><h3>🌐 Design Web (Carte SD)</h3>";
    html += "<p style='font-size:0.9em; color:#8a8ab0;'>Téléversez un fichier index.html personnalisé.</p>";
    html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
    html += "<input type='file' name='update' accept='.html' required><br>";
    html += "<input type='submit' value='📂 Téléverser index.html'>";
    html += "</form></div>";

    html += "<a href='/'>← Retour à la supervision</a></body></html>";
    
    server.send(200, "text/html", html);
}

// --- SAUVEGARDE DES VARIABLES EN MÉMOIRE FLASH ---
void handleSaveConfig() {
    if (!server.authenticate(nomUtilisateur, motDePasseAdmin)) return server.requestAuthentication();

    // On récupère les valeurs entrées dans le formulaire et on les sauvegarde
    if(server.hasArg("nom_barrage")) preferences.putString("nom", server.arg("nom_barrage"));
    if(server.hasArg("mdp_wifi")) preferences.putString("mdp", server.arg("mdp_wifi"));
    
    for(int i=0; i<8; i++){
        String argName = "ch_" + String(i);
        if(server.hasArg(argName)) preferences.putString(argName.c_str(), server.arg(argName));
    }

    // Affichage d'un message et redirection après 5 secondes
    String html = "<!DOCTYPE html><html><body style='background:#1e1e2f;color:white;text-align:center;font-family:sans-serif;padding:50px;'>";
    html += "<h2 style='color:#00ffcc;'>✅ Configuration Sauvegardée !</h2>";
    html += "<p>Le système redémarre pour appliquer le nouveau réseau Wi-Fi et les nouveaux noms...</p>";
    html += "<script>setTimeout(function(){window.location.href='/';}, 6000);</script></body></html>";
    
    server.send(200, "text/html", html);
    delay(1500); // Laisse le temps d'envoyer la page web
    
    ESP.restart(); // Redémarre physiquement la carte pour appliquer le Wi-Fi !
}

void handleFileUpload() {
    if (!server.authenticate(nomUtilisateur, motDePasseAdmin)) return server.requestAuthentication(); // Sécurise l'upload aussi

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

    if (!SD.begin(SD_CS_PIN)) {
        sdDisponible = false;
    } else {
        sdDisponible = true;
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.on("/telecharger", HTTP_GET, handleDownload); 
    
    // NOUVELLES ROUTES ADMIN ET SAUVEGARDE
    server.on("/admin", HTTP_GET, handleAdmin);
    server.on("/save_config", HTTP_POST, handleSaveConfig);
    server.on("/upload", HTTP_POST, []() { server.send(200, "text/plain", "Upload OK"); }, handleFileUpload);

    server.begin();
}
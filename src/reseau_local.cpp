#include "reseau_local.h"
#include "config.h" 
#include "modbus_adam.h"
#include "page_html.h"
#include <WiFi.h>
#include <Ethernet.h>

WebServer server(80);
byte mac[6];

void handleRoot() {
    String html = String(INDEX_HTML);
    for(int i = 0; i < 8; i++) {
        html.replace("%NOM" + String(i) + "%", NOMS_CANAUX[i]);
    }
    server.send(200, "text/html", html);
}

// --- PORTAIL ADMINISTRATEUR SÉCURISÉ ---
void handleAdmin() {
    if (!server.authenticate(ADMIN_USER.c_str(), ADMIN_PASS.c_str())) {
        return server.requestAuthentication();
    }

    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Admin - Supervision</title>";
    html += "<style>body{font-family:'Segoe UI',sans-serif;background:#1e1e2f;color:white;text-align:center;padding:20px;}";
    html += ".box{background:#2a2a40;padding:25px;border-radius:12px;display:inline-block;margin:15px;vertical-align:top;border-top:5px solid #00ffcc;max-width:450px;text-align:left;}";
    html += "input[type=text]{margin:5px 0 10px 0; padding:6px; width:95%; border-radius:4px; border:none;}";
    html += "input[type=submit]{background:#ff6600;color:white;border:none;padding:12px 24px;border-radius:5px;cursor:pointer;font-weight:bold;width:100%;margin-top:15px;}";
    html += "input[type=submit]:hover{background:#e65c00;}";
    html += "a{color:#8a8ab0;text-decoration:none;display:block;margin-top:20px;text-align:center;}</style></head><body>";
    
    html += "<h2>⚙️ Configuration Avancée & Sécurité</h2>";

    html += "<form method='POST' action='/save_config'>";
    
    // 1. Réseau & Sécurité Admin
    html += "<div class='box'><h3>🌐 Réseau & Sécurité</h3>";
    html += "Nom du Barrage (Wi-Fi) :<br><input type='text' name='nom_barrage' value='" + NOM_BARRAGE + "'>";
    html += "Mot de passe Wi-Fi :<br><input type='text' name='mdp_wifi' value='" + MOT_DE_PASSE_WIFI + "'>";
    html += "<hr style='border:1px solid #444; margin:15px 0;'>";
    html += "Nom utilisateur Admin :<br><input type='text' name='admin_user' value='" + ADMIN_USER + "'>";
    html += "Mot de passe Admin :<br><input type='text' name='admin_pass' value='" + ADMIN_PASS + "'>";
    html += "</div>";

    // 2. Configuration par canal (Nom, Min, Max, a, b)
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

// --- SAUVEGARDE ---
void handleSaveConfig() {
    if (!server.authenticate(ADMIN_USER.c_str(), ADMIN_PASS.c_str())) return server.requestAuthentication();

    if(server.hasArg("nom_barrage")) preferences.putString("nom", server.arg("nom_barrage"));
    if(server.hasArg("mdp_wifi")) preferences.putString("mdp", server.arg("mdp_wifi"));
    if(server.hasArg("admin_user")) preferences.putString("admin_user", server.arg("admin_user"));
    if(server.hasArg("admin_pass")) preferences.putString("admin_pass", server.arg("admin_pass"));
    
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

// --- FLUX JSON DE CALCUL (y = ax + b) ---
void handleData() {
    String json = "{";
    json += "\"adam_en_ligne\":" + String(adamEnLigne ? "true" : "false") + ",";
    json += "\"sd_present\":false,";
    json += "\"nom_barrage\":\"" + NOM_BARRAGE + "\",";
    
    json += "\"brutes\":[";
    for(int i = 0; i < 8; i++) {
        json += String(valeursBrutesADAM[i]);
        if(i < 7) json += ",";
    }
    json += "],";

    json += "\"tensions\":[";
    for(int i = 0; i < 8; i++) {
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

    server.on("/", HTTP_GET, handleRoot);
    server.on("/data", HTTP_GET, handleData);
    server.on("/admin", HTTP_GET, handleAdmin);
    server.on("/save_config", HTTP_POST, handleSaveConfig);

    server.begin();
}
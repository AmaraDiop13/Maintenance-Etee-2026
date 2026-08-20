#ifndef PAGE_HTML_H
#define PAGE_HTML_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>MAINTENANCE - SUPERVISION</title>
    <style>
        body { font-family: 'Segoe UI', sans-serif; background-color: #1e1e2f; color: #ffffff; margin: 0; padding: 20px; text-align: center; }
        h1 { color: #ff6600; margin-bottom: 5px; font-size: 2.2rem; text-transform: uppercase; }
        .subtitle { color: #a0a0b8; margin-bottom: 30px; font-size: 1.1rem; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); gap: 20px; max-width: 1100px; margin: 0 auto; }
        .card { background: #2a2a40; padding: 20px; border-radius: 12px; box-shadow: 0 4px 15px rgba(0,0,0,0.3); border-left: 5px solid #ff6600; position: relative; text-align: left; transition: all 0.3s; }
        .card.empty { border-left: 5px solid #555566; opacity: 0.6; }
        .card.no-adam { border-left: 5px solid #ff3333; opacity: 0.7; }
        .card h3 { margin: 0 0 5px 0; color: #ff6600; font-size: 1.1rem; }
        .card.empty h3 { color: #8a8ab0; }
        .card.no-adam h3 { color: #ff3333; }
        .alias { color: #ffffff; font-weight: bold; font-size: 0.95rem; margin-bottom: 15px; min-height: 20px; }
        .value { font-size: 1.9rem; font-weight: bold; margin: 10px 0; color: #00ffcc; }
        .value.empty-text { color: #8a8ab0; font-size: 1.4rem; }
        .value.no-adam-text { color: #ff3333; font-size: 1.3rem; }
        .raw { font-size: 0.85rem; color: #8a8ab0; }
        .status-led { position: absolute; top: 15px; right: 15px; width: 12px; height: 12px; background-color: #00ffcc; border-radius: 50%; box-shadow: 0 0 10px #00ffcc; animation: blinker 1s linear infinite; }
        .card.empty .status-led { background-color: #555566; box-shadow: none; animation: none; }
        .card.no-adam .status-led { background-color: #ff3333; box-shadow: none; animation: none; }
        @keyframes blinker { 50% { opacity: 0.3; } }

        /* Styles des boutons du bas */
        .btn-container { text-align: center; margin-top: 40px; margin-bottom: 30px; width: 100%; }
        
        .btn-actif { 
            display: inline-block; 
            padding: 14px 28px; 
            color: #ffffff; 
            background-color: transparent; 
            border: 2px solid #ff6600; 
            text-decoration: none; 
            font-weight: bold; 
            border-radius: 6px; 
            transition: all 0.3s ease; 
            box-shadow: 0 4px 15px rgba(255, 102, 0, 0.2); 
        }
        .btn-actif:hover { 
            background-color: #ff6600; 
            color: #ffffff; 
            box-shadow: 0 4px 20px rgba(255, 102, 0, 0.5); 
        }
        
        .btn-inactif { 
            display: inline-block; 
            padding: 14px 28px; 
            color: #8a8ab0; 
            background-color: transparent; 
            border: 2px dashed #555566; 
            text-decoration: none; 
            font-weight: bold; 
            border-radius: 6px; 
            cursor: not-allowed; 
            opacity: 0.6; 
            pointer-events: none; 
        }

        /* Style du bouton Admin */
        .btn-admin {
            display: inline-block;
            padding: 14px 28px;
            color: #ffffff;
            background-color: transparent;
            border: 2px solid #00ffcc;
            text-decoration: none;
            font-weight: bold;
            border-radius: 6px;
            margin-left: 15px;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(0, 255, 204, 0.2);
        }
        .btn-admin:hover {
            background-color: #00ffcc;
            color: #1e1e2f;
            box-shadow: 0 4px 20px rgba(0, 255, 204, 0.5);
        }
    </style>
    <script>
        setInterval(function() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('main-title').innerText = "MAINTENANCE - " + data.nom_barrage.replace('_', ' ');
                    
                    let btnSD = document.getElementById('btn-log');
                    if (data.sd_present) {
                        btnSD.className = 'btn-actif';
                        btnSD.innerText = '💾 Télécharger les Journaux (.txt)';
                    } else {
                        btnSD.className = 'btn-inactif';
                        btnSD.innerText = '⚠️ Carte SD non détectée';
                    }

                    for (let i = 0; i < 8; i++) {
                        let card = document.getElementById('card-' + i);
                        let valDiv = document.getElementById('val-' + i);
                        let rawDiv = document.getElementById('raw-' + i);
                        let modeDiv = document.getElementById('mode-' + i); 
                        
                        if (!data.adam_en_ligne) {
                            card.className = 'card no-adam';
                            valDiv.innerText = '[ PAS DE SIGNAL ]';
                            valDiv.className = 'value no-adam-text';
                            rawDiv.innerText = 'Erreur : Module ADAM introuvable';
                            modeDiv.innerText = 'Déconnecté'; 
                        } else if (data.est_vide[i]) {
                            card.className = 'card empty';
                            valDiv.innerText = '[ CANAL VIDE ]';
                            valDiv.className = 'value empty-text';
                            rawDiv.innerText = 'Aucun signal (Tension flottante)';
                            modeDiv.innerText = 'Flottant'; 
                        } else {
                            card.className = 'card';
                            valDiv.innerText = data.tensions[i] + ' ' + data.unites[i];
                            valDiv.className = 'value';
                            rawDiv.innerText = 'Registre Modbus : ' + data.brutes[i];
                            modeDiv.innerText = 'Configuration : ' + data.config_modes[i];
                        }
                    }
                }).catch(err => console.log('Erreur rafraîchissement'));
        }, 1000);
    </script>
</head>
<body>
    <h1 id="main-title">MAINTENANCE - CHARGEMENT</h1>
    <div class="subtitle">Système de Supervision Double Réseau (Wi-Fi Maintenance ↔ Câble ADAM)</div>
    <div class="grid">
        <div id="card-0" class="card"><div class="status-led"></div><h3>Canal AI0</h3><div class="alias">%NOM0%</div><div id="val-0" class="value">0.0 V</div><div id="raw-0" class="raw">Registre Modbus : 0</div><div id="mode-0" style="font-size:0.8rem; opacity:0.6; margin-top:5px;">Scan...</div></div>
        <div id="card-1" class="card"><div class="status-led"></div><h3>Canal AI1</h3><div class="alias">%NOM1%</div><div id="val-1" class="value">0.0 V</div><div id="raw-1" class="raw">Registre Modbus : 0</div><div id="mode-1" style="font-size:0.8rem; opacity:0.6; margin-top:5px;">Scan...</div></div>
        <div id="card-2" class="card"><div class="status-led"></div><h3>Canal AI2</h3><div class="alias">%NOM2%</div><div id="val-2" class="value">0.0 V</div><div id="raw-2" class="raw">Registre Modbus : 0</div><div id="mode-2" style="font-size:0.8rem; opacity:0.6; margin-top:5px;">Scan...</div></div>
        <div id="card-3" class="card"><div class="status-led"></div><h3>Canal AI3</h3><div class="alias">%NOM3%</div><div id="val-3" class="value">0.0 V</div><div id="raw-3" class="raw">Registre Modbus : 0</div><div id="mode-3" style="font-size:0.8rem; opacity:0.6; margin-top:5px;">Scan...</div></div>
        <div id="card-4" class="card"><div class="status-led"></div><h3>Canal AI4</h3><div class="alias">%NOM4%</div><div id="val-4" class="value">0.0 V</div><div id="raw-4" class="raw">Registre Modbus : 0</div><div id="mode-4" style="font-size:0.8rem; opacity:0.6; margin-top:5px;">Scan...</div></div>
        <div id="card-5" class="card"><div class="status-led"></div><h3>Canal AI5</h3><div class="alias">%NOM5%</div><div id="val-5" class="value">0.0 V</div><div id="raw-5" class="raw">Registre Modbus : 0</div><div id="mode-5" style="font-size:0.8rem; opacity:0.6; margin-top:5px;">Scan...</div></div>
        <div id="card-6" class="card"><div class="status-led"></div><h3>Canal AI6</h3><div class="alias">%NOM6%</div><div id="val-6" class="value">0.0 V</div><div id="raw-6" class="raw">Registre Modbus : 0</div><div id="mode-6" style="font-size:0.8rem; opacity:0.6; margin-top:5px;">Scan...</div></div>
        <div id="card-7" class="card"><div class="status-led"></div><h3>Canal AI7</h3><div class="alias">%NOM7%</div><div id="val-7" class="value">0.0 V</div><div id="raw-7" class="raw">Registre Modbus : 0</div><div id="mode-7" style="font-size:0.8rem; opacity:0.6; margin-top:5px;">Scan...</div></div>
    </div>

    <div class="btn-container">
        <a href="/telecharger" id="btn-log" class="btn-inactif">Vérification de la carte SD...</a>
        <a href="/admin" class="btn-admin">⚙️ Espace Administration</a>
    </div>
</body>
</html>
)rawliteral";

#endif
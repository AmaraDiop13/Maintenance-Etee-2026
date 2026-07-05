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
    </style>
    <script>
        setInterval(function() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('main-title').innerText = "MAINTENANCE - " + data.nom_barrage.replace('_', ' ');
                    for (let i = 0; i < 8; i++) {
                        let card = document.getElementById('card-' + i);
                        let valDiv = document.getElementById('val-' + i);
                        let rawDiv = document.getElementById('raw-' + i);
                        
                        if (!data.adam_en_ligne) {
                            card.className = 'card no-adam';
                            valDiv.innerText = '[ PAS DE SIGNAL ]';
                            valDiv.className = 'value no-adam-text';
                            rawDiv.innerText = 'Erreur : Module ADAM introuvable';
                        } else if (data.est_vide[i]) {
                            card.className = 'card empty';
                            valDiv.innerText = '[ CANAL VIDE ]';
                            valDiv.className = 'value empty-text';
                            rawDiv.innerText = 'Aucun signal (Tension flottante)';
                        } else {
                            card.className = 'card';
                            valDiv.innerText = data.tensions[i] + ' V';
                            valDiv.className = 'value';
                            rawDiv.innerText = 'Registre Modbus : ' + data.brutes[i];
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
        <div id="card-0" class="card"><div class="status-led"></div><h3>Canal AI0</h3><div class="alias">%NOM0%</div><div id="val-0" class="value">0.0 V</div><div id="raw-0" class="raw">Registre Modbus : 0</div></div>
        <div id="card-1" class="card"><div class="status-led"></div><h3>Canal AI1</h3><div class="alias">%NOM1%</div><div id="val-1" class="value">0.0 V</div><div id="raw-1" class="raw">Registre Modbus : 0</div></div>
        <div id="card-2" class="card"><div class="status-led"></div><h3>Canal AI2</h3><div class="alias">%NOM2%</div><div id="val-2" class="value">0.0 V</div><div id="raw-2" class="raw">Registre Modbus : 0</div></div>
        <div id="card-3" class="card"><div class="status-led"></div><h3>Canal AI3</h3><div class="alias">%NOM3%</div><div id="val-3" class="value">0.0 V</div><div id="raw-3" class="raw">Registre Modbus : 0</div></div>
        <div id="card-4" class="card"><div class="status-led"></div><h3>Canal AI4</h3><div class="alias">%NOM4%</div><div id="val-4" class="value">0.0 V</div><div id="raw-4" class="raw">Registre Modbus : 0</div></div>
        <div id="card-5" class="card"><div class="status-led"></div><h3>Canal AI5</h3><div class="alias">%NOM5%</div><div id="val-5" class="value">0.0 V</div><div id="raw-5" class="raw">Registre Modbus : 0</div></div>
        <div id="card-6" class="card"><div class="status-led"></div><h3>Canal AI6</h3><div class="alias">%NOM6%</div><div id="val-6" class="value">0.0 V</div><div id="raw-6" class="raw">Registre Modbus : 0</div></div>
        <div id="card-7" class="card"><div class="status-led"></div><h3>Canal AI7</h3><div class="alias">%NOM7%</div><div id="val-7" class="value">0.0 V</div><div id="raw-7" class="raw">Registre Modbus : 0</div></div>
    </div>
</body>
</html>
)rawliteral";

#endif
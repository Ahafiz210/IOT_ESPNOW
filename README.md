# ITP Projekt IoT und ESP-NOW

## Gruppenmitglieder
- Arafa
- Hafiz

## Projektbeschreibung
In diesem Projekt wurden zwei ESP32 als IoT-Station aufgebaut.  
Der erste ESP32 (Sender) liest die Werte eines Helligkeitssensors aus, zeigt den Status (HELL/DUNKEL) auf einem OLED-Display an und steuert eine RGB-LED. Danach wechselt er in den Sleep Mode, um Energie zu sparen.
Die Daten werden per ESP-NOW an den zweiten ESP32 (Empfänger) gesendet.  
Der Empfänger visualisiert die Daten über ein lokales Webinterface und leitet sie per HTTP an eine Node.js-API weiter, die die Werte in einer MariaDB-Datenbank speichert.

## Varianten
- Variante 1: Wi-Fi Manager
- Variante 5: Datenbank
- Variante 6: Sleep Mode (Sender)
- Variante 7: Display (OLED am Sender)

## Verwendete Komponenten
- 2x ESP32
- Helligkeitssensor
- RGB-LED
- OLED-Display (I2C, SSD1306)
- Breadboard
- Jumperkabel

## Webinterface & lokale API
Das Webinterface ist über die IP-Adresse des Empfänger-ESP32 erreichbar.  
Zusätzlich können die Daten lokal als JSON abgerufen werden:
- `/api/data`
- `/api/data/sensor`
- `/api/data/bright`

## Datenbank & Backend
Die empfangenen Daten werden von der `api.js` in der Datenbank `station` in der Tabelle `messdaten` gespeichert.  
Zusätzlich gibt es einen Discord-Bot (`discord.js`), mit dem man den aktuellen Sensorstatus über den Command `/status` abfragen kann.

## Dateien im Projekt
- `code/sender_esp32.ino`
- `code/empfaenger_esp32.ino`
- `code/api.js`
- `code/discord.js`

## Fotos und Schaltplan
Siehe Ordner `docs/`.

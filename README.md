# ESP32_Pumpenmonitor
ESP8266 Projekt fuer einen GPS Tracker mit GSM Benachrichtigung

## HW-Platform
Das Ding läuft auf einer ESP8266 D1_Mini Development Plattform, d.h. alles was D1Mini Devices sind o.ä. (z.B. Wemos D1 Mini)

## Installation
Es muss in jedem Fall node.js installiert sein, sonst koennen die NPM Module nicht installiert werden 
Nach dem Download bzw. Clonen des Repos im Verzeichnis lib/ESP8266 IOT Framework
einen cmd Prompt holen und 1) npm init -f und dann 2) npm ci eingeben damit die Node Module geladen werden  
evtl. muss noch npm install eingegeben werden wenn npm ci einen Fehler wirft  

im Git selber sind die Bin Datei und die ELF Datei mit dem Befehl
```
git add -f <relativer Pfad vom Projektroot auf bin und elf Datei>

```
ins git repo aufgenommen worden

## Compilieren
Nach dem Compilieren steht im Verzeichnis .pio\Build\d1_mini  die Datei gsmtracker.bin (wird in preFirmwareVersion.py eingestellt) zur Verfügung mit die dann geflasht werden kann
Aktuell belegt dieses Image den Speicher wie folgt:
```
RAM:   [====      ]  41.4% (used 33948 bytes from 81920 bytes)
Flash: [=====     ]  52.1% (used 544173 bytes from 1044464 bytes)
```

# iot Framework by MaakBaas
es wird ein react iot Framework basierend auf dem ESP8266 IoT Framework von MaakBaas verwendet
[maakbaas/esp8266-iot-framework](https://github.com/maakbaas/esp8266-iot-framework) aber als eigene Lib


## Neue Features im Framework zur Gliederung der Dashboard
### Gliedern des Dashboard und der Konfiguration 

für Überschriften
"type": "header",
"text": Uberschriftentext"

Für Label
"type": "label",
"text" : "labeltext"

Für Separatoren
 "type" : "separator"

# React Framework
Das iot Framework ist mit React geschrieben. Es wird aber nicht das Standard React Framework verwendet sondern preact was 100% kompatibel ist aber wesentlich kleiner. Die aktuelle Version von React wird auf der Firmwarepage angezeigt (die Version von preact ist dort hart codiert)
Doku zu [preactjs.com](https://preactjs.com)
Das hier ganz normal ein import von react gemacht werden kann 
z.B.:
``import useState from "react")`` liegt an der Configdatei für webpack ``webpack.config.js``
und zwar hier an dem Eintrag der den alias definiert:
```
    resolve: {
        alias: {
            react: "preact/compat",
            "react-dom": "preact/compat",
        },
    }, 
```
Damit ist dann sichergestellt das der Standardimport von react verwendet werden kann
### Fehler bei HTML Erzeugung
In der Configdatei für webpack ``webpack.config.js``ist auch noch ein Kommentar was bei Fehlern bei der HTML Erzeugung gemacht werden kann
```
    // Wenn die Compilierug / Erzeugung der HTML Seite durch webpack  fehlschlaegt, dann in der console npm run dev aufrufen und im Browser dann die URL http://localhost:8080/index.html aufrufen
    // n der Console wird dann angegeben was schief gelaufen ist und wo der Fehler in den JS Scripten liegt
    // die ganzen benoetigten Node.js module werden  in der windows console (cmd) mit npm ci installiert und zwar aus dem Verzeichnis ESP8266 iot Framework
    // die Nachfolgende Zeile kann fuer diesen zweck auskommentiert werden um evtl. noch weitere Infos zu erhalten
    //devtool: 'inline-source-map', 

```
## Updaten von node Komponenten
Im Verzeichnis ``lib\ESP8266 IoT Framework\node_modules`` kann nganz normal ein Commandprompt geholt werden und über npm die node.js Komponenten gemanaget werden um z.B. über ``npm list <packagename>`` die aktuell installierte Komponenten anzuzeigen oder ``npm update <packagename>`` upzudaten oder über ``npm view <packagename>`` sich die letzte verfügbare aktuelle Version anzuzeigen  
ACTUNG: react-vis muss die Version 11.11.7 sein, die andere (1.12.x) funktioniert nicht richtig und ist inkompatibel.  
Das wird in der package.jspon angegeben
# Funktion GPS Tracker 
Die Version 1.0 ist ein nacktes System ohne Funktion in dem nur der Webserver mit dem maakbas IOT Framework gestartet wird
Damit hat man dann erstmal alles was man braucht um mit einem leeren System mit Webservergrundfunktion loszulegen
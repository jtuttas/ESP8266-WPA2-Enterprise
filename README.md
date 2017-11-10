# ESP8266 mit DS18B20 und LED im Schulnetz der MMBBS
![Screenshot](doc/screenshot.PNG)
# Installation
## Voraussetzungen
Folgendes muss installiert sein!
- git
- python
- Arduino IDE
## Vorbereitungen
- Arduino IDE für den ESP8266 konfigurieren, wie hier beschrieben http://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/
- Als Board Node MCU 0.9 (ESP-12 Modul) wählen und richtigen COM Port auswählen
- Libary f. DS18B20 installieren, dazu ZIP hier laden (https://github.com/milesburton/Arduino-Temperature-Control-Library/archive/master.zip) entpacken und Umbenennen in DallasTemperature. Anschließend die Files kopieren in Arduino/libaries
- WebSocketsServer Libary herunter laden unter (https://github.com/Links2004/arduinoWebSockets/tree/master/src) und unter libaries einen Ordner erzeugen "Websockets" dort die heruntergeladenen Dateien hin kopieren.
## SDK2.0 f. WPA2 Enterprise installieren
- unter "c:\Users\{UserName}\AppData\Local\Arduino15\packages\esp8266\hardware" den Ordner wsp8266 löschen
- in das Verzeichnis "c:\Users\{UserName}\AppData\Local\Arduino15\packages\esp8266\hardware" wechseln und "git clone https://github.com/esp8266/Arduino.git esp8266" aufrufen.
- "cd esp8266"
- "git checkout update_sdk_2.0.0"
- "cd tools"
- "python get.py"
- Anschließend diese Repository auschecken oder Download auswählen
- die Datei wpa2_enterprise.h aus dem Ordner "res" kopieren nach "c:\Users\{UserName}\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\tools\sdk\include\"
- die Datei libwpa2.a aus dem Ordner "res" kopieren nach "c:\Users\{UserName}\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\tools\sdk\lib\"
## Kopilieren
Anschließend kann webserver.ino aus diesem Repository laden und *username* und *password* anpassen  kompilieren und auf den ESP8266 geladen werden!
## Verkabelung
![Wireing](doc/sketch_Steckplatine.PNG)
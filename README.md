# soyosource-powercontroller
Mit diesem Projekt ist es möglich die Einspeiseleistung einens SoyoSource GTN-1000W / GTN-1200W per Webinterface durch manuelle Steuerung, Zeitplan, MQTT oder mit einem Shelly EnergyMeter als Nulleinspeisung (3EM PRO, 3EM, EM, 1PM) zu steuern.
Der SoyoSource kann die Energie DC-Seitig aus PV-Module oder aus einer Batterie beziehen. Die AC-Einspeiseleistung kann im Einstellmenü des SoyoSource als Festwert in Watt oder durch einen auf einer Phase angeschlossenen SoyoSource Limiter bereitgestellt werden. Der Limiter wird per RS485-Schnittstelle am SoyoSource angeschlossen und sendet dann die auf der Phase anliegende Leistung an den SoyoSource.

Hinweis. Die aktuellen Versionen der SoyoSource Einspeisewechselrichter geben keine Daten mehr über die RS485-Schnittstelle aus, somit ist ein Auslesen von SoyoSource Informationen nicht möglich. Leider liegen mir aktuell keine Informationen vor ob der Sendevorgang per Software/Hardware deaktiviert wurde oder ob es nur neue Parameter bedarf um den SoyoSource zum Senden zu bewegen.

Diese Steuerung in Verbindung mit der Schaltung aus Bild 1 ersetzt den SoyoSource Limiter. Damit die Leistungsvorgabe dieser Steuerung funktioniert, muss im Einstellmenü des SoyoSource der Limitermode aktiviert werden (Bild 2).
Die manuelle Steuerung über das Webinterface sowie per MQTT oder Zeitplan funktionieren soweit, lediglich die Nulleinspeisung habe ich erst im Dezember 2023 mit eingebaut und kann diese erst im Frühjar 2024 testen und optimieren.

Achtung, ich überneheme keinerlei Haftung für Schäden an Personen oder Hardware die durch dieses Projekt entstehen. Arbeiten an Spannungen größer 24V sollten nur von Fachpersonal durchgeführt werden!  

## Update Verlauf
  16.03.2024 -> Speichern der Checkboxzustände: aktiv Timer1 / Timer2
  03.04.2024 -> Statusübersicht bei geschlossenen details/summary boxen
  14.04.2024 -> Falls Batterieschutz aktiviert, deaktiviere Regelung der Nulleinspeisung
  25.04.2024 -> Leistungspunkt bei Nulleinspeisung festlegen
                (Bei mir funktioniert gut Intervall Shelly 1000ms & Intervall Nulleinspeisung 4000ms)
  26.04.2024 -> Auswahl der aktiven Leiter (L1, L2, L3) beim Shelly
  
  27.04.2024 -> Fehlerbehebung Shelly 3EM, Shelly Plus 1PM mit zugefügt
  
  V1.240428.1:
  Teiler unter 'SoyoSource Output' hinzugefügt, um die Leistung auf mehere Geräte aufzuteilen
  

## PlatformIO
Dieses Projet wurde von der Ardunino IDE zu PlatformIO portiert

## Arduino IDE 2.1.0
Wer dieses Projekt weiterhin mit der Arduino IDE nutzen möchte muss die Datei main.cpp nach 'soyosource-powercontroller.ino' umbenennen und diese  mit der html.h in einen Ordner mit den Namen 'soyosource-powercontroller' kopieren.

#benötigte Librarys
 - ESPAsync_WiFiManager (https://github.com/khoih-prog/ESPAsync_WiFiManager)
 - ESPAsyncWebServer    (https://github.com/me-no-dev/ESPAsyncWebServer) Bitte Hinweis lesen
 - ESPAsyncTCP          (https://github.com/me-no-dev/ESPAsyncTCP)
 - ElegantOTA           (https://github.com/ayushsharma82/AsyncElegantOTA)
 - Uptime               (https://github.com/XbergCode/Uptime)


#### Hinweis ESPAsyncWebServer (Arduino IDE & PlatformIO)
Innerhalb der Library ist das Prozentzeichen '%' als Platzhalter definiert. Variablen die vom Platzhalter umschlossen sind können so später durch gesendeten Code vom Webserver ersetzt werden um z.B. Daten von Sensoren dazustellen. Leider interpretiert der Webserver aber auch das Prozentzeichen in CSS oder HTML Code falsch, so das 
z.B. bei der Angabe des property's wie xyz{ widht: 90%; } das % Zeichen entfernt wird. Dieses führt folglich zu Fehldarstellungen der Website. Als Workaround hilft Angaben mit Prozentzeichen immer doppelt anzugeben xyz{ width:90%%; } oder man ersetzt in der Library das Platzhalter Zeichen.
Ich habe in meiner Library unter dem Library-Ordner ESP Async WebServer/src die Datei 'WebResponseImpl.h' angepasst und den Platzhalter ersetzt:

#define TEMPLATE_PLACEHOLDER '%' 

durch

#define TEMPLATE_PLACEHOLDER '~'

ersetzen



## Schaltung
### Bauteile
- NodeMCU mit ESP8266 (ESP-12F) (4MB Flash)
- RS485 Entwicklungsboard TTL zu RS485, MAX485

Hinweis: Das RS485 Entwicklungsboard verwendet einen MAX485 Pegelwandler der für eine Versorgungsspannung von 5V ausgelegt ist. Da die GPIO's des ESP8266 dauerhaft nur 3.3V vertragen wird die Spannung Vcc vom RS485 Entwicklungsboard am 3.3V Ausgang des NodeMCU abgegriffen. Das RS485 Etwicklungsboard arbeitet auch zuverlässig mit 3.3V. Die 5V Spannungsversorgung des NodeMCU kann entweder über USB oder den Anschlus-Pin VIN erfolgen.


### Bild 1: Schaltung
<img src="https://github.com/matlen67/soyosource-powercontroller/blob/main/image/wiring_nodemcu_rs485.png" width="512">


### Bild 2: Einstellmenü SoyoSource
Hier muss 'Bat AutoLimit Grid' auf Y stehen

<img src="https://github.com/matlen67/soyosource-powercontroller/blob/main/image/display_setup.jpg" width="256">
  

## Webif
<img src="https://github.com/matlen67/soyosource-powercontroller/blob/main/image/webif_240426_1020.png" width="512"> 



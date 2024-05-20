# soyosource-powercontroller
With this project it is possible to control the feed-in power of a SoyoSource GTN-1000W / GTN-1200W via web interface through manual control, schedule, MQTT or with a Shelly or Homewizard EnergyMeter as zero feed-in (3EM PRO, 3EM, EM, 1PM, Homewizard). The SoyoSource can draw energy on the DC side from PV modules or from a battery. The AC feed-in power can be provided in the SoyoSource settings menu as a fixed value in watts or by a SoyoSource limiter connected to one phase. The limiter is connected to the SoyoSource via RS485 interface and then sends the power present on that phase to the SoyoSource.

A notice. The current versions of the SoyoSource feed-in inverters no longer output data via the RS485 interface, so it is not possible to read out SoyoSource information. Unfortunately, I currently have no information as to whether the sending process has been deactivated via software/hardware or whether new parameters are only required to get the SoyoSource to send.

This control in conjunction with the circuit in Figure 1 replaces the SoyoSource Limiter. In order for the performance specification of this control to work, the limiter mode must be activated in the SoyoSource settings menu (Figure 2). The manual control via the web interface as well as via MQTT or schedule works so far, I only installed the zero feed-in in December 2023 and can only test and optimize it in spring 2024.

Attention, I assume no liability for any damage to people or hardware caused by this project. Work on voltages greater than 24V should only be carried out by qualified personnel!

PlatformIO
This project was ported from the Ardunino IDE to PlatformIO

Arduino IDE 2.1.0
If you want to continue using this project with the Arduino IDE, you must rename the main.cpp file to 'soyosource-powercontroller.ino' and copy it with the html.h into a folder called 'soyosource-powercontroller'.

#required libraries

 - ESPAsync_WiFiManager (https://github.com/khoih-prog/ESPAsync_WiFiManager)
 - ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer) Please read note
 - ESPAsyncTCP (https://github.com/me-no-dev/ESPAsyncTCP)
 - ElegantOTA (https://github.com/ayushsharma82/AsyncElegantOTA)
 - Uptime (https://github.com/XbergCode/Uptime)


Note ESPAsyncWebServer when using the Arduino IDE
The percent sign '%' is defined as a placeholder within the library. Variables that are enclosed in the placeholder can later be replaced by code sent from the web server, for example to provide data from sensors. Unfortunately, the web server also misinterprets the percent sign in CSS or HTML code, for example when specifying the property like xyz{ widht: 90%; } the % character is removed. This consequently leads to misrepresentations of the website. As a workaround, it helps to always specify information with a percent sign twice xyz{ width:90%%; } or you replace the placeholder character in the library. I have adapted the file 'WebResponseImpl.h' in my library under the library folder ESP Async WebServer/src and replaced the placeholder:

#define TEMPLATE_PLACEHOLDER '%'

through

#define TEMPLATE_PLACEHOLDER '~'

substitute

If you use platformio, you don't need to change the %, as it is stored in the platformi.ini as a build flag
circuit
Components
NodeMCU with ESP8266 (ESP-12F) (4MB Flash)
Alternative Wemos D1  mini also with ESP8266
RS485 development board TTL to RS485, MAX485
Note: The RS485 development board uses a MAX485 level converter which is designed for a supply voltage of 5V. Since the GPIOs of the ESP8266 can only tolerate 3.3V permanently, the voltage Vcc is tapped from the RS485 development board at the 3.3V output of the NodeMCU. The RS485 development board also works reliably with 3.3V. The 5V power supply to the NodeMCU can be provided either via USB or the connection pin VI

### Figure 1: Schematic
<img src="https://github.com/krulkip/soyosource-powercontroller/blob/main/image/wiring_nodemcu_rs485.png" width="512">
<img src="https://github.com/krulkip/soyosource-powercontroller/blob/english/image/SoyoSource2.jpg" width="512">

### Figure 2: Control Menu SoyoSource
The 'Bat AutoLimit Grid' must be on Y

<img src="https://github.com/matlen67/soyosource-powercontroller/blob/main/image/display_setup.jpg" width="256">
 

## Figure 3: Webif
<img src="https://github.com/matlen67/soyosource-powercontroller/blob/main/image/webif_240426_1020.png" width="512"> 


### Figure 4: Addition of Homewizard
<img src="https://github.com/krulkip/soyosource-powercontroller/blob/main//SoyoSource.jpg" width="512">
 

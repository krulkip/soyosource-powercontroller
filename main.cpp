/***************************************************************************
  soyosource-powercontroller @matlen67
  Version: 1.0 /  17.12.2023

  # new in this Version
  

  Wiring
  NodeMCU D1 - RS485 RO
  NodeMCU D3 - RS485 DE/RE
  NodeMCU D4 - RS485 DI

****************************************************************************/
#include <Arduino.h> 
#include <LittleFS.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>      
#include <ESPAsync_WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Uptime.h>
#include <time.h>
#include "html.h"
#include <TelnetSpy.h>


#define ISDEBUG true

TelnetSpy SerialAndTelnet;
#define DEBUG_SERIAL SerialAndTelnet


#define RXPin        D1  // Serial Receive pin (D1)
#define TXPin        D4  // Serial Transmit pin (D4)
 
//RS485 control
#define SERIAL_COMMUNICATION_CONTROL_PIN D3 // Transmission set pin (D3)
#define RS485_TX_PIN_VALUE HIGH
#define RS485_RX_PIN_VALUE LOW

// time server
#define MY_NTP_SERVER "de.pool.ntp.org"           
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"   

 
SoftwareSerial RS485Serial(RXPin, TXPin); // RX, TX

WiFiClient espClient;
PubSubClient client(espClient);

AsyncWebServer server(80);
AsyncEventSource events("/events");
AsyncDNSServer dns;



// Uptime Global Variables
Uptime uptime;
uint8_t Uptime_Years = 0U, Uptime_Months = 0U, Uptime_Days = 0U, Uptime_Hours = 0U, Uptime_Minutes = 0U, Uptime_Seconds = 0U;
uint16_t Uptime_TotalDays = 0U; // Total Uptime Days
char uptime_str[37];  

//Timer
unsigned long lastTimeSoyo = 0;  
unsigned long timerDelaySoyo = 800;  // send readings timer

unsigned long lastTimeUptime = 0;  
unsigned long timerDelayUptime = 60000;  // send readings timer

unsigned long lastTimeShelly = 0;  
unsigned long timerDelayShelly = 5000;  // read shelly timer

unsigned long lastTimeNES = 0;  
unsigned long timerDelayNES = 10000;  // read shelly timer

String msg = "";
char msgData[64];

//mqtt
char mqtt_server[16] = "192.168.178.10";
char mqtt_port[5] = "1889";

//default custom static IP
char static_ip[16] = "192.168.178.250";
char static_gw[16] = "192.168.178.1";
char static_sn[16] = "255.255.255.0";

String dataReceived;
int data;
bool isDataReceived = false;
uint8_t byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7; 
int byteSend;
int data_array[8];
int soyo_hello_data[8] = {0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // bit7 org 0x00, CRC 0xFF
int soyo_power_data[8] = {0x24, 0x56, 0x00, 0x21, 0x00, 0x00, 0x80, 0x08}; // 0 Watt
int soyo_text_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

char buffer[8];
int old_soyo_power = 0;
int soyo_power = 0;

unsigned char mac[6];

char mqtt_root[32] = "SoyoSource/";
char clientId[12];

char topic_alive[40];
char topic_power[40];
char soyo_text[40];

long rssi;

time_t now;                       
tm timeInfo;


const char* shelly3empro;
const char* shelly3em;

const int shelly_3em_pro = 1;
const int shelly_3em = 2;
const int shelly_em = 3;
const int shelly_1pm = 4;

char currentTime[20];
char time_1[6] = "06:00";
char time_2[6] = "20:00";
char watt_1[6] = "0";
char watt_2[6] = "0";

char meteripaddr[16] = "";
char maxwatt[6] = "0";

//state checkboxes
bool checkboxT1 = false;
bool checkboxT2 = false;
bool mqttenabled = false;
bool nulleinspeisung = false;

char metername[24] = "Meter";


char mqtt_state[20] = "disabled";

// variablen Shelly 3em
String shelly_ip = "";
int shelly_typ = 0 ; 

//nulleinspeisung
//int ac_limit = 0;
int soyo_ac_out= 0;
int shelly_power = 0;

int meterpower = 0;
int meterl1 = 0;
int meterl2 = 0;
int meterl3 = 0;

bool new_connect = true;


const char* PARAM_MESSAGE = "message";


//flag for saving data
bool shouldSaveConfig = false;


//callback notifying us of the need to save config
void saveConfigCallback () {
  DEBUG_SERIAL.println("Should save config");
  shouldSaveConfig = true;
}


void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void myUptime(){
  uptime.calculateUptime();                                  

  // Get The Uptime Values To Global Variables
  Uptime_Years      = uptime.getYears();
  Uptime_Months     = uptime.getMonths();
  Uptime_Days       = uptime.getDays();
  Uptime_Hours      = uptime.getHours();
  Uptime_Minutes    = uptime.getMinutes();
  Uptime_Seconds    = uptime.getSeconds();
  Uptime_TotalDays  = uptime.getTotalDays();

  if (Uptime_Years == 0U) {                                  // Uptime Is Less Than One Year
    // First 60 Seconds
    if (Uptime_Minutes == 0U && Uptime_Hours == 0U && Uptime_Days == 0U && Uptime_Months == 0U)
      sprintf(uptime_str, "00:00:%02i", Uptime_Seconds);
    // First Minute
    else if (Uptime_Minutes == 1U && Uptime_Hours == 0U && Uptime_Days == 0U && Uptime_Months == 0U)
      sprintf(uptime_str, "00:%02i:%02i", Uptime_Minutes, Uptime_Seconds);
    // Second Minute And More But Less Than Hours, Days, Months
    else if (Uptime_Minutes >= 2U && Uptime_Hours == 0U && Uptime_Days == 0U && Uptime_Months == 0U)
      sprintf(uptime_str, "00:%02i:%02i", Uptime_Minutes, Uptime_Seconds);
    // First Hour And More But Less Than Days, Months
    else if (Uptime_Hours >= 1U && Uptime_Days == 0U && Uptime_Months == 0U)
      sprintf(uptime_str, "%02i:%02i:%02i", Uptime_Hours, Uptime_Minutes, Uptime_Seconds);
    // First Day And Less Than Month
    else if (Uptime_Days == 1U && Uptime_Months == 0U)
      sprintf(uptime_str, "%iday %02i:%02i:%02i", Uptime_Days, Uptime_Hours, Uptime_Minutes, Uptime_Seconds);
    // Second Day And More But Less Than Month
    else if (Uptime_Days >= 2U && Uptime_Months == 0U)
      sprintf(uptime_str, "%idays %02i:%02i:%02i", Uptime_Days, Uptime_Hours, Uptime_Minutes, Uptime_Seconds);
    // First Month And More But Less Than One Year
    else if (Uptime_Months >= 1U)
      sprintf(uptime_str, "%im, %id %02i:%02i", Uptime_Months, Uptime_Days, Uptime_Hours, Uptime_Minutes);
    // If There Is Any Error In This If Loop Then Make Full String.
    else sprintf(uptime_str, "%iy %im %id %02i:%02i", Uptime_Years, Uptime_Months, Uptime_Days, Uptime_Hours, Uptime_Minutes);
  } else                                                     // Uptime Is More Than One Year
    sprintf(uptime_str, "%iy %im %id %02i:%02i", Uptime_Years, Uptime_Months, Uptime_Days, Uptime_Hours, Uptime_Minutes);
}

//callback from mqtt
void mqtt_callback(char* topic, byte* payload, unsigned int length) {  
  unsigned int i = 0;
  for (i=0;i<length;i++) {
    buffer[i] = char(payload[i]);
  }

  buffer[i] = '\0';
    
  int arrived_value = atoi(buffer);
  if (arrived_value >= 0 && arrived_value <= 3000) {
    soyo_power = arrived_value;
  }
  
}


int calc_checksumme(int b1, int b2, int b3, int b4, int b5, int b6 ){
  int calc = (0xFF - b1 - b2 - b3 - b4 - b5 - b6) % 256;
  return calc & 0xFF;
}


String processor(const String& var){
  //DEBUG_SERIAL.println(var);
         
  return String();
}


void reconnect() {
  DEBUG_SERIAL.println("reconnect MQTT connection!");

  //set callback again
  client.setCallback(mqtt_callback);
  
  uint8_t timeout = 15;

  // wait for connection
  while (!client.connected()){

    DEBUG_SERIAL.println();
        
    if (client.connect(clientId)) {
      DEBUG_SERIAL.println("connection established");

      client.publish(topic_alive, "1");
      client.publish(topic_power, "0"); 
      client.subscribe(topic_power);

      strcpy(mqtt_state, "connected");

      DEBUG_SERIAL.print("subscrible: ");
      DEBUG_SERIAL.print(topic_power);
      DEBUG_SERIAL.println(" ");
    } else {
      DEBUG_SERIAL.println("reconnect failed! ");
      strcpy(mqtt_state, "connection error");
      
      //give a litle time to connect
      while (timeout){
        DEBUG_SERIAL.print(".");
        timeout--;
        delay(1000);
      }
    }

  }


  if(!mqttenabled){
    strcpy(mqtt_state, "disabled");
  }
  
}


void setSoyoPowerData(int power){
  soyo_power_data[0] = 0x24;
  soyo_power_data[1] = 0x56;
  soyo_power_data[2] = 0x00;
  soyo_power_data[3] = 0x21;
  soyo_power_data[4] = power >> 0x08;
  soyo_power_data[5] = power & 0xFF;
  soyo_power_data[6] = 0x80;
  soyo_power_data[7] = calc_checksumme(soyo_power_data[1], soyo_power_data[2], soyo_power_data[3], soyo_power_data[4], soyo_power_data[5], soyo_power_data[6]);
}


void saveConfig(){

  #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
  #else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
  #endif
  json["mqtt_server"] = mqtt_server;
  json["mqtt_port"] = mqtt_port;

  if(mqttenabled){
    json["mqttenabled"] = "1";
  }else{
    json["mqttenabled"] = "0";
  }

  if(nulleinspeisung){
    json["nulleinspeisung"] = "1";
  }else{
    json["nulleinspeisung"] = "0";
  }
  
  json["ip"] = WiFi.localIP().toString();
  json["gateway"] = WiFi.gatewayIP().toString();
  json["subnet"] = WiFi.subnetMask().toString();
  json["time1"] = time_1;
  json["watt1"] = watt_1;
  json["time2"] = time_2;
  json["watt2"] = watt_2;
  json["maxwatt"] = maxwatt;
  json["meteripaddr"] = meteripaddr;
  
  
  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    DEBUG_SERIAL.println("failed to open config file for writing");
  }

  #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    serializeJson(json, Serial);
    serializeJson(json, configFile);
  #else
    json.printTo(Serial);
    json.printTo(configFile);
  #endif
  configFile.close();
  DEBUG_SERIAL.println();

}


void telnetConnected() {
  DEBUG_SERIAL.println(F("Telnet connection established."));
}

void telnetDisconnected() {
  DEBUG_SERIAL.println(F("Telnet connection closed."));
}

void disconnectClientWrapper() {
  SerialAndTelnet.disconnectClient();
}


// get shelly typ(3em, 3em pro, 1pm...)
int getShellyTyp(){ 
  String shelly_url = "http://" + shelly_ip +  "/shelly";
  int typ = 0;

  memset(metername, 0, sizeof(metername)); 
  strcat(metername, "no device");    
   
  DynamicJsonDocument doc(2048);
 
  WiFiClient client_shelly;
  HTTPClient http;

    
  if (http.begin(client_shelly, shelly_url)) { 
    int httpCode = http.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();   
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
          DEBUG_SERIAL.print(F("deserializeJson() failed: "));
          DEBUG_SERIAL.println(error.f_str());
        }

        //test auf Shelly 3EM Pro
        //index = payload.indexOf("SPEM");
        if( payload.indexOf("SPEM") >= 0 ) {
          typ = shelly_3em_pro;
          memset(metername, 0, sizeof(metername)); 
          strcat(metername, "Shelly 3EM Pro");     
        } 

        //test auf Shelly 3EM
        if( payload.indexOf("SEM") >= 0 ){
          typ = shelly_3em;
          memset(metername, 0, sizeof(metername)); 
          strcat(metername, "Shelly 3EM");   
        }

        //test auf Shelly EM
        if(payload.indexOf("SHEM") >= 0 ){
          typ = shelly_em;
          memset(metername, 0, sizeof(metername)); 
          strcat(metername, "Shelly EM");   
        }

         //test auf Shelly 1PM
        if(payload.indexOf("SHSW-PM") >= 0){
          typ = shelly_1pm;
          memset(metername, 0, sizeof(metername)); 
          strcat(metername, "Shelly 1PM");   
        } 
      }
    }
    http.end();
  }
  DEBUG_SERIAL.print("getShellyTyp() = ");
  DEBUG_SERIAL.println(String(metername));
  return typ;
}


// read shelly3EM
int getMeterData(int typ) {
  String shelly_url;
  int power = 0;
  int power1 = 0;
  int power2 = 0;
  int power3 = 0; 
  
  DynamicJsonDocument doc(2048);
 
  WiFiClient client_shelly;
  HTTPClient http;
   
  
  if (typ == 1) { 
    shelly_url = "http://" + shelly_ip +  "/rpc/Shelly.GetStatus"; // Shelly PRO 3EM
  } else if(typ >= 2) {
    shelly_url = "http://" + shelly_ip +  "/status";  // Shelly 3EM und Andere
  } else{
    return 0;
  }                      
  
  if (http.begin(client_shelly, shelly_url))  {  
    int httpCode = http.GET();         
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();   
        DeserializationError error = deserializeJson(doc, payload);
                
        if (error) {
          DEBUG_SERIAL.print(F("deserializeJson() failed: "));
          DEBUG_SERIAL.println(error.f_str());
        }

        if (typ == 1) {
          power1 = doc["em:0"]["a_act_power"];  //Shelly 3EM PRO
          power2 = doc["em:0"]["b_act_power"];
          power3 = doc["em:0"]["c_act_power"]; 
        } else if (typ == 2) {
          power1 = doc["emeter"]["0"]["power"]; //Shelly 3EM
          power2 = doc["emeter"]["1"]["power"]; 
          power3 = doc["emeter"]["2"]["power"]; 
        } else if (typ == 3) {
          power1 = doc["meters"]["0"]["power"]; // Shelly EM Kanal 1
          power2 = doc["meters"]["1"]["power"]; // Shelly EM Kanal 2
          power3 = 0; 
        } else if (typ == 4) {
          power1 = doc["meters"]["power"]; // Shelly 1PM
          power2 = 0;
          power3 = 0;   
        }
        
        power = power1 + power2 + power3;

        meterpower = power;
        meterl1 = power1;
        meterl2 = power2;
        meterl3 = power3;
      }

    } else {
      DEBUG_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      shelly_typ = 0;
    }
    http.end();
  } else {
    DEBUG_SERIAL.printf("[HTTP} Unable to connect\n");
    shelly_typ = 0;
  }
  return power;
}





//#################### SETUP #######################
void setup() {
  
  //value = "value='%S'"; //neu 13.12.2023 wegen Compiler warnings
  shelly3empro = "Shelly 3EM Pro";
  shelly3em = "Shelly 3EM";

  SerialAndTelnet.setWelcomeMsg(F("Welcome to the TelnetSpy example\r\n\n"));
  SerialAndTelnet.setCallbackOnConnect(telnetConnected);
  SerialAndTelnet.setCallbackOnDisconnect(telnetDisconnected);
  SerialAndTelnet.setFilter(char(1), F("\r\nNVT command: AO\r\n"), disconnectClientWrapper);

  DEBUG_SERIAL.begin(115200);
  delay(250);

  WiFi.macAddress(mac);
  
  DEBUG_SERIAL.println("Start");
  DEBUG_SERIAL.printf("ESP_%02X%02X%02X", mac[3], mac[4], mac[5]);
  DEBUG_SERIAL.println();

  configTime(MY_TZ, MY_NTP_SERVER);
  
  //clientId = "soyoxxxxxx";
  sprintf(clientId, "soyo_%02x%02x%02x", mac[3], mac[4], mac[5] );
  
  //mqtt_root = "SoyoSource/soyo_xxxxxx";
  strcat(mqtt_root, clientId);
  
  //topic_alive = "SoyoSource/soyo_xxxxxx/alive";
  strcat(topic_alive, mqtt_root);
  strcat(topic_alive, "/alive");

  //topic_power = "SoyoSource/soyo_xxxxxx/power";
  strcat(topic_power, mqtt_root);
  strcat(topic_power, "/power");
  
     
  pinMode(SERIAL_COMMUNICATION_CONTROL_PIN, OUTPUT);
  digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN, RS485_RX_PIN_VALUE);
  RS485Serial.begin(4800);   // set RS485 baud

  //read configuration from json
  DEBUG_SERIAL.println("mounting FS...");

  if (LittleFS.begin()) {
    DEBUG_SERIAL.println("mounted file system");
    if (LittleFS.exists("/config.json")) {
      //file exists, reading and loading
      DEBUG_SERIAL.println("reading config file");
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        DEBUG_SERIAL.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) {
       
         DEBUG_SERIAL.println("\nparsed json");

        strcpy(mqtt_server, json["mqtt_server"]);
        strcpy(mqtt_port, json["mqtt_port"]);

        if(json.containsKey("mqttenabled")){
          char json_mqttstate[2];
          strcpy(json_mqttstate, json["mqttenabled"]);

          if(strcmp(json_mqttstate, "1") == 0){
            mqttenabled = true;
            }else{
              mqttenabled = false;
            }
          }

          if(json.containsKey("nulleinspeisung")){
            char json_nullstate[2];
            strcpy(json_nullstate, json["nulleinspeisung"]);

            if(strcmp(json_nullstate, "1") == 0){
              nulleinspeisung = true;
            }else{
              nulleinspeisung = false;
            }
          }

          if(json.containsKey("time1")){
            strcpy(time_1, json["time1"]);            
          }

          if(json.containsKey("time2")){
            strcpy(time_2, json["time2"]);
          }

          if(json.containsKey("watt1")){
            strcpy(watt_1, json["watt1"]);
          }

          if(json.containsKey("watt2")){
            strcpy(watt_2, json["watt2"]);  
          }

          if(json.containsKey("maxwatt")){
            strcpy(maxwatt, json["maxwatt"]);  
          }

          if(json.containsKey("meteripaddr")){
            strcpy(meteripaddr, json["meteripaddr"]);  
            shelly_ip = String(meteripaddr);
          }

        } else {
          DEBUG_SERIAL.println("failed to load json config");
        }
      }
    }
  } else {
    DEBUG_SERIAL.println("failed to mount FS");
  }
  //end read config data

  
  WiFi.persistent(true); // sonst verliert er nach einem Neustart die IP !!!

  ESPAsync_WMParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  ESPAsync_WMParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5); 

  ESPAsync_WiFiManager wifiManager(&server, &dns);

  
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);

  String apName = "soyo_esp_" + String(mac[3], HEX) + String(mac[4], HEX) + String(mac[5], HEX);

  bool res;
  res = wifiManager.autoConnect(apName.c_str());

  if(!res) {
    DEBUG_SERIAL.println("Failed to connect");
    ESP.restart();
  } else {
    //if you get here you have connected to the WiFi    
    DEBUG_SERIAL.print("WiFi connected to ");
    DEBUG_SERIAL.println(String(WiFi.SSID()));
    DEBUG_SERIAL.print("RSSI = ");
    DEBUG_SERIAL.print(String(WiFi.RSSI()));
    DEBUG_SERIAL.println(" dB");
    DEBUG_SERIAL.print("IP address  ");
    DEBUG_SERIAL.println(WiFi.localIP());
    DEBUG_SERIAL.println();

    //read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    
    //save the custom parameters to FS
    if (shouldSaveConfig) {
      saveConfig();
    }

    DEBUG_SERIAL.println("set mqtt server!");
    DEBUG_SERIAL.println(String("mqtt_server: ") + mqtt_server);
    DEBUG_SERIAL.println(String("mqtt_port: ") + mqtt_port);

    client.setServer(mqtt_server, atoi(mqtt_port));
    client.setCallback(mqtt_callback);
    client.publish(topic_alive, "0");
    

    // Handle Web Server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      new_connect = true;
      request->send_P(200, "text/html", index_html, processor);
    });


    // Handle Web Server Events
    events.onConnect([](AsyncEventSourceClient *client){
      if(client->lastId()){
        DEBUG_SERIAL.println("");
        DEBUG_SERIAL.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
        DEBUG_SERIAL.println("");
      }
      client->send("hello!", NULL, millis(), 10000);
    });

    // crate json and fetch data
    server.on("/json", HTTP_GET, [] (AsyncWebServerRequest *request){
      DynamicJsonDocument myJson(1024);
      String message = "";

      if(new_connect){
        new_connect = false;

        myJson["WIFIRSSI"] = rssi;
        myJson["CLIENTID"] = clientId;
        myJson["METERNAME"] = metername;
        myJson["MAXWATTINPUT"] = maxwatt;
        myJson["METERIP"] = meteripaddr;
        myJson["TIMER1TIME"] = time_1;
        myJson["TIMER1WATT"] = watt_1;
        myJson["TIMER2TIME"] = time_2;
        myJson["TIMER2WATT"] = watt_2;
        myJson["MQTTROOT"] = mqtt_root;
        myJson["MQTTSTATE"] = mqtt_state;
        myJson["CBNULL"] = nulleinspeisung; //checkbox
        myJson["CBMQTTSTATE"] = mqttenabled; //checkbox
       
        serializeJson(myJson, message);
      }else{
        myJson["UPTIME"] = uptime_str;
        myJson["SOYOPOWER"] = soyo_power;
        myJson["METERPOWER"] = meterpower;
        myJson["METERL1"] = meterl1;
        myJson["METERL2"] = meterl2;
        myJson["METERL3"] = meterl3;
        myJson["MAXWATT"] = maxwatt;

        serializeJson(myJson, message);
      }

      //DEBUG_SERIAL.print("myJson = " );
      //DEBUG_SERIAL.println(message);

      request->send(200, "application/json", message);
    });

    
    // start AP Mode
    server.on("/apmode", HTTP_GET, [](AsyncWebServerRequest *request) {
      ESPAsync_WiFiManager wifiManager(&server,&dns);
      wifiManager.resetSettings();
      
      ESP.restart();
      request->send_P(200, "text/html", index_html, processor);
    });


    // restart system
    server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {   
      DEBUG_SERIAL.println("/restart");  
      ESP.restart();
      request->send_P(200, "text/html", index_html, processor);
    });


    server.on("/acoutput", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String parm1;
  
      if (request->hasParam("value") ) {
        parm1 = request->getParam("value")->value();
        DEBUG_SERIAL.print("/acoutput?value = ");
        DEBUG_SERIAL.println(parm1);

        if(parm1.equals("/s0") ){
          soyo_power = 0;
          sprintf(msgData, "%d", soyo_power);
          if(mqttenabled){
            client.publish(topic_power, msgData);
          }
        }
        else if(parm1.equals("/p1")){
          soyo_power +=1;
          sprintf(msgData, "%d", soyo_power);
          if(mqttenabled){
            client.publish(topic_power, msgData);
          }
        }
        else if(parm1.equals("/p10")){
          soyo_power +=10;
          sprintf(msgData, "%d", soyo_power);
          if(mqttenabled){
            client.publish(topic_power, msgData);
          }
        }
        else if(parm1.equals("/m1")){
          soyo_power -=1;
          if(soyo_power < 0){
            soyo_power = 0;
           }
          sprintf(msgData, "%d", soyo_power);
          if(mqttenabled){
            client.publish(topic_power, msgData);
          }
        }
        else if(parm1.equals("/m10")){
          soyo_power -=10;
          if(soyo_power < 0){
            soyo_power = 0;
           }
          sprintf(msgData, "%d", soyo_power);
          if(mqttenabled){
            client.publish(topic_power, msgData);
          }
        }
      }
      request->send_P(200, "text/html", index_html, processor);
    });


    
    server.on("/checkbox", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String checkbox_id;
      String checkbox_value;
     
      if (request->hasParam("cbid") && request->hasParam("state")) {
        checkbox_id = request->getParam("cbid")->value();
        checkbox_value = request->getParam("state")->value();
       
        if(checkbox_id.equals("CBTIMER1")){
          if(checkbox_value.equals("1")){
            checkboxT1 = true;
          } else {
            checkboxT1 = false;
          }
        }
        else if(checkbox_id.equals("CBTIMER2")){
          if(checkbox_value.equals("1")){
            checkboxT2 = true;
          } else { 
            checkboxT2 = false;
          }
        }
        else if(checkbox_id.equals("CBMQTTSTATE")){
          if(checkbox_value.equals("1")){
            mqttenabled = true;
          } else {
            mqttenabled = false;
          }
        }
        else if(checkbox_id.equals("CBNULL")){
          if(checkbox_value.equals("1")){
            nulleinspeisung = true;
          } else {
            nulleinspeisung = false;
          }
        }
      }
      
      request->send_P(200, "text/html", index_html, processor);
    });


    server.on("/savesettings", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String value1, value2, value3, value4, value5, value6 ;

      value1 = request->getParam("t1")->value();
      value2 = request->getParam("w1")->value();

      memset(time_1, 0, sizeof(time_1)); 
      strcat(time_1, value1.c_str());     
        
      memset(watt_1, 0, sizeof(watt_1)); 
      strcat(watt_1, value2.c_str());     
    
      
      value3 = request->getParam("t2")->value();
      value4 = request->getParam("w2")->value();

      memset(time_2, 0, sizeof(time_2)); 
      strcat(time_2, value3.c_str());     
        
      memset(watt_2, 0, sizeof(watt_2)); 
      strcat(watt_2, value4.c_str());     
               
        
      value5 = request->getParam("maxwatt")->value();
      value6 = request->getParam("meteripaddr")->value();

      memset(maxwatt, 0, sizeof(maxwatt)); 
      strcat(maxwatt, value5.c_str());     
        
      memset(meteripaddr, 0, sizeof(meteripaddr)); 
      strcat(meteripaddr, value6.c_str());      
      

      saveConfig(); 

      shelly_ip = String(meteripaddr);

      request->send_P(200, "text/html", index_html, processor);
    });


    AsyncElegantOTA.begin(&server);
    server.onNotFound(notFound);
    server.addHandler(&events);
    server.begin();
    DEBUG_SERIAL.println("Server start");

    rssi = WiFi.RSSI();
   
    myUptime();
    shelly_typ = getShellyTyp(); // get shelly typ, 3em / 3empro

    digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN, RS485_TX_PIN_VALUE); // RS485 Modul -> set board to transmit 
  }

  
  // end setup()  
}


void loop() {
  SerialAndTelnet.handle();
  
 
  if(mqttenabled){
    if (!client.connected()) {
      DEBUG_SERIAL.println("lost mqtt connection -> start reconncect");
      reconnect();
    }
    client.loop(); 
  }

  
  // send power to SoyoSource every 1000 ms
  if ((millis() - lastTimeSoyo) > timerDelaySoyo) {

    myUptime();
    setSoyoPowerData(soyo_power);

    // send data to RS485 
    for(int i=0; i<8; i++){
      RS485Serial.write(soyo_power_data[i]);
      //DEBUG_SERIAL.println(soyo_power_data[i], HEX);
    }
    
    if(soyo_power != old_soyo_power){
      old_soyo_power = soyo_power;
      
      DEBUG_SERIAL.print("new soyo_power = ");
      DEBUG_SERIAL.print(soyo_power);
      DEBUG_SERIAL.print(" ( ");
      if(ISDEBUG){
        Serial.printf("%02X %02X %02X %02X %02X %02X %02X %02X",soyo_power_data[0],soyo_power_data[1],soyo_power_data[2],soyo_power_data[3],soyo_power_data[4],soyo_power_data[5],soyo_power_data[6],soyo_power_data[7]);
      }
      DEBUG_SERIAL.print(" ) ");
      DEBUG_SERIAL.println();
    }
    
    
    if(mqttenabled){
      client.publish(topic_alive, "0");
      client.publish(topic_alive, "1");
    }
    
    //DEBUG_SERIAL.println();
        
    if(checkboxT1 || checkboxT2){
      time(&now);
      localtime_r(&now, &timeInfo);
    }
      
    if (checkboxT1 == true){      
      int t1_hour = String(time_1).substring(0,2).toInt();
      int t1_min = String(time_1).substring(3).toInt();

      if((timeInfo.tm_hour == t1_hour && timeInfo.tm_min == t1_min && timeInfo.tm_sec == 0) || (timeInfo.tm_hour == t1_hour && timeInfo.tm_min == t1_min && timeInfo.tm_sec == 1) ){
        soyo_power = atoi(watt_1);
        sprintf(msgData, "%d", soyo_power);
        if(mqttenabled){
          client.publish(topic_power, msgData);
        }
      }
    }

    if (checkboxT2 == true){    
      int t2_hour = String(time_2).substring(0,2).toInt();
      int t2_min = String(time_2).substring(3).toInt();  
      
      if((timeInfo.tm_hour == t2_hour && timeInfo.tm_min == t2_min && timeInfo.tm_sec == 0) || (timeInfo.tm_hour == t2_hour && timeInfo.tm_min == t2_min && timeInfo.tm_sec == 1)){
        soyo_power = atoi(watt_2);
        sprintf(msgData, "%d", soyo_power);
        if(mqttenabled){
          client.publish(topic_power, msgData);
        }
      }
    }
    
    lastTimeSoyo = millis();
  }


 
  // timer to get Shelly3EM data (5 sek)
  if ((millis() - lastTimeShelly) > timerDelayShelly) {  
    if (shelly_typ > 0){
      shelly_power = getMeterData(shelly_typ);
    } else{
      shelly_typ = getShellyTyp();
      DEBUG_SERIAL.println("Kein Shelly erkannt! Bitte IP eintragen, speichern und ESP neu starten.");
    }
    lastTimeShelly = millis();
  }


  //timer to update soyo power nulleinspeisung (10 sek)
  if ((millis() - lastTimeNES) > timerDelayNES) { 

    if(nulleinspeisung){      
      int limit = String(maxwatt).toInt();
      DEBUG_SERIAL.print("nulleinspeisung limit = ");
      DEBUG_SERIAL.println(limit);
        
      if(shelly_power > 20){  
        soyo_ac_out += shelly_power - 10;

        if(soyo_ac_out >= limit){
          soyo_ac_out = limit;
        } 
      } 

      if(shelly_power < 0){
        soyo_ac_out -= shelly_power + 10;

        if(soyo_ac_out < 0){
          soyo_ac_out = 0;
        } 
      }

      DEBUG_SERIAL.print("nulleinspeisung soyo_Ac_out = ");
      DEBUG_SERIAL.println(soyo_ac_out);

      soyo_power = soyo_ac_out;
    }
    lastTimeNES = millis();
  }


}



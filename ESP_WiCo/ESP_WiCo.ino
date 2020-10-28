//MIT License
//
//Copyright (c) 2020 CatEllite312
//
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation 
//files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, 
//modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
//Software is furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
//WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
//COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <FS.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager


#include <string.h>
#include <EEPROM.h>
#include "CTBot.h"

CTBot myBot;
CTBotInlineKeyboard myKbd;  // custom inline keyboard object helper

#define RELAY1_ON_CALLBACK  "RELAY1ON"
#define RELAY1_OFF_CALLBACK "RELAY1OFF"
#define RELAY2_ON_CALLBACK  "RELAY2ON"
#define RELAY2_OFF_CALLBACK "RELAY2OFF"
#define RELAY3_ON_CALLBACK  "RELAY3ON"
#define RELAY3_OFF_CALLBACK "RELAY3OFF"
#define RELAY4_ON_CALLBACK  "RELAY4ON"
#define RELAY4_OFF_CALLBACK "RELAY4OFF"
#define STATE "RELAYSTATE"

ESP8266WebServer server(80);

int relay1_pin = D1;
int relay2_pin = D2;
int relay3_pin = D3;
int relay4_pin = D4;



boolean relay1_state;
boolean relay2_state;
boolean relay3_state;
boolean relay4_state;

String link;

boolean relay_module_active = LOW;  // Реле активируются подтягиванием к земле

char tg_token[64] = "YOUR_TG_TOKEN";

//настройки сети по умолчанию в каптивпортале
char static_ip[16] = "192.168.88.175";
char static_gw[16] = "192.168.88.1";
char static_sn[16] = "255.255.255.0";

//flag for saving data
bool shouldSaveConfig = false;

//тут у нас страничка с переключателями
String page = "<!DOCTYPE html>\n<html>\n<head>\n<meta http-equiv=Content-Type content=\"text/html; charset=utf-8\" />\n<meta http-equiv=X-UA-Compatible content=\"IE=8,IE=9,IE=10\">\n<meta name=viewport content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0\"/>\n<style>body{font-family:sans-serif}.switch{cursor:pointer;width:5.5em;border:1px solid #5f645b;color:#fff;border-radius:.8em;margin-left:20px}.toggle,.state{margin:.1em;font-size:130%;font-weight:normal;text-align:center;float:left}.toggle{width:1.1em;background-color:#f5f5f5;color:#000;text-align:center;border:1px solid grey;border-radius:.5em;margin-right:.1em;margin-left:.2em}.state{padding-top:.05em;width:2em}.on{background-color:#56c94d}.off{background-color:#eceeef;color:#aaaab8}.on .toggle{float:right}.off .toggle{float:left}.clearfix{clear:both}table td{vertical-align:middle}table h2{margin:0;padding:0;font-weight:normal;margin-top:4px}</style>\n<script>function toggle(a){document.location.href=\"relay\"+a+\"/toggle\"};</script>\n</head>\n<body>\n<h1>Relay Mangager</h1>\n<table border=0>\n<tr><td>\n<h2>Relay 1</h1>\n</td><td>\n<div class=\"switch {{relay1_state}}\" onclick=toggle(1)>\n<div class=toggle>&nbsp;</div>\n<div class=state>{{relay1_state}}</div><br class=clearfix />\n</div>\n</td></tr>\n<tr><td>\n<h2>Relay 2</h1>\n</td><td>\n<div class=\"switch {{relay2_state}}\" onclick=toggle(2)>\n<div class=toggle>&nbsp;</div>\n<div class=state>{{relay2_state}}</div><br class=clearfix />\n</div>\n</td>\n</tr>\n<tr><td>\n<h2>Relay 3</h1>\n</td><td>\n<div class=\"switch {{relay3_state}}\" onclick=toggle(3)>\n<div class=toggle>&nbsp;</div>\n<div class=state>{{relay3_state}}</div><br class=clearfix />\n</div>\n</td>\n</tr>\n<tr><td>\n<h2>Relay 4</h1>\n</td><td>\n<div class=\"switch {{relay4_state}}\" onclick=toggle(4)>\n<div class=toggle>&nbsp;</div>\n<div class=state>{{relay4_state}}</div><br class=clearfix />\n</div>\n</td>\n</tr>\n</table>\n</body>\n</html>";

String get_human_state(bool relay_state) {
  if (relay_state == 1) {
    return "on";
  } else {
    return "off";
  }
}

String get_page() {
  String x = page;
  x.replace("{{relay1_state}}", get_human_state(relay1_state));
  x.replace("{{relay2_state}}", get_human_state(relay2_state));
  x.replace("{{relay3_state}}", get_human_state(relay3_state));
  x.replace("{{relay4_state}}", get_human_state(relay4_state));
  return x;
}

void handle_root() {
  Serial.println("Request incoming...");
  server.send(200, "text/html", get_page());
  delay(100);
  Serial.println("Request handled.");
}

void handle_not_found() {
  Serial.println("404 Not Found ");
  server.send(404, "text/plain", "Not Found. You requested \"" + server.uri() + "\"");
  delay(100);
  Serial.println("Request handled.");
}

int get_relay_pin(int relay_number) {
  if (relay_number == 1) {
    return relay1_pin;
  }

  if (relay_number == 2) {
    return relay2_pin;
  }
  if (relay_number == 3) {
    return relay3_pin;
  }
  if (relay_number == 4) {
    return relay4_pin;
  }
}

bool get_relay_state(int relay_number) {
  if (relay_number == 1) {
    return relay1_state;
  }
  if (relay_number == 2) {
    return relay2_state;
  }
  if (relay_number == 3) {
    return relay3_state;
  }
  if (relay_number == 4) {
    return relay4_state;
  }
}

void set_relay_state(int relay_number, bool state) { //пишу состояния пинов в еепром, так контроллер будет помнить состояние до выключения
  if (relay_number == 1) {
    relay1_state = state;
    EEPROM.write(0, state);
    EEPROM.commit();
  }
  if (relay_number == 2) {
    relay2_state = state;
    EEPROM.write(1, state);
    EEPROM.commit();
  }
  if (relay_number == 3) {
    relay3_state = state;
    EEPROM.write(2, state);
    EEPROM.commit();
  }
  if (relay_number == 4) {
    relay4_state = state;
    EEPROM.write(3, state);
    EEPROM.commit();
  }
}

void handle_relay() {     //web api
  int relay_number;
  int relay_pin;

  if (server.uri() == "/api/relay1") {
    relay_pin = relay1_pin;
    relay_number = 1;
  } else if (server.uri() == "/api/relay2") {
    relay_pin = relay2_pin;
    relay_number = 2;
  } else if (server.uri() == "/api/relay3") {
    relay_pin = relay3_pin;
    relay_number = 3;
  } else if (server.uri() == "/api/relay4") {
    relay_pin = relay4_pin;
    relay_number = 4;
  }

  if (server.method() == HTTP_GET) { //обрабатываем get запросы
    Serial.println("GET REQUESTed");
    server.send(200, "text/json", "{\"state\": " + String(get_relay_state(relay_number)) + "}");

  } else if (server.method() == HTTP_POST) { //обрабатываем POST запросы
    Serial.println("POST REQUESTed");

    for ( uint8_t i = 0; i < server.args(); i++ ) {
      Serial.println( "" + server.argName ( i ) + ": " + server.arg ( i ));
      if (server.argName(i) == "state") {
        set_relay(relay_number, bool(server.arg(i).toInt()));
        server.send(200, "text/json", "{\"state\": " + String(get_relay_state(relay_number)) + ", \"set\": 1}");
      }
    }
  } else {
    server.send(501, "text/json", "{\"error\": \"Not Implemented: " + String(server.method()) + "\"}");
  }

  delay(100);
}

void handle_toggle() {
  int relay_number;
  int relay_pin;

  if (server.uri() == "/relay1/toggle") {
    relay_pin = relay1_pin;
    relay_number = 1;
  } else if (server.uri() == "/relay2/toggle") {
    relay_pin = relay2_pin;
    relay_number = 2;
  } else if (server.uri() == "/relay3/toggle") {
    relay_pin = relay3_pin;
    relay_number = 3;
  } else if (server.uri() == "/relay4/toggle") {
    relay_pin = relay4_pin;
    relay_number = 4;
  }

  set_relay(relay_number, !get_relay_state(relay_number));

  server.sendHeader("Location", "/", true);
  server.send (302, "text/plain", "");
  server.client().stop();
}

void set_relay(int relay_number, bool state) {

  if (state == 1) {
    digitalWrite(get_relay_pin(relay_number), relay_module_active);
    set_relay_state(relay_number, 1);

  } else {
    digitalWrite(get_relay_pin(relay_number), !relay_module_active);
    set_relay_state(relay_number, 0);

  }
}


//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void beep(unsigned char delayms) {  //бип бип
  analogWrite(D8, 128);
  delay(delayms);
  analogWrite(D8, 0);
  delay(delayms);
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(4); //храню в еепроме всего 4 состояния релюшек
  Serial.println();
  //настраиваю цифровые выходы
  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  pinMode(relay3_pin, OUTPUT);
  pinMode(relay4_pin, OUTPUT);
  pinMode(D7, OUTPUT); //выход на транзистор который выключает реле на время старта

  //задаю последние сохранённые в еепром состояния цифровым выходам
  set_relay(1, EEPROM.read(0));
  set_relay(2, EEPROM.read(1));
  set_relay(3, EEPROM.read(2));
  set_relay(4, EEPROM.read(3));
  /////////////////////////////////////////////////////////////////////////////////////////тут даже не вникал, взял из примера в комплекте с библиотекой
  //clean FS, for testing
  //SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(tg_token, json["tg_token"]);

          if (json["ip"]) {
            Serial.println("setting custom ip from config");
            //static_ip = json["ip"];
            strcpy(static_ip, json["ip"]);
            strcpy(static_gw, json["gateway"]);
            strcpy(static_sn, json["subnet"]);
            //strcat(static_ip, json["ip"]);
            //static_gw = json["gateway"];
            //static_sn = json["subnet"];
            Serial.println(static_ip);
            /*            Serial.println("converting ip");
                        IPAddress ip = ipFromCharArray(static_ip);
                        Serial.println(ip);*/
          } else {
            Serial.println("no custom ip in config");
          }
        } else {
          Serial.println("failed to load json config");
          beep(100);
          beep(100);
          beep(100);
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read
  //Serial.println(tg_token);
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



  WiFiManagerParameter custom_tg_token("tg", "tg token", tg_token, 64); //кастомный параметр вайфай менеджера для токена тг

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //set static ip
  IPAddress _ip, _gw, _sn;
  _ip.fromString(static_ip);
  _gw.fromString(static_gw);
  _sn.fromString(static_sn);

  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);

  //add all your parameters here
  wifiManager.addParameter(&custom_tg_token); // добавляю параметр с токеном тг в каптив портал

  //reset settings - for testing
  //wifiManager.resetSettings();

  //задаём минимальный уровень сигнала для подключаемой точки (по умол 8%)
  wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  if (!wifiManager.autoConnect("ESP WiConnect")) {
    Serial.println("failed to connect and hit timeout");
    beep(100);
    beep(100);
    beep(100);
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(tg_token, custom_tg_token.getValue());

  //Сохраняем токен в файловую систему
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["tg_token"] = tg_token;

    json["ip"] = WiFi.localIP().toString();
    json["gateway"] = WiFi.gatewayIP().toString();
    json["subnet"] = WiFi.subnetMask().toString();

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
      beep(100);
      beep(100);
      beep(100);
    }

    json.prettyPrintTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());
  //какие адреса слушаем
  server.on("/", handle_root);
  server.on("/api/relay1", handle_relay);
  server.on("/api/relay2", handle_relay);
  server.on("/relay1/toggle", handle_toggle);
  server.on("/relay2/toggle", handle_toggle);
  server.on("/api/relay3", handle_relay);
  server.on("/api/relay4", handle_relay);
  server.on("/relay3/toggle", handle_toggle);
  server.on("/relay4/toggle", handle_toggle);

  server.onNotFound (handle_not_found);
  server.begin();
  Serial.println("Server Started!");

  digitalWrite(D7, HIGH); //после того как у нас мк раздуплился, дергаем за транзистор и возвращаем землю релюшкам

  //Serial.println(tg_token);
  myBot.setTelegramToken(tg_token); //инициализруем бота с нашим токеном если он есть
  // check if all things are ok
  if (myBot.testConnection()) {
    Serial.println("\ntestConnection OK");
    link = "http://" + WiFi.localIP().toString() + "/";
  } else {
    Serial.println("\ntestConnection NOK");
    beep(100);
    beep(100);
  }

  // Вот тут живёт клавиатура тг бота
  // add a query button to the first row of the inline keyboard
  myKbd.addButton("Relay1 ON", RELAY1_ON_CALLBACK, CTBotKeyboardButtonQuery);
  // add another query button to the first row of the inline keyboard
  myKbd.addButton("Relay1 OFF", RELAY1_OFF_CALLBACK, CTBotKeyboardButtonQuery);
  // add a new empty button row
  myKbd.addRow();
  myKbd.addButton("Relay2 ON", RELAY2_ON_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addButton("Relay2 OFF", RELAY2_OFF_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addRow();
  myKbd.addButton("Relay3 ON", RELAY3_ON_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addButton("Relay3 OFF", RELAY3_OFF_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addRow();
  myKbd.addButton("Relay4 ON", RELAY4_ON_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addButton("Relay4 OFF", RELAY4_OFF_CALLBACK, CTBotKeyboardButtonQuery);
  myKbd.addRow();
  myKbd.addButton("State", STATE, CTBotKeyboardButtonQuery);
  myKbd.addRow();
  myKbd.addButton("Local WEB", link, CTBotKeyboardButtonURL);
  Serial.println("\nEndSetup");

  beep(1000);//длинный бип при старте который говорит нам чо всё супер

}

void loop() {
  server.handleClient();

  TBMessage msg;

  // смотрим что прислали в бота, команду или нажали на кнопку
  if (myBot.getNewMessage(msg)) {
    // check what kind of message I received
    if (msg.messageType == CTBotMessageText) {
      // received a text message
      if (msg.text.equalsIgnoreCase("/show")) {
        myBot.sendMessage(msg.sender.id, "ESP WiCo beta 0.6", myKbd);
      } else if (msg.text.equalsIgnoreCase("/r11")) {
        set_relay(1, 1);
        myBot.sendMessage(msg.sender.id, "Relay 1 ON");
      } else if (msg.text.equalsIgnoreCase("/r21")) {
        set_relay(2, 1);
        myBot.sendMessage(msg.sender.id, "Relay 2 ON");
      } else if (msg.text.equalsIgnoreCase("/r31")) {
        set_relay(3, 1);
        myBot.sendMessage(msg.sender.id, "Relay 3 ON");
      } else if (msg.text.equalsIgnoreCase("/r41")) {
        set_relay(4, 1);
        myBot.sendMessage(msg.sender.id, "Relay 4 ON");
      } else if (msg.text.equalsIgnoreCase("/r10")) {
        set_relay(1, 0);
        myBot.sendMessage(msg.sender.id, "Relay 1 OFF");
      } else if (msg.text.equalsIgnoreCase("/r20")) {
        set_relay(2, 0);
        myBot.sendMessage(msg.sender.id, "Relay 2 OFF");
      } else if (msg.text.equalsIgnoreCase("/r30")) {
        set_relay(3, 0);
        myBot.sendMessage(msg.sender.id, "Relay 3 OFF");
      } else if (msg.text.equalsIgnoreCase("/r40")) {
        set_relay(4, 0);
        myBot.sendMessage(msg.sender.id, "Relay 4 OFF");
      } else if (msg.text.equalsIgnoreCase("/state")) {
        myBot.sendMessage(msg.sender.id, "Relay1 - " + String(EEPROM.read(0)) + "\n" + "Relay2 - " + String(EEPROM.read(1)) + "\n" + "Relay3 - " + String(EEPROM.read(2)) + "\n" + "Relay4 - " + String(EEPROM.read(3)));
      }
      else {
        // подсказка
        myBot.sendMessage(msg.sender.id, "Try '/show' or '/state'");
      }
    } else if (msg.messageType == CTBotMessageQuery) {
      // received a callback query message
      if (msg.callbackQueryData.equals(RELAY1_ON_CALLBACK)) {
        // pushed "LIGHT ON" button...
        set_relay(1, 1);
        // terminate the callback with an alert message
        myBot.endQuery(msg.callbackQueryID, "RELAY1 on");
      } else if (msg.callbackQueryData.equals(RELAY1_OFF_CALLBACK)) {
        // pushed "LIGHT OFF" button...
        set_relay(1, 0);
        // terminate the callback with a popup message
        myBot.endQuery(msg.callbackQueryID, "RELAY1 off");
      } else if (msg.callbackQueryData.equals(RELAY2_ON_CALLBACK)) {
        // pushed "LIGHT ON" button...
        set_relay(2, 1);
        // terminate the callback with a popup message
        myBot.endQuery(msg.callbackQueryID, "RELAY2 on");
      } else if (msg.callbackQueryData.equals(RELAY2_OFF_CALLBACK)) {
        // pushed "LIGHT OFF" button...
        set_relay(2, 0);
        // terminate the callback with a popup message
        myBot.endQuery(msg.callbackQueryID, "RELAY2 off");
      } else if (msg.callbackQueryData.equals(RELAY3_ON_CALLBACK)) {
        // pushed "LIGHT ON" button...
        set_relay(3, 1);
        // terminate the callback with a popup message
        myBot.endQuery(msg.callbackQueryID, "RELAY3 on");
      } else if (msg.callbackQueryData.equals(RELAY3_OFF_CALLBACK)) {
        // pushed "LIGHT OFF" button...
        set_relay(3, 0);
        // terminate the callback with a popup message
        myBot.endQuery(msg.callbackQueryID, "RELAY3 off");
      } else if (msg.callbackQueryData.equals(RELAY4_ON_CALLBACK)) {
        // pushed "LIGHT ON" button...
        set_relay(4, 1);
        // terminate the callback with a popup message
        myBot.endQuery(msg.callbackQueryID, "RELAY4 on");
      } else if (msg.callbackQueryData.equals(RELAY4_OFF_CALLBACK)) {
        // pushed "LIGHT OFF" button...
        set_relay(4, 0);
        // terminate the callback with a popup message
        myBot.endQuery(msg.callbackQueryID, "RELAY4 off");
      } else if (msg.callbackQueryData.equals(STATE)) {

        myBot.endQuery(msg.callbackQueryID, "Relay1 - " + String(EEPROM.read(0)) + "\n" + "Relay2 - " + String(EEPROM.read(1)) + "\n" + "Relay3 - " + String(EEPROM.read(2)) + "\n" + "Relay4 - " + String(EEPROM.read(3)), true);
      }
    }
  }
  delay(500); //на скорости особо не сказывается (код и без этого вдумчивый) но камушек заметно холоднее
}
//Хотел бы я сказать что рад "читателю" который дожил до этого места но увы, я искренне не расчитываю что кто то будет читать а темболее разбирать такой
//страшный код. Данную приблуду делал для себя из доступных деталей и с максимально простой конфигурацией. После прошивки устройство готово к работе.

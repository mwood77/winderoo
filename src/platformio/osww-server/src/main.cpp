#include <Arduino.h>
#include <WiFiManager.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>

#include "FS.h"
#include "ESPAsyncWebServer.h"

HTTPClient http;
WiFiClient client;

WiFiManager wm;
AsyncWebServer server(80);

bool reset = false;

String status = "";
String rotationsPerDay = "460";
String direction = "CW";

String getFullMemoryUsage() {
  String usage = "Total heap: ";
  usage = String(usage + ESP.getHeapSize());
  usage = String(usage + "\nFree heap: ");
  usage = String(usage + String(ESP.getFreeHeap()));
  usage = String(usage + "\nTotal PSRAM: ");
  usage = String(usage + String(ESP.getPsramSize()));
  usage = String(usage + "\nFree PSRAM: ");
  usage = String(usage + String(ESP.getFreePsram()));
  return usage;
}

String getTime() {
  // Query dateTime based upon IP
  http.begin(client, "http://worldtimeapi.org/api/ip");
  int httpCode = http.GET();

  String usage = "Winderoo is live!";

  if (httpCode > 0) {
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, http.getStream());
      
      const char* datetime = doc["datetime"];
      const char* timezone = doc["timezone"];

      usage = String(usage + "\n\nThe time is:\n- ");
      usage = String(usage + datetime);                 // "2023-01-30T17:16:39.560102+01:00"
      usage = String(usage + "\n\nTimezone:\n- ");
      usage = String(usage + timezone);                 // "Europe/Amsterdam"
      
      return usage;
    }

    http.end();
    return usage;
}

void notFound(AsyncWebServerRequest *request) {
  // Handle HTTP_OPTIONS requests
  if (request->method() == 64) {
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Ok");
      request->send(response);
  } else {
    request->send(404, "text/plain", "Winderoo\n\n404 - Resource Not found");
  }
}

void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  if(!index){
    Serial.printf("BodyStart: %u B\n", total);
  }
  for(size_t i=0; i<len; i++){
    Serial.write(data[i]);
  }
  if(index + len == total){
    Serial.printf("BodyEnd: %u B\n", total);
  }
}

void startWebserver() { 

  server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", getTime());
  });

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {

    Serial.println(WiFi.RSSI());

    AsyncResponseStream *response = request->beginResponseStream("application/json");
      DynamicJsonDocument json(1024);
      json["status"] = status;
      json["rotationsPerDay"] = rotationsPerDay;
      json["direction"] = direction;
      json["db"] = WiFi.RSSI();
      json["batteryLevel"] = 0;   // @todo - get battery level
      serializeJson(json, *response);
      request->send(response);
  });

  server.on("/api/update", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for ( int i = 0; i < params; i++ ) {
        AsyncWebParameter* p = request->getParam(i);
        Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
          if( strcmp(p->name().c_str(), "action") == 0) {
            if ( strcmp(p->value().c_str(), "START") == 0 ) {
              status = "Winding";
            } else {
              status = "Stopped";
            }
          } 
          if( strcmp(p->name().c_str(), "rotationDirection") == 0 ) {
            direction = p->value().c_str();
          }
          if( strcmp(p->name().c_str(), "tpd") == 0 ) {
            rotationsPerDay = p->value().c_str();
          }
        request->send(204);
  }});

  server.on("/api/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    json["status"] = "Resetting";
    serializeJson(json, *response);
    request->send(response);
    
    reset = true;
  });

  server.on("/api/system/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", getFullMemoryUsage());
  });

  server.serveStatic("/css/", LittleFS, "/css/").setCacheControl("max-age=31536000");
  server.serveStatic("/js/", LittleFS, "/js/").setCacheControl("max-age=31536000");
  server.serveStatic("/favicon.ico", LittleFS, "/favicon.ico").setCacheControl("max-age=31536000");
  server.serveStatic("/icon.jpeg", LittleFS, "/icon.jpeg").setCacheControl("max-age=31536000");
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  
  server.onNotFound(notFound);

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With");

  server.begin();
}


// Initialize File System
void initFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }

  Serial.println("LittleFS mounted successfully");
}


 
void setup() {
    WiFi.mode(WIFI_STA);
    Serial.begin(115200);

    pinMode (LED_BUILTIN, OUTPUT);
    status = "Stopped";
    
    wm.setDarkMode(true);
    wm.setConfigPortalBlocking(false);
    wm.setDebugOutput(true);
    wm.setHostname("Winderoo");
    
    //automatically connect using saved credentials if they exist
    //If connection fails it starts an access point with the specified name
    if(wm.autoConnect("Winderoo Setup")) {
        initFS();
        Serial.println("connected to saved network");
        if (!MDNS.begin("winderoo")) {
          Serial.println("Failed to start mDNS");
        }
        Serial.println("mDNS started");

        startWebserver();
    }
    else {
        Serial.println("WiFi Config Portal running");
        digitalWrite(LED_BUILTIN, HIGH);
        startWebserver();
        if (!MDNS.begin("winderoo")) {
          Serial.println("Failed to start mDNS");
        }
        Serial.println("mDNS started");
    }
}
 
void loop() {
  if (reset) {
    for ( int i = 0; i < 40; i++ ) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }

    Serial.println("Stopping webserver");
    server.end();
    delay(600);
    Serial.println("Stopping File System");
    LittleFS.end();
    delay(200);
    Serial.println("Resetting Wifi Manager settings");
    wm.resetSettings();
    delay(200);
    Serial.println("Restart device...");
    ESP.restart();
    delay(2000);
  }

  wm.process();
}
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
String timeURL = "http://worldtimeapi.org/api/ip";

String settingsFile = "/settings.txt";
String status = "";
String rotationsPerDay = "";
String direction = "";

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


// For prioritizing winding time
String getTime() {
  http.begin(client, timeURL);
  int httpCode = http.GET();

  String time = "";

  if (httpCode > 0) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, http.getStream());
    const char* datetime = doc["datetime"];
    String time = String(time + datetime);         // "2023-01-30T17:16:39.560102+01:00"
    return time;
  }

  http.end();
  return time;
}

String getTextFormatedTime() {
  // Query dateTime based upon IP
  http.begin(client, timeURL);
  int httpCode = http.GET();

  String usage = "Winderoo is live!";

  if (httpCode > 0) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, http.getStream());
    
    const char* datetime = doc["datetime"];
    const char* timezone = doc["timezone"];

    usage = String(usage + "\n\nThe time is:\n- ");
    usage = String(usage + datetime);
    usage = String(usage + "\n\nTimezone:\n- ");
    usage = String(usage + timezone);
    
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

String loadConfigVarsFromFile(String file_name) {
  String result = "";
  
  File this_file = LittleFS.open(file_name, "r");

  if (!this_file) {
    Serial.println("[STATUS] - Failed to open configuration file, returning empty result");
    return result;
  }
  while (this_file.available()) {
      result += (char)this_file.read();
  }
  
  this_file.close();
  return result;
}

bool writeConfigVarsToFile(String file_name, String contents) {
  File this_file = LittleFS.open(file_name, "w");
  
  if (!this_file) {
    Serial.println("[STATUS] - Failed to open configuration file");
    return false;
  }

  int bytesWritten = this_file.print(contents);

  if (bytesWritten == 0) {
      Serial.println("[STATUS] - Failed to write to configuration file");
      return false;
  }
   
  this_file.close();
  return true;
}

void parseSettings(String settings) {
  String savedStatus = settings.substring(0, 7);     // Winding || Stopped = 7char
  String savedTPD = settings.substring(8, 11);       // min = 100 || max = 960
  String savedDirection = settings.substring(12);    // CW || CCW || BOTH

  status = savedStatus;
  rotationsPerDay = savedTPD;
  direction = savedDirection;
}


void startWebserver() { 

  server.on("/api/time", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", getTextFormatedTime());
  });

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
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
      Serial.printf("RECEIVED[%s]: %s\n", p->name().c_str(), p->value().c_str());

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

      String configs = status + "," + rotationsPerDay + "," + direction;

      bool writeSuccess = writeConfigVarsToFile(settingsFile, configs);

      if ( !writeSuccess ) {
        request->send(500);
      }
    

      request->send(204);
  }});

  server.on("/api/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("[STATUS] - Received reset command");
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
    Serial.println("[STATUS] - An error has occurred while mounting LittleFS");
  }
  Serial.println("[STATUS] - LittleFS mounted");
}

void saveWifiCallback() {
  // Slow blink to confirm success & restart
  for ( int i = 0; i < 6; i++ ) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
  ESP.restart();
}

 
void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  pinMode (LED_BUILTIN, OUTPUT);

  // WiFi Manager config    
  wm.setConfigPortalTimeout(60);
  wm.setDarkMode(true);
  wm.setConfigPortalBlocking(false);
  wm.setHostname("Winderoo");
  wm.setSaveConfigCallback(saveWifiCallback);
  
  // Connect using saved credentials, if they exist
  // If connection fails, start setup Access Point
  if (wm.autoConnect("Winderoo Setup")) {
    initFS();
    Serial.println("[STATUS] - connected to saved network");

    // retrieve & read saved settings
    String savedSettings = loadConfigVarsFromFile(settingsFile);
    parseSettings(savedSettings);
    
    if (!MDNS.begin("winderoo")) {
      Serial.println("[STATUS] - Failed to start mDNS");
    }
    Serial.println("[STATUS] - mDNS started");

    startWebserver();
  } else {
    Serial.println("[STATUS] - WiFi Config Portal running");
    digitalWrite(LED_BUILTIN, HIGH);
  };
}
 
void loop() {
  if (reset) {
    for ( int i = 0; i < 40; i++ ) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }

    Serial.println("[STATUS] - Stopping webserver");
    server.end();
    delay(600);
    Serial.println("[STATUS] - Stopping File System");
    LittleFS.end();
    delay(200);
    Serial.println("[STATUS] - Resetting Wifi Manager settings");
    wm.resetSettings();
    delay(200);
    Serial.println("[STATUS] - Restart device...");
    ESP.restart();
    delay(2000);
  }

  wm.process();
}
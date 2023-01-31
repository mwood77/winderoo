#include <Arduino.h>
#include <WiFiManager.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "FS.h"
#include "ESPAsyncWebServer.h"

HTTPClient http;
WiFiClient client;

WiFiManager wm;
AsyncWebServer server(80);

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");

            Serial.print(file.name());
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);

            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");

            Serial.print(file.size());
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
        }
        file = root.openNextFile();
    }
}

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

  String usage = "Watcheroo is live!";

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
    request->send(404, "text/plain", "Watcheroo\n\n404 - Resource Not found");
}

// /**
//  * Sets up all REST endpoints and their responses.
//  * Angular artifacts are treated specially here to allow pre-compression.
//  */
void startWebserver() {
  /*
  Server reqeusts for Angular artifacts
  Hardcoded requests for all requested angular files.
  */

  // TODO: files don't appear to exist on mount
  // server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");    

  server.serveStatic("/css/", LittleFS, "/css/").setCacheControl("max-age=31536000");
  server.serveStatic("/js/", LittleFS, "/js/").setCacheControl("max-age=31536000");
  server.serveStatic("/favicon.ico", LittleFS, "/favicon.ico").setCacheControl("max-age=31536000");
  server.serveStatic("/icon.jpeg", LittleFS, "/icon.jpeg").setCacheControl("max-age=31536000");

  server.serveStatic("/", LittleFS, "/")
      .setDefaultFile("index.html");
 
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(LittleFS, "/www/index.html", "text/html");
  // });
  
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(LittleFS, "/www/index.html", "text/html");
  // });

  // //@todo - serves "Not Found"
  // webServer.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(SPIFFS, "/www/index.html", "text/html");
  // });
  // webServer.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(SPIFFS, "/www/favicon.ico", "text/html");
  // });
  // webServer.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/styles.css.gz", "text/css");
  //   response->addHeader("Content-Encoding", "gzip");
  //   request->send(response);
  // });
  // webServer.on("/runtime.js", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/runtime.js.gz", "application/javascript");
  //   response->addHeader("Content-Encoding", "gzip");
  //   request->send(response);
  // });
  // webServer.on("/polyfills.js", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/polyfills.js.gz", "application/javascript");
  //   response->addHeader("Content-Encoding", "gzip");
  //   request->send(response);
  // });
  // webServer.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/main.js.gz", "application/javascript");
  //   response->addHeader("Content-Encoding", "gzip");
  //   request->send(response);
  // });

  /*
  API: System
  */
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", getTime());
  });

  server.on("/api/system/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", getFullMemoryUsage());
  });



  // server.serveStatic("/fs", LittleFS, "/");
  
  server.onNotFound(notFound);

  server.begin();
}


// // Initialize File System with LittleFS
void initFS() {

  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }

  Serial.println("LittleFS mounted successfully");
  listDir(LittleFS, "/", 0);
}


 
void setup() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP    
    // put your setup code here, to run once:
    Serial.begin(115200);
    initFS();
    
    //reset settings - wipe credentials for testing
    // wm.resetSettings();

    wm.setConfigPortalBlocking(false);
    //automatically connect using saved credentials if they exist
    //If connection fails it starts an access point with the specified name
    if(wm.autoConnect("Watcheroo Setup")) {
        Serial.println("connected to saved network");
        startWebserver();
    }
    else {
        Serial.println("WiFi Config Portal running");
    }
}
 
void loop() {
    wm.process();
    // put your main code here, to run repeatedly:
}
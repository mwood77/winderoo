#include <Arduino.h>
#include "WiFi.h"
#include "FS.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "configurables.h"

AsyncWebServer webServer(80);

String getFullMemoryUsage()
{
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

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

/**
 * Sets up all REST endpoints and their responses.
 * Angular artifacts are treated specially here to allow pre-compression.
 */
void startWebserver()
{
  /*
  Server reqeusts for Angular artifacts
  Hardcoded requests for all requested angular files.
  TODO: Replace with prezip-handler for all root file requests
  */

  //@todo - redirect does not work
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/www/index.html", "text/html");
  });

  //@todo - serves "Not Found"
  webServer.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/www/index.html", "text/html");
  });
  webServer.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/www/favicon.ico", "text/html");
  });
  webServer.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/styles.css.gz", "text/css");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  webServer.on("/runtime.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/runtime.js.gz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  webServer.on("/polyfills.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/polyfills.js.gz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  webServer.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/www/main.js.gz", "application/javascript");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  /*
  API: System
  */
  webServer.on("/api/system/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", getFullMemoryUsage());
  });

  webServer.onNotFound(notFound);

  webServer.begin();
}

boolean initWifi()
{
  WiFi.begin(ssid, password);
  long wifiConnStartMS = millis();
  Serial.printf("\nConnecting to: %s...",ssid);

  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(200);
      if( millis() - wifiConnStartMS >= 5000 ){
          Serial.println("\nTimeout on wifi connection. Wrong credentials?");
          return false; 
      }
  }
  Serial.printf("\nWiFi Connected to: %s, wih ip: ", ssid);
  Serial.print(WiFi.localIP());
  return true;
}


void setup()
{
  Serial.begin(115200);
  
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  if( initWifi()) {
    startWebserver();
  };
}


void loop() { 
  
}

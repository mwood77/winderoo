#include <Arduino.h>
#include <WiFiManager.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <ESP32Time.h>

#include "./utils/LedControl.h"
#include "./utils/MotorControl.h"

#include "FS.h"
#include "ESPAsyncWebServer.h"

// *************************************************************************************
// ********************************* CONFIGURABLES *************************************
// *************************************************************************************
/*
* durationInSecondsToCompleteOneRevolution = how long it takes the watch to complete one rotation on the winder. If you purchased the motor listed in the guide / Bill Of Materials, then this default value is correct!
* directionalPinA = this is the pin that's wired to IN1 on your L298N circuit board
* directionalPinB = this is the pin that's wired to IN2 on your L298N circuit board
* ledPin = by default this is set to the ESP32's onboard LED. If you've wired an external LED, change this value to the GPIO pin the LED is wired to.
*/
int durationInSecondsToCompleteOneRevolution = 8;
int directionalPinA = 25;
int directionalPinB = 26;
int ledPin = 0;

/*
* DO NOT CHANGE THESE VARIABLES!
*/
String timeURL = "http://worldtimeapi.org/api/ip";
String settingsFile = "/settings.txt";
unsigned long rtc_offset;
unsigned long rtc_epoch;
unsigned long estimatedRoutineFinishEpoch;
unsigned long previousEpoch;
unsigned long startTimeEpoch;
bool reset = false;
bool routineRunning = false;
struct RUNTIME_VARS {
  String status = "";
  String rotationsPerDay = "";
  String direction = "";
  String hour = "00";
  String minutes = "00";
  String winderEnabled = "1";
};

/*
* DO NOT CHANGE THESE VARIABLES!
*/
RUNTIME_VARS userDefinedSettings;
LedControl LED(ledPin);
MotorControl motor(directionalPinA, directionalPinB);
WiFiManager wm;
AsyncWebServer server(80);
HTTPClient http;
WiFiClient client;
ESP32Time rtc;

unsigned long calculateWindingTime() {
  int tpd = atoi(userDefinedSettings.rotationsPerDay.c_str());

  long totalSecondsSpentTurning = tpd * durationInSecondsToCompleteOneRevolution;
  
  // We want to rest every 3 minutes for 15 seconds
  long totalNumberOfRestingPeriods = totalSecondsSpentTurning / 180;
  long totalRestDuration = totalNumberOfRestingPeriods * 180;
  long finalRoutineDuration = totalRestDuration + totalSecondsSpentTurning;

  Serial.print("[STATUS] - Total winding duration: ");
  Serial.println(finalRoutineDuration);

  unsigned long epoch = rtc.getEpoch();
  unsigned long estimatedFinishTime = epoch + finalRoutineDuration;

  return estimatedFinishTime;
}

void beginWindingRoutine() {
  startTimeEpoch = rtc.getEpoch();
  previousEpoch = startTimeEpoch;
  routineRunning = true;
  userDefinedSettings.status = "Winding";
  Serial.println("[STATUS] - Begin winding routine");

  unsigned long finishTime = calculateWindingTime();
  estimatedRoutineFinishEpoch = finishTime;

  Serial.print("[STATUS] - Current time: ");
  Serial.println(rtc.getEpoch());

  Serial.print("[STATUS] - Estimated finish time: ");
  Serial.println(finishTime);
}

void getTime() {
  http.begin(client, timeURL);
  int httpCode = http.GET();

  if (httpCode > 0) {
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, http.getStream());
    const unsigned long epoch = doc["unixtime"];
    const unsigned long offset = doc["raw_offset"];

    rtc.offset = offset;
    rtc.setTime(epoch);
  }

  http.end();
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
  String savedStatus = settings.substring(0, 7);    // Winding || Stopped = 7char
  String savedTPD = settings.substring(8, 11);      // min = 100 || max = 960
  String savedHour = settings.substring(12, 14);    // 00
  String savedMinutes = settings.substring(15, 17); // 00
  String savedDirection = settings.substring(18);   // CW || CCW || BOTH

  userDefinedSettings.status = savedStatus;
  userDefinedSettings.rotationsPerDay = savedTPD;
  userDefinedSettings.hour = savedHour;
  userDefinedSettings.minutes = savedMinutes;
  userDefinedSettings.direction = savedDirection;
}


void startWebserver() { 

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    json["status"] = userDefinedSettings.status;
    json["rotationsPerDay"] = userDefinedSettings.rotationsPerDay;
    json["direction"] = userDefinedSettings.direction;
    json["hour"] = userDefinedSettings.hour;
    json["minutes"] = userDefinedSettings.minutes;
    json["durationInSecondsToCompleteOneRevolution"] = durationInSecondsToCompleteOneRevolution;
    json["startTimeEpoch"] = startTimeEpoch;
    json["currentTimeEpoch"] = rtc.getEpoch();
    json["estimatedRoutineFinishEpoch"] = estimatedRoutineFinishEpoch;
    json["winderEnabled"] = userDefinedSettings.winderEnabled;
    json["db"] = WiFi.RSSI();
    serializeJson(json, *response);

    request->send(response);

    // Update RTC time ref
    getTime();
  });

  server.on("/api/power", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    
    for ( int i = 0; i < params; i++ ) {
      AsyncWebParameter* p = request->getParam(i);

        if( strcmp(p->name().c_str(), "winderEnabled") == 0 ) {
          userDefinedSettings.winderEnabled = p->value().c_str();

          if (userDefinedSettings.winderEnabled == "0") {
            Serial.println("[STATUS] - Switched off!");
            userDefinedSettings.status = "Stopped";
            routineRunning = false;
            motor.stop();
          }
        }
    }
    
    request->send(204);
  });

  server.on("/api/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    
    for ( int i = 0; i < params; i++ ) {
      AsyncWebParameter* p = request->getParam(i);
    
        if( strcmp(p->name().c_str(), "rotationDirection") == 0 ) {
          userDefinedSettings.direction = p->value().c_str();

          motor.stop();
          delay(250);

          // Update motor direction
          if (userDefinedSettings.direction == "CW" ) {
            motor.setMotorDirection(1);
          } else if (userDefinedSettings.direction == "CCW") {
            motor.setMotorDirection(0);
          }

          Serial.println("[STATUS] - direction set: " + userDefinedSettings.direction);
        }
    
        if( strcmp(p->name().c_str(), "tpd") == 0 ) {
          const char* newTpd = p->value().c_str();

          if (strcmp(newTpd, userDefinedSettings.rotationsPerDay.c_str()) != 0) {
            userDefinedSettings.rotationsPerDay = p->value().c_str();

            unsigned long finishTime = calculateWindingTime();
            estimatedRoutineFinishEpoch = finishTime;
          }
        }

        if( strcmp(p->name().c_str(), "hour") == 0 ) {
          userDefinedSettings.hour = p->value().c_str();
        }

        if( strcmp(p->name().c_str(), "minutes") == 0 ) {
          userDefinedSettings.minutes = p->value().c_str();
        }

        if( strcmp(p->name().c_str(), "action") == 0) {
          if ( strcmp(p->value().c_str(), "START") == 0 ) {
            if (!routineRunning) {
              userDefinedSettings.status = "Winding";
              beginWindingRoutine();
            }
          } else {
            motor.stop();
            routineRunning = false;
            userDefinedSettings.status = "Stopped";
          }
        }
    }

    String configs = userDefinedSettings.status + "," + userDefinedSettings.rotationsPerDay + "," + userDefinedSettings.hour + "," + userDefinedSettings.minutes + "," + userDefinedSettings.direction;

    bool writeSuccess = writeConfigVarsToFile(settingsFile, configs);

    if ( !writeSuccess ) {
      request->send(500);
    }

    request->send(204);
  });

  server.on("/api/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("[STATUS] - Received reset command");
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    json["status"] = "Resetting";
    serializeJson(json, *response);
    request->send(response);
    
    reset = true;
  });


  server.serveStatic("/css/", LittleFS, "/css/").setCacheControl("max-age=31536000");
  server.serveStatic("/js/", LittleFS, "/js/").setCacheControl("max-age=31536000");
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

void triggerLEDCondition(int blinkState) {
  // remove any previous LED state (aka turn LED off)
  LED.off();
  delay(50);

  switch(blinkState) {
    case 1:
      LED.slowBlink();
      break;
    case 2:
      LED.fastBlink();
      break;
    case 3:
      LED.pwm();
      break;
    default:
      Serial.println("[WARN] - blinkState not recognized");
      break;
  }
}

void saveWifiCallback() {
  // slow blink to confirm connection success
  triggerLEDCondition(1);
  ESP.restart();
  delay(2000);
}
 
void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  setCpuFrequencyMhz(80);
  
  // Prepare pins
  pinMode(directionalPinA, OUTPUT);
  pinMode(directionalPinB, OUTPUT);
  ledcSetup(LED.getChannel(), LED.getFrequency(), LED.getResolution());
  ledcAttachPin(LED_BUILTIN, LED.getChannel());

  // WiFi Manager config    
  wm.setConfigPortalTimeout(3600);
  wm.setDarkMode(true);
  wm.setConfigPortalBlocking(false);
  wm.setHostname("Winderoo");
  wm.setSaveConfigCallback(saveWifiCallback);

  userDefinedSettings.winderEnabled = true;

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
    MDNS.addService("_winderoo", "_tcp", 80);
    Serial.println("[STATUS] - mDNS started");

    getTime();
    startWebserver();

    if (strcmp(userDefinedSettings.status.c_str(), "Winding") == 0) {
      beginWindingRoutine();
    }
  } else {
    Serial.println("[STATUS] - WiFi Config Portal running");
    ledcWrite(LED.getChannel(), 255);
  };
}
 
void loop() {

  if (reset) {
    // fast blink
    triggerLEDCondition(2);

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

  if (rtc.getHour(true) == userDefinedSettings.hour.toInt() && 
      rtc.getMinute() == userDefinedSettings.minutes.toInt() && 
      !routineRunning && 
      userDefinedSettings.winderEnabled == "1") {
    beginWindingRoutine();
  }

  if (routineRunning) {
    unsigned long currentTime = rtc.getEpoch();
    
    if (rtc.getEpoch() < estimatedRoutineFinishEpoch) {
      
      // turn motor in direction
      motor.determineMotorDirectionAndBegin();
      int r = rand() % 100;

      if (r <= 25) {
        if ((strcmp(userDefinedSettings.direction.c_str(), "BOTH") == 0) && (currentTime - previousEpoch) > 180) {
          motor.stop();
          delay(3000);

          previousEpoch = currentTime;

          int currentDirection = motor.getMotorDirection();
          motor.setMotorDirection(!currentDirection);
          Serial.println("[STATUS] - Motor changing direction, mode: " + userDefinedSettings.direction);
          
          motor.determineMotorDirectionAndBegin();
        } 
        
        if ((currentTime - previousEpoch) > 180) {
          Serial.println("[STATUS] - Pause");
          previousEpoch = currentTime;
          motor.stop();
          delay(3000);
        }
      }
    } else {
      // Routine has finished
      userDefinedSettings.status = "Stopped";
      routineRunning = false;
      motor.stop();
    }
  }
  
  if (userDefinedSettings.winderEnabled == "0") {
    // pulse LED
    triggerLEDCondition(3);
  }

  wm.process();
  delay(1000);
}

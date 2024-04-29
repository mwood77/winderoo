#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <ESP32Time.h>
#include <WiFiManager.h>
#include <QuickEspNow.h>
#include "soc/rtc_wdt.h"
#include <list>

#ifdef OLED_ENABLED
	#include <SPI.h>
	#include <Wire.h>
	#include <Adafruit_GFX.h>
	#include <Adafruit_SSD1306.h>
#endif

#include "./utils/LedControl.h"
#include "./utils/MotorControl.h"

#include "FS.h"
#include "ESPAsyncWebServer.h"
using namespace std;
/*
 * *************************************************************************************
 * ********************************* CONFIGURABLES *************************************
 * *************************************************************************************
 *
 * If you purchased the motor listed in the guide / Bill Of Materials, these default values are correct!
 *
 * durationInSecondsToCompleteOneRevolution = how long it takes the watch to complete one rotation on the winder.
 * directionalPinA = this is the pin that's wired to IN1 on your L298N circuit board
 * directionalPinB = this is the pin that's wired to IN2 on your L298N circuit board
 * ledPin = by default this is set to the ESP32's onboard LED. If you've wired an external LED, change this value to the GPIO pin the LED is wired to.
 * externalButton = OPTIONAL - If you want to use an external ON/OFF button, connect it to this pin 13. If you need to use another pin, change the value here.
 *
 * If you're using a NeoPixel equipped board, you'll need to change directionalPinA, directionalPinB and ledPin (pin 18 on most, I think) to appropriate GPIOs.
 * Faiulre to set these pins on NeoPixel boards will result in kernel panics.
 */
int durationInSecondsToCompleteOneRevolution = 8;
int directionalPinA = 25;
int directionalPinB = 26;
int ledPin = 0;
int externalButton = 13;

// OLED CONFIG
bool OLED_INVERT_SCREEN = false;
bool OLED_ROTATE_SCREEN_180 = false;
int SCREEN_WIDTH = 128; // OLED display width, in pixels
int SCREEN_HEIGHT = 64; // OLED display height, in pixels
int OLED_RESET = -1; // Reset pin number (or -1 if sharing Arduino reset pin)
/*
 * *************************************************************************************
 * ******************************* END CONFIGURABLES ***********************************
 * *************************************************************************************
 */

/*
 * DO NOT CHANGE THESE VARIABLES!
 */
String timeURL = "http://worldtimeapi.org/api/ip";
String settingsFile = "/settings.json";
unsigned long rtc_offset;
unsigned long rtc_epoch;
unsigned long estimatedRoutineFinishEpoch;
unsigned long previousEpoch;
unsigned long startTimeEpoch;
bool reset = false;
bool routineRunning = false;
bool configPortalRunning = false;
bool screenSleep = false;
bool screenEquipped = OLED_ENABLED;
String rootMac;
bool selfIsChildNode = false;
bool meshEnabled = false;
String knownMacs[6] = {"", "", "", "", "", ""};
bool meshMessageReady = false;
bool meshMessageSent = true;

struct RUNTIME_VARS
{
	String status = "";
	String rotationsPerDay = "";
	String direction = "";
	String hour = "00";
	String minutes = "00";
	String winderEnabled = "1";
	String timerEnabled = "0";
};
struct MESH_MESSAGE
{
	String destinationMacAddress = "";
	String rootMessage = "";
	bool forRoot = false;
	String status = "";
	String rotationsPerDay = "";
	String direction = "";
	String hour = "00";
	String minutes = "00";
	String screenSleep = "1";
	String timerEnabled = "0";
	String screenEquipped;
	int db = 0;
};

struct NODE
{
	String nodeMac = "";
	String nodeStatus = "";
	String nodeTPD = "";
	String nodeDirection = "";
	String nodeHour = "";
	String nodeMinutes = "";
	String nodeScreenSleep = "";
	String nodeTimerEnabled = "";
	String nodeScreenEquipped = "";
	int nodeDb = 0;

};

/*
 * DO NOT CHANGE THESE VARIABLES!
 */
RUNTIME_VARS userDefinedSettings;
MESH_MESSAGE meshMessage;
LedControl LED(ledPin);
MotorControl motor(directionalPinA, directionalPinB);
WiFiManager wm;
AsyncWebServer server(80);
HTTPClient http;
WiFiClient client;
ESP32Time rtc;
list<NODE> nodes;

#ifdef OLED_ENABLED
	Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif


void drawCentreStringToMemory(const char *buf, int x, int y)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, 0, y, &x1, &y1, &w, &h); //calc width of new string
    display.setCursor(x - (w / 2), y);
    display.print(buf);
}

static void drawStaticGUI(bool drawHeaderTitle = false, String title = "Winderoo") {
	if (OLED_ENABLED)
	{
		display.clearDisplay();

		display.setTextSize(1);
		display.setTextColor(WHITE);

		if (drawHeaderTitle)
		{
			drawCentreStringToMemory(title.c_str(), 64, 3);
		}
		// top horizontal line
		display.drawLine(0, 14, display.width(), 14, WHITE);
		// vertical line
		display.drawLine(64, 14, 64, 50, WHITE);
		// bottom horizontal line
		display.drawLine(0, 50, display.width(), 50, WHITE);

		display.setCursor(4, 18);
		display.println(F("TPD"));

		display.setCursor(71, 18);
		display.println(F("DIR"));

		display.display();
	}
}

static void drawTimerStatus() {
	if (OLED_ENABLED)
	{
		if (userDefinedSettings.timerEnabled == "1")
		{
			// right aligned timer
			display.fillRect(60, 51, 64, 13, BLACK);
			display.setCursor(60, 56);
			display.print("TIMER " + userDefinedSettings.hour + ":" + userDefinedSettings.minutes);
		}
		else
		{
			display.fillRect(60, 51, 68, 13, BLACK);
		}
	}
}

static void drawMeshStatus() {
	if (OLED_ENABLED)
	{
		if (meshEnabled)
		{

			int count = 0;
			for ( String item : knownMacs )
			{
				if ( item != "" ) // This is a filter
					++count;
			}

			display.fillRect(30, 51, 12, 13, BLACK);
			display.drawCircle(38, 56, 1, WHITE);
			display.drawCircle(34, 59, 1, WHITE);
			display.drawCircle(38, 62, 1, WHITE);
			display.setCursor(43, 56);
			display.print(count);
		}
		else
		{
			display.fillRect(30, 51, 12, 13, BLACK);
		}
	}
}

static void drawWifiStatus() {
	if (OLED_ENABLED)
	{
		// left aligned cell reception icon
		display.drawTriangle(4, 54, 10, 54, 7, 58, WHITE);
		display.drawLine(7, 58, 7, 62, WHITE);

		// Clear reception bars
		display.fillRect(12, 54, 58, 10, BLACK);

		if (WiFi.RSSI() > -50)
		{
			// Excelent reception - 4 bars
			display.fillRect(14, 55+8, 2, 2, WHITE);
			display.fillRect(18, 55+6, 2, 4, WHITE);
			display.fillRect(22, 55+4, 2, 6, WHITE);
			display.fillRect(26, 55+2, 2, 8, WHITE);
		}
		else if (WiFi.RSSI() > -60)
		{
			// Good reception - 3 bars
			display.fillRect(14, 55+8, 2, 2, WHITE);
			display.fillRect(18, 55+6, 2, 4, WHITE);
			display.fillRect(22, 55+4, 2, 6, WHITE);
		}
		else if (WiFi.RSSI() > -70)
		{
			// Fair reception - 2 bars
			display.fillRect(14, 55+8, 2, 2, WHITE);
			display.fillRect(18, 55+6, 2, 4, WHITE);
		}
		else
		{
			// Terrible reception - 1 bar
			display.fillRect(14, 55+8, 2, 2, WHITE);
		}
	}
}

static void drawDynamicGUI() {
	if (OLED_ENABLED && !screenSleep)
	{

		display.fillRect(8, 25, 54, 25, BLACK);
		display.setCursor(8, 30);
		display.setTextSize(2);
		display.print(userDefinedSettings.rotationsPerDay);

		display.fillRect(66, 25, 62, 25, BLACK);
		display.setCursor(74, 30);
		display.print(userDefinedSettings.direction);
		display.setTextSize(1);

		drawWifiStatus();
		drawTimerStatus();
		drawMeshStatus();

		display.display();
	}
}

static void drawNotification(String message) {
	if (OLED_ENABLED && !screenSleep)
	{
		display.setCursor(0, 0);
		display.drawRect(0, 0, 128, 14, WHITE);
		display.fillRect(0, 0, 128, 14, WHITE);
		display.setTextColor(BLACK);
		drawCentreStringToMemory(message.c_str(), 64, 3);
		display.display();
		display.setTextColor(WHITE);
		delay(200);
		display.setCursor(0, 0);
		display.drawRect(0, 0, 128, 14, BLACK);
		display.fillRect(0, 0, 128, 14, BLACK);
		display.setTextColor(WHITE);
		drawCentreStringToMemory(message.c_str(), 64, 3);

		// Underline notification, which is shared with Static GUI
		display.drawLine(0, 14, display.width(), 14, WHITE);
		display.display();
	}
}

template <int N> static void drawMultiLineText(const String (&message)[N]) {
	if (OLED_ENABLED && !screenSleep)
	{
		int yInitial = 20;
		int yOffset = 16;

		display.fillRect(0, 18, 128, 64, BLACK);

		for (int i = 0; i < N; i++)
		{
			if (i == 0)
			{
				drawCentreStringToMemory(message[i].c_str(), 64, yInitial);
			}
			else
			{
				drawCentreStringToMemory(message[i].c_str(), 64, yInitial + (yOffset * i));
			}
		}
	display.display();
	}
}

/**
 * Calclates the duration and estimated finish time of the winding routine
 *
 * @return epoch - estimated epoch when winding routine will finish
 */
unsigned long calculateWindingTime()
{
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

/**
 * Sets running conditions to TRUE & calculates winding time parameters
 */
void beginWindingRoutine()
{
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

	drawNotification("Winding");
}

/**
 * Calls external time API & updates ESP32's onboard real time clock
 */
void getTime()
{
	http.begin(client, timeURL);
	int httpCode = http.GET();

	if (httpCode > 0)
	{
		JsonDocument json;
		deserializeJson(json, http.getStream());
		const String datetime = json["datetime"];

		String date = datetime.substring(0, datetime.indexOf("T") - 1);
		int day = date.substring(8, 10).toInt();
		int month = date.substring(5, 7).toInt();
		int year = date.substring(0, 4).toInt();

		String time = datetime.substring(datetime.indexOf("T") + 1);
		int seconds = time.substring(6, 8).toInt();
		int hours = time.substring(0, 2).toInt();
		int minutes = time.substring(3, 5).toInt();

		rtc.setTime(seconds, minutes, hours, day, month, year);
	}
	else
	{
		Serial.println("[ERROR] - Failed to get time from Worldtime API");
	}

	http.end();
}

/**
 * Loads user defined settings from data file
 *
 * @param file_name fully qualified name of file to load
 * @return contents of file as a single string
 */
void loadConfigVarsFromFile(String file_name)
{
	String result = "";

	File this_file = LittleFS.open(file_name, "r");

	JsonDocument json;
	DeserializationError error = deserializeJson(json, this_file);

	if (!this_file || error)
	{
		Serial.println("[STATUS] - Failed to open configuration file, returning empty result");
	}
	while (this_file.available())
	{
		result += (char)this_file.read();
	}

	userDefinedSettings.status = json["savedStatus"].as<String>();						// Winding || Stopped = 7char
	userDefinedSettings.rotationsPerDay = json["savedTPD"].as<String>();				// min = 100 || max = 960
	userDefinedSettings.hour = json["savedHour"].as<String>();							// 00
	userDefinedSettings.minutes = json["savedMinutes"].as<String>();					// 00
	userDefinedSettings.timerEnabled = json["savedTimerState"].as<String>();			// 0 || 1
	userDefinedSettings.direction = json["savedDirection"].as<String>();				// CW || CCW || BOTH

	this_file.close();
}

/**
 * Saves user defined settings to data file
 *
 * @param file_name fully qualified name of file to save data to
 * @param contents entire contents to write to file
 * @return true if successfully wrote to file; else false
 */
bool writeConfigVarsToFile(String file_name, const RUNTIME_VARS& userDefinedSettings)
{
	File this_file = LittleFS.open(file_name, "w");

	JsonDocument json;

	if (!this_file)
	{
		Serial.println("[STATUS] - Failed to open configuration file");
		return false;
	}

	json["savedStatus"] = userDefinedSettings.status;
	json["savedTPD"] = userDefinedSettings.rotationsPerDay;
	json["savedHour"] = userDefinedSettings.hour;
	json["savedMinutes"] = userDefinedSettings.minutes;
	json["savedTimerState"] = userDefinedSettings.timerEnabled;
	json["savedDirection"] = userDefinedSettings.direction;

	if (serializeJson(json, this_file) == 0)
	{
		Serial.println("[STATUS] - Failed to write to configuration file");
		return false;
	}

	this_file.close();
	return true;
}

/**
 * 404 handler for webserver
 */
void notFound(AsyncWebServerRequest *request)
{
	// Handle HTTP_OPTIONS requests
	if (request->method() == 64)
	{
		AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Ok");
		request->send(response);
	}
	else
	{
		request->send(404, "text/plain", "Winderoo\n\n404 - Resource Not found");
	}
}

void registerSelfAsChildNode(char* localMacString) {
	http.begin(client, "http://winderoo.local/api/winder");
	http.addHeader("Content-Type", "application/json");

	JsonDocument jsonPut;
	jsonPut["localNode"] = localMacString;
	int httpCode = http.POST("{\"localNode\":\"" + (String)localMacString + "\"}");

	if (httpCode == 200)
	{
		JsonDocument json;
		deserializeJson(json, http.getStream());
		const char* rootNode = json["rootNode"];
		Serial.println("[STATUS] - received root node mac: " + (String)rootNode);
		rootMac = rootNode;

		http.end();
		Serial.println("[STATUS] - root node mac saved");

		// set mesh message body with winder body and set Message Ready flag true
		meshMessage.rootMessage = "node_status";
		meshMessage.forRoot = true;
		meshMessage.direction = userDefinedSettings.direction;
		meshMessage.rotationsPerDay = userDefinedSettings.rotationsPerDay;
		meshMessage.status = userDefinedSettings.status;
		meshMessage.hour = userDefinedSettings.hour;
		meshMessage.minutes = userDefinedSettings.minutes;
		meshMessage.screenSleep = screenSleep;
		meshMessage.timerEnabled = userDefinedSettings.timerEnabled;
		meshMessage.rotationsPerDay = userDefinedSettings.rotationsPerDay;
		meshMessage.screenEquipped = screenEquipped;
		meshMessage.db = WiFi.RSSI();

		meshMessageReady = true;
	}
	else
	{
		http.end();
		Serial.println("[ERROR] - Failed to add local node mac to root node");
		Serial.println("[ERROR] - Retrying in 2 seconds...");
		delay(2000);
		registerSelfAsChildNode(localMacString);
	}
}

/**
 * Handle mesh messages
 */
void meshResetMessageBody() {
	meshMessage.destinationMacAddress = "";
	meshMessage.rootMessage = "";
	meshMessage.forRoot = false;
	meshMessage.status = "";
	meshMessage.rotationsPerDay = "";
	meshMessage.direction = "";
	meshMessage.hour = "00";
	meshMessage.minutes = "00";
	meshMessage.screenSleep = "1";
	meshMessage.timerEnabled = "0";
}

byte* getMacAddress(String macAddress) {
	byte* nodeMac = new byte[6];
	String hexByte;
	int j = 0;
	for (int i = 0; i < macAddress.length(); i++) {
		if (macAddress.charAt(i) != ':') {
			hexByte += macAddress.charAt(i);
			if (hexByte.length() == 2) {
				nodeMac[j] = strtol(hexByte.c_str(), NULL, 16);
				hexByte = "";
				j++;
			}
		}
	}

	return nodeMac;
}


void parseReceivedMessageBody(String address, uint8_t* data, uint8_t len) 
{
    JsonDocument doc;
    deserializeJson(doc, data);
	DeserializationError error = deserializeJson(doc, data);
	if (error)
	{
		Serial.println("[ERROR] - Failed to deserialize [mesh] received body");
		return;
	}

	// We assume the message is an child update message
	if (doc["forRoot"].as<bool>()) 
	{
		Serial.println("[STATUS] - Adding child state to nodes list");
		NODE node;
		node.nodeMac = address;
		node.nodeStatus = doc["status"].as<String>();
		node.nodeTPD = doc["rotationsPerDay"].as<String>();
		node.nodeDirection = doc["direction"].as<String>();
		node.nodeHour = doc["hour"].as<String>();
		node.nodeMinutes = doc["minutes"].as<String>();
		node.nodeScreenSleep = doc["screenSleep"].as<String>();
		node.nodeTimerEnabled = doc["timerEnabled"].as<String>();
		node.nodeScreenEquipped = doc["screenEquipped"].as<String>();
		node.nodeDb = doc["db"].as<int>();
		nodes.push_back(node);
		return;
	}

	// tx_status = root node requesting winder state
	if (doc["rootMessage"].as<String>() != "tx_status")
	{
		Serial.println("[STATUS] - Updating local state with received message");
		userDefinedSettings.status = doc["status"].as<String>();
		userDefinedSettings.rotationsPerDay = doc["rotationsPerDay"].as<String>();
		userDefinedSettings.direction = doc["direction"].as<String>();
		userDefinedSettings.hour = doc["hour"].as<String>();
		userDefinedSettings.minutes = doc["minutes"].as<String>();
		screenSleep = doc["screenSleep"].as<String>();
		userDefinedSettings.timerEnabled = doc["timerEnabled"].as<String>();
		return;
	}
}

void meshDataReceived(uint8_t* address, uint8_t* data, uint8_t len, signed int rssi, bool broadcast) {
	Serial.println();
    Serial.print ("Received: ");
    Serial.printf ("%.*s\n", len, data);
    Serial.printf ("RSSI: %d dBm\n", rssi);
    Serial.printf ("From: " MACSTR "\n", MAC2STR (address));
    Serial.printf ("%s\n", broadcast ? "Broadcast" : "Unicast");
	Serial.println();

	String macAddress;
	for (byte i = 0; i < 6; ++i)
	{
		char buf[3];
		sprintf(buf, "%2X", address[i]);
		macAddress += buf;
		if (i < 5) macAddress += ':';
	}

	parseReceivedMessageBody(macAddress, data, len);

}

void meshBroadcastMessage(String message) {
	int retries = 0;
	Serial.println("[STATUS] - Broadcasting message: " + message);
	if (!quickEspNow.send(ESPNOW_BROADCAST_ADDRESS, (uint8_t*)message.c_str(), message.length()))
	{
		Serial.println("[STATUS] - >>>>>>>>>> Broadcast message sent");
		meshMessageSent = true;
		meshResetMessageBody();
	}
	else if (retries < 3)
	{
		Serial.println("[WARN] - >>>>>>>>>> Broadcast message failed to send");
		Serial.println("[WARN] - Retrying...: " + retries);
		delay(500);
		retries++;
		meshBroadcastMessage(message);
	}
	else
	{
		retries = 0;
		meshMessageSent = true;
		Serial.println("[ERROR] - meshBroadcastMessage - Failed to broadcast message");
	}
}

void meshMessageRoot(String message) {
	Serial.println("[STATUS] - Sending message to root node:" + message);
	int retries = 0;
	byte* nodeMac = new byte[6];
	String hexByte;
	int j = 0;
	for (int i = 0; i < rootMac.length(); i++) {
		if (rootMac.charAt(i) != ':') {
			hexByte += rootMac.charAt(i);
			if (hexByte.length() == 2) {
				nodeMac[j] = strtol(hexByte.c_str(), NULL, 16);
				hexByte = "";
				j++;
			}
		}
	}

    if (!quickEspNow.send(nodeMac, (uint8_t*)message.c_str(), message.length()))
	{
        Serial.println("[STATUS] - >>>>>>>>>> Sent message to Root node: " + rootMac);
		meshMessageSent = true;
		meshResetMessageBody();
    }
	else if (retries < 3)
	{
        Serial.println("[WARN] - >>>>>>>>>> Failed to send message to root node: " + rootMac);
        Serial.println("[WARN] - Retrying...: " + retries);
		delay(127);
		retries++;
		meshMessageRoot(message);
    }
	else 
	{
		retries = 0;
		meshMessageSent = true;
		Serial.println("[ERROR] - meshMessageRoot - Failed to send message to node: " + rootMac);
	}
}

void requestNodeStatus(String macAddress, String message) {
	int retries = 0;
	Serial.println("[STATUS] - Requesting status of node: " + macAddress);
	byte* nodeMac = new byte[6];
	String hexByte;
	int j = 0;
	for (int i = 0; i < macAddress.length(); i++) {
		if (macAddress.charAt(i) != ':') {
			hexByte += macAddress.charAt(i);
			if (hexByte.length() == 2) {
				nodeMac[j] = strtol(hexByte.c_str(), NULL, 16);
				hexByte = "";
				j++;
			}
		}
	}

	if (!quickEspNow.send(nodeMac, (uint8_t*)message.c_str(), message.length()))
	{
        Serial.println("[STATUS] - >>>>>>>>>> Message sent to node: " + macAddress);
		meshMessageSent = true;
		meshResetMessageBody();
    }
	else if (retries < 3)
	{
        Serial.println("[WARN] - >>>>>>>>>> Message failed to send to node: " + macAddress);
		Serial.println("[WARN] - Retrying...: " + retries);
		delay(320);
		retries++;
		requestNodeStatus(macAddress, message);
    }
	else 
	{
		retries = 0;
		meshMessageSent = true;
		Serial.println("[ERROR] - requestNodeStatus - Failed to send message to node: " + macAddress);
	}
}

/**
 * API for front end
 */
void startWebserver()
{

	server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
	{
		AsyncResponseStream *response = request->beginResponseStream("application/json");
		JsonDocument json;
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
		json["timerEnabled"] = userDefinedSettings.timerEnabled;
		json["db"] = WiFi.RSSI();
		json["screenSleep"] = screenSleep;
		json["screenEquipped"] = screenEquipped;
		
		// Add known nodes to response
		JsonArray meshNodes = json["networkedUnits"].to<JsonArray>();
		for (NODE node : nodes)
		{
			JsonObject nodeData;
			nodeData["address"] = node.nodeMac;
			nodeData["status"] = node.nodeStatus;
			nodeData["nodeDirection"] = node.nodeDirection;
			nodeData["rotationsPerDay"] = node.nodeTPD;
			nodeData["hour"] = node.nodeHour;
			nodeData["minutes"] = node.nodeMinutes;
			nodeData["timerEnabled"] = node.nodeTimerEnabled;
			nodeData["screenSleep"] = node.nodeScreenSleep;
			nodeData["screenEquipped"] = node.nodeScreenEquipped;
			nodeData["db"] = node.nodeDb;
			meshNodes.add(meshNodes);
		}
		
		// @todo - gitting hit by watchdog here
		serializeJson(json, *response);

		request->send(response);

		// Update RTC time ref
		getTime();
	});

	server.on("/api/timer", HTTP_POST, [](AsyncWebServerRequest *request)
	{
		int params = request->params();

		for ( int i = 0; i < params; i++ )
		{
			AsyncWebParameter* p = request->getParam(i);

			if( strcmp(p->name().c_str(), "timerEnabled") == 0 )
			{
				userDefinedSettings.timerEnabled = p->value().c_str();
			}
		}

		bool writeSuccess = writeConfigVarsToFile(settingsFile, userDefinedSettings);
		if ( !writeSuccess )
		{
			Serial.println("[ERROR] - Failed to write [timer] endpoint data to file");
			request->send(500, "text/plain", "Failed to write new configuration to file");
		}

		request->send(204);
	});

	server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
	{

		if (request->url() == "/api/power")
		{
			JsonDocument json;
			DeserializationError error = deserializeJson(json, data);

			if (error)
			{
				Serial.println("[ERROR] - Failed to deserialize [power] request body");
				request->send(500, "text/plain", "Failed to deserialize request body");
				return;
			}

			if (!json.containsKey("winderEnabled"))
			{
				request->send(400, "text/plain", "Missing required field: 'winderEnabled'");
			}

			userDefinedSettings.winderEnabled = json["winderEnabled"].as<String>();

			if (userDefinedSettings.winderEnabled == "0")
			{
				Serial.println("[STATUS] - Switched off!");
				userDefinedSettings.status = "Stopped";
				routineRunning = false;
				motor.stop();
				display.clearDisplay();
				display.display();
			} else {
				drawStaticGUI(true);
				drawDynamicGUI();
			}

			request->send(204);
		}

		if (request->url() == "/api/update")
		{
			JsonDocument json;
			DeserializationError error = deserializeJson(json, data);
			int arraySize = 7;
			String requiredKeys[arraySize] = {"rotationDirection", "tpd", "action", "hour", "minutes", "timerEnabled", "screenSleep"};

			if (error)
			{
				Serial.println("[ERROR] - Failed to deserialize [update] request body");
				request->send(500, "text/plain", "Failed to deserialize request body");
				return;
			}

			// validate request body
				for (int i = 0; i < arraySize; i++)
				{
					if(!json.containsKey(requiredKeys[i]))
					{
						request->send(400, "text/plain", "Missing required field: '" + requiredKeys[i] +"'");
					}
				}

			// These values can be mutated / saved directly
			userDefinedSettings.hour = json["hour"].as<String>();
			userDefinedSettings.minutes = json["minutes"].as<String>();
			userDefinedSettings.timerEnabled = json["timerEnabled"].as<String>();

			// These values need to be compared to the current settings / running state
			String requestRotationDirection = json["rotationDirection"].as<String>();
			String requestTPD = json["tpd"].as<String>();
			String requestAction = json["action"].as<String>();
			screenSleep = json["screenSleep"].as<bool>();

			// Update motor direction
			if (strcmp(requestRotationDirection.c_str(), userDefinedSettings.direction.c_str()) != 0)
			{
				userDefinedSettings.direction = requestRotationDirection;
				motor.stop();
				delay(250);

				// Update motor direction
				if (userDefinedSettings.direction == "CW" )
				{
					motor.setMotorDirection(1);
				}
				else if (userDefinedSettings.direction == "CCW")
				{
					motor.setMotorDirection(0);
				}

				Serial.println("[STATUS] - direction set: " + userDefinedSettings.direction);
			}
			else
			{
				userDefinedSettings.direction = requestRotationDirection;
			}

			// Update (turns) rotations per day
			if (strcmp(requestTPD.c_str(), userDefinedSettings.rotationsPerDay .c_str()) != 0)
			{
				userDefinedSettings.rotationsPerDay = requestTPD;

				unsigned long finishTime = calculateWindingTime();
				estimatedRoutineFinishEpoch = finishTime;
			}

			// Update action (START/STOP)
			if ( strcmp(requestAction.c_str(), "START") == 0 )
			{
				if (!routineRunning)
				{
					userDefinedSettings.status = "Winding";
					beginWindingRoutine();
				}
			}
			else
			{
				motor.stop();
				routineRunning = false;
				userDefinedSettings.status = "Stopped";
				drawNotification("Stopped");
			}

			// Update screen sleep state
			if (screenSleep && OLED_ENABLED)
			{
				display.clearDisplay();
				display.display();
			}
			else
			{
				if (OLED_ENABLED)
				{
					// Draw gui with updated values from _this_ update request
					drawStaticGUI(true, userDefinedSettings.status);
					drawDynamicGUI();
				}
			}

			// Write new parameters to file
			bool writeSuccess = writeConfigVarsToFile(settingsFile, userDefinedSettings);
			if ( !writeSuccess )
			{
				Serial.println("[ERROR] - Failed to write [update] endpoint data to file");
				request->send(500, "text/plain", "Failed to write new configuration to file");
			}

			request->send(204);
		}

		if (request->url() == "/api/winder")
		{
			Serial.println("[STATUS] - Received add winder POST request");

			JsonDocument json;
			DeserializationError error = deserializeJson(json, data);
			String requiredKeys[1] = {"localNode"};

			if (error)
			{
				Serial.println("[ERROR] - Failed to deserialize [winder] request body");
				request->send(500, "text/plain", "Failed to deserialize request body");
				return;
			}

			String childNode = json["localNode"].as<String>();
			if (childNode == "")
			{
				request->send(400, "text/plain", "Missing required field: 'localNode'");
			}
			else
			{
				Serial.println("[STATUS] - Adding child node: " + childNode);
				// Macs array has a max size of 6; add to array if not already present
				for (int i = 0; i < 6; i++)
				{
					if (knownMacs[i] == childNode)
					{
						break;
					}
					
					if (knownMacs[i] == "")
					{
						knownMacs[i] = childNode;
						break;
					}
					Serial.print(knownMacs[i] + ", ");
				}

				Serial.print("[STATUS] - known macs: ");
				for (int i = 0; i < 6; i++)
				{
					Serial.print(knownMacs[i] + ", ");
				}
				Serial.println();

				AsyncResponseStream *response = request->beginResponseStream("application/json");
				JsonDocument json;
				json["rootNode"] = WiFi.macAddress();
				serializeJson(json, *response);
				request->send(response);

				// set mesh message body and set meshMessageReady flag for loop execution
				meshMessage.forRoot = false;
				meshMessage.rootMessage = "tx_status";
				meshMessage.destinationMacAddress = childNode;

				meshEnabled = true;
				meshMessageReady = true;
			}
		}

	});

	server.on("/api/reset", HTTP_GET, [](AsyncWebServerRequest *request)
	{
		Serial.println("[STATUS] - Received reset command");
		AsyncResponseStream *response = request->beginResponseStream("application/json");
		JsonDocument json;
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

/**
 * Initialize File System
 */
void initFS()
{
	if (!LittleFS.begin(true))
	{
		Serial.println("[STATUS] - An error has occurred while mounting LittleFS");
	}
	Serial.println("[STATUS] - LittleFS mounted");
}

/**
 * Change LED's state
 *
 * @param blinkState 1 = slow blink, 2 = fast blink, 3 = snooze state
 */
void triggerLEDCondition(int blinkState)
{
	// remove any previous LED state (aka turn LED off)
	LED.off();
	delay(50);

	switch (blinkState)
	{
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

/**
 * This is a non-block button listener function.
 * Credit to github OSWW ontribution from user @danagarcia
 *
 * @param pauseInSeconds the amount of time to pause and listen
*/
void awaitWhileListening(int pauseInSeconds)
{
  // While waiting for the 1 second to pass, actively monitor/listen for button press.
  int delayEnd = millis() + (1000 * pauseInSeconds);
  while (millis() < delayEnd) {
    // get physical button state
    int buttonState = digitalRead(externalButton);

	if (buttonState == HIGH)
	{
		if (userDefinedSettings.winderEnabled == "0")
		{
			motor.stop();
			routineRunning = false;
			userDefinedSettings.status = "Stopped";
			Serial.println("[STATUS] - Switched off!");
		}
	}
	else
	{
		userDefinedSettings.winderEnabled == "1";
	}

  }
}

/**
 * Callback triggered from WifiManager when successfully connected to new WiFi network
 */
void saveParamsCallback()
{
	if (OLED_ENABLED)
	{
		display.clearDisplay();
		display.display();
		drawNotification("Connecting...");
	}
}

/**
 * Callback triggered from WifiManager when successfully connected to new WiFi network
 */
void saveWifiCallback()
{
	if (OLED_ENABLED)
	{
		display.clearDisplay();
		display.display();
		drawNotification("Connected to WiFi");
		String rebootingMessage[2] = {"Device is", "rebooting..."};
		drawMultiLineText(rebootingMessage);
	}

	// slow blink to confirm connection success
	triggerLEDCondition(1);

	ESP.restart();
	delay(1500);
}


void setup()
{
	WiFi.mode(WIFI_STA);
	Serial.begin(115200);
	setCpuFrequencyMhz(80);

	// Prepare pins
	pinMode(directionalPinA, OUTPUT);
	pinMode(directionalPinB, OUTPUT);
	pinMode(externalButton, INPUT);
	ledcSetup(LED.getChannel(), LED.getFrequency(), LED.getResolution());
	ledcAttachPin(LED_BUILTIN, LED.getChannel());

	// WiFi Manager config
	wm.setConfigPortalTimeout(3600);
	wm.setDarkMode(true);
	wm.setConfigPortalBlocking(false);
	wm.setHostname("Winderoo");
	wm.setSaveConfigCallback(saveWifiCallback);
	wm.setSaveParamsCallback(saveParamsCallback);

	userDefinedSettings.winderEnabled = true;

	if(OLED_ENABLED)
	{
		display.begin(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
		if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
		{
			Serial.println(F("SSD1306 allocation failed"));
			for(;;); // Don't proceed, loop forever
		}
		drawStaticGUI();

		int rotate = OLED_ROTATE_SCREEN_180 ? 2 : 4;
		display.clearDisplay();
		display.invertDisplay(OLED_INVERT_SCREEN);
		display.setRotation(rotate);
		drawNotification("Winderoo");
	}

	String savedNetworkMessage[2] = {"Connecting to", "saved network..."};
	drawMultiLineText(savedNetworkMessage);

	// Connect using saved credentials, if they exist
	// If connection fails, start setup Access Point
	if (wm.autoConnect("Winderoo Setup"))
	{
		initFS();
		Serial.println("[STATUS] - connected to saved network");

		// retrieve & read saved settings
		loadConfigVarsFromFile(settingsFile);

		String searchingForNodesMessage[3] = {"Looking for", "winderoo nodes on", "your network..."};
		drawNotification("Searching");
		drawMultiLineText(searchingForNodesMessage);

		IPAddress winderooIP;
		byte localMac[6];
		WiFi.hostByName("winderoo.local", winderooIP);

		if (winderooIP == IPAddress(0, 0, 0, 0)) {
			Serial.println("[STATUS] - winderoo node has not been found on network");
			if (!MDNS.begin("winderoo"))
			{
				Serial.println("[STATUS] - Failed to start mDNS");
				drawNotification("Failed to start mDNS");
			}
			MDNS.addService("_winderoo", "_tcp", 80);
			drawNotification("mdns started");
			Serial.println("[STATUS] - mDNS started");

			// Initialize ESP-NOW
			quickEspNow.begin(1, 0, false);
			quickEspNow.onDataRcvd(meshDataReceived);

		} else {
			WiFi.macAddress(localMac);
			Serial.println("[STATUS] - winderoo node found on network at IP: " + winderooIP.toString());
			char localMacString[18];
			sprintf(localMacString, "%02x:%02x:%02x:%02x:%02x:%02x", localMac[0], localMac[1], localMac[2], localMac[3], localMac[4], localMac[5]);
			selfIsChildNode = true;

			// Initialize ESP-NOW
			quickEspNow.begin();
			quickEspNow.onDataRcvd(meshDataReceived);

			registerSelfAsChildNode(localMacString);
		}

		if (OLED_ENABLED)
		{
			display.clearDisplay();
			drawStaticGUI();
			drawNotification("Connected to WiFi");
		}

		getTime();
		startWebserver();

		if (strcmp(userDefinedSettings.status.c_str(), "Winding") == 0)
		{
			beginWindingRoutine();
		}
		else
		{
			drawNotification("Winderoo");
		}
	}
	else
	{
		configPortalRunning = true;
		Serial.println("[STATUS] - WiFi Config Portal running");
		ledcWrite(LED.getChannel(), 255);

		String setupNetworkMessage[3] = {"Connect to", "\"Winderoo Setup\"", "wifi to begin"};
		drawMultiLineText(setupNetworkMessage);
	};
}

void loop()
{
	if (configPortalRunning)
	{
		wm.process();
		return;
	}

	if (reset)
	{
		if (OLED_ENABLED)
		{
			display.clearDisplay();
			drawNotification("Resetting");

			String rebootingMessage[2] = {"Device is", "rebooting..."};
			drawMultiLineText(rebootingMessage);
		}
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

	if (userDefinedSettings.timerEnabled == "1")
	{
		if (rtc.getHour(true) == userDefinedSettings.hour.toInt() &&
			rtc.getMinute() == userDefinedSettings.minutes.toInt() &&
			!routineRunning &&
			userDefinedSettings.winderEnabled == "1")
		{
			beginWindingRoutine();
			drawNotification("Winding Started");
		}
	}

	if (routineRunning)
	{
		unsigned long currentTime = rtc.getEpoch();

		if (rtc.getEpoch() < estimatedRoutineFinishEpoch)
		{

			// turn motor in direction
			motor.determineMotorDirectionAndBegin();
			int r = rand() % 100;

			if (r <= 25)
			{
				if ((strcmp(userDefinedSettings.direction.c_str(), "BOTH") == 0) && (currentTime - previousEpoch) > 180)
				{
					motor.stop();
					delay(3000);

					previousEpoch = currentTime;

					int currentDirection = motor.getMotorDirection();
					motor.setMotorDirection(!currentDirection);
					Serial.println("[STATUS] - Motor changing direction, mode: " + userDefinedSettings.direction);

					motor.determineMotorDirectionAndBegin();
				}

				if ((currentTime - previousEpoch) > 180)
				{
					Serial.println("[STATUS] - Pause");
					previousEpoch = currentTime;
					motor.stop();
					delay(3000);
				}
			}
		}
		else
		{
			// Routine has finished
			userDefinedSettings.status = "Stopped";
			routineRunning = false;
			motor.stop();
			if (OLED_ENABLED && !screenSleep)
			{
				drawNotification("Winding Complete");
			}

			bool writeSuccess = writeConfigVarsToFile(settingsFile, userDefinedSettings);
			if ( !writeSuccess )
			{
				Serial.println("[ERROR] - Failed to write updated configuration to file");
			}
		}
	}

	// non-blocking button listener
	awaitWhileListening(1);	// 1 second

	if (userDefinedSettings.winderEnabled == "0")
	{
		triggerLEDCondition(3);
	}
	else
	{
		drawDynamicGUI();
	}

	static time_t lastMessageSendTime = 0;

	if (meshMessageReady && quickEspNow.readyToSendData() && meshMessageSent && ((millis () - lastMessageSendTime) > 2000))
	{
		lastMessageSendTime = millis();
		meshMessageSent = false;
		
		JsonDocument doc;
		doc["destinationMacAddress"] = meshMessage.destinationMacAddress;
		doc["rootMessage"] = meshMessage.rootMessage;
		doc["forRoot"] = meshMessage.forRoot;
		doc["status"] = meshMessage.status;
		doc["rotationsPerDay"] = meshMessage.rotationsPerDay;
		doc["direction"] = meshMessage.direction;
		doc["hour"] = meshMessage.hour;
		doc["minutes"] = meshMessage.minutes;
		doc["screenSleep"] = meshMessage.screenSleep;
		doc["timerEnabled"] = meshMessage.timerEnabled;
		doc["screenEquipped"] = meshMessage.screenEquipped;
		doc["db"] = meshMessage.db;
		doc.shrinkToFit();

		String output;
		serializeJson(doc, output);

		if (meshMessage.forRoot)
		{
			meshMessageRoot(output);
		}
		else if (meshMessage.destinationMacAddress != "")
		{
			requestNodeStatus(meshMessage.destinationMacAddress, output);
		}
		else
		{
			meshBroadcastMessage(output);
		}
		delay(100);

		meshMessageReady = false;
	}

	if (!selfIsChildNode) {
		wm.process();
	}
}

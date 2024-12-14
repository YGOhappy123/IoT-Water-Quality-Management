#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
// #include <BlynkSimpleEsp8266.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "PTIT.HCM_CanBo"
#define WIFI_PASSWORD ""
#define FIREBASE_DATABASE_URL "https://water-quality-management-ptit-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_API_KEY "AIzaSyDJ-hOSSUSqD19g0xfG9XledGUfWOBE9ek"
#define BLYNK_TEMPLATE_ID "xxx"
#define BLYNK_TEMPLATE_NAME "xxx"
#define BLYNK_AUTH_TOKEN "xxx"
#define BAUD_RATE 115200
#define DATA_POSTING_INTERVAL 5000

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool isAutomatic = true;
bool isUpdated = false;
int device1 = -1;
int device2 = -1;
int device3 = -1;
int device4 = -1;
int device5 = -1;

unsigned long sendDataPrevMillis = 0;
float waterLevel = 0;
float temperature = 0;
float tdsLevel = 0;
float humidity = 0;

void setup() {
    Serial.begin(BAUD_RATE);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(300);
    }
    // Serial.println("Connected to WiFi!");

    // Blynk.config(BLYNK_AUTH_TOKEN);
    // Blynk.connect();
    // Serial.println("Connected to Blynk!");

    config.database_url = FIREBASE_DATABASE_URL;
    config.api_key = FIREBASE_API_KEY;
    Firebase.signUp(&config, &auth, "", "");

    config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    // Serial.println("Connected to Firebase!");
}

void loop() {
    // Blynk.run();
    String jsonInput;

    while (Serial.available()) {
        jsonInput = Serial.readStringUntil('\n');
        delay(20);
    }

    if (jsonInput.length() > 0) {
        if (Firebase.ready() && (millis() - sendDataPrevMillis > DATA_POSTING_INTERVAL || sendDataPrevMillis == 0)) {
            parseJsonObjectFromSerial(jsonInput);
            sendDataPrevMillis = millis();
            
            Firebase.RTDB.setBool(&fbdo, "IsProcessed", false);
            Firebase.RTDB.setFloat(&fbdo, "Sensors/env_humidity", humidity);
            Firebase.RTDB.setFloat(&fbdo, "Sensors/tdsLevel", tdsLevel);
            Firebase.RTDB.setFloat(&fbdo, "Sensors/temperature", temperature);
            Firebase.RTDB.setFloat(&fbdo, "Sensors/water_level", waterLevel);
        }
    }

    if (isUpdated) {
        printJsonObjectToSerial(device1, device2, device3, device4, device5);
        isUpdated = false;
    }
}

void printJsonObjectToSerial (int device1, int device2, int device3, int device4, int device5) {
    StaticJsonDocument<200> doc;
    String jsonOutput;

    doc["device_1"] = device1;
    doc["device_2"] = device2;
    doc["device_3"] = device3;
    doc["device_4"] = device4;
    doc["device_5"] = device5;

    serializeJson(doc, jsonOutput);
    Serial.println(jsonOutput);
}

void parseJsonObjectFromSerial (const String& jsonInput) {
    // Serial.println(jsonInput);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, jsonInput);

    if (error) return;

    if (doc.containsKey("waterLevel")) waterLevel = doc["waterLevel"];
    if (doc.containsKey("temperature")) temperature = doc["temperature"];
    if (doc.containsKey("humidity")) humidity = doc["humidity"];
    if (doc.containsKey("tdsLevel")) tdsLevel = doc["tdsLevel"];
}

// BLYNK_WRITE(V1) {
//     int newValue = param.asInt();
//     if (newValue != device1) {
//         device1 = newValue;
//         isUpdated = true;
//     }
// }
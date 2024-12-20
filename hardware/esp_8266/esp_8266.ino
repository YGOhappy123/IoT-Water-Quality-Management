#define WIFI_SSID "PTIT.HCM_CanBo"
#define WIFI_PASSWORD ""
#define FIREBASE_DATABASE_URL "https://water-quality-management-ptit-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_API_KEY "AIzaSyDJ-hOSSUSqD19g0xfG9XledGUfWOBE9ek"
#define BLYNK_TEMPLATE_ID "TMPL65P-da3kl"
#define BLYNK_TEMPLATE_NAME "water quality management"
#define BLYNK_AUTH_TOKEN "dT9RH_W4aDicptdOi0kyRyzpudbnjdQz"
#define AUTO_BUTTON_PIN 12
#define AUTO_LED_PIN 16
#define BAUD_RATE 115200
#define DATA_POSTING_INTERVAL 5000

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp8266.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String jsonInput;
bool isAutomatic = false;
bool isUpdated = false;
bool buttonState = false;
bool lastButtonState = false;
int pumpIn = 0;
int pumpOut = 0;
int cooler = 0;
int heater = 0;
int tdsPlants = 0;
int tdsFishes = 0;
int mistSprayer = 0;
int ventilation = 0;

unsigned long sendDataPrevMillis = 0;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
float waterLevel = 0;
float temperature = 0;
float tdsLevel = 0;
float humidity = 0;

void setup() {
    Serial.begin(BAUD_RATE);

    pinMode(AUTO_BUTTON_PIN, INPUT);
    pinMode(AUTO_LED_PIN, OUTPUT);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(300);
    }
    Serial.println("Connected to WiFi!");

    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    Serial.println("Connected to Blynk!");

    config.database_url = FIREBASE_DATABASE_URL;
    config.api_key = FIREBASE_API_KEY;
    Firebase.signUp(&config, &auth, "", "");

    config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    Serial.println("Connected to Firebase!");

    Blynk.syncVirtual(V0);
    Blynk.syncVirtual(V1);
    Blynk.syncVirtual(V2);
    Blynk.syncVirtual(V3);
    Blynk.syncVirtual(V4);
    Blynk.syncVirtual(V5);
    Blynk.syncVirtual(V6);
    Blynk.syncVirtual(V7);
    Blynk.syncVirtual(V8);
}

void loop() {
    Blynk.run();

    checkToggleAutomaticStatus();

    while (Serial.available()) {
        jsonInput = Serial.readStringUntil('\n');
        delay(20);
    }

    if (jsonInput.length() > 0) {
        if (Firebase.ready() && (millis() - sendDataPrevMillis > DATA_POSTING_INTERVAL || sendDataPrevMillis == 0)) {
            parseJsonObjectFromSerial(jsonInput);
            sendDataPrevMillis = millis();
            
            Firebase.RTDB.setBool(&fbdo, "IsProcessed", false);
            Firebase.RTDB.setFloat(&fbdo, "Sensors/temperature", temperature);
            Firebase.RTDB.setFloat(&fbdo, "Sensors/water_level", waterLevel);
            Firebase.RTDB.setFloat(&fbdo, "Sensors/tds_level", tdsLevel);
            Firebase.RTDB.setFloat(&fbdo, "Sensors/env_humidity", humidity);
        }

        jsonInput = "";
    }

    if (isUpdated) {
        printJsonObjectToSerial(pumpIn, pumpOut, cooler, heater, tdsPlants, tdsFishes, mistSprayer, ventilation);
        isUpdated = false;
    }
}

void printJsonObjectToSerial (int pumpIn, int pumpOut, int cooler, int heater, int tdsPlants, int tdsFishes, int mistSprayer, int ventilation) {
    StaticJsonDocument<200> doc;
    String jsonOutput;

    doc["pumpIn"] = pumpIn;
    doc["pumpOut"] = pumpOut;
    doc["cooler"] = cooler;
    doc["heater"] = heater;
    doc["tdsPlants"] = tdsPlants;
    doc["tdsFishes"] = tdsFishes;
    doc["mistSprayer"] = mistSprayer;
    doc["ventilation"] = ventilation;

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
    if (doc.containsKey("tdsLevel")) tdsLevel = doc["tdsLevel"];
    if (doc.containsKey("humidity")) humidity = doc["humidity"];
}

void checkToggleAutomaticStatus() {
    bool currentButtonState = digitalRead(AUTO_BUTTON_PIN);

    if (currentButtonState != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentButtonState != buttonState) {
            buttonState = currentButtonState;

            if (buttonState == HIGH) {
                isAutomatic = !isAutomatic;
                Blynk.virtualWrite(V0, isAutomatic);
            }
        }
    }

    lastButtonState = currentButtonState;
    digitalWrite(AUTO_LED_PIN, isAutomatic);
}

BLYNK_WRITE(V0) {
    isAutomatic = param.asInt();
}

BLYNK_WRITE(V1) {
    int newValue = param.asInt();
    if (newValue != cooler) {
        cooler = newValue;
        isUpdated = true;
    }
}

BLYNK_WRITE(V2) {
    int newValue = param.asInt();
    if (newValue != heater) {
        heater = newValue;
        isUpdated = true;
    }
}

BLYNK_WRITE(V3) {
    int newValue = param.asInt();
    if (newValue != tdsPlants) {
        tdsPlants = newValue;
        isUpdated = true;
    }
}

BLYNK_WRITE(V4) {
    int newValue = param.asInt();
    if (newValue != tdsFishes) {
        tdsFishes = newValue;
        isUpdated = true;
    }
}

BLYNK_WRITE(V5) {
    int newValue = param.asInt();
    if (newValue != pumpIn) {
        pumpIn = newValue;
        isUpdated = true;
    }
}

BLYNK_WRITE(V6) {
    int newValue = param.asInt();
    if (newValue != pumpOut) {
        pumpOut = newValue;
        isUpdated = true;
    }
}

BLYNK_WRITE(V7) {
    int newValue = param.asInt();
    if (newValue != ventilation) {
        ventilation = newValue;
        isUpdated = true;
    }
}

BLYNK_WRITE(V8) {
    int newValue = param.asInt();
    if (newValue != mistSprayer) {
        mistSprayer = newValue;
        isUpdated = true;
    }
}


#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

#define DISTANCE_TRIG_PIN 2
#define DISTANCE_ECHO_PIN 3
#define TEMPERATURE_PIN 8
#define BAUD_RATE 115200
#define LOOP_DELAY 1000

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature temperatureSensor(&oneWire);
LiquidCrystal_I2C lcdDisplay(0x27, 20, 4);

float waterTankHeight = 0;
float waterLevel = 0;
float temperature = 0;
float tdsLevel = 0;
float humidity = 0;

void setup () {
    Serial.begin(BAUD_RATE);
    
    lcdDisplay.init();
    lcdDisplay.backlight();

    pinMode(DISTANCE_TRIG_PIN, OUTPUT);
    pinMode(DISTANCE_ECHO_PIN, INPUT);
}

void loop() {
    // waterLevel = waterTankHeight - measureDistance();
    // temperature = getWaterTemperature();
    // tdsLevel = 0;

    String jsonInput;

    while (Serial.available()) {
        jsonInput = Serial.readStringUntil('\n');
        delay(20);
    }

    if (jsonInput.length() > 0) {
        parseJsonObjectFromSerial(jsonInput);

        Serial.print("Water level: ");
        Serial.println(waterLevel);
        Serial.print("Temperature: ");
        Serial.println(temperature);
        Serial.print("Humidity: ");
        Serial.println(humidity);
        Serial.print("TDS level: ");
        Serial.println(tdsLevel);
    }

    delay(LOOP_DELAY);
}

float getWaterTemperature () {
    temperatureSensor.requestTemperatures();

    float celsius = temperatureSensor.getTempCByIndex(0);

    return celsius;
}

float measureDistance () {
    digitalWrite(DISTANCE_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(DISTANCE_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(DISTANCE_TRIG_PIN, LOW);

    long duration = pulseIn(DISTANCE_ECHO_PIN, HIGH);
    float distance = (duration * 0.034) / 2;

    return distance;
}

void displayMessageToLCD (int row, String message) {
    if (row < 0 || row > 3) return;
    
    lcdDisplay.setCursor(0, row);
    lcdDisplay.print("                    ");
    lcdDisplay.setCursor(0, row);
    lcdDisplay.print(message);
}

void printJsonObjectToSerial (float waterLevel, float temperature, float tdsLevel, float humidity) {
    StaticJsonDocument<200> doc;
    String jsonOutput;

    doc["waterLevel"] = waterLevel;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["tdsLevel"] = tdsLevel;

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



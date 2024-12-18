#define DISTANCE_TRIG_PIN 2
#define DISTANCE_ECHO_PIN 3
#define DHT_PIN 4
#define TEMPERATURE_PIN 8
#define TDS_PIN A1
#define VREF 5.0
#define BAUD_RATE 115200
#define LOOP_DELAY 1000

#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

DHT dht(DHT_PIN, DHT11);
OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature temperatureSensor(&oneWire);
LiquidCrystal_I2C lcdDisplay(0x27, 20, 4);

String jsonInput;
float waterTankHeight = 35.0;
float waterLevel = 0;
float temperature = 0;
float tdsLevel = 0;
float humidity = 0;

int pumpIn = 0;
int pumpOut = 0;
int cooler = 0;
int heater = 0;
int tdsPlants = 0;
int tdsFishes = 0;
int mistSprayer = 0;
int ventilation = 0;

void setup () {
    Serial.begin(BAUD_RATE);
    
    dht.begin();
    lcdDisplay.init();
    lcdDisplay.backlight();

    pinMode(DISTANCE_TRIG_PIN, OUTPUT);
    pinMode(DISTANCE_ECHO_PIN, INPUT);
    pinMode(TDS_PIN, INPUT);
}

void loop() {
    // waterLevel = waterTankHeight - getDistanceToWaterSurface();
    // temperature = getWaterTemperature();
    // tdsLevel = getWaterTdsValue();
    // humidity = getEnvironmentHumidity();
    waterLevel = 26.687;
    temperature = 31.6875;
    tdsLevel = 72;
    humidity = 56.54631;
    printJsonObjectToSerial (waterLevel, temperature, tdsLevel, humidity);

    while (Serial.available()) {
        jsonInput = Serial.readStringUntil('\n');
        delay(20);
    }

    if (jsonInput.length() > 0) {
        parseJsonObjectFromSerial(jsonInput);
    }

    delay(LOOP_DELAY);
}

float getWaterTemperature () {
    temperatureSensor.requestTemperatures();

    float celsius = temperatureSensor.getTempCByIndex(0);

    return celsius;
}

float getWaterTdsValue() {
    float rawEc = analogRead(TDS_PIN) * VREF / 1023.0;
    float temperatureCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float volatge = rawEc / temperatureCoefficient;
    float totalDissolvedSolids = (133.42 * volatge * volatge * volatge - 255.86 * volatge * volatge + 857.39 * volatge) * 0.5;

    return totalDissolvedSolids;
}

float getDistanceToWaterSurface () {
    digitalWrite(DISTANCE_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(DISTANCE_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(DISTANCE_TRIG_PIN, LOW);

    long duration = pulseIn(DISTANCE_ECHO_PIN, HIGH);
    float distance = (duration * 0.034) / 2;

    return distance;
}

float getEnvironmentHumidity () {
    float envHumidity = dht.readHumidity();
    
    return envHumidity;
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

    doc["waterLevel"] = roundf(waterLevel * 100) / 100.0;
    doc["temperature"] = roundf(temperature * 100) / 100.0;
    doc["tdsLevel"] = roundf(tdsLevel * 100) / 100.0;
    doc["humidity"] = roundf(humidity * 100) / 100.0;

    serializeJson(doc, jsonOutput);
    Serial.println(jsonOutput);
}

void parseJsonObjectFromSerial (const String& jsonInput) {
    // Serial.println(jsonInput);
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, jsonInput);

    if (error) return;

    if (doc.containsKey("pumpIn")) pumpIn = doc["pumpIn"];
    if (doc.containsKey("pumpOut")) pumpOut = doc["pumpOut"];
    if (doc.containsKey("cooler")) cooler = doc["cooler"];
    if (doc.containsKey("heater")) heater = doc["heater"];
    if (doc.containsKey("tdsPlants")) tdsPlants = doc["tdsPlants"];
    if (doc.containsKey("tdsFishes")) tdsFishes = doc["tdsFishes"];
    if (doc.containsKey("mistSprayer")) mistSprayer = doc["mistSprayer"];
    if (doc.containsKey("ventilation")) ventilation = doc["ventilation"];
}

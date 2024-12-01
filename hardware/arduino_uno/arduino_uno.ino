#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

#define DISTANCE_TRIG_PIN 2
#define DISTANCE_ECHO_PIN 3
#define TEMPERATURE_PIN 8
#define LOOP_DELAY 1000

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature temperatureSensor(&oneWire);
LiquidCrystal_I2C lcdDisplay(0x27, 20, 4);

float waterTankHeight = 0;
float waterLevel = 0;
float temperature = 0;
float tdsLevel = 0;

void setup () {
    Serial.begin(115200);
    
    lcdDisplay.init();
    lcdDisplay.backlight();

    pinMode(DISTANCE_TRIG_PIN, OUTPUT);
    pinMode(DISTANCE_ECHO_PIN, INPUT);
}

void loop() {
    waterLevel = waterTankHeight - measureDistance();
    temperature = getWaterTemperature();
    tdsLevel = 0;
      
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


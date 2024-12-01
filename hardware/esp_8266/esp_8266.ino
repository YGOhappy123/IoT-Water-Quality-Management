#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define FIREBASE_HOST "https://water-quality-management-ptit-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_AUTH "XneXetcIV7RCy4RQPb3WhWkKPWwKcjwbjHpdeUhS"
#define LOOP_DELAY 1000

void setup () {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

    Serial.begin(115200);

    pinMode(12, OUTPUT);
}

void loop() {
    String input = Serial.readString();
    float temperature = input.toFloat();

    bool isProcessed = Firebase.getBool("IsProcessed")
    bool isAutomatic = Firebase.getBool("IsAutomatic")

    digitalWrite(12, isProcessed);

    Firebase.setFloat("Sensor/temperature", temperature);
      
    delay(LOOP_DELAY);
}
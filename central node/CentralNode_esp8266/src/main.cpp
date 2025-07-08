#include <Arduino.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>

// LoRa Pins
#define SS 5
#define RST 14
#define DIO0 2

// Wi-Fi credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// Supabase details
const char* supabaseUrl = "https://your-supabase-url.supabase.co/rest/v1/water_levels";
const char* supabaseKey = "your_supabase_api_key";

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Initialize LoRa
    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(433E6)) { // Adjust frequency for your region
        Serial.println("LoRa Error");
        while (1);
    }
    LoRa.setSpreadingFactor(12);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(8);
    Serial.println("LoRa Receiver Ready");
}

void loop() {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        String message = "";
        while (LoRa.available()) {
            message += (char)LoRa.read();
        }
        Serial.print("Received via LoRa: ");
        Serial.println(message);

        // Send to Supabase
        HTTPClient http;
        http.begin(supabaseUrl);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", String("Bearer ") + supabaseKey);
        int httpResponseCode = http.POST(message);
        if (httpResponseCode > 0) {
            Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        } else {
            Serial.printf("HTTP Error: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end();
    }
}

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <LoRa.h>
#include <ArduinoJson.h>

// LoRa Pins
#define SS 5      // SPI Chip Select
#define RST 14    // Reset
#define DIO0 2    // Interrupt pin
#define ledpin 27 // LED indicator

// Function to get current timestamp (assuming an RTC or time source is available)
#include <RTClib.h>

// Initialize RTC object
RTC_DS1307 rtc;

unsigned long getCurrentTimestamp()
{
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        while (1)
            ; // Halt if RTC is not found
    }

    if (!rtc.isrunning())
    {
        Serial.println("RTC is NOT running!");
        // Set the RTC to a default time if needed
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Get the current time
    DateTime now = rtc.now();
    return now.unixtime(); // Return current UNIX timestamp
}

// Callback function for when ESP-NOW data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    char data[64];
    memcpy(data, incomingData, len);
    data[len] = '\0'; // Null terminate the string

    // Parse the received JSON
    StaticJsonDocument<64> doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error)
    {
        Serial.println("Failed to parse JSON");
        return;
    }

    // Add timestamp
    unsigned long timestamp = getCurrentTimestamp();
    StaticJsonDocument<128> newDoc;
    newDoc["device_id"] = doc["device_id"];
    newDoc["timestamp"] = timestamp;
    newDoc["water_level_cm"] = doc["water_level_cm"];
    Serial.print("Received data: ");
    Serial.println(data);

    // Serialize to JSON string
    char jsonBuffer[128];
    serializeJson(newDoc, jsonBuffer);

    // Send via LoRa
    LoRa.beginPacket();
    LoRa.print(jsonBuffer);
    LoRa.endPacket();
    Serial.println("LoRa packet sent");

    // Indicate transmission with LED
    digitalWrite(ledpin, HIGH);
    delay(100);
    digitalWrite(ledpin, LOW);
}

void setup()
{
    Serial.begin(115200);

    String macAddress = WiFi.macAddress();

    // Print it to the serial monitor
    Serial.println("ESP32 MAC Address:");
    Serial.println(macAddress);

    // Initialize LED pin

    pinMode(ledpin, OUTPUT);
    digitalWrite(ledpin, LOW);

    // Set ESP32 to Wi-Fi Station mode for ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100); // Ensure Wi-Fi stabilizes

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        while (1)
        {
            digitalWrite(ledpin, HIGH);
            delay(500);
            digitalWrite(ledpin, LOW);
            delay(500);
        }
    }

    // Register ESP-NOW receive callback
    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("ESP-NOW Receiver Ready");

    // Initialize LoRa
    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(433E6))
    { // Adjust frequency for your region
        Serial.println("LoRa Error");
        while (1)
        {
            digitalWrite(ledpin, HIGH);
            delay(300);
            digitalWrite(ledpin, LOW);
            delay(300);
        }
    }

    // Configure LoRa for maximum range
    LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setSpreadingFactor(12);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(8);
    Serial.println("LoRa Transmitter Started");
    digitalWrite(ledpin, HIGH); // Indicate ready state
}

void loop()
{
    // No periodic sending; transmission is handled in OnDataRecv callback
    delay(100); // Prevent watchdog reset
}

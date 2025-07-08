#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

// LoRa pins for ESP8266
#define SS_PIN 15    // GPIO15 (D8) as Chip Select
#define RST_PIN 16   // GPIO16 (D0) as Reset
#define DIO0_PIN 5   // GPIO5 (D1) as Interrupt pin
#define LED_PIN 2    // GPIO2 (D4) built-in LED (inverted)

// WiFi credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// Supabase details
const char* supabaseUrl = "https://your-supabase-url.supabase.co/rest/v1/water_levels";
const char* supabaseKey = "your_supabase_api_key";

// Connection state
bool wifiConnected = false;
bool loraInitialized = false;
unsigned long lastWifiAttempt = 0;
const unsigned long wifiRetryInterval = 30000; // 30 seconds

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nESP8266 LoRa Receiver Starting");
  
  // Configure LED (inverted logic on ESP8266 built-in LED)
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // OFF
  
  // Initialize SPI for LoRa
  SPI.begin();
  
  // Initialize LoRa
  Serial.println("Initializing LoRa...");
  LoRa.setPins(SS_PIN, RST_PIN, DIO0_PIN);
  
  // Try to initialize LoRa
  int attempts = 0;
  while (!LoRa.begin(433E6) && attempts < 5) {
    Serial.println("LoRa initialization failed. Retrying...");
    attempts++;
    digitalWrite(LED_PIN, LOW); // ON
    delay(200);
    digitalWrite(LED_PIN, HIGH); // OFF
    delay(200);
  }
  
  if (attempts >= 5) {
    Serial.println("Failed to initialize LoRa! Check your wiring.");
    while (1) {
      digitalWrite(LED_PIN, LOW); // Error indicator
      delay(100);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
    }
  }
  
  loraInitialized = true;
  Serial.println("LoRa initialized successfully");
  
  // Configure LoRa parameters
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(8);
  
  // Connect to WiFi
  connectToWifi();
  
  Serial.println("Ready to receive LoRa packets");
}

void connectToWifi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  // Wait up to 20 seconds for connection
  int timeout = 20;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(1000);
    Serial.print(".");
    timeout--;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Success indicator
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, LOW); // ON
      delay(100);
      digitalWrite(LED_PIN, HIGH); // OFF
      delay(100);
    }
  } else {
    wifiConnected = false;
    Serial.println("\nFailed to connect to WiFi. Will retry later.");
  }
  
  lastWifiAttempt = millis();
}

void sendToSupabase(String jsonData) {
  if (!wifiConnected) {
    Serial.println("WiFi not connected. Cannot send data to Supabase.");
    return;
  }
  
  Serial.println("Sending data to Supabase...");
  
  // Create a WiFiClient object to establish the connection
  WiFiClient client;
  HTTPClient http;
  
  // Begin the HTTP connection
  if (http.begin(client, supabaseUrl)) {
    // Add headers
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", supabaseKey);
    http.addHeader("Authorization", String("Bearer ") + supabaseKey);
    
    // Send the POST request
    int httpResponseCode = http.POST(jsonData);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.printf("HTTP Response code: %d\n", httpResponseCode);
      Serial.println("Response: " + response);
      
      // Data sent successfully
      digitalWrite(LED_PIN, LOW); // ON
      delay(200);
      digitalWrite(LED_PIN, HIGH); // OFF
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("Unable to connect to server");
  }
}

void processLoRaData() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Received a packet
    digitalWrite(LED_PIN, LOW); // ON
    
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }
    
    digitalWrite(LED_PIN, HIGH); // OFF
    
    Serial.print("Received LoRa packet (RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.print("): ");
    Serial.println(message);
    
    // Check if message is JSON format
    if (message.startsWith("{") && message.endsWith("}")) {
      sendToSupabase(message);
    } else {
      // Convert to JSON format
      String jsonData = "{\"water_level\":\"" + message + "\",\"received_at\":\"" + String(millis()) + "\"}";
      sendToSupabase(jsonData);
    }
  }
}

void loop() {
  // Check for LoRa data
  processLoRaData();
  
  // Check and manage WiFi connection
  if (!wifiConnected && (millis() - lastWifiAttempt > wifiRetryInterval)) {
    connectToWifi();
  }
  
  // If WiFi disconnects, try to reconnect
  if (wifiConnected && WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost!");
    wifiConnected = false;
    lastWifiAttempt = millis() - wifiRetryInterval + 5000; // Retry after 5 seconds
  }
  
  // Small delay to prevent watchdog issues
  delay(10);
}
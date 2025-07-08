
#include <Arduino.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TimeLib.h> // Include TimeLib for time conversion

// LoRa Pins
#define SS 5
#define RST 14
#define DIO0 2

// OLED Display Config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // No reset pin on I2C OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define OLED_POWER_PIN 4  // Pin to control OLED power

void setup()
{
    Serial.begin(115200);

    pinMode(OLED_POWER_PIN, OUTPUT);
    digitalWrite(OLED_POWER_PIN, HIGH); // Power ON OLED

    delay(100); // Give time for OLED to stabilize

    // Initialize OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Default I2C address for OLED
        Serial.println("OLED initialization failed");
        while (true);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("LoRa Receiver Starting...");
    display.display();

    // Initialize LoRa
    LoRa.setPins(SS, RST, DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa init failed!");
        display.clearDisplay();
        display.println("LoRa Error!");
        display.display();
        while (true);
    }
    LoRa.setSpreadingFactor(12);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(8);
    Serial.println("LoRa Receiver Ready");
    display.clearDisplay();
    display.println("LoRa Ready!");
    display.display();
}

void loop()
{
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        String message = "";
        while (LoRa.available()) {
            message += (char)LoRa.read();
        }

        Serial.print("Received via LoRa: ");
        Serial.println(message);

        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, message);
        if (error) {
            Serial.println("Failed to parse JSON");
            return;
        }

        const char *device_id = doc["device_id"];
        long timestamp = doc["timestamp"];
        float water_level_cm = doc["water_level_cm"];

        // Convert the Unix timestamp to real-time
        setTime(timestamp);  // Set system time to Unix timestamp

        // Get the real-time values
        int hours = hour();
        int minutes = minute();
        int seconds = second();
        int day_of_month = day();
        int current_month = month();
        int current_year = year();

        // Display parsed data on Serial Monitor
        Serial.println("Parsed Data:");
        Serial.printf("Device ID: %s\n", device_id);
        Serial.printf("Timestamp: %ld\n", timestamp);
        Serial.printf("Water Level (cm): %.2f\n", water_level_cm);
        Serial.printf("Time: %02d:%02d:%02d %02d/%02d/%04d\n", hours, minutes, seconds, day_of_month, current_month, current_year);

        // Display on OLED
        display.clearDisplay();
        display.setCursor(0, 0);
        display.printf("Device: %s\n", device_id);
        display.printf("Time: %02d:%02d:%02d\n", hours, minutes, seconds);
        // display.printf("Date: %02d/%02d/%04d\n", day_of_month, current_month, current_year);
        display.printf("Level: %.2f cm\n", water_level_cm);
        display.display();
    }

    delay(10); // Prevent watchdog reset
}



// #include <Arduino.h>

// // OLED Libraries
// #include <Wire.h> // For I2C communication
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// // LED indicator (optional, you can remove this if not using)
// #define ledpin 27

// // --- OLED Display Definitions ---
// #define SCREEN_WIDTH 128 // OLED display width, in pixels
// #define SCREEN_HEIGHT 64 // OLED display height, in pixels
// #define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
// #define SCREEN_ADDRESS 0x3C // I2C address for 128x64 OLED (common is 0x3C or 0x3D)

// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// // --- Configuration for the 1-liter bottle/tank ---
// const char* device_id = "ESP32-TANK-A1"; // Device ID to display

// // --- Calibration Constants ---
// // Based on your input:
// // 20 cm = Full capacity (1000ml)
// // 25 cm = 80% capacity (800ml)
// // This means the sensor measures distance from the top,
// // so higher 'cm' values mean less water.
// // Volume (ml) = slope * Water_Level_Reading_cm + intercept
// const float CALIBRATION_SLOPE = -40.0; // ml/cm
// const float CALIBRATION_INTERCEPT = 1800.0; // ml

// // Define a function to calculate volume in liters
// float calculateVolumeLiters(float sensor_reading_cm) {
//   float volume_ml = (CALIBRATION_SLOPE * sensor_reading_cm) + CALIBRATION_INTERCEPT;

//   // Ensure volume doesn't go below 0 or above max (1000ml for 1 liter)
//   // This also handles edge cases where sensor might read below 20cm or above 45cm
//   volume_ml = max(0.0f, min(volume_ml, 1000.0f)); // Cap at 0ml and 1000ml

//   return volume_ml / 1000.0f; // Convert ml to liters
// }

// void setup() {
//     Serial.begin(115200);

//     // Initialize LED pin (optional)
//     pinMode(ledpin, OUTPUT);
//     digitalWrite(ledpin, LOW);

//     // --- Initialize OLED Display ---
//     Wire.begin(); // Initialize I2C communication for OLED
//     if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
//         Serial.println(F("SSD1306 allocation failed"));
//         while(true) { // Loop forever if display fails
//             digitalWrite(ledpin, HIGH); delay(500);
//             digitalWrite(ledpin, LOW); delay(500);
//         }
//     }
//     display.display(); // Clear the buffer
//     delay(100);
//     display.clearDisplay();
//     display.setTextSize(1);
//     display.setTextColor(SSD1306_WHITE);
//     display.setCursor(0,0);
//     display.println("Display Ready!");
//     display.display();
//     delay(1000); // Show "Display Ready!" for a second
// }

// void loop() {
//     // --- Simulate Data ---
//     // Simulate water level sensor reading around 800ml mark (25cm)
//     // The range for random.uniform(-0.1, 0.1) in Python corresponds to random(low*100, high*100)/100.0
//     // So, random(-10, 10)/100.0 gives +/- 0.1
//     float base_level = 25.0; // Corresponds to 800ml
//     float water_level_reading = base_level + (random(-10, 10) / 100.0);
//     water_level_reading = round(water_level_reading * 100.0) / 100.0; // Round to 2 decimal places

//     // Calculate water available in liters
//     float water_available_liters = calculateVolumeLiters(water_level_reading);

//     // --- Print to Serial Monitor ---
//     Serial.println("\n--- Simulated Reading ---");
//     Serial.print("Device ID: ");
//     Serial.println(device_id);
//     Serial.print("Water Level Sensor Reading (cm): ");
//     Serial.println(water_level_reading, 2); // Print with 2 decimal places
//     Serial.print("Water Available (liters): ");
//     Serial.println(water_available_liters, 2); // Print with 2 decimal places
//     Serial.print("Timestamp (millis): "); // Using millis() for a simple timestamp
//     Serial.println(millis());

//     // --- Display on OLED ---
//     display.clearDisplay();
//     display.setTextSize(1); // Small text
//     display.setTextColor(SSD1306_WHITE);

//     // Line 1: Device ID
//     display.setCursor(0, 0);
//     display.print("Dev ID: ");
//     display.println(device_id);

//     // Line 2: Sensor Reading (cm)
//     display.setCursor(0, 16); // Move down 16 pixels
//     display.print("Level(cm): ");
//     display.print(water_level_reading, 2);

//     // Line 3: Water Available (liters)
//     display.setCursor(0, 32); // Move down
//     display.print("Volume(L): ");
//     display.print(water_available_liters, 2);

//     // Line 4: Simple Time (from ESP32's internal millis)
//     display.setCursor(0, 48); // Move down
//     display.print("Time: ");
//     display.print(millis() / 1000); // Display seconds since boot

//     display.display(); // Show the buffer on the screen

//     // Optional LED blink
//     digitalWrite(ledpin, HIGH);
//     delay(50);
//     digitalWrite(ledpin, LOW);

//     delay(3000); // Wait for 3 seconds before next update
// }
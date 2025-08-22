#include <ESP8266WiFi.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <PulseSensorPlayground.h>
#include <ThingSpeak.h>

// ---- WiFi & ThingSpeak Config ----
const char* ssid = "YOUR_WIFI_SSID";       // 🔹 Replace with your WiFi SSID
const char* password = "YOUR_WIFI_PASS";   // 🔹 Replace with your WiFi Password
unsigned long channelID = YOUR_CHANNEL_ID; // 🔹 Replace with your ThingSpeak Channel ID
const char* writeAPIKey = "YOUR_WRITE_API_KEY"; // 🔹 Replace with your ThingSpeak Write API Key

WiFiClient client;

// ---- Sensor Config ----
#define DHTPIN D5
#define DHTTYPE DHT11
#define PULSE_PIN A0
#define ONE_WIRE_BUS D2

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
PulseSensorPlayground pulseSensor;

float roomTemp, roomHumidity, bodyTemp;
int heartRate;

void setup() {
  Serial.begin(115200);

  // Start sensors
  dht.begin();
  sensors.begin();
  pulseSensor.analogInput(PULSE_PIN);
  pulseSensor.begin();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Init ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  // Room Temp & Humidity
  roomTemp = dht.readTemperature();
  roomHumidity = dht.readHumidity();
  if (isnan(roomTemp) || isnan(roomHumidity)) {
    Serial.println("Failed to read from DHT sensor!");
  }

  // Body Temperature
  sensors.requestTemperatures();
  bodyTemp = sensors.getTempCByIndex(0);

  // Heart Rate
  heartRate = pulseSensor.getBeatsPerMinute();
  if (pulseSensor.sawStartOfBeat()) {
    Serial.print("Heart Rate: ");
    Serial.print(heartRate);
    Serial.println(" BPM");
  }

  // Debug Output
  Serial.println("--- Sensor Data ---");
  Serial.print("Room Temp: "); Serial.println(roomTemp);
  Serial.print("Room Humidity: "); Serial.println(roomHumidity);
  Serial.print("Body Temp: "); Serial.println(bodyTemp);
  Serial.print("Heart Rate: "); Serial.println(heartRate);

  // ---- Upload to ThingSpeak ----
  ThingSpeak.setField(1, roomTemp);
  ThingSpeak.setField(2, roomHumidity);
  ThingSpeak.setField(3, bodyTemp);
  ThingSpeak.setField(4, heartRate);

  int response = ThingSpeak.writeFields(channelID, writeAPIKey);
  if (response == 200) {
    Serial.println("Data successfully sent to ThingSpeak!");
  } else {
    Serial.print("Error sending data. HTTP error code: ");
    Serial.println(response);
  }

  delay(20000); // ThingSpeak requires min. 15 sec delay
}

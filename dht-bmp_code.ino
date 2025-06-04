#include <Adafruit_BMP085_U.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <dht11.h>

#define dht_apin 2
dht11 dhtObject; 
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
// WiFi credentials
const char* WIFI_SSID = "Redmi Note 12 Pro 5G";
const char* WIFI_PASSWORD = "qwertyuiop";

// ThingSpeak credentials
unsigned long CHANNEL_ID = 2945813;  
const char* THINGSPEAK_API_KEY = "UNF7YFXYT9NXS3RX";
WiFiClient client;

void setup() {
  Serial.begin(9600);
  // Initialize BMP180
  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 sensor.");
    while (1);
  }

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected!");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  // Read DHT11 data
  dhtObject.read(dht_apin);
  int temp = dhtObject.temperature;
  dhtObject.read(dht_apin);
  int humidity = dhtObject.humidity;
  

  // Read BMP180 data
  sensors_event_t event;
  bmp.getEvent(&event);
  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    Serial.print("Pressure:    ");
    Serial.print(event.pressure);
    Serial.println(" hPa");
  }
  
  // Print data to Serial Monitor
  Serial.print("Temperature (DHT11): ");
  Serial.print(temp);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.println(humidity);

  // Send data to ThingSpeak
  ThingSpeak.setField(5, temp);  // Field 1: Temperature
  ThingSpeak.setField(6, humidity);     // Field 2: Humidity
  ThingSpeak.setField(7, event.pressure);     // Field 3: Pressure

  int responseCode = ThingSpeak.writeFields(CHANNEL_ID, THINGSPEAK_API_KEY);
  if (responseCode == 200) {
    Serial.println("Data successfully sent to ThingSpeak.");
  } else {
    Serial.print("Error sending data to ThingSpeak. HTTP code: ");
    Serial.println(responseCode);
  }

  delay(15000);  // ThingSpeak rate limit is 15 seconds
}
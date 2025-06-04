#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>

// ADXL345 Sensor
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// WiFi credentials
const char *ssid = "Redmi Note 12 Pro 5G";
const char *password = "qwertyuiop";

// ThingSpeak configuration
unsigned long myChannelNumber = 2945813;
const char *myWriteAPIKey = "UNF7YFXYT9NXS3RX";
WiFiClient client;

// Threshold for significant impact (in m/s²)
const float impactThreshold = 11.50;

// LED Pins
const int redLedPin = D6;   // Red LED or Buzzer for impact alert
const int greenLedPin = D7; // Green LED for normal state

void displaySensorDetails(void) {
  sensor_t sensor;
  accel.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print("Sensor:       "); Serial.println(sensor.name);
  Serial.print("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
  Serial.print("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
  Serial.print("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" m/s^2");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void displayDataRate(void) {
  Serial.print("Data Rate:    ");
  switch (accel.getDataRate()) {
    case ADXL345_DATARATE_3200_HZ: Serial.print("3200 "); break;
    case ADXL345_DATARATE_1600_HZ: Serial.print("1600 "); break;
    case ADXL345_DATARATE_800_HZ: Serial.print("800 "); break;
    case ADXL345_DATARATE_400_HZ: Serial.print("400 "); break;
    case ADXL345_DATARATE_200_HZ: Serial.print("200 "); break;
    case ADXL345_DATARATE_100_HZ: Serial.print("100 "); break;
    case ADXL345_DATARATE_50_HZ: Serial.print("50 "); break;
    default: Serial.print("Unknown "); break;
  }
  Serial.println(" Hz");
}

void displayRange(void) {
  Serial.print("Range:         +/- ");
  switch (accel.getRange()) {
    case ADXL345_RANGE_16_G: Serial.print("16 "); break;
    case ADXL345_RANGE_8_G: Serial.print("8 "); break;
    case ADXL345_RANGE_4_G: Serial.print("4 "); break;
    case ADXL345_RANGE_2_G: Serial.print("2 "); break;
    default: Serial.print("Unknown "); break;
  }
  Serial.println(" g");
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("Helmet Impact Detection System");

  // Initialize WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  // Initialize ADXL345
  if (!accel.begin()) {
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while (1);
  }

  accel.setRange(ADXL345_RANGE_16_G);
  displaySensorDetails();
  displayDataRate();
  displayRange();

  // Set up alert pins for LEDs
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  digitalWrite(redLedPin, LOW);   // Red LED off initially
  digitalWrite(greenLedPin, LOW); // Green LED off initially
}

void loop(void) {
  // Get a new sensor event
  sensors_event_t event;
  accel.getEvent(&event);

  // Get acceleration values
  float x = event.acceleration.x;
  float y = event.acceleration.y;
  float z = event.acceleration.z;

  // Calculate the resultant acceleration
  float totalAcceleration = sqrt(x * x + y * y + z * z);

  Serial.print("X: "); Serial.print(x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(z); Serial.print("  ");
  Serial.print("Total Acceleration: "); Serial.print(totalAcceleration); Serial.println(" m/s^2");

  // Check if the impact exceeds the threshold
  if (totalAcceleration >= impactThreshold) {
    Serial.println("Impact detected! Threshold exceeded.");
    digitalWrite(redLedPin, HIGH);  // Turn on the Red LED/Buzzer
    digitalWrite(greenLedPin, LOW); // Turn off the Green LED
    delay(1000);                    // Keep it on for 1 second
    digitalWrite(redLedPin, LOW);   // Turn off the Red LED/Buzzer
  } else {
    // No impact detected, blink Green LED
    digitalWrite(greenLedPin, HIGH);
    delay(200);
    digitalWrite(greenLedPin, LOW);
  }

  // Send data to ThingSpeak
  ThingSpeak.setField(1, x);
  ThingSpeak.setField(2, y);
  ThingSpeak.setField(3, z);
  ThingSpeak.setField(4, totalAcceleration);

  int responseCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (responseCode == 200) {
    Serial.println("Data successfully sent to ThingSpeak!");
  } else {
    Serial.print("Failed to send data. Error Code: ");
    Serial.println(responseCode);
  }

  // Delay before next update
  delay(15000);
}
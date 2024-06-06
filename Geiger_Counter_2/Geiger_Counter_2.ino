#include "RadiationWatch.h"
#include <SPI.h>
#include <SD.h>

// Constants
const int PIN_TONE = 8;
const int chipSelect = 53;  // Change this to match your SD shield or module

// Objects
RadiationWatch radiationWatch;
File myFile;

// Callback function for radiation detection
void onRadiation() {
  // Output classic geiger counter tick noise
  tone(PIN_TONE, 800, 1);

  // Read radiation data
  float radiationValue = radiationWatch.uSvh();
  float radiationError = radiationWatch.uSvhError();

  // Print to serial monitor
  Serial.println("A wild gamma ray appeared");
  Serial.print(radiationValue);
  Serial.print(" uSv/h +/- ");
  Serial.println(radiationError);

  // Write to SD card
  String radiationData = String(radiationValue, 6) + "," + String(radiationError, 6);
  write2Sd("radiationData.txt", radiationData);
}

// Callback function for noise detection
void onNoise() {
  Serial.println("Argh, noise, please stop moving");
}

// Setup function
void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ;  // Wait for serial port to connect. Needed for native USB port only
  }

  // Initialize radiation sensor
  radiationWatch.setup();
  radiationWatch.registerRadiationCallback(&onRadiation);
  radiationWatch.registerNoiseCallback(&onNoise);

  // Initialize SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization failed!");
    while (1);
  }
  Serial.println("Initialization done.");
  SD.remove("radiationData.txt")
}

// Loop function
void loop() {
  radiationWatch.loop();
}

// Function to write data to the SD card
void write2Sd(const char *file_name, String data) {
  unsigned long currentTime = millis();
  unsigned long seconds = currentTime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  seconds = seconds % 60;
  minutes = minutes % 60;

  // Create a buffer to hold the time string
  String timeStr = String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);

  timeStr = timeStr + "," + data;
  Serial.println(timeStr);
  myFile = SD.open(file_name, FILE_WRITE);
  if (myFile) {
    myFile.println(timeStr);
    myFile.close();
  } else {
    Serial.println("Error opening file: " + String(file_name));
  }
}


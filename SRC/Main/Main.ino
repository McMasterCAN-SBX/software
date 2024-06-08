// How to use the DHT-22 sensor with Arduino Uno
// Temperature and humidity sensor

// Libraries
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "DFRobot_OzoneSensor.h"
#include "IntersemaBaro.h"
#include <SPI.h>
#include <SD.h>

// DHT-22 Configuration
#define DHTPIN 7           // Pin connected to DHT22
#define DHTTYPE DHT22      // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);  // Initialize DHT sensor for normal 16MHz Arduino

// Ozone Sensor Configuration
#define COLLECT_NUMBER 20  // Collect number, the collection range is 1-100
#define Ozone_IICAddress OZONE_ADDRESS_3
DFRobot_OzoneSensor Ozone;

// Barometric Pressure Sensor
Intersema::BaroPressure_MS5607B baro(true);

// SD Card Configuration
const int chipSelect = 53;  // Change this to match your SD shield or module
Sd2Card card;
SdVolume volume;
SdFile root;
File myFile;

// Variables for DHT-22
float hum;   // Stores humidity value
float temp;  // Stores temperature value
// char flstr[20], output[100];

void setup() {
  // Serial setup
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(1, OUTPUT);
  // Initialize DHT22
  dht.begin();

  // Initialize Ozone Sensor
  while (!Ozone.begin(Ozone_IICAddress)) {
    Serial.println(F("I2c device number error!"));
    delay(1000);
  }
  Serial.println(F("I2c connect success!"));
  Ozone.setModes(MEASURE_MODE_PASSIVE);

  // Initialize Barometric Pressure Sensor
  baro.init();

  // Initialize SD Card
  // Serial.print(F("\nInitializing SD card..."));
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    // Serial.println(F("Initialization failed. Things to check:"));
    // Serial.println(F("* Is a card inserted?"));
    // Serial.println(F("* Is your wiring correct?"));
    // Serial.println(F("* Did you change the chipSelect pin to match your shield or module?"));
    while (1)
      ;
  } else {
    Serial.println(F("Wiring is correct and a card is present."));
  }

  // Print the type of card
  Serial.print(F("Card type: "));
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println(F("SD1"));
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println(F("SD2"));
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println(F("SDHC"));
      break;
    default:
      Serial.println(F("Unknown"));
  }

  // Open the volume/partition
  if (!volume.init(card)) {
    Serial.println(F("Could not find FAT16/FAT32 partition. Make sure you've formatted the card."));
    while (1)
      ;
  }

  // Print the volume details
  // Serial.print(F("Clusters: "));
  // Serial.println(volume.clusterCount());
  // Serial.print(F("Blocks x Cluster: "));
  // Serial.println(volume.blocksPerCluster());
  // Serial.print(F("Total Blocks: "));
  // Serial.println(volume.blocksPerCluster() * volume.clusterCount());

  uint32_t volumesize = volume.blocksPerCluster() * volume.clusterCount() / 2;  // SD card blocks are always 512 bytes (2 blocks are 1KB)
  // Serial.print(F("Volume size (Kb): "));
  // Serial.println(volumesize);
  // Serial.print(F("Volume size (Mb): "));
  // Serial.println(volumesize / 1024);
  // Serial.print(F("Volume size (Gb): "));
  // Serial.println((float)volumesize / 1024.0);

  // Serial.println(F("\nFiles found on the card (name, date, and size in bytes):"));
  root.openRoot(volume);
  root.ls(LS_R | LS_DATE | LS_SIZE);

  // Additional SD Card Initialization and File Operations
  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(chipSelect)) {
    Serial.println(F("Initialization failed!"));
    while (1)
      ;
  }
  Serial.println(F("Initialization done."));
  myFile = SD.open("dht22.csv", FILE_WRITE);
  if (myFile) {
    myFile.println("*******************************");
    myFile.close();
  } else {
    Serial.println(F("Error opening dht22.csv"));
  }

  myFile = SD.open("Ozone.csv", FILE_WRITE);
  if (myFile) {
    myFile.println("*******************************");
    myFile.close();
  } else {
    Serial.println(F("Error opening Ozone.csv"));
  }
  
  myFile = SD.open("Altitude.csv", FILE_WRITE);
  if (myFile) {
    myFile.println("*******************************");
    myFile.close();
  } else {
    Serial.println(F("Error opening Altitude.csv"));
  }
}

void loop() {

  hum = dht.readHumidity();
  temp = dht.readTemperature();
  String humStr = String(hum, 2) + "," + String(temp, 2);
  write2Sd("dht22.csv", humStr);
  // Serial.print("Humidity: ");
  // Serial.print(hum);
  // Serial.print(" %, Temp: ");
  // Serial.print(temp);
  // Serial.println(" Celsius");


  int16_t ozoneConcentration = Ozone.readOzoneData(COLLECT_NUMBER);
  String ozoneConc = String(ozoneConcentration);
  write2Sd("Ozone.csv", ozoneConc);
  // Serial.print("Ozone concentration is ");
  // Serial.print(ozoneConcentration);
  // Serial.println(" PPB.");

  // Barometric Pressure Sensor: Read and print height in centimeters and feet
  int alt = baro.getHeightCentiMeters();
  float altInCm = (float)alt;
  String altStr = String(altInCm, 2);  // Convert float to String with 2 decimal places
  // Convert float to char array
  Serial.println(altStr);
  write2Sd("Altitude.csv", altStr);

  digitalWrite(1, LOW);  // turn the LED on (HIGH is the voltage level)
  delay(1000);            // wait for a second
  digitalWrite(1, HIGH);   // turn the LED off by making the voltage LOW
  delay(1000);            // wait for a second   // turn the LED off by making the voltage LOW
}


void write2Sd(char *file_name, String data) {
  unsigned long currentTime = millis();
  unsigned long seconds = currentTime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long mil = currentTime % 1000;
  seconds = seconds % 60;
  minutes = minutes % 60;

  // Create a buffer to hold the time string
  String timeStr = String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds) + "." + String(mil);

  timeStr = timeStr + "," + data;
  Serial.println(timeStr);
  myFile = SD.open(file_name, FILE_WRITE);
  Serial.println(file_name);
  if (myFile) {
    Serial.println(file_name);
    myFile.println(timeStr);
    Serial.println(timeStr);
    myFile.close();
  } else {
    Serial.println(F("Error opening test.csv"));
  }
}

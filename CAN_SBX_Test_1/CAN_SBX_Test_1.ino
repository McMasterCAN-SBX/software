#include <Arduino_FreeRTOS.h>
#include <Adafruit_BMP085.h>
#include <semphr.h>
#include "DHT.h"

#define BUFFER_SIZE 100
#define DHTPIN 5  // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
#define DHTTYPE DHT11  // DHT 11
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
SemaphoreHandle_t sem;
// define two tasks for Blink & AnalogRead
void TaskGetBMPData(void *pvParameters);
void TaskGetDHTData(void *pvParameters);

char bmp_buffer[BUFFER_SIZE];
char dht_buffer[BUFFER_SIZE];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }
  dht.begin();
  xTaskCreate(
    TaskGetBMPData, "Blink"  // A name just for humans
    ,
    255  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,
    NULL, 2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,
    NULL);

  xTaskCreate(
    TaskGetDHTData, "AnalogRead", 255  // Stack size
    ,
    NULL, 3  // Priority
    ,
    NULL);
  Serial.println("Start X)");
  if (sem == NULL) {
    sem = xSemaphoreCreateMutex();
    if ((sem) != NULL) xSemaphoreGive((sem));
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

void TaskGetDHTData(void *pvParameters)  // This is a task.
{
  (void)pvParameters;
  float temp, hum;
  char t[10], h[10];
  for (;;) {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    // read temp
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    if (isnan(temp) || isnan(hum)) {
      if (xSemaphoreTake(sem, (TickType_t)5) == pdTRUE) {
        Serial.println(F("Failed to read from DHT sensor!"));
        xSemaphoreGive(sem);
      }
      continue;
    }
    dtostrf(temp, 5, 3, t);
    dtostrf(hum, 5, 3, h);
    snprintf(dht_buffer, BUFFER_SIZE, "%s,%s", t, h);
    // retries every 5 * 15 ms
    if (xSemaphoreTake(sem, (TickType_t)5) == pdTRUE) {
      Serial.println(dht_buffer);
      xSemaphoreGive(sem);
    }
    memset(t, 0, sizeof(t));
    memset(h, 0, sizeof(h));
    memset(dht_buffer, 0, BUFFER_SIZE);
    // delay 500 ms
  }
}
void TaskGetBMPData(void *pvParameters) {
  (void)pvParameters;
  float temp, alt;
  long pres;
  char t[10], a[10];
  for (;;) {
    // read temp
    temp = bmp.readTemperature();
    alt = bmp.readAltitude();
    pres = bmp.readPressure();
    dtostrf(temp, 5, 3, t);
    dtostrf(alt, 5, 3, a);
    snprintf(bmp_buffer, BUFFER_SIZE, "%s,%ld,%s", t, pres, a);
    // retries every 5 * 15 ms
    if (xSemaphoreTake(sem, (TickType_t)5) == pdTRUE) {
      Serial.println(bmp_buffer);
      xSemaphoreGive(sem);
    }
    // memset(t, 0, sizeof(t));
    // memset(a, 0, sizeof(a));
    // memset(bmp_buffer, 0, BUFFER_SIZE);
    // delay 500 ms
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

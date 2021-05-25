#include <Arduino.h>
#include <Measurement.h>
#include <Connection.h>
#include <utils.h>
#include <Timing.h>

int FLOWMETER_PIN = 17; // As marked on board
unsigned long readInterval = 10000;
unsigned long uploadInterval = 60000;

unsigned long currentMillis, lastReadMillis, lastUploadMillis, pulseCount;
float calibrationFactor = 1.25;
float ws;

Measurement windSpeed;

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
  if (pulseCount == 0)
  {
    Serial.println("pulseCount reset to 0 due to overflow.");
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(FLOWMETER_PIN, INPUT_PULLUP); // make sure that your pin has internal pullup
  attachInterrupt(digitalPinToInterrupt(FLOWMETER_PIN), pulseCounter, FALLING);
  WiFi.mode(WIFI_STA); // config WiFi as client
  // printPins();
  waitForNextMinute();
  lastUploadMillis = millis();
}

void updateJson()
{
  jsonDoc["count"] = windSpeed.count();
  jsonDoc["WSmin"] = windSpeed.min();
  jsonDoc["WSmax"] = windSpeed.max();
  jsonDoc["WSmean"] = windSpeed.mean();
  jsonDoc["WSstdev"] = windSpeed.stdev();
  jsonDoc["freeHeap"] = ESP.getFreeHeap();
}

void uploadJson()
{
  updateJson();
  windSpeed.reset();
  upload();
}

void loop()
{
  if (statusOK())
  {
    currentMillis = millis();
    if (currentMillis - lastReadMillis >= readInterval)
    {
      ws = pulseCount * calibrationFactor / ((currentMillis - lastReadMillis) / 1000);
      windSpeed.sample(ws);
      windSpeed.print();
      lastReadMillis = currentMillis;
      pulseCount = 0;
    }
    currentMillis = millis();
    if (currentMillis - lastUploadMillis >= uploadInterval)
    {
      lastUploadMillis = currentMillis;
      uploadJson();
    }
    check();
  }
}
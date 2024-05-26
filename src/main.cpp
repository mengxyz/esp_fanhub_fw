#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cSht4x.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiConfig.h>
#include <SwitchSource.h>
#include <Thermister.h>
#include <FanControl.h>
#include <DataStore.h>
#include <VoltageSensor.h>

#define BTN_UTIL_PIN_3 17 // ARGB RND

#define ARGB_PIN 8
#define COLOR_ORDER GRB
#define CHIPSET WS2811
#define NUM_LEDS 40
#define BRIGHTNESS 200
#define FRAMES_PER_SECOND 60

int ARGB_BRIGHTNESS = 60;
int ARGB_MODE = 1;
bool ARGB_FILLED = false;
Adafruit_NeoPixel pixels(NUM_LEDS, ARGB_PIN, NEO_GRB + NEO_KHZ800);
// create static varables store a colors for test named argbTestColors
uint32_t argbTestColors[8] = {
    0xFF000000,
    0xFF0000FF,
    0xFF00FF00,
    0xFF00FFFF,
    0xFFFFFF00,
    0xFFFF0000,
    0xFFFF00FF,
    0xFFFFFF,
};

VoltageSensor voltageSensor;

void initArgb()
{
  pixels.setBrightness(ARGB_BRIGHTNESS);
  pixels.clear();
  pixels.show();
}

#define NO_ERROR 0

Thermister thermister;
SwitchSource swSource;
DataStore dataStore;
FanControl fanControl;
SensirionI2cSht4x sht;
void initSht40()
{
  char errorMessage[64];
  int16_t error;
  sht.begin(Wire, SHT40_I2C_ADDR_44);
  sht.softReset();
  delay(10);
  uint32_t serialNumber = 0;
  error = sht.serialNumber(serialNumber);
  if (error != NO_ERROR)
  {
    Serial.println("SHT40 not ready");
    return;
  }
  Serial.println("SHT40 ready");
  Serial.printf("SHT40 sn %d \n", serialNumber);
}

int testColorsIndex = 0;
bool testColorsIndexChanged = false;
// loop the colors
void IRAM_ATTR BTN_UTIL_3_ISR()
{
  testColorsIndex++;
  if (testColorsIndex > 7)
  {
    testColorsIndex = 0;
  }
  testColorsIndexChanged = true;
}

void initBtnUtility()
{
  pinMode(BTN_UTIL_PIN_3, INPUT_PULLUP);
  attachInterrupt(BTN_UTIL_PIN_3, BTN_UTIL_3_ISR, FALLING);
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(12, 11);
  fanControl.begin();
  initBtnUtility();
  initArgb();
  swSource.begin();
  voltageSensor.begin();
  thermister.begin();
  initSht40();
  dataStore.begin();
  initWifi(pixels);
}

unsigned long previousMillis = 0;
const long interval = 1000;

void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    ws.cleanupClients(4);
    if (ws.count() >= 1)
    {
      dataStore.setSht40Data(sht);
      dataStore.setThermisterData(thermister);
      dataStore.setVoltageSensorData(voltageSensor);
      // dataStore.printSensorData();
      String out = dataStore.getSensorDataJson();
      ws.textAll(out);
      Serial.println("Data sent");
    }
    previousMillis = currentMillis;
  }
}

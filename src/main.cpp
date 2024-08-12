#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <DataStore.h>
#include <FanControl.h>
#include <SwitchSource.h>
#include <Thermister.h>
#include <VoltageSensor.h>
#include <WS2812FX.h>
#include <WiFiConfig.h>
#include <MolexPowerMeter.h>
#include <BoardTempSensor.h>
#include <Oled.h>
#include <ConfigData.h>
#include <Adalight.h>

#define BTN_UTIL_PIN_3 17 // ARGB RND

#define ARGB_PIN 8
#define ARGB_DEBUG_PIN 3
#define COLOR_ORDER GRB
#define CHIPSET WS2811
#define NUM_LEDS 40
#define BRIGHTNESS 200
#define FRAMES_PER_SECOND 60

#define PCF8574_I2C_ADDRESS 0x21
#define INA219_5V_I2C_ADDRESS 0x40
#define INA219_12V_I2C_ADDRESS 0x41
#define INA3221_MOLEX_I2C_ADDRESS 0x43
#define SHT30_I2C_ADDRESS 0x44
#define ADS1115_I2C_ADDRESS 0x48
#define EEPROM_I2C_ADDRESS 0x50
#define EEPROM_I2C_ADDRESS 0x50
#define DS3231_I2C_ADDRESS 0x68
#define SERIAL_BAUD_RATE 115200
// #define SCL_PIN 11
// #define SDA_PIN 12
#define SCL_PIN 14
#define SDA_PIN 13
#define EEPROM_RESET_PIN 12


int ARGB_BRIGHTNESS = 60;
int ARGB_MODE = 1;
bool ARGB_FILLED = false;
WS2812FX ws2812fx = WS2812FX(NUM_LEDS, ARGB_PIN, NEO_GRB + NEO_KHZ800);

/// Define Sensors
VoltageSensor voltageSensor(INA219_5V_I2C_ADDRESS, INA219_12V_I2C_ADDRESS);
MolexPowerMeter molexPowerMeter(INA3221_MOLEX_I2C_ADDRESS);
BoardTempSensor boardTempSensor(SHTSensor::SHT3X);
Oled oled;
Thermister thermister;
SwitchSource swSource(PCF8574_I2C_ADDRESS);
DataStore dataStore(EEPROM_I2C_ADDRESS, I2C_DEVICESIZE_24LC256, EEPROM_RESET_PIN);
FanControl fanControl;
WiFiConfig wifiConfig(&dataStore, &ws2812fx);

void IRAM_ATTR BTN_UTIL_3_ISR()
{
}

void initBtnUtility()
{
  pinMode(BTN_UTIL_PIN_3, INPUT_PULLUP);
  attachInterrupt(BTN_UTIL_PIN_3, BTN_UTIL_3_ISR, FALLING);
}

void loadArgbConfig()
{
  ws2812fx.init();
  ws2812fx.setSpeed(dataStore.configData.argb.speed);
  ws2812fx.setMode(dataStore.configData.argb.mode);
  ws2812fx.setBrightness(dataStore.configData.argb.brightness);
  ws2812fx.start();
}

void mainLoopTask(void *parameter)
{
  while (true)
  {
    oled.service(dataStore.sensorData);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void fanTasks(void *parameter)
{
  for (;;)
  {
    fanControl.finalizePcnt();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  Wire.begin(SCL_PIN, SDA_PIN);
  dataStore.begin();
  loadArgbConfig();
  swSource.begin();
  swSource.initSource(dataStore.configData.fanSource, dataStore.configData.argb.source);
  fanControl.begin();
  fanControl.initAllDuty(dataStore.configData.fanDuty);
  // fanControl.setAllDuty(127);
  initBtnUtility();
  voltageSensor.begin();
  thermister.begin();
  boardTempSensor.begin();
  wifiConfig.begin(ws2812fx, swSource, dataStore, fanControl);
  oled.begin();

  // Create the main loop task and pin it to core 1
  xTaskCreatePinnedToCore(
      mainLoopTask,   // Task function
      "mainLoopTask", // Name of the task
      10000,          // Stack size (in bytes)
      NULL,           // Task input parameter
      1,              // Priority of the task
      NULL,           // Task handle
      0               // Core number (0 or 1)
  );

  xTaskCreatePinnedToCore(
      fanTasks,   // Task function
      "fanTasks", // Name of the task
      10000,      // Stack size (in bytes)
      NULL,       // Task input parameter
      2,          // Priority of the task
      NULL,       // Task handle
      0           // Core number (0 or 1)
  );              // Enable the alarm                // Enable the alarm

  pinMode(20, OUTPUT);
  delay(10);
  digitalWrite(20, LOW);
  delay(10);
}
unsigned long lastMillis = 0;
unsigned long lastIntervalMillis = 0;
void loop()
{
  unsigned long currentMillis = millis();
  wifiConfig.service();
  dataStore.service();
  ws2812fx.service();
  if (wifiConfig.ws.count() >= 1 && wifiConfig.ready() && currentMillis - lastMillis > 1000)
  {
    String out = dataStore.getSensorDataJson();
    wifiConfig.ws.textAll(out);
    lastMillis = currentMillis;
  }

  if (lastIntervalMillis - currentMillis > 1000)
  {
    boardTempSensor.readSensors(dataStore.sensorData.boardTemp.temp, dataStore.sensorData.boardTemp.humi, dataStore.sensorData.boardTemp.cpuTemp);
    dataStore.setThermisterData(thermister);
    swSource.readState(dataStore.configData.fanSource, dataStore.configData.argb.source);
    dataStore.setVoltageSensorData(voltageSensor);
    fanControl.readFanData(&dataStore.sensorData.fanData);
    lastIntervalMillis = currentMillis;
  }
}

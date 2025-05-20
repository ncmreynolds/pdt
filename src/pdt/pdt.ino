/*

   Sketch to work as a personal data transmitter device and associated tracker

   It can in principle be built across several different microcontrollers as I was indecisive during development but it's intended for the following...

   Beacon - ESP8266/8285 on custom PCB to keep the size down
   Tracker - ESP32-C3 on custom PCB to keep the size down

   The code is deliberately split into many several files to make it more manageable

   The same sketch has the code for both devices, uncomment the '#define ACT_AS_TRACKER' below to build for the tracker, otherwise it is a beacon

*/



//Hardware variant supports different PCBs and GPIO usage for different boards. This is not as modular as I would like

#define C3PDT 1
#define C3PDTasBeacon 2

#define C3TrackedSensor 3
#define C3TrackedSensorAsBeacon 4

#define C3LoRaBeacon 5

#define CYDTracker 6

#define HARDWARE_VARIANT C3PDT
//#define HARDWARE_VARIANT C3PDTasBeacon
//#define HARDWARE_VARIANT C3TrackedSensor
//#define HARDWARE_VARIANT C3TrackedSensorAsBeacon
//#define HARDWARE_VARIANT C3LoRaBeacon
//#define HARDWARE_VARIANT CYDTracker

#define PDT_MAJOR_VERSION 0
#define PDT_MINOR_VERSION 5
#define PDT_PATCH_VERSION 5
/*

   Various nominally optional features that can be switched off during testing/development

*/

#define SERIAL_DEBUG
#define SERIAL_LOG

#define USE_LITTLEFS
#define TREACLE_SEND_FAST

#if HARDWARE_VARIANT == C3PDT
  #define ACT_AS_TRACKER
  #define SUPPORT_BUTTON
  #define SUPPORT_DISPLAY
  #define SUPPORT_BEEPER
  #define DEBUG_BEEPER
  #define USE_SSD1331
  #define SUPPORT_GPS
  #define SUPPORT_WIFI
  //#define DEBUG_TREACLE
  //#define SUPPORT_SIGNAL_STRENGTH
  #define SUPPORT_BATTERY_METER
  #define ENABLE_LOCAL_WEBSERVER
#elif HARDWARE_VARIANT == C3PDTasBeacon
  #define ACT_AS_BEACON
  #define SUPPORT_GPS
  //#define DEBUG_GPS
  #define DEBUG_BEACON_SELECTION
  #define SUPPORT_WIFI
  //#define DEBUG_TREACLE
  #define DEBUG_UPDATES
  #define ENABLE_LOCAL_WEBSERVER
#elif HARDWARE_VARIANT == C3TrackedSensor
  #define ACT_AS_BEACON
  #define ACT_AS_SENSOR
  #define SUPPORT_BUTTON
  #define SUPPORT_BEEPER
  #define SUPPORT_VIBRATION
  #define SUPPORT_LED
  #define SUPPORT_GPS
  #define DEBUG_GPS
  #define SUPPORT_WIFI
  #define DEBUG_TREACLE
  #define SUPPORT_BATTERY_METER
  #define SUPPORT_HACKING
  #define ALLOW_DOWNLOAD_FILES
  //#define ENABLE_HACKING_TESTING
  #define ENABLE_LOCAL_WEBSERVER
#elif HARDWARE_VARIANT == C3TrackedSensorAsBeacon
  #define ACT_AS_BEACON
  #define SUPPORT_GPS
  #define DEBUG_GPS
  #define SUPPORT_WIFI
  //#define DEBUG_TREACLE
  #define SUPPORT_BATTERY_METER
  #define ENABLE_LOCAL_WEBSERVER
#elif HARDWARE_VARIANT == C3LoRaBeacon
  #define ACT_AS_BEACON
  #define SUPPORT_WIFI
  #define SUPPORT_BATTERY_METER
  #define SUPPORT_BUTTON
  #define SUPPORT_SOFT_POWER_OFF
  #define SUPPORT_SOFT_PERIPHERAL_POWER_OFF
  #define SUPPORT_LED
  #define SUPPORT_GPS
  //#define DEBUG_GPS
  //#define DEBUG_TREACLE
  //#define DEBUG_UPDATES
  #define ENABLE_LOCAL_WEBSERVER
#elif HARDWARE_VARIANT == CYDTracker
  #define ACT_AS_TRACKER
  #define SUPPORT_BEEPER
  #define SUPPORT_WIFI
  #define SUPPORT_GPS
  #define SUPPORT_LVGL
  #define LVGL_MANAGE_BACKLIGHT
  #define LVGL_SUPPORT_HOME_TAB
  //#define LVGL_SUPPORT_MAP_TAB
  #define LVGL_SUPPORT_SCAN_INFO_TAB
  #define LVGL_SUPPORT_GPS_TAB
  #define LVGL_SUPPORT_SETTINGS_TAB
  #define SUPPORT_TOUCHSCREEN
  #define SUPPORT_TOUCHSCREEN_BITBANG //Use bitbang code
  #define ENABLE_LOCAL_WEBSERVER
  //#define DEBUG_GPS
  #define DEBUG_BEACON_SELECTION
  #define DEBUG_TREACLE
  #define DEBUG_UPDATES
  //#define DEBUG_LVGL
#endif

#if HARDWARE_VARIANT == C3TrackedSensor
  #if defined(SUPPORT_HACKING)
    #undef ENABLE_LOCAL_WEBSERVER
  #endif
#endif
/*
 * 
 * This section is ALL the hardware specific stuff like pin numbering
 * 
 */
/*
 * 
 * Soft power off for device
 * 
 */
#if defined(SUPPORT_SOFT_POWER_OFF)
  #if HARDWARE_VARIANT == C3LoRaBeacon
    static const int8_t softPowerOffPin = 1; //Keeping this high keeps the PCB powered up. Low powers it off
    static const bool softPowerOffPinInverted = false;  //Is GPIO inverted
  #endif
#endif
/*
 * 
 * Peripheral power
 * 
 */
#if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
  #if HARDWARE_VARIANT == C3LoRaBeacon
    static const int8_t peripheralPowerOffPin = 3; //Switches GPS and other peripherals on/off
    static const bool peripheralPowerOffPinInverted = false;  //Is GPIO inverted
  #endif
#endif
/*
 * 
 * Battery meter
 * 
 */
#if defined(SUPPORT_BATTERY_METER)
  #if HARDWARE_VARIANT == C3PDT || HARDWARE_VARIANT == C3PDTasBeacon
    static const int8_t voltageMonitorPin = A0;    
    float ADCpeakVoltage = 2.5;
    float topLadderResistor = 330.0;
    float bottomLadderResistor = 89; //Actually 100k, but the ESP itself has a parallel resistance that effectively lowers it
  #elif HARDWARE_VARIANT == C3TrackedSensor || HARDWARE_VARIANT == C3TrackedSensorAsBeacon
    static const int8_t voltageMonitorPin = A0;    
    float ADCpeakVoltage = 2.5;
    float topLadderResistor = 330.0;
    float bottomLadderResistor = 104; //Actually 100k, but the ESP itself has a parallel resistance that effectively lowers it
  #elif HARDWARE_VARIANT == C3LoRaBeacon
    static const int8_t voltageMonitorPin = 0;
    float ADCpeakVoltage = 1.3;
    float topLadderResistor = 330.0;
    float bottomLadderResistor = 104; //Actually 100k, but the ESP itself has a parallel resistance that effectively lowers it
  #endif
#endif
/*
 * 
 * GPS support
 * 
 */
#if defined(SUPPORT_GPS)
  #if ARDUINO_USB_CDC_ON_BOOT == 1
    #define GPS_PORT Serial0
  #else
    #if defined(ARDUINO_ESP32C3_DEV)
      #define GPS_PORT Serial
    #else
      #define GPS_PORT Serial1
    #endif
  #endif
  #if HARDWARE_VARIANT == C3PDT || HARDWARE_VARIANT == C3PDTasBeacon
    static const int8_t RXPin = 20;              //GPS needs an RX pin, but it can be moved wherever, within reason
    static const int8_t TXPin = -1;              //No TX pin
  #elif HARDWARE_VARIANT == C3TrackedSensor || HARDWARE_VARIANT == C3TrackedSensorAsBeacon
    static const int8_t RXPin = 20;              //GPS needs an RX pin, but it can be moved wherever, within reason
    static const int8_t TXPin = -1;              //No TX pin
  #elif HARDWARE_VARIANT == C3LoRaBeacon
    static const int8_t RXPin = 20;              //GPS needs an RX pin, but it can be moved wherever, within reason
    static const int8_t TXPin = -1;              //No TX pin
  #elif HARDWARE_VARIANT == CYDTracker
    static const int8_t RXPin = 35;              //GPS needs an RX pin, but it can be moved wherever, within reason
    static const int8_t TXPin = -1;              //No TX pin
  #endif
#endif
/*
 * 
 * Treacle multi-protocol wireless library
 * 
 */
/*
 * 
 * LoRa support
 * 
 */
#if HARDWARE_VARIANT == C3PDT || HARDWARE_VARIANT == C3PDTasBeacon
  static const int8_t loRaCsPin = 7;          // LoRa radio chip select
  static const int8_t loRaResetPin = 8;       // LoRa radio reset
  static const int8_t loRaIrqPin = 10;        // change for your board; must be a hardware interrupt pin
#elif HARDWARE_VARIANT == C3TrackedSensor || HARDWARE_VARIANT == C3TrackedSensorAsBeacon
  static const int8_t loRaCsPin = 7;          // LoRa radio chip select
  static const int8_t loRaResetPin = 8;       // LoRa radio reset
  static const int8_t loRaIrqPin = 10;        // change for your board; must be a hardware interrupt pin
#elif HARDWARE_VARIANT == C3LoRaBeacon
  static const int8_t loRaCsPin = 7;          // LoRa radio chip select
  static const int8_t loRaResetPin = 2;       // LoRa radio reset
  static const int8_t loRaIrqPin = 10;        // change for your board; must be a hardware interrupt pin
#elif HARDWARE_VARIANT == CYDTracker
  //Sd card adaptor attachment
  //static const int8_t loRaCsPin = 5;        // LoRa radio chip select
  //static const int8_t loRaResetPin = 22;    // LoRa radio reset
  //static const int8_t loRaIrqPin = 27;      // change for your board; must be a hardware interrupt pin
  //Hand soldered attachment
  static const int8_t loRaCsPin = 17;         // LoRa radio chip select
  static const int8_t loRaResetPin = 16;      // LoRa radio reset
  static const int8_t loRaIrqPin = 4;         // change for your board; must be a hardware interrupt pin
#endif
#if defined(SUPPORT_BEEPER)
  static const uint8_t beeperChannel = 2;
  #if HARDWARE_VARIANT == C3PDT || HARDWARE_VARIANT == C3PDTasBeacon
    static const int8_t beeperPin = 21;
  #elif HARDWARE_VARIANT == C3TrackedSensor || HARDWARE_VARIANT == C3TrackedSensorAsBeacon
    static const int8_t beeperPin = 3;
  #elif HARDWARE_VARIANT == CYDTracker
    static const int8_t beeperPin = 26;
  #endif
#endif
#if defined(SUPPORT_LED)
  #if HARDWARE_VARIANT == C3TrackedSensor || HARDWARE_VARIANT == C3TrackedSensorAsBeacon
    static const int8_t ledPin = 2;
    static const bool ledPinInverted = false;
  #elif HARDWARE_VARIANT == C3LoRaBeacon
    static const int8_t ledPin = TX;
    static const bool ledPinInverted = false;
  #endif
#endif
#if defined(SUPPORT_VIBRATION)
  #if HARDWARE_VARIANT == C3TrackedSensor
    static const int8_t vibrationPin = 9;
    static const bool vibrationPinInverted = false;
  #endif
#endif
#if defined(SUPPORT_BUTTON)
  #if HARDWARE_VARIANT == C3PDT || HARDWARE_VARIANT == C3PDTasBeacon
    static const int8_t buttonPin = 9;
  #elif HARDWARE_VARIANT == C3TrackedSensor || HARDWARE_VARIANT == C3TrackedSensorAsBeacon
    static const int8_t buttonPin = 21;
  #elif HARDWARE_VARIANT == C3LoRaBeacon
    static const int8_t buttonPin = 9;
  #elif HARDWARE_VARIANT == CYDTracker
    static const int8_t buttonPin = 0;
  #endif
#endif
#if defined(SUPPORT_DISPLAY)
  #if HARDWARE_VARIANT == C3PDT
    static const uint8_t displayCSpin = 1;
    static const uint8_t displayResetPin = 3;
    static const uint8_t displayDCpin = 2;
  #endif
#endif
#if defined(SUPPORT_LVGL)
  #if HARDWARE_VARIANT == CYDTracker
    static const int8_t ldrPin = 34;
    static const int8_t lcdBacklightPin = 21;
  #endif
#endif
/*
 * 
 * These are 'behaviour control #defines
 * 
 */
#if defined(ACT_AS_TRACKER)
  //#pragma message "Acting as tracker"
#elif defined(ACT_AS_BEACON)
  //#pragma message "Acting as beacon"
#endif
/*
 * 
 * Web server options
 * 
 */
#if defined(ENABLE_LOCAL_WEBSERVER)
  #define ENABLE_LOCAL_WEBSERVER_SEMAPHORE
  #define SERVE_CONFIG_FILE
  #define DEBUG_LOCAL_WEBSERVER
  #define ENABLE_REMOTE_RESTART
  #define ENABLE_LOG_DELETION
  //#define ENABLE_LOCAL_WEBSERVER_FIRMWARE_UPDATE
  //#define ENABLE_OTA_UPDATE
  //#define SUPPORT_OTA
  //#define ENABLE_LOCAL_WEBSERVER_BASIC_AUTH //Uncomment to password protect the web configuration interface
#endif
/*

   Block of includes that are always used

*/
#include <USBCDC.h>
#include <USB.h>
#include <LittleFS.h> //LittleFS storage
#include <FS.h> //Filesystem library
#include <time.h> //Time/NTP library
#include <ArduinoJson.h>  //Used to serialise/deserialise configuration files and server responses
/*
 * 
 * WiFi and IP
 * 
 */
#if defined(SUPPORT_WIFI)
  #include <espBoilerplate.h>
  #if defined(ENABLE_LOCAL_WEBSERVER) || defined(SUPPORT_HACKING)
    #include <DNSServer.h>
  #endif
  #if defined(ENABLE_OTA_UPDATE)
    #include <ArduinoOTA.h> //Over-the-air update library
  #endif
  #define WIFI_SSID "YOUR_SSID"
  #define WIFI_PSK "YOUR_PSK"
#endif
/*
 * 
 * Lasertag sensor
 * 
 */
#if defined(ACT_AS_SENSOR)
  #include <lasertag.h>
  lasertag sensor;
  #include <Preferences.h>
  Preferences sensorPersitentData;
  #define IR_RECEIVER_PIN 1
  bool sensorReset = false;
  uint8_t defaultNumberOfStartingHits = 6;
  uint8_t defaultNumberOfStartingStunHits = 6;
  uint8_t armourValue = 0;
  uint8_t bleedOutCounter = 0;
  uint8_t bleedOutTime = 5;
  bool EP_flag = false; //Sensor requires EP set in signal to be hit
  bool ig_healing_flag = false; //Sensor ignores healing
  bool ig_stun_flag = false; //Sensor ignores stun hits
  bool ig_ongoing_flag = false; //Sensor ignores ongoing hits
  bool regen_while_zero = false; //Sensor can regenerate from zero
  bool treat_as_one = false; //Sensor treats all hits as one damage
  bool treat_stun_as_one = false; //Sensor treats all stun as one
  bool ongoing_is_cumulative = false; //Sensor adds ongoing damage to current ongoing value
  bool ig_non_dot = false; //Sensor ignores non-DOT signals
  enum class sensorState : uint8_t {
    starting = 0,
    playStartupAnimation = 1,
    active = 2,
    playCurrentHits = 3,
    nearMiss = 4,
    invalidSignal = 5,
    takenHit = 6,
    dead = 7,
    bleedOut = 8,
    stunned = 9,
    resetting = 10
  };
  sensorState currentSensorState = sensorState::starting;
  sensorState previousSensorState = sensorState::starting;
  uint32_t lastSensorStateChange = 0;
  uint32_t timeOfLastHit = 0;
  uint32_t sensorTones[4] = {3777, 800, 1200, 1000};
  uint32_t sensorToneOnTimes[4] = {200, 250, 500, 199};
  uint32_t sensorToneOffTimes[4] = {0, 250, 500, 1};
  const char* startingHitsKey = "startHits";
  const char* currentHitsKey =  "currHits";
  const char* startingStunKey = "startStun";
  const char* currentStunKey =  "currStun";
  const char* EP_flag_key = "EP_flag";
  const char* ig_healing_flag_key = "ig_healing_flag";
  const char* ig_stun_flag_key = "ig_stun_flag";
  const char* ig_ongoing_flag_key = "ig_ongoing_flag";
  const char* regen_from_zero_key = "regen_from_zero";
  const char* hit_as_one_key = "hit_as_one";
  const char* stun_as_one_key = "stun_as_one";
  const char* ongoing_adds_key = "ongoing_adds";
  const char* ig_non_dot_key = "ig_non_dot";
  //const char* currentSensorStateKey = "currentSensorState";
  const char* bleedOutCounterKey = "bleedOutCounter";
  uint32_t saveSensorConfigurationSoon = 0;
#endif
/*
 * 
 * Block of includes for ESPAsyncWebServer, which is used to make a configuration interface and allow you to download log files
 * 
 */
#if defined(ENABLE_LOCAL_WEBSERVER)
  #if defined(ESP32)
    #include <AsyncTCP.h>
  #elif defined(ESP8266)
    #include <ESPAsyncTCP.h>
  #endif
  #include <ESPAsyncWebServer.h>
  AsyncWebServer* adminWebServer;
  uint16_t adminWebServerPort = 80;
  char normalize[] PROGMEM =
#include "css/normalizecss.h"
    ;
  char skeleton[] PROGMEM =
#include "css/skeletoncss.h"
    ;
  void addPageHeader(AsyncResponseStream *response, uint8_t refresh, const char* refreshTo);
  void addPageFooter(AsyncResponseStream *response);
  #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
    SemaphoreHandle_t webserverSemaphore = NULL;
    const uint16_t webserverSemaphoreTimeout = 50;
  #endif
#endif
/*

   Includes for SPI peripherals

*/
#if defined(SUPPORT_DISPLAY)
  #include <SPI.h>
#endif
/*

   Block of includes for the display

*/
#if defined(SUPPORT_DISPLAY)
  #if defined(USE_SSD1331)
    #include "ssd1306.h"
  #endif
#endif
/*

   Block of includes to get extra information about the microcontroller status

*/
#if defined(ESP32)
  #if ESP_IDF_VERSION_MAJOR > 3 // IDF 4+
    #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
      #include "esp32/rom/rtc.h"
    #elif CONFIG_IDF_TARGET_ESP32S2
      #include "esp32s2/rom/rtc.h"
    #elif CONFIG_IDF_TARGET_ESP32C3
      #include "esp32c3/rom/rtc.h"
    #else
      #error Target CONFIG_IDF_TARGET is not supported
    #endif
  #else // ESP32 Before IDF 4.0
    #include "rom/rtc.h"
  #endif
#endif
/*
 * 
 * FTM
 * 
 */
#if defined(SUPPORT_FTM)
  bool ftmEnabled = false;
  char* ftmSSID = nullptr;
  bool ftmHideSSID = false;
  char* ftmPSK = nullptr;
  const char* default_ftmWiFi_SSID = "_ftm";
  const char* default_ftmWiFi_PSK = "12345678";
#endif
/*
 * 
 * Needed to encode the data in packets
 * 
 */
#include <MsgPack.h>  //MsgPack is used to transmit data
#include "CRC16.h" //A CRC16 is used to check the packet is LIKELY to be sent in a known format
#include "CRC.h"
#define LORA_CRC_POLYNOME 0xac9a  //Taken from https://users.ece.cmu.edu/~koopman/crc/

//These are message packing IDs
static const uint8_t locationUpdateId = 0x00;           //LoRa packet contains location info from a beacon
static const uint8_t deviceStatusUpdateId = 0x10;       //LoRa packet contains device info, shared infrequently
static const uint8_t deviceIcInfoId = 0x11;             //LoRa packet contains IC game info, shared very infrequently
/*
 * 
 * Treacle multi-protocol wireless library
 * 
 */
#include <treacle.h>
const char string_id[] PROGMEM = "id";
bool treacleIntialised = false;
uint8_t localMacAddress[6] = {};
bool treacleEncryptionEnabled = false;
const char string_treacleEncryptionEnabled[] PROGMEM = "treacleEncryptionEnabled";
uint32_t lastTreacleDeviceInfoSendTime = 0;       //Track the last time of updates
uint32_t treacleDeviceInfoInterval = 10E3;        //Send device info every 60s
uint8_t typeOfLastTreacleUpdate = 0;              //Use to cycle through update types
//ESP-Now
bool espNowEnabled =  true;
const char string_espNowEnabled[] PROGMEM = "espNowEnabled";
uint8_t espNowPhyMode =  0; //0 = BGN, 1 = B, 2 = LR
const char string_espNowPhyMode[] PROGMEM = "espNowPhyMode";
uint32_t espNowTickInterval = 10e3;
const char string_espNowTickInterval[] PROGMEM = "espNowTickInterval";
bool espNowInitialised =  false;
//LoRa
bool loRaEnabled  = true;
const char string_loRaEnabled[] PROGMEM = "loRaEnabled";
uint32_t loRaTickInterval = 30e3;
const char string_loRaTickInterval[] PROGMEM = "loRaTickInterval";
uint32_t loRaFrequency = 868E6;
const char string_loRaFrequency[] PROGMEM = "loRaFrequency";
uint8_t loRaTxPower = 17;
const char string_loRaTxPower[] PROGMEM = "loRaTxPower";
uint8_t loRaRxGain = 0;
const char string_loRaRxGain[] PROGMEM = "loRaRxGain";
uint8_t loRaSpreadingFactor = 7;
const char string_loRaSpreadingFactor[] PROGMEM = "loRaSpreadingFactor";
uint32_t loRaSignalBandwidth = 62.5E3;
const char string_loRaSignalBandwidth[] PROGMEM = "loRaSignalBandwidth";
uint32_t validLoRaSignalBandwidth[] = {7800, 10400, 15600, 20800, 31250, 41700, 62500, 125000, 250000, 500000};
bool loRaInitialised = false;
//MQTT
//COBS
/*

   Pin configurations

*/
#if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
  bool debugPortAvailable = true;
  uint16_t debugPortStartingBufferSize = 0;
  uint32_t serialBufferCheckTime = 0;
  #if ARDUINO_USB_CDC_ON_BOOT == 1
    #pragma message "USB CDC configured on boot for debug messages"
    #define SERIAL_DEBUG_PORT Serial
  #else
    #if defined(ARDUINO_ESP32C3_DEV)
      #pragma message "Configuring USB CDC for debug messages"
      #if ESP_IDF_VERSION_MAJOR > 3
        #include "USB.h"
        #if ARDUINO_USB_CDC_ON_BOOT
          #define SERIAL_DEBUG_PORT Serial
        #else
          USBCDC USBSerial;
          #define SERIAL_DEBUG_PORT USBSerial
        #endif
      #else
        #define SERIAL_DEBUG_PORT USBSerial
      #endif
    #else
      #pragma message "Configuring Hardware UART for debug messages"
      #define SERIAL_DEBUG_PORT Serial
    #endif
  #endif
#endif
/*
 * 
 * GPS support
 * 
 */
#if defined(SUPPORT_GPS)
  #include <TinyGPS++.h>
  TinyGPSPlus gps;
  TaskHandle_t gpsManagementTask = NULL;
  SemaphoreHandle_t gpsSemaphore = NULL;
  static const uint16_t gpsSemaphoreTimeout = 5;
  static const uint16_t gpsYieldTime = 100;
  static const uint8_t excellentHdopThreshold = 1;
  static const uint8_t goodHdopThreshold = 2;
  static const uint8_t normalHdopThreshold = 4;
  static const uint8_t poorHdopThreshold = 6;
  static const uint8_t minimumViableHdop = 10;
  static const uint32_t GPSBaud = 9600;
  uint32_t lastGpsTimeCheck = 0;
  uint32_t lastGPSstateChange = 0;
  uint32_t gpsTimeCheckInterval = 30000;
  uint32_t lastDistanceCalculation = 0;
  uint16_t distanceCalculationInterval = 2E3;
  uint32_t lastGPSstatus = 0;
  uint32_t gpsChars = 0;
  uint16_t gpsSentences = 0;
  uint16_t gpsErrors = 0;
  bool useGpsForTimeSync = true;
  const char string_useGpsForTimeSync[] PROGMEM = "useGpsForTimeSync";
#endif
/*
 * 
 * Battery meter
 * 
 */
#if defined(SUPPORT_BATTERY_METER)
  bool enableBatteryMonitor = true;
  const char string_enableBatteryMonitor[] PROGMEM = "enableBatteryMonitor";
  const char string_topLadderResistor[] PROGMEM = "topLadderResistor";
  const char string_bottomLadderResistor[] PROGMEM = "bottomLadderResistor";
  float chargingVoltage = 4.19;
#endif
/*
 * 
 * Button
 * 
 */
#if defined(SUPPORT_BUTTON)
  uint32_t buttonPushTime = 0;
  uint32_t buttonDebounceTime = 50;
  uint32_t buttonLongPressTime = 1500;
  bool buttonHeld = false;
  bool buttonLongPress = false;
  #if defined(SUPPORT_BEEPER)
    bool beepOnPress = false;
    const char string_beepOnPress[] PROGMEM = "beepOnPress";
  #endif
#endif
/*
 * 
 * Default values
 * 
 */
#if defined(ACT_AS_BEACON)
  const char* default_deviceName = "PDT beacon";
#else
  #if HARDWARE_VARIANT == CYDTracker
    const char* default_deviceName = "CYD tracker";
  #else
    const char* default_deviceName = "PDT tracker";
  #endif
#endif
char* configurationComment = nullptr;
char default_configurationComment[] = "";
//Username for the Web UI
#if defined(ENABLE_LOCAL_WEBSERVER) && defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
  const char default_http_user[] = "pdt";
  char* http_user = nullptr;
  const char string_http_user[] PROGMEM = "http_user";
#endif
//Password for the Web UI and over-the-air update
#if (defined(ENABLE_LOCAL_WEBSERVER) && defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)) || defined(ENABLE_OTA_UPDATE)
  const char default_http_password[] = "pdtpassword";
  char* http_password = nullptr;
  const char string_http_password[] PROGMEM = "http_password";
  bool basicAuthEnabled = false;
  const char string_basicAuthEnabled[] PROGMEM = "basicAuthEnabled";
#endif
File openFileForReading(const char* filename); //This only exists to improve code readibility by removing the SPIFFS/LittleFS conditional compilation
File openFileForWriting(const char* filename); //This only exists to improve code readibility by removing the SPIFFS/LittleFS conditional compilation
File openFileForAppend(const char* filename); //This only exists to improve code readibility by removing the SPIFFS/LittleFS conditional compilation
/*

   Block of conditional includes for LittleFS

*/
bool filesystemMounted = false; // used to discern whether SPIFFS/LittleFS has correctly initialised
#if defined(USE_SPIFFS)
  char configurationFile[] = "configuration.json";  //Does not require a leading slash
#elif defined(USE_LITTLEFS)
  char configurationFile[] = "/configuration.json"; //Requires a leading slash
  //void mountFilesystem(bool showInfo = true);  //Attempt to mount the filesystem, showinfo false shows no info
#endif
//String configurationMd5;  //An MD5 hash of the configuration, to check for changes
uint32_t saveConfigurationSoon = 0; //Used to delay saving configuration immediately
/*
 * 
 * NTP/time configuration
 * 
 */
const char* default_timeServer = "pool.ntp.org";  //The default time server
char* timeServer = nullptr;
const char string_timeServer[] PROGMEM = "timeServer";
const char* default_timeZone = "GMT0BST,M3.5.0/1,M10.5.0"; //The default DST setting for Europe/London, see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
char* timeZone = nullptr;
const char string_timeZone[] PROGMEM = "timeZone";
//const char* timeZone = "UTC0"; //Use this for UTC
char timestamp[] = "XX:XX:XX XX-XX-XXXX"; //A string overwritten with the current timestamp for logging.
uint64_t bootTime = 0; //Use the moment the system got a valid NTP time to calculate the boot time for approximate uptime calculations
uint32_t restartTimer = 0;  //Used to schedule a restart
uint32_t wipeTimer = 0;  //Used to schedule a wipe
/*
 * 
 * WiFi management
 * 
 */
#if defined(SUPPORT_WIFI)
  bool startWiFiClientOnBoot = false;
  const char string_startWiFiClientOnBoot[] PROGMEM = "startWiFiClientOnBoot";
  bool startWiFiApOnBoot = true;
  const char string_startWiFiApOnBoot[] PROGMEM = "startWiFiApOnBoot";
  #if defined(ENABLE_LOCAL_WEBSERVER) || defined(SUPPORT_HACKING)
    bool enableCaptivePortal = true;
    const char string_enableCaptivePortal[] PROGMEM = "enableCaptivePortal";
    DNSServer* dnsServer = nullptr; //May not be used so don't create the object unless it is enabled
  #endif
  uint32_t wiFiClientInactivityTimer = 0;
  const char string_wiFiClientInactivityTimer[] PROGMEM = "wiFiClientInactivityTimer";
  uint32_t lastWifiActivity = 0;
  const char* default_WiFi_SSID = WIFI_SSID;
  const char* default_WiFi_PSK = WIFI_PSK;
  char* SSID = nullptr;
  const char string_SSID[] PROGMEM = "SSID";
  char* PSK = nullptr;
  const char string_PSK[] PROGMEM = "PSK";
  const char* default_AP_PSK = "12345678";
  char* APSSID = nullptr;
  const char string_APSSID[] PROGMEM = "APSSID";
  char* APPSK = nullptr;
  const char string_APPSK[] PROGMEM = "APPSK";
  uint8_t softApChannel = 1;
  const char string_softApChannel[] PROGMEM = "softApChannel";
  uint8_t wifiClientTimeout = 30;
  const char string_wifiClientTimeout[] PROGMEM = "wifiClientTimeout";
  const int8_t networkTimeout = 30;  //Timeout in seconds for network connection
  static bool wifiClientConnected = false; //Is the network connected?
  static bool wifiApStarted = false; //Is the WiFi AP started?
  //static bool networkStateChanged = false;  //Has the state changed since initial connection
#endif
/*
 * 
 * Over-the-update
 * 
 */
#if defined(ENABLE_OTA_UPDATE)
  volatile bool otaInProgress = false;
  uint8_t otaProgress = 0;
  bool otaEnabled = true;
  bool otaAuthenticationEnabled = false;
  //void configureOTA();
#endif
/*
 * 
 * Local logging
 * 
 */
#if defined(USE_SPIFFS)
  const char *logDirectory PROGMEM = "logs";
  const char *logfilenameTemplate PROGMEM = "%s/log-%04u-%02u-%02u.txt";
#elif defined(USE_LITTLEFS)
  const char *logDirectory PROGMEM = "/logs";
  const char *logfilenameTemplate PROGMEM = "%s/log-%04u-%02u-%02u.txt";
#endif
const uint8_t logFilenameLength = 25;
char logFilename[logFilenameLength]; //Big enough for with or without leading /
uint8_t logfileDay = 0; //Used to detect rollover
uint8_t logfileMonth = 0; //Used to detect rollover
uint16_t logfileYear = 0; //Used to detect rollover
bool startOfLogLine = true; //If true then the logging add the time/date at the start of the line
String loggingBuffer = ""; //A logging backlog buffer
uint32_t loggingBufferSize = 4096; //The space to reserve for a logging backlog. This is not a hard limit, it is to reduce heap fragmentation.
const char string_loggingBufferSize[] PROGMEM = "loggingBufferSize";
uint32_t logLastFlushed = 0;  //Time (millis) of the last log flush
bool autoFlush = false;
bool flushLogNow = false;
uint32_t logFlushInterval = 60; //Frequency in seconds of log flush, this is 60s
const char string_logFlushInterval[] PROGMEM = "logFlushInterval";
uint32_t logFlushThreshold = loggingBufferSize*3/4; //Threshold for forced log flush
const char string_logFlushThreshold[] PROGMEM = "logFlushThreshold";
SemaphoreHandle_t loggingSemaphore = NULL;
static const uint16_t loggingSemaphoreTimeout = 50;
TaskHandle_t loggingManagementTask = NULL;
static const uint16_t loggingYieldTime = 100;
/*
 * 
 * GPS support variables
 * 
 */
#if defined(SUPPORT_GPS)
  struct deviceInfo {
    uint8_t id = 0;
    char* name = nullptr;
    bool hasGpsFix = false;
    uint8_t typeOfDevice = 0; // bitmask 0x00 = beacon, 0x01 = tracker, 0x02 = sensor, 0x04 = emitter, 0x08 = FTM beacon, 0x40 = battery monitor, 0x80 = mostly stationary
    float supplyVoltage = 0;  // Battery health can be guesstimated from this
    uint32_t uptime = 0;  // Check for reboots
    uint8_t majorVersion = 0; //Software version
    uint8_t minorVersion = 0;
    uint8_t patchVersion = 0;
    double latitude = 0;  //Location info
    double longitude = 0;
    float course = 0;
    float speed = 0;
    float hdop = 0;
    float distanceTo = 0;
    float courseTo = 0;
    float smoothedSpeed = 1;
    bool moving = true;
    uint8_t numberOfStartingHits = 0;
    uint8_t numberOfStartingStunHits = 0;
    uint8_t currentNumberOfHits = 0;
    uint8_t currentNumberOfStunHits = 0;
    //IC information
    char* icName = nullptr;
    char* icDescription = nullptr;
    uint32_t diameter = 1;
    uint32_t height = 2;
  };
  const char string_icName[] PROGMEM = "icName";
  const char string_icDescription[] PROGMEM = "icDescription";
  const char string_diameter[] PROGMEM = "diameter";
  const char string_height[] PROGMEM = "height";
  const char string_deviceName[] PROGMEM = "deviceName";
  const char string_configurationComment[] PROGMEM = "configurationComment";
  static const uint8_t maximumNumberOfDevices = 16;
  deviceInfo device[maximumNumberOfDevices];
  float movementThreshold = 0.2;
  const char string_movementThreshold[] PROGMEM = "movementThreshold";
  float smoothingFactor = 0.75;
  const char string_smoothingFactor[] PROGMEM = "smoothingFactor";
  bool deviceUsuallyStatic = false;
  const char string_deviceUsuallyStatic[] PROGMEM = "deviceUsuallyStatic";
  uint8_t numberOfDevices = 0;
  #if defined(ACT_AS_TRACKER)
    uint8_t trackingSensitivity = 2;
  #else
    uint8_t trackingSensitivity = 3;
  #endif
  const char string_trackingSensitivity[] PROGMEM = "trackingSensitivity";
  uint16_t sensitivityValues[4] = {0x0FFF, 0x00FF, 0x000F, 0x0007};
  //Control of effective range
  double effectivelyUnreachable = 1E10;
  #if defined(ACT_AS_TRACKER)
    uint32_t maximumEffectiveRange = 250;
  #else
    uint32_t maximumEffectiveRange = 10000;
  #endif
  const char string_maximumEffectiveRange[] PROGMEM = "maximumEffectiveRange";
  uint8_t currentlyTrackedDevice = maximumNumberOfDevices;              //maximumNumberOfDevices implies none found
  uint32_t lastChangeOfTrackedDevice = 0;
  bool distanceToCurrentlyTrackedDeviceChanged = false;
  #if defined(ACT_AS_TRACKER)
    uint8_t trackerPriority = 0;
    const char string_trackerPriority[] PROGMEM = "trackerPriority";
    uint32_t lastDistanceChangeUpdate = 0;
    bool currentlyTrackedDeviceStateChanged = false; //Note when it has been show
    enum class trackingMode : std::uint8_t {
      nearest = 0,
      furthest = 1,
      fixed = 2
    };
    trackingMode currentTrackingMode = trackingMode::nearest;
  #endif
#endif
/*
 * 
 * Display support variables
 * 
 */
#if defined(SUPPORT_DISPLAY)
  void displayStatus();
  enum class displayState : std::int8_t {
    blank,
    welcome,
    distance,
    course,
    status,
    accuracy,
    trackingMode,
    signal,
    battery,
  #if defined(SUPPORT_BEEPER)
    beeper,
  #endif
    version,
    menu
  };
  displayState currentDisplayState = displayState::blank;
  static const uint8_t screenWidth = 96;
  static const uint8_t screenHeight = 64;
  uint32_t lastDisplayUpdate = 0;
  static const uint32_t longDisplayTimeout = 30000;
  static const uint32_t shortDisplayTimeout = 5000;
  uint32_t displayTimeout = 30000;
#endif
/*
 * 
 * Block of includes and variable for LVGL on CYD
 * 
 */
#if defined(SUPPORT_LVGL)
  #include <TFT_eSPI.h>
  TFT_eSPI tft = TFT_eSPI();
  #include <lvgl.h>

  //Screen resolution
  static const uint16_t screenWidth  = 240;
  static const uint16_t screenHeight = 320;
  static const uint8_t bufferFraction = 16;
  uint8_t screenRotation = 0; //0 = USB at bottom, 1 = USB on right, 2 = USB at top, 3 = USB on left
  const char string_screenRotation[] PROGMEM = "screenRotation";

  #if defined(SUPPORT_TOUCHSCREEN) || defined(SUPPORT_TOUCHSCREEN_BITBANG)
    #include <XPT2046_Touchscreen.h>
    // The CYD touch uses some non default SPI pins
    #define XPT2046_IRQ 36
    #define XPT2046_MOSI 32
    #define XPT2046_MISO 39
    #define XPT2046_CLK 25
    #define XPT2046_CS 33
    #ifndef SUPPORT_TOUCHSCREEN_BITBANG
      SPIClass touchscreenSPI = SPIClass(VSPI);
      XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
    #endif
    #if defined(SUPPORT_TOUCHSCREEN_BITBANG)
      #define XPT2046_CMD_READ_X  0xD0 // Command for XPT2046 to read X position    #define XPT2046_MOSI 32
      #define XPT2046_CMD_READ_Y  0x90 // Command for XPT2046 to read Y position
      uint32_t lastCheckForTouch = 0;
    #endif
    uint16_t touchScreenMinimumX = 0, touchScreenMaximumX = 0, touchScreenMinimumY = 0,touchScreenMaximumY = 0;
    const char string_touchScreenMinimumX[] PROGMEM = "touchScreenMinimumX";
    const char string_touchScreenMaximumX[] PROGMEM = "touchScreenMaximumX";
    const char string_touchScreenMinimumY[] PROGMEM = "touchScreenMinimumY";
    const char string_touchScreenMaximumY[] PROGMEM = "touchScreenMaximumY";
    bool touchscreenInitialised = false;
  #endif
  
  enum class deviceState: std::int8_t {
    starting,
    detectingGpsPins,
    detectingGpsBaudRate,
    calibrateScreen,
    gpsDetected,
    gpsLocked,
    tracking
  };
  deviceState currentLvglUiState = deviceState::starting;

  //LVGL
  
  static lv_disp_draw_buf_t draw_buf;
  lv_color_t *buf;  //This is assigned dynamically at startup to reduce static space usage
  
  //Tab view
  static lv_obj_t * tabview;
  uint8_t tabHeight = 40;

  #if defined(LVGL_SUPPORT_HOME_TAB) || defined(LVGL_SUPPORT_GPS_TAB)
    uint32_t lastLvglTabUpdate = 0;
    uint32_t lvglTabUpdateInterval = 500;
  #endif

  //Home tab
  #if defined(LVGL_SUPPORT_HOME_TAB)
    bool enableHomeTab = true;
    bool homeTabInitialised = false;
    const char string_enableHomeTab[] PROGMEM = "enableHomeTab";
    lv_obj_t * homeTab = nullptr;
    static const char homeTabLabel[] = "Home";
    lv_obj_t * status_spinner = nullptr;
    lv_obj_t * status_label = nullptr;

    static const char statusLabel_0[] PROGMEM = "Starting";
    static const char statusLabel_1[] PROGMEM = "Detecting hardware";
    static const char statusLabel_2[] PROGMEM = "Calibrating hardware";
    static const char statusLabel_3[] PROGMEM = "Touchscreen\ncalibration needed\nTouch top left\nthen bottom right.";
    static const char statusLabel_4[] PROGMEM = "Getting location";
    static const char statusLabel_5[] PROGMEM = "Scanning";

    bool metersShowing = false;

    static lv_obj_t * trackedItemLabel;
    static const uint8_t trackedItemLabelSpacing = 100;

    static lv_style_t style_meter;
    static uint8_t meterDiameter = 100;
    static const uint8_t meterXspacing = 8;
    static const int8_t meterYspacing = -50;
    
    static lv_obj_t * meter0;
    static lv_obj_t * meter0withoutNeedle;
    static lv_meter_indicator_t * needle0;
    static lv_obj_t * meter0label0;
    
    static lv_obj_t * meter1;
    static lv_meter_indicator_t * needle1;
    static lv_obj_t * meter1label0;

    const char meter1text0[] PROGMEM = "To centre(m)";
    const char meter1text1[] PROGMEM = "To edge(m)";
    
    static lv_obj_t * chart0;
    static lv_chart_series_t * chart0ser0;
    static lv_chart_series_t * chart0ser1;
    static lv_obj_t * chart0label0;
    static const uint8_t chart0Spacing = 8;

    #if defined(LVGL_INCLUDE_CHART1)
      static lv_obj_t * chart1;
      static lv_chart_series_t * chart1ser0;
      static lv_chart_series_t * chart1ser1;
      static lv_obj_t * chart1label0;
    #endif
  
    static const uint16_t chartX = 220, chartY = 50;
    static const uint8_t chartSpacing = 76;
    static const uint8_t chartLabelSpacing = 22;
    static const uint8_t chartPoints = 16;
  #endif
  //GPS tab
  #if defined(LVGL_SUPPORT_GPS_TAB)
    bool enableGpsTab = false;
    bool gpsTabInitialised = false;
    const char string_enableGpsTab[] PROGMEM = "enableGpsTab";
    lv_obj_t * gpsTab = nullptr;
    static const char gpsTabLabel[] = "GPS";
    lv_obj_t * gpsTabtable = nullptr;
    static const char statusTableLabel_0[] PROGMEM = "Date";
    static const char statusTableLabel_1[] PROGMEM = "Time";
    static const char statusTableLabel_2[] PROGMEM = "Satellites";
    static const char statusTableLabel_3[] PROGMEM = "HDOP";
    static const char statusTableLabel_4[] PROGMEM = "Lat";
    static const char statusTableLabel_5[] PROGMEM = "Lon";
    static const char statusTableLabel_6[] PROGMEM = "Speed";
    static const char statusTableLabel_7[] PROGMEM = "Course";
    static const char statusTableLabel_8[] PROGMEM = "Bearing";
    static const char statusTableLabel_Unknown[] = "??";
  #endif
  //Scan info tab
  #if defined(LVGL_SUPPORT_SCAN_INFO_TAB)
    bool enableInfoTab = true;
    const char string_enableInfoTab[] PROGMEM = "enableInfoTab";
    static const char infoTabLabel[] = "Search";
    lv_obj_t * scanInfoTab = nullptr;
    lv_obj_t * button0 = nullptr; //Nearest
    static const char button0Label[] PROGMEM = "Nearest";
    lv_obj_t * button1 = nullptr; //Furthest
    static const char button1Label[] PROGMEM = "Furthest";
    lv_obj_t * devices_dd = nullptr;
    //lv_obj_t * icName_label = nullptr;
    lv_obj_t * icDescription_label = nullptr;
    uint32_t lastScanInfoTabUpdate = 0;
    uint8_t numberOfDevicesInDeviceDropdown = 0;
    uint8_t selectDeviceDropdownIndices[maximumNumberOfDevices];
    bool findableDevicesChanged = true;
  #endif
  #if defined(LVGL_SUPPORT_MAP_TAB)
    bool enableMapTab = false;
    const char string_enableMapTab[] PROGMEM = "enableMapTab";
    static const char mapTabLabel[] = "Map";
    lv_obj_t* mapTab = nullptr;
    uint32_t lastMapTabUpdate = 0;

    const uint8_t mapWidth = 200;
    const uint8_t mapHeight = 150;
    lv_obj_t* mapCanvas = nullptr;  //The map itself
    //lv_color_t mapCanvasBuffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR(mapWidth, mapHeight)];
  #endif
  //Settings/preferences tab
  #if defined(LVGL_SUPPORT_SETTINGS_TAB)
    bool enableSettingsTab = true;
    const char string_enableSettingsTab[] PROGMEM = "enableSettingsTab";
    lv_obj_t * settingsTab = nullptr;
    static const char settingsTabLabel[] = "Pref";

    lv_obj_t * units_dd = nullptr;
    lv_obj_t * dateFormat_dd = nullptr;
    lv_obj_t * sensitivity_dd = nullptr;
    lv_obj_t * priority_dd = nullptr;
    lv_obj_t * displayTimeout_dd = nullptr;
    lv_obj_t * rotation_dd = nullptr;
    #if defined(SUPPORT_BEEPER)
      lv_obj_t * beeper_dd = nullptr;
    #endif
    lv_obj_t * displayBrightness_slider = nullptr;
  #endif
  #if defined(LVGL_MANAGE_BACKLIGHT)
    //Backlight management
    uint32_t backlightLastSet = 0;
    uint32_t backlightChangeInterval = 10;
    static const uint8_t absoluteMinimumBrightnessLevel = 4;
    static const uint8_t uiInactiveBrightnessLevel = absoluteMinimumBrightnessLevel;
    static const uint8_t absoluteMaximumBrightnessLevel = 255;
    uint8_t minimumBrightnessLevel = 64;
    const char string_minimumBrightnessLevel[] PROGMEM = "minimumBrightnessLevel";
    uint8_t maximumBrightnessLevel = 192;
    const char string_maximumBrightnessLevel[] PROGMEM = "maximumBrightnessLevel";
    uint8_t currentBrightnessLevel = 128;
    // use first channel of 16 channels (started from zero)
    #define LEDC_CHANNEL_0     1
    // use 12 bit precission for LEDC timer
    #define LEDC_TIMER_12_BIT  12
    // use 5000 Hz as a LEDC base frequency
    #define LEDC_BASE_FREQ     500
  #endif
  
  //User interface settings
  uint8_t units = 0;
  const char string_units[] PROGMEM = "units";
  uint8_t dateFormat = 0;
  const char string_dateFormat[] PROGMEM = "dateFormat";
  uint8_t displayTimeout = 2;
  const char string_displayTimeout[] PROGMEM = "displayTimeout";
  uint32_t displayTimeouts[] = {0, 60E3, 60E3 * 5, 60E3 * 15}; //No/1/5/15 minute timeouts on screen
  uint32_t lastUiActivity = 0;
  bool uiActive = true;
  /*
   * 
   * Sadly these function prototypes end up here because of build system aggro
   * 
   */
  void flushDisplay( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
  {
      uint32_t w = ( area->x2 - area->x1 + 1 );
      uint32_t h = ( area->y2 - area->y1 + 1 );
  
      tft.startWrite();
      tft.setAddrWindow( area->x1, area->y1, w, h );
      tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
      tft.endWrite();
  
      lv_disp_flush_ready( disp_drv );
  }
  #if defined(SUPPORT_TOUCHSCREEN)
    #if defined(SUPPORT_TOUCHSCREEN_BITBANG)
      int readSPI(byte command)
      {
          int result = 0;
          for (int i = 7; i >= 0; i--) {
              digitalWrite(XPT2046_MOSI, command & (1 << i));
              digitalWrite(XPT2046_CLK, HIGH);
              delayMicroseconds(10);
              digitalWrite(XPT2046_CLK, LOW);
              delayMicroseconds(10);
          }
          for (int i = 11; i >= 0; i--) {
              digitalWrite(XPT2046_CLK, HIGH);
              delayMicroseconds(10);
              result |= (digitalRead(XPT2046_MISO) << i);
              digitalWrite(XPT2046_CLK, LOW);
              delayMicroseconds(10);
          }
          return result;
      }
    #endif
    void readTouchscreen(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
    {
      #if defined(SUPPORT_TOUCHSCREEN_BITBANG)
        //if(millis() - lastCheckForTouch > 500)
        {
          //lastCheckForTouch = millis();
          if(digitalRead(XPT2046_IRQ) == LOW)
          {
            digitalWrite(XPT2046_CS, LOW);
            int x = readSPI(XPT2046_CMD_READ_X);
            int y = readSPI(XPT2046_CMD_READ_Y);
            digitalWrite(XPT2046_CS, HIGH);
            if(x != 0)
            {
              bool touchOutOfRange = false;
              if(currentLvglUiState == deviceState::calibrateScreen)
              {
                if(touchScreenMinimumX == 0 && touchScreenMaximumX == 0 && touchScreenMinimumY == 0 && touchScreenMaximumY == 0)  //No touch calibration data
                {
                  touchScreenMinimumX = x;
                  touchScreenMaximumX = x;
                  touchScreenMinimumY = y;
                  touchScreenMaximumY = y;
                  #if defined(SERIAL_DEBUG) && defined(DEBUG_TOUCHSCREEN)
                    SERIAL_DEBUG_PORT.println("Touch calibration started");
                  #endif
                }
                else
                {
                  if(x < touchScreenMinimumX){ touchScreenMinimumX = x; touchOutOfRange = true;}
                  if(x > touchScreenMaximumX){ touchScreenMaximumX = x; touchOutOfRange = true;}
                  if(y < touchScreenMinimumY){ touchScreenMinimumY = y; touchOutOfRange = true;}
                  if(y > touchScreenMaximumY){ touchScreenMaximumY = y; touchOutOfRange = true;}
                }
                if(touchOutOfRange == true)  //Recalibration is underway
                {
                  saveConfigurationSoon = millis();
                  #if defined(SERIAL_DEBUG) && defined(DEBUG_TOUCHSCREEN)
                    SERIAL_DEBUG_PORT.printf_P(PSTR("Touch x(%04u-%04u):%03u y(%04u-%04u):%03u\r\n"), touchScreenMinimumX, touchScreenMaximumX, data->point.x, touchScreenMinimumY, touchScreenMaximumY, data->point.y);
                  #endif
                }
              }
              x = map(x, touchScreenMaximumX, touchScreenMinimumX, 0, screenWidth);
              y = map(y, touchScreenMinimumY, touchScreenMaximumY, 0, screenHeight);
              if (x > screenWidth){x = screenWidth;}
              if (x < 0){x = 0;}
              if (y > screenHeight){y = screenHeight;}
              if (y < 0){y = 0;}
              #if defined(SERIAL_DEBUG) && defined(DEBUG_TOUCHSCREEN)
                SERIAL_DEBUG_PORT.print(" maps to X: ");
                SERIAL_DEBUG_PORT.print(x);
                SERIAL_DEBUG_PORT.print(", Y: ");
                SERIAL_DEBUG_PORT.println(y);
              #endif
              if (x != screenWidth && y != screenHeight)
              {
                if(screenRotation == 0)
                {
                  data->point.x = x;
                  data->point.y = y;
                }
                else if(screenRotation == 2)
                {
                  data->point.x = screenWidth - x;
                  data->point.y = screenHeight - y;
                }
                data->state = LV_INDEV_STATE_PR;
                lastUiActivity = millis();
                return;
              }
            }
          }
        }
        data->state = LV_INDEV_STATE_REL;
    #else
        if(touchscreen.touched())
        {
          TS_Point touchpoint = touchscreen.getPoint();
          //Some very basic auto calibration so it doesn't go out of range
          bool touchOutOfRange = false;
          if(touchpoint.x < touchScreenMinimumX){ touchScreenMinimumX = touchpoint.x; touchOutOfRange = true;}
          if(touchpoint.x > touchScreenMaximumX){ touchScreenMaximumX = touchpoint.x; touchOutOfRange = true;}
          if(touchpoint.y < touchScreenMinimumY){ touchScreenMinimumY = touchpoint.y; touchOutOfRange = true;}
          if(touchpoint.y > touchScreenMaximumY){ touchScreenMaximumY = touchpoint.y; touchOutOfRange = true;}
          //Map this to the pixel position
          data->point.x = map(touchpoint.x,touchScreenMinimumX,touchScreenMaximumX,1,screenWidth);
          data->point.y = map(touchpoint.y,touchScreenMinimumY,touchScreenMaximumY,1,screenHeight);
          data->state = LV_INDEV_STATE_PR;
          if(touchOutOfRange == true)  //Recalibration is underway
          {
            saveConfigurationSoon = millis();
            #if defined(SERIAL_DEBUG) && defined(DEBUG_LVGL)
              SERIAL_DEBUG_PORT.printf_P(PSTR("GUI Touch x(%04u-%04u):%03u y(%04u-%04u):%03u\r\n"), touchScreenMinimumX, touchScreenMaximumX, data->point.x, touchScreenMinimumY, touchScreenMaximumY, data->point.y);
            #endif
          }
        }
        else
        {
          data->state = LV_INDEV_STATE_REL;
        }
      #endif
    }
  #endif
#endif
/*
 * 
 * Peripheral power
 * 
 */
#if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
  bool peripheralsEnabled = false;
#endif
/*
 * 
 * Simple beeper
 * 
 */
#if defined(SUPPORT_BEEPER)
  SemaphoreHandle_t beeperSemaphore = NULL;
  TaskHandle_t beeperManagementTask = NULL;
  static const uint16_t beeperYieldTime = 50;
  static const uint16_t beeperSemaphoreTimeout = 5;
  uint32_t singleBeepOnTime = 25; //One shot beeps have their own on time
  uint32_t singleBeepLastStateChange = 0;
  uint32_t repeatingBeepOnTime = 20;  //Repeating beep on time
  uint32_t repeatingBeepOffTime = 0;  //Repeating beep off time
  uint32_t repeatingBeepLastStateChange = 0;
  bool beeperState = false;
  uint16_t beeperTone = 1400;
  static const uint16_t beeperButtonTone = 900;  //Button push tone
  static const uint16_t beeperButtonOnTime = 10; //Button push on time
  bool beeperEnabled = false;
  const char string_beeperEnabled[] PROGMEM = "beeperEnabled";
#endif
/*
 * 
 * Simple LED indicator
 * 
 */
#if defined(SUPPORT_LED)
  TaskHandle_t ledManagementTask = NULL;
  SemaphoreHandle_t ledSemaphore = NULL;
  static const uint16_t ledYieldTime = 50;
  static const uint16_t ledSemaphoreTimeout = 5;
  uint16_t ledPulseOnTime = 50;
  uint16_t ledSlowBlinkOnTime = 500;
  uint16_t ledSlowBlinkOffTime = 500;
  uint16_t ledFastBlinkOnTime = 10;
  uint16_t ledFastBlinkOffTime = 90;
  uint32_t ledOnTime = 20;
  uint32_t ledOffTime = 0;
  uint32_t ledLastStateChange = 0;
  bool ledState = false;
  uint16_t ledTone = 1400;
  static const uint16_t ledButtonTone = 900;
  bool ledEnabled = true;
#endif
/*
 * 
 * Haptic feedback
 * 
 */
#if defined(SUPPORT_VIBRATION)
  TaskHandle_t vibrationManagementTask = NULL;
  SemaphoreHandle_t vibrationSemaphore = NULL;
  static const uint16_t vibrationYieldTime = 50;
  static const uint16_t vibrationSemaphoreTimeout = 5;
  uint32_t vibrationOnTime = 20;
  uint32_t vibrationOffTime = 0;
  uint32_t vibrationLastStateChange = 0;
  bool vibrationState = false;
  bool vibrationEnabled = true;
  const char string_vibrationEnabled[] PROGMEM = "vibrationEnabled";
  uint8_t vibrationLevel = 100;
  const char string_vibrationLevel[] PROGMEM = "vibrationLevel";
#endif
/*
 * 
 * Soft power off for device
 * 
 */
#if defined(SUPPORT_SOFT_POWER_OFF)
  uint32_t powerOffTimer = 0;  //Used to schedule a power off
  #if defined(SUPPORT_GPS)
    uint32_t gpsStationaryTimeout = 300E3;   //Don't switch off by default
    const char string_gpsStationaryTimeout[] PROGMEM = "gpsStationaryTimeout";
    uint32_t gpsCheckInterval = 300E3;      //Wake up the GPS to see if moving every 5 minutes
    const char string_gpsCheckInterval[] PROGMEM = "gpsCheckInterval";
  #endif
#endif
/*
 * 
 * Battery meter, which may or may not be usable
 * 
 */
#if defined(SUPPORT_BATTERY_METER)
  uint32_t lastBatteryStatus = 0;
  uint32_t batteryStatusInterval = 60000;
  uint8_t batteryPercentage = 100;
#endif
/*
 * 
 * Hacking games
 * 
 */
#if defined(SUPPORT_HACKING)
  #include <ESPUI.h>
  #include <ESPUIgames.h>
  bool gameEnabled = true;
  uint8_t gameLength = 10;
  const char string_gameLength[] PROGMEM = "gameLength";
  uint8_t gameRetries = 0;
  const char string_gameRetries[] PROGMEM = "gameRetries";
  uint32_t gameSpeedup = 500;
  const char string_gameSpeedup[] PROGMEM = "gameSpeedup";
  ESPUIgames::gameType gametype = ESPUIgames::gameType::simon;
  #if defined(ALLOW_VIEW_FILES) || defined(ALLOW_DOWNLOAD_FILES)
    #include <FS.h>
    #include <LittleFS.h>
    uint16_t numberOfFiles = 0;
    char fileLabelTemplate[] PROGMEM = "%s%s";
    char **fileLabel;
    #if defined(ALLOW_VIEW_FILES)
      uint16_t *viewControlID = nullptr;
      char **fileViewLink;
      char fileViewLinkTemplate[] PROGMEM = "<a href=\"%s%s\" target=\"_blank\">View</a>";
    #endif
    #if defined(ALLOW_DOWNLOAD_FILES)
      uint16_t *downloadControlID = nullptr;
      char **fileDownloadLink;
      char fileDownloadLinkTemplate[] PROGMEM = "<a href=\"%s%s\" target=\"_blank\" download>Download</a>";
    #endif
    const char* filesTabTitle PROGMEM = "File storage";
    char filesRoot[] = "/files/";
    bool filesTabVisible = false;
    bool filesVisible = false;
    bool hackComplete = false;
    uint32_t turnOffWifiSoon = 0;
    uint16_t filesTabID = 0;
  #endif
  uint16_t controlsTabID = 0;
  uint16_t shutdownButtonID = 0;
  uint16_t selfDestructButtonID = 0;
  uint16_t statusWidgetID = 0;
  //uint32_t shutdownNow = 0;
  //uint8_t shutdownCountdown = 255;
  uint32_t selfDestructNow = 0;
  uint8_t selfDestructCountdown = 255;
  void selfDestructButtonCallback(Control* sender, int value)
  {
    #ifdef ESP8266
    { //HeapSelectIram doAllocationsInIRAM;
    #endif
      switch (value)
      {
      case B_DOWN:
          if(selfDestructNow == 0)
          {
            ESPUI.updateControlValue(statusWidgetID, "Self destruct down in 10s, hold to confirm");
            ESPUI.getControl(statusWidgetID)->color = ControlColor::Carrot;
            ESPUI.updateControl(statusWidgetID);
            selfDestructNow = millis();
          }
        break;
  
      case B_UP:
          if(millis() - selfDestructNow > 10000)
          {
            ESPUI.updateControlValue(statusWidgetID, "Shutdown" );
            ESPUI.getControl(statusWidgetID)->color = ControlColor::Alizarin;
            ESPUI.updateControl(statusWidgetID);
          }
          else
          {
            ESPUI.updateControlValue(statusWidgetID, "Active" );
            ESPUI.getControl(statusWidgetID)->color = ControlColor::Emerald;
            ESPUI.updateControl(statusWidgetID);
            selfDestructNow = 0;
          }
        break;
      }
    #ifdef ESP8266
    } // HeapSelectIram
    #endif
  }
#endif

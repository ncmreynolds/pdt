/*

   Sketch to work as a personal data transmitter device and associated tracker

   It can in principle be built across several different microcontrollers as I was indecisive during development but it's intended for the following...

   Beacon - ESP8266/8285 on custom PCB to keep the size down
   Tracker - ESP32-C3 on custom PCB to keep the size down

   The code is deliberately split into many several files to make it more manageable

   The same sketch has the code for both devices, uncomment the '#define ACT_AS_TRACKER' below to build for the tracker, otherwise it is a beacon

*/
#define ACT_AS_TRACKER

uint8_t majorVersion = 0;
uint8_t minorVersion = 3;
uint8_t patchVersion = 2;
/*

   Various nominally optional features that can be switched off during testing/development

*/
#ifndef ACT_AS_TRACKER
  #define ACT_AS_BEACON
#endif
#define SERIAL_DEBUG
#define SERIAL_LOG
//#define DEBUG_LOGGING
#define SUPPORT_LORA
#define DEBUG_LORA
#define SUPPORT_GPS
//#define DEBUG_GPS
#define SUPPORT_WIFI
#define SUPPORT_BATTERY_METER
#define SUPPORT_BUTTON
#define SUPPORT_OTA
#define USE_LITTLEFS
#define ENABLE_LOCAL_WEBSERVER
#define DEBUG_LOCAL_WEBSERVER
#define ENABLE_REMOTE_RESTART
#define ENABLE_LOCAL_WEBSERVER_FIRMWARE_UPDATE
#define ENABLE_OTA_UPDATE
#define ENABLE_LOG_DELETION
#if defined(ACT_AS_TRACKER)
  #define SUPPORT_DISPLAY
  #define SUPPORT_BEEPER
  #define DEBUG_BEEPER
  #define USE_SSD1331
  #pragma message "Acting as tracker"
#else
  #define ACT_AS_SENSOR
  #if defined(ACT_AS_SENSOR)
    #define SUPPORT_BEEPER
    #define SUPPORT_LED
    #include <Preferences.h>
    Preferences sensorPersitentData;
  #endif
  #pragma message "Acting as beacon"
#endif
#define ENABLE_LOCAL_WEBSERVER_BASIC_AUTH //Uncomment to password protect the web configuration interface
//#define SERVE_CONFIG_FILE
/*

   Block of includes that are always used

*/
#include <USBCDC.h>
#include <USB.h>
#include <LittleFS.h> //LittleFS storage
#include <FS.h> //Filesystem library
#include <time.h> //Time/NTP library
//#include <MD5Builder.h> //MD5 has library to hash authorisation lists
#include <ArduinoJson.h>  //Used to serialise/deserialise configuration files and server responses
#ifdef SUPPORT_WIFI
  #include <espBoilerplate.h>
  #include <DNSServer.h>
  /*
  #if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESP8266mDNS.h>
    #include <WiFiUdp.h>
  #elif defined(ESP32)
    #include <WiFi.h>
  #endif
  */
  #if defined(ENABLE_OTA_UPDATE)
    #include <ArduinoOTA.h> //Over-the-air update library
  #endif
#endif
#if defined(ACT_AS_SENSOR)
  #include <lasertag.h>
  lasertag sensor;
  #define IR_RECEIVER_PIN 1
  uint8_t numberOfStartingHits = 15;
  uint8_t numberOfStartingStunHits = 15;
  uint8_t currentNumberOfHits = 0;
  uint8_t currentNumberOfStunHits = 0;
  uint8_t armourValue = 0;
  uint8_t bleedOutCounter = 0;
  uint8_t bleedOutTime = 5;
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
    stunned = 9
  };
  sensorState currentSensorState = sensorState::starting;
  sensorState previousSensorState = sensorState::starting;
  uint32_t lastSensorStateChange = 0;
  uint32_t timeOfLastHit = 0;
  uint32_t sensorTones[4] = {3777, 800, 1200, 1000};
  uint32_t sensorToneOnTimes[4] = {200, 250, 500, 199};
  uint32_t sensorToneOffTimes[4] = {0, 250, 500, 1};
  const char* currentNumberOfHitsKey = "currentHits";
  const char* currentNumberOfStunHitsKey = "currentStunHits";
  //const char* currentSensorStateKey = "currentSensorState";
  const char* bleedOutCounterKey = "bleedOutCounter";
#endif

/*

   Block of includes for ESPAsyncWebServer, which is used to make a configuration interface and allow you to download log files

*/
#if defined(ENABLE_LOCAL_WEBSERVER)
  #if defined(ESP32)
    #include <AsyncTCP.h>
  #elif defined(ESP8266)
    #include <ESPAsyncTCP.h>
  #endif
  #include <ESPAsyncWebServer.h>
  AsyncWebServer webServer(80);  //Web server instance
  char normalize[] PROGMEM =
#include "css/normalizecss.h"
    ;
  char skeleton[] PROGMEM =
#include "css/skeletoncss.h"
    ;
  void addPageHeader(AsyncResponseStream *response, uint8_t refresh, const char* refreshTo);
  void addPageFooter(AsyncResponseStream *response);
#endif
/*

   Includes for SPI peripherals

*/
#if defined(SUPPORT_LORA) || defined(SUPPORT_DISPLAY)
  #include <SPI.h>
#endif
/*

   Includes for LoRa

*/
#if defined(SUPPORT_LORA)
  #include <LoRa.h>
  #include <MsgPack.h>  //MsgPack is used to transmit data
  #include "CRC16.h" //A CRC16 is used to check the packet is LIKELY to be sent in a known format
  #include "CRC.h"
  #define LORA_CRC_POLYNOME 0xac9a  //Taken from https://users.ece.cmu.edu/~koopman/crc/
  #define LORA_NON_BLOCKING //Uncomment to use callbacks, rather than polling for LoRa events
  uint8_t defaultLoRaTxPower = 17;
  uint8_t defaultLoRaSpreadingFactor = 7;
  uint32_t defaultLoRaSignalBandwidth = 250E3; //125E3; //Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3(default), 250E3, and 500E3.
  // Each nibble of the SX127x SyncWord must be strictly below 8 to be compatible with SX126x
  // Each nibble of the SX127x SynchWord must be different from each other and from 0 or you might experience a slight loss of sensitivity
  // Translation from SX127x to SX126x : 0xYZ -> 0xY4Z4 : if you do not set the two 4 you might lose sensitivity
  // There is more to it, but this should be enough to setup your networks and hopefully the official response will be more complete.
  uint8_t loRaSyncWord = 0x12;  //Don't use 0x34 as that is LoRaWAN, valid options are 0x12, 0x56, 0x78
  uint32_t loRaTxTime = 0; //Time in ms spent transmitting
  float maximumLoRaDutyCycle = 1.0;
  float calculatedLoRaDutyCycle = 0.0;  //Calculated from loRaTxTime and millis()
#endif
/*

   Block of includes for the display

*/
#ifdef SUPPORT_DISPLAY
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

   Block of includes for GPS support

*/
#ifdef SUPPORT_GPS
  #include <TinyGPS++.h>
#endif
/*

   Pin configurations

*/
#if defined(ARDUINO_ESP32C3_DEV)
  // NUM_DIGITAL_PINS        22
  // NUM_ANALOG_INPUTS       6
  // SS    = 7
  // MOSI  = 6
  // MISO  = 5
  // SCK   = 4
  // SDA = 8
  // SCL = 9
  // TX = 21
  // RX = 20
  // A0 = 0
  // A1 = 1;
  // A2 = 2;
  // A3 = 3;
  // A4 = 4;
  // A5 = 5;
  #if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
    bool debugPortAvailable = true;
    uint16_t debugPortStartingBufferSize = 0;
    uint32_t serialBufferCheckTime = 0;
    #if ARDUINO_USB_CDC_ON_BOOT == 1
      #pragma message "USB CDC configured on boot for debug messages"
      #define SERIAL_DEBUG_PORT Serial
    #else
      #pragma message "Configuring USB CDC for debug messages"
      #define SERIAL_DEBUG_PORT USBSerial
    #endif
  #else
  #endif
  #ifdef SUPPORT_GPS
    #ifdef SUPPORT_DISPLAY
      #if ARDUINO_USB_CDC_ON_BOOT == 1
        #define GPS_PORT Serial0
      #else
        #define GPS_PORT Serial
      #endif
      const int8_t RXPin = 20;              //GPS needs an RX pin, but it can be moved wherever, within reason
    #else
      #define GPS_PORT Serial0
      const int8_t RXPin = 20;              //GPS needs an RX pin, but it can be moved wherever, within reason
    #endif
    const int8_t TXPin = -1;              //No TX pin
  #endif
  #ifdef SUPPORT_LORA
    const int8_t loRaCSpin = 7;          // LoRa radio chip select
    const int8_t loRaResetPin = 8;       // LoRa radio reset
    const int8_t loRaIrqPin = 10;        // change for your board; must be a hardware interrupt pin
  #endif
  #ifdef SUPPORT_DISPLAY
    const uint8_t displayCSpin = 1;
    const uint8_t displayResetPin = 3;
    const uint8_t displayDCpin = 2;
  #endif
  #ifdef SUPPORT_BATTERY_METER
    int8_t voltageMonitorPin = A0;
    float topLadderResistor = 330.0;
    float bottomLadderResistor = 89; //Actually 100k, but the ESP itself has some resistance that effectively lowers it
    float ADCpeakVoltage = 2.5;
    float chargingVoltage = 4.19;
  #endif
  #ifdef SUPPORT_BEEPER
    uint8_t beeperChannel = 0;
    int8_t beeperPin = 21;
  #endif
  #ifdef SUPPORT_LED
    int8_t ledPin = 2;
    void ledOn(uint32_t ontime, uint32_t offtime);
  #endif
  #ifdef SUPPORT_BUTTON
    int8_t buttonPin = 9;
    uint32_t buttonPushTime = 0;
    uint32_t buttonDebounceTime = 50;
    uint32_t buttonLongPressTime = 1500;
    bool buttonHeld = false;
    bool buttonLongPress = false;
  #endif
#else
  #ifdef SUPPORT_LORA
    const int8_t loRaCSpin = 15;          // LoRa radio chip select
    const int8_t loRaResetPin = 16;       // LoRa radio reset
    const int8_t loRaIrqPin = 5;         // change for your board; must be a hardware interrupt pin
  #endif
#endif
/*

   Variables, depending on supported features

*/
#if defined(ACT_AS_BEACON)
  const char* default_deviceName = "PDT beacon";
#else
  const char* default_deviceName = "PDT tracker";
#endif
char* configurationComment = nullptr;
char default_configurationComment[] = "";
//Username for the Web UI
#if defined(ENABLE_LOCAL_WEBSERVER) && defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
  const char default_http_user[] = "pdt";
  char* http_user = nullptr;
#endif
//Password for the Web UI and over-the-air update
#if (defined(ENABLE_LOCAL_WEBSERVER) && defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)) || defined(ENABLE_OTA_UPDATE)
  const char default_http_password[] = "pdtpassword";
  char* http_password = nullptr;
  bool basicAuthEnabled = false;
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

   NTP/time configuration

*/
const char* default_timeServer = "pool.ntp.org";  //The default time server
char* timeServer = nullptr;
const char* default_timeZone = "GMT0BST,M3.5.0/1,M10.5.0"; //The default DST setting for Europe/London, see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
char* timeZone = nullptr;
//const char* timeZone = "UTC0"; //Use this for UTC
char timestamp[] = "XX:XX:XX XX-XX-XXXX"; //A string overwritten with the current timestamp for logging. It uses this blank XX format until time is set by NTP after booting.
uint64_t bootTime = 0; //Use the moment the system got a valid NTP time to calculate the boot time for approximate uptime calculations, millis() wraps pretty quickly so we shouldn't use that
#if defined ENABLE_REMOTE_RESTART
uint32_t restartTimer = 0;  //Used to schedule a restart
#endif
/*

   WiFi credentials

   Note the use of __has_include to conditionally include credentials from elsewhere IF it can be found.

   You can edit the sketch directly to set your WiFi credentials below, or add a credentials.h in the sketch directory

*/
#if defined __has_include
  #if __has_include("credentials.h")
    #include "credentials.h"
    #pragma message "Using external default WiFi credentials"
  #else
    #define WIFI_PSK "Your WiFi PSK"
    #define WIFI_SSID "Your WiFi SSID"
    #pragma message "Using default WiFi credentials from sketch"
  #endif
#endif
#if defined(ESP8266) || defined(ESP32)
  //uint8_t _localMacAddress[6] = {0, 0, 0, 0, 0, 0}; //MAC address of this device, which is ALWAYS used to identify it even if WiFi is disabled
  uint8_t _remoteMacAddress[6] = {0, 0, 0, 0, 0, 0};  //MAC address of the remote device
#endif
#if defined(SUPPORT_WIFI)
  bool startWiFiClientOnBoot = true;
  bool startWiFiApOnBoot = true;
  bool enableCaptivePortal = true;
  DNSServer* dnsServer = nullptr; //May not be used so don't create the object unless it is enabled
  uint32_t wiFiClientInactivityTimer = 0;
  uint32_t lastWifiActivity = 0;
  const char* default_WiFi_SSID = WIFI_PSK;
  const char* default_WiFi_PSK = WIFI_SSID;
  char* SSID = nullptr;
  char* PSK = nullptr;
  const char* default_AP_PSK = "12345678";
  char* APSSID = nullptr;
  char* APPSK = nullptr;
  uint8_t wifiClientTimeout = 30;
#endif
const int8_t networkTimeout = 30;  //Timeout in seconds for network connection
static bool wifiClientConnected = false; //Is the network connected?
static bool wifiApStarted = false; //Is the WiFi AP started?
//static bool networkStateChanged = false;  //Has the state changed since initial connection
/*
   Over-the-update
*/
#if defined(ENABLE_OTA_UPDATE)
  volatile bool otaInProgress = false;
  uint8_t otaProgress = 0;
  bool otaEnabled = true;
  bool otaAuthenticationEnabled = false;
  //void configureOTA();
#endif
/*

   Local logging

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
//uint16_t loggingBufferSize = 2048; //The space to reserve for a logging backlog. This is not a hard limit, it is to reduce heap fragmentation.
uint16_t loggingBufferSize = 32768; //The space to reserve for a logging backlog. This is not a hard limit, it is to reduce heap fragmentation.
uint32_t logLastFlushed = 0;  //Time (millis) of the last log flush
bool autoFlush = false;
bool flushLogNow = false;
uint32_t logFlushInterval = 57600; //Frequency in seconds of log flush, this is 16h
uint32_t logFlushThreshold = 2000; //Threshold for forced log flush
SemaphoreHandle_t loggingSemaphore = NULL;
TaskHandle_t loggingManagementTask = NULL;
const uint16_t loggingYieldTime = 10;
const uint16_t loggingSemaphoreTimeout = 5;


#if defined(SUPPORT_LORA)
  #define MAX_LORA_BUFFER_SIZE 255
  bool loRaConnected = false;   // Has the radio initialised OK
  #if defined(LORA_NON_BLOCKING)
    #ifdef ESP32
      portMUX_TYPE loRaRxSynch = portMUX_INITIALIZER_UNLOCKED;  //Mutex for multi-core ESP32s
      portMUX_TYPE loRaTxSynch = portMUX_INITIALIZER_UNLOCKED;  //Mutex for multi-core ESP32s
    #endif
    volatile uint32_t loRaTxStartTime = 0; //Used to calculate TX time for each packet
    volatile bool loRaTxBusy = false;
    volatile bool loRaRxBusy = false;
    volatile uint8_t loRaSendBufferSize = 0;
    volatile uint8_t loRaReceiveBufferSize = 0;
    volatile float lastRssi = 0.0;
    volatile uint32_t loRaTxPackets = 0;
    volatile uint32_t loRaRxPackets = 0;
  #else
    volatile bool loRaTxBusy = false;
    volatile bool loRaRxBusy = false;
    volatile uint8_t loRaSendBufferSize = 0;
    volatile uint8_t loRaReceiveBufferSize = 0;
    volatile float lastRssi = 0.0;
    volatile uint32_t loRaTxPackets = 0;
    volatile uint32_t loRaRxPackets = 0;
  #endif
  uint8_t loRaSendBuffer[MAX_LORA_BUFFER_SIZE];
  uint8_t loRaReceiveBuffer[MAX_LORA_BUFFER_SIZE];
  uint32_t lastDeviceInfoSendTime = 0;    // last send time
  uint32_t deviceInfoSendInterval = 60000;    // Send info every 60s
  uint32_t lastLocationSendTime = 0;    // last send time
  uint32_t defaultLocationSendInterval = 60000;
  uint16_t loRaPerimiter1 = 25;             //Range at which beacon 1 applies
  uint32_t locationSendInterval1 = 5000;    // interval between sends
  uint16_t loRaPerimiter2 = 50;             //Range at which beacon 2 applies
  uint32_t locationSendInterval2 = 15000;   // interval between sends
  uint16_t loRaPerimiter3 = 100;            //Range at which beacon 3 applies
  uint32_t locationSendInterval3 = 30000;   // interval between sends
  float rssiAttenuation = -6.0;           //Rate at which double the distance degrades RSSI (should be -6)
  float rssiAttenuationBaseline = -40;    //RSSI at 10m
  float rssiAttenuationPerimeter = 10;
  const uint8_t locationUpdateId = 0x00;    //LoRa packet contains location info from a beacon
  //const uint8_t trackerLocationUpdateId = 0x01;   //LoRa packet contains location info from a tracker
  const uint8_t deviceStatusUpdateId = 0x10;             //LoRa packet contains device info, shared infrequently
#endif
/*

   Display support variables

*/
#ifdef SUPPORT_DISPLAY
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
  #ifdef SUPPORT_BEEPER
    beeper,
  #endif
    version,
    menu
  };
  displayState currentDisplayState = displayState::blank;
  const uint8_t screenWidth = 96;
  const uint8_t screenHeight = 64;
  uint32_t lastDisplayUpdate = 0;
  const uint32_t longDisplayTimeout = 30000;
  const uint32_t shortDisplayTimeout = 5000;
  uint32_t displayTimeout = 30000;
#endif
/*

   GPS support variables

*/
#ifdef SUPPORT_GPS
  TaskHandle_t gpsManagementTask = NULL;
  SemaphoreHandle_t gpsSemaphore = NULL;
  const uint16_t gpsSemaphoreTimeout = 5;
  const uint16_t gpsYieldTime = 100;
  #define MINIMUM_VIABLE_HDOP 10
  TinyGPSPlus gps;
  const uint32_t GPSBaud = 9600;
  uint32_t lastGpsTimeCheck = 0;
  uint32_t gpsTimeCheckInterval = 30000;
  uint32_t lastDistanceCalculation = 0;
  uint32_t distanceCalculationInterval = 5000;
  //double trackerLatitude = 51.508131; //London
  //double trackerLongitude = -0.128002;
  struct deviceLocationInfo {
    uint8_t id[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    char* name = nullptr;
    bool hasFix = false;
    uint8_t typeOfDevice = 0; // bitmask 0 = beacon, 1 = tracker, 2 = sensor
    float supplyVoltage = 0;  // Battery health can be guesstimated from this
    uint32_t uptime = 0;  // Check for reboots
    uint8_t majorVersion = 0; //Software version
    uint8_t minorVersion = 0;
    uint8_t patchVersion = 0;
    uint32_t lastLocationUpdate = 0;  // Used to track packet loss
    uint16_t nextLocationUpdate = 0;  // Used to track packet loss
    uint16_t updateHistory = 0xffff;  // Rolling bitmask of packets received/not received based on expected arrival times
    double latitude = 0;  //Location info
    double longitude = 0;
    float course = 0;
    float speed = 0;
    float hdop = 0;
    float distanceTo = 0;
    float courseTo = 0;
    float lastRssi = 0; // Can also be used to estimate distance
    uint8_t numberOfStartingHits = 0;
    uint8_t numberOfStartingStunHits = 0;
    uint8_t currentNumberOfHits = 0;
    uint8_t currentNumberOfStunHits = 0;
  };
  const uint8_t maximumNumberOfDevices = 16;
  uint8_t numberOfDevices = 0;
  deviceLocationInfo device[maximumNumberOfDevices];
  #if defined(ACT_AS_TRACKER)
    #define BEACONUNREACHABLE 100000
    double maximumEffectiveRange = 99;
    uint32_t distanceToCurrentBeacon = BEACONUNREACHABLE;
    bool distanceToCurrentBeaconChanged = false;
    uint8_t currentBeacon = maximumNumberOfDevices;
    enum class trackingMode : std::int8_t {
      nearest,
      furthest,
      fixed
    };
    trackingMode currentTrackingMode = trackingMode::nearest;
    #ifdef SUPPORT_LED
      uint32_t ledOnTime = 20;
      uint32_t ledOffTime = 1000;
      uint32_t ledLastStateChange = 0;
      bool ledState = false;
    #endif
  #elif defined(ACT_AS_BEACON)
    #define TRACKERUNREACHABLE 100000
    uint8_t closestTracker = maximumNumberOfDevices;
    uint16_t distanceToClosestTracker = TRACKERUNREACHABLE;
  #endif
  uint32_t lastGPSstatus = 0;
  uint32_t chars;
  uint16_t sentences, failed;
  bool useGpsForTimeSync = true;
#endif
/*

   Beeper

*/
#ifdef SUPPORT_BEEPER
  SemaphoreHandle_t beeperSemaphore = NULL;
  TaskHandle_t beeperManagementTask = NULL;
  const uint16_t beeperYieldTime = 10;
  const uint16_t beeperSemaphoreTimeout = 5;
  uint32_t singleBeepOnTime = 25; //One shot beeps have their own on time
  uint32_t singleBeepLastStateChange = 0;
  uint32_t repeatingBeepOnTime = 20;  //Repeating beep on time
  uint32_t repeatingBeepOffTime = 0;  //Repeating beep off time
  uint32_t repeatingBeepLastStateChange = 0;
  bool beeperState = false;
  uint16_t beeperTone = 1400;
  const uint16_t beeperButtonTone = 900;  //Button push tone
  const uint16_t beeperButtonOnTime = 25; //Button push on time
  bool beeperEnabled = true;
#endif
#ifdef SUPPORT_LED
  TaskHandle_t ledManagementTask = NULL;
  SemaphoreHandle_t ledSemaphore = NULL;
  const uint16_t ledYieldTime = 10;
  const uint16_t ledSemaphoreTimeout = 5;
  uint32_t ledOnTime = 20;
  uint32_t ledOffTime = 0;
  uint32_t ledLastStateChange = 0;
  bool ledState = false;
  uint16_t ledTone = 1400;
  const uint16_t ledButtonTone = 900;
  bool ledEnabled = true;
#endif
/*

   Battery meter, which may or may not be usable

*/
#ifdef SUPPORT_BATTERY_METER
  uint32_t lastDeviceStatus = 0;
  uint32_t deviceStatusInterval = 60000;
  uint8_t batteryPercentage = 100;
#endif

/*
 *
 * This file contains functions related to initial setup
 * 
 */
void setup() {
  WiFi.macAddress(_localMacAddress);
  #ifdef ACT_AS_TRACKER
    tracker[0].id[0] = _localMacAddress[0];  //Copy in local MAC address as 'tracker 0'
    tracker[0].id[1] = _localMacAddress[1];
    tracker[0].id[2] = _localMacAddress[2];
    tracker[0].id[3] = _localMacAddress[3];
    tracker[0].id[4] = _localMacAddress[4];
    tracker[0].id[5] = _localMacAddress[5];
    numberOfTrackers++;
    pinMode(buttonPin, INPUT_PULLUP);
    #if defined(ARDUINO_ESP32C3_DEV)
      #ifdef SUPPORT_DOSTAR
        dotStar.begin();
        dotStar.setPixelColor(0, dotStar.Color(0, 255, 0, 0));
        dotStar.show();
      #endif
    #endif
  #elif defined(ACT_AS_BEACON)
    beacon[0].id[0] = _localMacAddress[0];  //Copy in local MAC address as 'beacon 0'
    beacon[0].id[1] = _localMacAddress[1];
    beacon[0].id[2] = _localMacAddress[2];
    beacon[0].id[3] = _localMacAddress[3];
    beacon[0].id[4] = _localMacAddress[4];
    beacon[0].id[5] = _localMacAddress[5];
    numberOfBeacons++;
  #endif
  #if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
    SERIAL_DEBUG_PORT.begin(115200);
  #endif
  loggingBuffer.reserve(loggingBufferSize); //Reserve heap for the logging backlog
  #if defined(ACT_AS_TRACKER)
    localLogLn(F("============================ Booting tracker ============================"));
  #elif defined(ACT_AS_BEACON)
    localLogLn(F("============================ Booting beacon ============================="));
  #endif
  localLog(F("Firmware: ")); localLog(majorVersion); localLog('.'); localLog(minorVersion); localLog('.'); localLogLn(patchVersion);
  localLog(F("Built: ")); localLog(__TIME__); localLog(' '); localLogLn(__DATE__);
  #ifdef ESP_IDF_VERSION_MAJOR
    localLog(F("IDF version: "));      
    #ifdef ESP_IDF_VERSION_MINOR
      localLog(ESP_IDF_VERSION_MAJOR);
      localLog('.');
      localLogLn(ESP_IDF_VERSION_MINOR);
    #else
      localLogLn(ESP_IDF_VERSION_MAJOR);
    #endif
  #endif
  localLog(F("Core: "));
  #if defined(ESP32)
    #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
      #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
        localLogLn(F("ESP32"));
      #elif CONFIG_IDF_TARGET_ESP32S2
        localLogLn(F("ESP32S2"));
      #elif CONFIG_IDF_TARGET_ESP32C3
        localLogLn(F("ESP32C3"));
      #else 
        #error Target CONFIG_IDF_TARGET is not supported
      #endif
      localLog(F("Processor clock: "));
      localLog(getCpuFrequencyMhz());
      localLogLn(F("Mhz"));
    #else // ESP32 Before IDF 4.0
      localLogLn(F("ESP32"));
    #endif
  #else
    localLogLn(F("Uknown"));
  #endif
  localLog(F("Board: ")); localLogLn(ARDUINO_BOARD);
  #if defined(ESP32)
    #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
      #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
        localLog(F("Restart reason core 0: "));
        localLogLn(es32ResetReason(0));
        localLog(F("Restart reason core 1: "));
        localLogLn(es32ResetReason(1));
      #elif CONFIG_IDF_TARGET_ESP32S2
        localLog(F("Restart reason: "));
        localLogLn(es32ResetReason(0));
      #elif CONFIG_IDF_TARGET_ESP32C3
        localLog(F("Restart reason: "));
        localLogLn(es32ResetReason(0));
      #else 
        #error Target CONFIG_IDF_TARGET is not supported
      #endif
    #else // ESP32 Before IDF 4.0
      localLog(F("Restart reason core 0: "));
      localLogLn(es32ResetReason(0));
      localLog(F("Restart reason core 1: "));
      localLogLn(es32ResetReason(1));
    #endif
  #endif
  //Mount file system so configuration can be read
  mountFilesystem();
  if(loadConfiguration(configurationFile) == true)
  {
    printConfiguration();
  }
  else
  {
    loadDefaultConfiguration();
  }
  #ifdef ACT_AS_TRACKER
    if(digitalRead(buttonPin) == false) //Start WiFi if button pushed
    {
      startWiFiOnBoot = true;
    }
  #endif
  #ifdef SUPPORT_WIFI
    if(startWiFiOnBoot == true)
    {
      localLog(F("Connecting to SSID \""));
      localLog(SSID);
      localLog('"');
      WiFi.mode(WIFI_STA);
      WiFi.begin(SSID, PSK);
      while (WiFi.status() != WL_CONNECTED && millis() < 10000)
      {
        localLog('.');
        delay(500);
      }
      if(WiFi.status() == WL_CONNECTED)
      {
        localLogLn(F("OK"));
        networkConnected = true;
      }
      else
      {
        localLogLn(F("failed"));
        networkConnected = false;
      }
      if(networkConnected == true && otaEnabled == true)
      {
        #ifdef SUPPORT_OTA
          configureOTA();
        #endif
      }
    }
    else
    {
      localLogLn(F("Disabling WiFi"));
      //WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
    }
    if(networkConnected == true)
    {
      configureTimeServer();  //Set up the time server
    }
  #endif
  if(networkConnected == true)
  {
    showNetworkInformation();
    networkStateChanged = false; //Suppress normal handling of network state changes
  }
  #if defined(ENABLE_LOCAL_WEBSERVER)
    if(filesystemMounted == true)
    {
      setupWebServer(); //Set up the web server callbacks which are used to get hold of logs
    }
  #endif
  #if defined(ENABLE_OTA_UPDATE)
    configureOTA(); //Set up OTA callbacks
  #endif
  #ifdef SUPPORT_LORA
    setupLoRa();  //Needs to be before SPI display
  #endif
  #ifdef SUPPORT_DISPLAY
    setupScreen();  //Needs to be after LoRa
  #endif
  #ifdef SUPPORT_GPS
    setupGps();
    #ifdef USE_RTOS
      //Start processing GPS data as an RTOS task
      xTaskCreate(
        processGpsSentences,    // Function
        "processGpsSentences",  // Name
        1024,                   // Stack size
        NULL,                   // Parameters
        1,                      // Priority 1-24
        &gpsTask                // Handle
      );
      gpsSemaphore = xSemaphoreCreateBinary();
      xSemaphoreGive(gpsSemaphore);
    #endif
  #endif
  #ifdef SUPPORT_BEEPER
    setupBeeper();
  #endif
  #ifdef SUPPORT_BATTERY_METER
    analogSetAttenuation(ADC_11db); //Set ADC to read up to 2600mV
    checkBatteryVoltage();  //Set initial voltage reading
    batteryPercentage = estimateBatteryPercentage(batteryVoltage); //Set initial capacity estimate
  #endif
}
void configureTimeServer()
{
  localLog(F("Configuring time server: "));
  localLogLn(timeServer);
  localLog(F("Configuring time zone: "));
  localLogLn(timeZone);
  configTime(0, 0, timeServer);
  setenv("TZ",timeZone,1);
  tzset();
}
#if defined(SUPPORT_ETHERNET) || defined(SUPPORT_WIFI)
void showNetworkInformation()
{
  localLogLn(F("======== Network information ========"));
  #if defined(SUPPORT_ETHERNET) && defined(SUPPORT_WIFI)
    localLog(F("Primary connection: "));
    if(ethernetPrimaryConnection == true)
    {
      localLog(F(" Ethernet"));
    }
    else
    {
      localLog(F(" WiFi"));
    }
    if(primaryConnectionInUse == true)
    {
      localLogLn(F(" - connected"));
    }
    else
    {
      localLogLn(F(" - using backup"));
    }
  #endif
  #if defined(SUPPORT_ETHERNET)
    localLogLn(F("========       Ethernet      ========"));
    if(ethernetConnected == true)
    {
      localLog(F("        Speed: ")); localLogLn(ETH.linkSpeed());
      localLog(F("       Duplex: ")); localLogLn(ETH.fullDuplex());
      localLog(F("          MAC: ")); localLogLn(ETH.macAddress());
      localLog(F("           IP: ")); localLogLn(ETH.localIP());
      localLog(F("       Subnet: ")); localLogLn(ETH.subnetMask());
      localLog(F("      Gateway: ")); localLogLn(ETH.gatewayIP());
      localLog(F("        DNS 1: ")); localLogLn(ETH.dnsIP(0));
      localLog(F("        DNS 2: ")); localLogLn(ETH.dnsIP(1));
      localLog(F("        DNS 3: ")); localLogLn(ETH.dnsIP(2));
    }
    else
    {
      localLogLn(F("Not connected"));
    }
  #endif
  #if defined(SUPPORT_WIFI)
    localLogLn(F("========         WiFi        ========"));
    if(networkConnected == true)
    {
      localLog(F("         SSID: ")); localLogLn(WiFi.SSID());
      localLog(F("  Wifi Status: ")); localLogLn(WiFi.status());
      localLog(F("Wifi Strength: ")); localLog(WiFi.RSSI()); localLogLn(" dBm");
      localLog(F("          MAC: ")); localLogLn(WiFi.macAddress());
      localLog(F("           IP: ")); localLogLn(WiFi.localIP().toString());
      localLog(F("       Subnet: ")); localLogLn(WiFi.subnetMask().toString());
      localLog(F("      Gateway: ")); localLogLn(WiFi.gatewayIP().toString());
      localLog(F("        DNS 1: ")); localLogLn(WiFi.dnsIP(0).toString());
      localLog(F("        DNS 2: ")); localLogLn(WiFi.dnsIP(1).toString());
      localLog(F("        DNS 3: ")); localLogLn(WiFi.dnsIP(2).toString());
    }
    else
    {
      localLogLn(F("Not connected"));
    }
  #endif
  localLogLn(F("====================================="));
}
#endif

/*
 * 
 * This file contains functions related to storing/retrieving configuration from JSON files on the local flash filesystem
 * 
 */
bool saveConfiguration(const char* filename)  //Saves the configuration
{
  backupConfiguration(filename);
  localLog(F("Saving configuration to \""));
  localLog(filename);
  localLog(F("\": "));
  DynamicJsonDocument configuration(2048);
  configuration["deviceName"] = device[0].name;
  configuration["configurationComment"] = configurationComment;
  #if defined(SUPPORT_WIFI)
    configuration["SSID"] = SSID;
    configuration["PSK"] = PSK;
    configuration["startWiFiClientOnBoot"] = startWiFiClientOnBoot;
    configuration["APSSID"] = APSSID;
    configuration["APPSK"] = APPSK;
    configuration["startWiFiApOnBoot"] = startWiFiApOnBoot;
    configuration["softApChannel"] = softApChannel;
    #if defined(ENABLE_LOCAL_WEBSERVER)
      configuration["enableCaptivePortal"] = enableCaptivePortal;
    #endif
    configuration["wiFiClientInactivityTimer"]  = wiFiClientInactivityTimer;
    configuration["wifiClientTimeout"]  = wifiClientTimeout;
  #endif
  #if defined(ENABLE_LOCAL_WEBSERVER)
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      configuration["http_user"] = http_user;
      configuration["basicAuthEnabled"] = basicAuthEnabled;
    #endif
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH) || defined(ENABLE_OTA_UPDATE)
      configuration["http_password"] = http_password;
    #endif
  #endif
  #if defined(SUPPORT_HACKING)
    configuration["gameLength"] = gameLength;
    configuration["gameRetries"] = gameRetries;
    configuration["gameSpeedup"] = gameSpeedup;
  #endif
  configuration["timeServer"] = timeServer;
  configuration["timeZone"] = timeZone;
  #if defined(SUPPORT_GPS)
    configuration["useGpsForTimeSync"] = useGpsForTimeSync;
    #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
      configuration["gpsStationaryTimeout"] = gpsStationaryTimeout;
      configuration["gpsCheckInterval"] = gpsCheckInterval;
    #endif
  #endif
  #if defined(SUPPORT_BATTERY_METER)
    configuration["enableBatteryMonitor"] = enableBatteryMonitor;
    configuration["topLadderResistor"] = topLadderResistor;
    configuration["bottomLadderResistor"] = bottomLadderResistor;
  #endif
  #if defined(SUPPORT_BEEPER)
    configuration["beeperEnabled"] = beeperEnabled;
    #if defined(SUPPORT_BUTTON)
      configuration["beepOnPress"] = beepOnPress;
    #endif
  #endif
  #if defined(SUPPORT_VIBRATION)
    configuration["vibrationEnabled"] = vibrationEnabled;
    configuration["vibrationLevel"] = vibrationLevel;
  #endif
  #if defined(ACT_AS_TRACKER)
    configuration["maximumEffectiveRange"] = maximumEffectiveRange;
    configuration["trackingSensitivity"] = trackingSensitivity;
    configuration["trackerPriority"] = trackerPriority;
  #elif defined(ACT_AS_BEACON)
    configuration["icName"] = device[0].icName;
    configuration["icDescription"] = device[0].icDescription;
    configuration["diameter"] = device[0].diameter;
    configuration["height"] = device[0].height;
  #endif
  #if defined(SUPPORT_ESPNOW)
    configuration["espNowEnabled"] = espNowEnabled;
    configuration["espNowPreferredChannel"] = espNowPreferredChannel;
    configuration["espNowDeviceInfoInterval"] = espNowDeviceInfoInterval;
    configuration["defaultEspNowLocationInterval"] = defaultEspNowLocationInterval;
    configuration["espNowLocationInterval1"] = espNowLocationInterval1;
    configuration["espNowPerimiter1"] = espNowPerimiter1;
    configuration["espNowLocationInterval2"] = espNowLocationInterval2;
    configuration["espNowPerimiter2"] = espNowPerimiter2;
    configuration["espNowLocationInterval3"] = espNowLocationInterval3;
    configuration["espNowPerimiter3"] = espNowPerimiter3;
  #endif
  #if defined(SUPPORT_FTM)
    configuration["ftmEnabled"] = ftmEnabled;
    configuration["ftmSSID"] = ftmSSID;
    configuration["ftmHideSSID"] = ftmHideSSID;
    configuration["ftmPSK"] = ftmPSK;
  #endif
  #if defined(SUPPORT_LORA)
    configuration["loRaEnabled"] = loRaEnabled;
    configuration["loRaDeviceInfoInterval"] = loRaDeviceInfoInterval;
    configuration["defaultLoRaLocationInterval"] = defaultLoRaLocationInterval;
    configuration["loRaLocationInterval1"] = loRaLocationInterval1;
    configuration["loRaPerimiter1"] = loRaPerimiter1;
    configuration["loRaLocationInterval2"] = loRaLocationInterval2;
    configuration["loRaPerimiter2"] = loRaPerimiter2;
    configuration["loRaLocationInterval3"] = loRaLocationInterval3;
    configuration["loRaPerimiter3"] = loRaPerimiter3;
    #if defined(MEASURE_DISTANCE_WITH_LORA)
      configuration["rssiAttenuationPerimeter"] = rssiAttenuationPerimeter;
      configuration["rssiAttenuation"] = rssiAttenuation;
      configuration["rssiAttenuationBaseline"] = rssiAttenuationBaseline;
    #endif
  #endif
  #if defined(SUPPORT_TREACLE)
    configuration["id"] = device[0].id;
    configuration["espNowEnabled"] = espNowEnabled;
    configuration["loRaEnabled"] = loRaEnabled;
    configuration["loRaFrequency"] = loRaFrequency;
    configuration["loRaTxPower"] = loRaTxPower;
    configuration["loRaRxGain"] = loRaRxGain;
    configuration["loRaSpreadingFactor"] = loRaSpreadingFactor;
    configuration["loRaSignalBandwidth"] = loRaSignalBandwidth;
  #endif
  #if defined(SUPPORT_LVGL)
    configuration["units"] = units;
    configuration["dateFormat"] = dateFormat;
    configuration["displayTimeout"] = displayTimeout;
    configuration["minimumBrightnessLevel"] = minimumBrightnessLevel;
    configuration["maximumBrightnessLevel"] = maximumBrightnessLevel;
    configuration["screenRotation"] = screenRotation;
    #if defined(SUPPORT_TOUCHSCREEN)
      configuration["touchScreenMinimumX"] = touchScreenMinimumX;
      configuration["touchScreenMaximumX"] = touchScreenMaximumX;
      configuration["touchScreenMinimumY"] = touchScreenMinimumY;
      configuration["touchScreenMaximumY"] = touchScreenMaximumY;
    #endif
  #endif
  configuration["loggingBufferSize"] = loggingBufferSize;
  configuration["logFlushThreshold"] = logFlushThreshold;
  configuration["logFlushInterval"] = logFlushInterval;
  updateTimestamp();  //Put a timestamp on the change
  configuration["configurationSaved"] = timestamp;
  File f = openFileForWriting(filename);
  if(!f)
  {
    localLogLn(F("failed, unable to open file"));
  }
  else
  {
    if(serializeJsonPretty(configuration, f))
    {
      localLogLn(F("OK"));
      f.close();
      return true;
    }
    else
    {
      localLogLn(F("failed"));
    }
  }
  f.close();
  return false;
}

void backupConfiguration(const char* filename)  //Keeps backups of the configuration by renaming files
{
  if(fileExists(filename))
  {
    localLog(F("Backing up configuration: \""));
    localLog(filename);
    localLogLn('"');
    if(timeIsValid())
    {
      time_t now;
      time(&now);
      struct tm * timeinfo;
      timeinfo = localtime(&now);
      char newFilename[strlen(filename) + 21];
      sprintf_P(newFilename,PSTR("%s-%02u%02u%02u%02u%02u%04u"),filename,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,timeinfo->tm_mday,timeinfo->tm_mon + 1,timeinfo->tm_year+1900);
      renameFile(filename,newFilename);
    }
    else
    {
      char newFilename[strlen(filename) + 5];
      sprintf_P(newFilename,PSTR("%s.old"),filename);
      renameFile(filename,newFilename);
    }
  }
}

bool configurationChanged() //Compares the current configuration with the previously loaded values
{
  return true;
}

bool loadConfiguration(const char* filename)  //Loads configuration from the default file
{
  localLog(F("Loading configuration from \""));
  localLog(filename);
  localLog(F("\": "));
  if(true)
  {
    File f = openFileForReading(filename);
    if(!f)
    {
      localLogLn(F("failed, can't open file"));
      return false;
    }
    StaticJsonDocument<2048> configuration;
    DeserializationError error = deserializeJson(configuration, f);
    if(error)
    {
      localLogLn(F("failed, can't deserialise JSON"));
      return false;
    }
    //Simple values
    #if defined(SUPPORT_GPS)
      useGpsForTimeSync = configuration["useGpsForTimeSync"] | true;
      #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
        gpsStationaryTimeout = configuration["gpsStationaryTimeout"] | 300E6;
        gpsCheckInterval = configuration["gpsCheckInterval"] | 300E6;
      #endif
    #endif
    #if defined(ACT_AS_TRACKER)
      maximumEffectiveRange = configuration["maximumEffectiveRange"] | 99;
      trackingSensitivity = configuration["trackingSensitivity"] | 1;
      trackerPriority = configuration["trackerPriority"];
    #elif defined(ACT_AS_BEACON)
      if(configuration["icName"])
      {
        device[0].icName = new char[strlen(configuration["icName"]) + 1];
        strlcpy(device[0].icName,configuration["icName"],strlen(configuration["icName"]) + 1);
      }
      if(configuration["icDescription"])
      {
        device[0].icDescription = new char[strlen(configuration["icDescription"]) + 1];
        strlcpy(device[0].icDescription,configuration["icDescription"],strlen(configuration["icDescription"]) + 1);
      }
      device[0].diameter = configuration["diameter"] | 0;
      device[0].height = configuration["height"] | 0;
    #endif
    #if defined(SUPPORT_FTM)
      ftmEnabled = configuration["ftmEnabled"] | true;
      if(configuration["ftmSSID"])
      {
        ftmSSID = new char[strlen(configuration["ftmSSID"]) + 1];
        strlcpy(ftmSSID,configuration["ftmSSID"],strlen(configuration["ftmSSID"]) + 1);
      }
      else
      {
        ftmSSID = new char[strlen(default_ftmWiFi_SSID) + 1];
        strlcpy(ftmSSID,default_ftmWiFi_SSID,strlen(default_ftmWiFi_SSID) + 1);
      }
      ftmHideSSID = configuration["ftmHideSSID"] | false;
      if(configuration["ftmPSK"])
      {
        ftmPSK = new char[strlen(configuration["ftmPSK"]) + 1];
        strlcpy(ftmPSK,configuration["ftmPSK"],strlen(configuration["ftmPSK"]) + 1);
      }
      else
      {
        ftmPSK = new char[strlen(default_ftmWiFi_PSK) + 1];
        strlcpy(ftmPSK,default_ftmWiFi_PSK,strlen(default_ftmWiFi_PSK) + 1);
      }
    #endif
    #if defined(SUPPORT_ESPNOW)
      espNowEnabled = configuration["espNowEnabled"] | true;
      espNowPreferredChannel = configuration["espNowPreferredChannel"] | 1;
      espNowDeviceInfoInterval = configuration["espNowDeviceInfoInterval"] | 60000;
      defaultEspNowLocationInterval = configuration["defaultEspNowLocationInterval"] | 60000;
      espNowLocationInterval1 = configuration["espNowLocationInterval1"] | 5000;
      espNowPerimiter1 = configuration["espNowPerimiter1"] | 20;
      espNowLocationInterval2 = configuration["espNowLocationInterval2"] | 15000;
      espNowPerimiter2 = configuration["espNowPerimiter2"] | 50;
      espNowLocationInterval3 = configuration["espNowLocationInterval3"] | 30000;
      espNowPerimiter3 = configuration["espNowPerimiter3"] | 100;
    #endif
    #if defined(SUPPORT_LORA)
      loRaEnabled = configuration["loRaEnabled"] | true;
      loRaDeviceInfoInterval = configuration["loRaDeviceInfoInterval"] | 60000;
      defaultLoRaLocationInterval = configuration["defaultLoRaLocationInterval"] | 60000;
      loRaLocationInterval1 = configuration["loRaLocationInterval1"] | 5000;
      loRaPerimiter1 = configuration["loRaPerimiter1"] | 20;
      loRaLocationInterval2 = configuration["loRaLocationInterval2"] | 15000;
      loRaPerimiter2 = configuration["loRaPerimiter2"] | 50;
      loRaLocationInterval3 = configuration["loRaLocationInterval3"] | 30000;
      loRaPerimiter3 = configuration["loRaPerimiter3"] | 100;
      #if defined(MEASURE_DISTANCE_WITH_LORA)
        rssiAttenuationPerimeter = configuration["rssiAttenuationPerimeter"];
        rssiAttenuation = configuration["rssiAttenuation"];
        rssiAttenuationBaseline = configuration["rssiAttenuationBaseline"];
      #endif
    #endif
    #if defined(SUPPORT_TREACLE)
      device[0].id = configuration["id"] | 0;
      espNowEnabled = configuration["espNowEnabled"] | true;
      loRaEnabled = configuration["loRaEnabled"] | true;
      loRaFrequency = configuration["loRaFrequency"] | 868E6;
      loRaTxPower = configuration["loRaTxPower"] | 17;
      loRaRxGain = configuration["loRaRxGain"] | 0;
      loRaSpreadingFactor = configuration["loRaSpreadingFactor"] | 9;
      loRaSignalBandwidth = configuration["loRaSignalBandwidth"] | 62.5E3;
    #endif
    #if defined(SUPPORT_BATTERY_METER)
      enableBatteryMonitor = configuration["enableBatteryMonitor"] | true;
      topLadderResistor = configuration["topLadderResistor"] | 330;
      bottomLadderResistor = configuration["bottomLadderResistor"] | 90;
    #endif
    loggingBufferSize = configuration["loggingBufferSize"] | 2048;
    logFlushThreshold = configuration["logFlushThreshold"] | 2000;
    logFlushInterval = configuration["logFlushInterval"] | 57600;
    if(configuration["deviceName"])
    {
      device[0].name = new char[strlen(configuration["deviceName"]) + 1];
      strlcpy(device[0].name,configuration["deviceName"],strlen(configuration["deviceName"]) + 1);
    }
    else
    {
      device[0].name = new char[strlen(default_deviceName) + 5];
      #if defined(SUPPORT_ESPNOW) || defined(SUPPORT_LORA)
        sprintf_P(device[0].name, PSTR("%s%02X%02X"), default_deviceName, device[0].id[4], device[0].id[5]);  //Add some hex from the MAC address on the end
      #elif defined(SUPPORT_TREACLE)
        sprintf_P(device[0].name, PSTR("%s%02X%02X"), default_deviceName, localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
      #endif
    }
    if(configuration["configurationComment"])
    {
      configurationComment = new char[strlen(configuration["configurationComment"]) + 1];
      strlcpy(configurationComment,configuration["configurationComment"],strlen(configuration["configurationComment"]) + 1);
    }
    else
    {
      configurationComment = new char[strlen(default_configurationComment) + 1];
      strlcpy(configurationComment,default_configurationComment,strlen(default_configurationComment) + 1);
    }
    #if defined(SUPPORT_WIFI)
      if(configuration["SSID"] && strlen(configuration["SSID"]) > 0)
      {
        SSID = new char[strlen(configuration["SSID"]) + 1];
        strlcpy(SSID,configuration["SSID"],strlen(configuration["SSID"]) + 1);
      }
      else
      {
        SSID = new char[strlen(default_WiFi_SSID) + 1];
        strlcpy(SSID,default_WiFi_SSID,strlen(default_WiFi_SSID) + 1);
      }
      if(configuration["PSK"])
      {
        PSK = new char[strlen(configuration["PSK"]) + 1];
        strlcpy(PSK,configuration["PSK"],strlen(configuration["PSK"]) + 1);
      }
      else
      {
        PSK = new char[strlen(default_WiFi_PSK) + 1];
        strlcpy(PSK,default_WiFi_PSK,strlen(default_WiFi_PSK) + 1);
      }
      startWiFiClientOnBoot = configuration["startWiFiClientOnBoot"] | true;
      startWiFiApOnBoot = configuration["startWiFiApOnBoot"] | true;
      softApChannel = configuration["softApChannel"] | 1;
      if(configuration["APSSID"] && strlen(configuration["APSSID"]) > 0)
      {
        APSSID = new char[strlen(configuration["APSSID"]) + 1];
        strlcpy(APSSID,configuration["APSSID"],strlen(configuration["APSSID"]) + 1);
        if(configuration["APPSK"] && strlen(configuration["APPSK"]) > 0)
        {
          APPSK = new char[strlen(configuration["APPSK"]) + 1];
          strlcpy(APPSK,configuration["APPSK"],strlen(configuration["APPSK"]) + 1);
        }
        else
        {
          APPSK = new char[strlen(default_AP_PSK) + 1];
          strlcpy(APPSK,default_AP_PSK,strlen(default_AP_PSK) + 1);
        }
      }
      else
      {
        APSSID = new char[strlen(default_deviceName) + 5];
        #if defined(SUPPORT_ESPNOW) || defined(SUPPORT_LORA)
          sprintf_P(APSSID, PSTR("%s%02X%02X"), default_deviceName, device[0].id[4], device[0].id[5]);  //Add some hex from the MAC address on the end
        #elif defined(SUPPORT_TREACLE)
          sprintf_P(APSSID, PSTR("%s%02X%02X"), default_deviceName, localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
        #endif
        APPSK = new char[strlen(default_AP_PSK) + 1];
        strlcpy(APPSK,default_AP_PSK,strlen(default_AP_PSK) + 1);
      }
      wiFiClientInactivityTimer = configuration["wiFiClientInactivityTimer"] | 0;
      wifiClientTimeout = configuration["wifiClientTimeout"] | 30;
    #endif
    #if defined(ENABLE_LOCAL_WEBSERVER)
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
        if(configuration["http_user"])
        {
          http_user = new char[strlen(configuration["http_user"]) + 1];
          strlcpy(http_user,configuration["http_user"],strlen(configuration["http_user"]) + 1);
        }
        else
        {
          http_user = new char[strlen(default_http_user) + 1];
          strlcpy(http_user,default_http_user,strlen(default_http_user) + 1);
        }
        basicAuthEnabled = configuration["basicAuthEnabled"] | false;
      #endif
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH) || defined(ENABLE_OTA_UPDATE)
        if(configuration["http_password"])
        {
          http_password = new char[strlen(configuration["http_password"]) + 1];
          strlcpy(http_password,configuration["http_password"],strlen(configuration["http_password"]) + 1);
        }
        else
        {
          http_password = new char[strlen(default_http_password) + 1];
          strlcpy(http_password,default_http_password,strlen(default_http_password) + 1);
        }
      #endif
    #endif
    if(configuration["timeServer"])
    {
      timeServer = new char[strlen(configuration["timeServer"]) + 1];
      strlcpy(timeServer,configuration["timeServer"],strlen(configuration["timeServer"]) + 1);
    }
    else
    {
      timeServer = new char[strlen(default_timeServer) + 1];
      strlcpy(timeServer,default_timeServer,strlen(default_timeServer) + 1);
    }
    if(configuration["timeZone"])
    {
      timeZone = new char[strlen(configuration["timeZone"]) + 1];
      strlcpy(timeZone,configuration["timeZone"],strlen(configuration["timeZone"]) + 1);
    }
    else
    {
      timeZone = new char[strlen(default_timeZone) + 1];
      strlcpy(timeZone,default_timeZone,strlen(default_timeZone) + 1);
    }
    #if defined(SUPPORT_BEEPER)
      beeperEnabled = configuration["beeperEnabled"] | false;
      #if defined(SUPPORT_BUTTON)
        beepOnPress = configuration["beepOnPress"] | true;
      #endif
    #endif
    #if defined(SUPPORT_VIBRATION)
      vibrationEnabled = configuration["vibrationEnabled"] | true;
      vibrationLevel = configuration["vibrationLevel"] | 100;
    #endif
    #if defined(SUPPORT_LVGL)
      #if defined(SUPPORT_TOUCHSCREEN)
        touchScreenMinimumX = configuration["touchScreenMinimumX"] | 0;
        touchScreenMaximumX = configuration["touchScreenMaximumX"] | 0;
        touchScreenMinimumY = configuration["touchScreenMinimumY"] | 0;
        touchScreenMaximumY = configuration["touchScreenMaximumY"] | 0;
      #endif
      units = configuration["units"] | 0;
      dateFormat = configuration["dateFormat"] | 0;
      displayTimeout = configuration["displayTimeout"] | 0;
      minimumBrightnessLevel = configuration["minimumBrightnessLevel"] | absoluteMinimumBrightnessLevel;
      maximumBrightnessLevel = configuration["maximumBrightnessLevel"] | absoluteMaximumBrightnessLevel;
      screenRotation = configuration["screenRotation"];
    #endif
    #if defined(SUPPORT_HACKING)
      gameLength = configuration["gameLength"] | 10 ;
      gameRetries = configuration["gameRetries"] | 0;
      gameSpeedup = configuration["gameSpeedup"] | 500;
    #endif
    localLogLn(F("OK"));
    return true;
  }
  localLogLn(F("failed"));
  return false;
}
/*
 * 
 * Mostly this sets string based values where they start null, other things are set in the variable declarations
 * 
 */
bool loadDefaultConfiguration()
{
  localLogLn(F("Loading default configuration"));
  device[0].name = new char[strlen(default_deviceName) + 5];
  #if defined(SUPPORT_ESPNOW) || defined(SUPPORT_LORA)
    sprintf_P(device[0].name, PSTR("%s%02X%02X"), default_deviceName, device[0].id[4], device[0].id[5]);  //Add some hex from the MAC address on the end
  #elif defined(SUPPORT_TREACLE)
    sprintf_P(device[0].name, PSTR("%s%02X%02X"), default_deviceName, localMacAddress[4], localMacAddress[5]);  //Add some hex from the MAC address on the end
  #endif
  #if defined(ENABLE_LOCAL_WEBSERVER)
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      http_user = new char[strlen(default_http_user) + 1];
      strlcpy(http_user,default_http_user,strlen(default_http_user) + 1);
    #endif
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH) || defined(ENABLE_OTA_UPDATE)
      http_password = new char[strlen(default_http_password) + 1];
      strlcpy(http_password,default_http_password,strlen(default_http_password) + 1);
    #endif
  #endif
  #if defined(SUPPORT_WIFI)
    SSID = new char[strlen(default_WiFi_SSID) + 1];
    strlcpy(SSID,default_WiFi_SSID,strlen(default_WiFi_SSID) + 1);
    PSK = new char[strlen(default_WiFi_PSK) + 1];
    strlcpy(PSK,default_WiFi_PSK,strlen(default_WiFi_PSK) + 1);
    APSSID = new char[strlen(device[0].name) + 1];
    strlcpy(APSSID,device[0].name,strlen(device[0].name) + 1);
    APPSK = new char[strlen(default_AP_PSK) + 1];
    strlcpy(APPSK,default_AP_PSK,strlen(default_AP_PSK) + 1);
  #endif
  timeServer = new char[strlen(default_timeServer) + 1];
  strlcpy(timeServer,default_timeServer,strlen(default_timeServer) + 1);
  timeZone = new char[strlen(default_timeZone) + 1];
  strlcpy(timeZone,default_timeZone,strlen(default_timeZone) + 1);
  configurationComment = new char[strlen(default_configurationComment) + 1];  //Assign space on heap
  strlcpy(configurationComment,default_configurationComment,strlen(default_configurationComment) + 1);  //Copy in default
  return true;
}

void printConfiguration()
{
  localLogLn(F("==Current configuration=="));
  localLog(F("deviceName: "));
  if(device[0].name != nullptr)
  {
    localLogLn(device[0].name);
  }
  else
  {
    localLogLn(F("<none>"));
  }
  localLog(F("configurationComment: "));
  if(configurationComment != nullptr)
  {
    localLogLn(configurationComment);
  }
  else
  {
    localLogLn(F("<none>"));
  }
  #if defined(SUPPORT_WIFI)
    localLog(F("startWiFiClientOnBoot: "));
    if(startWiFiClientOnBoot)
    {
      localLogLn(F("enabled"));
    }
    else
    {
      localLogLn(F("disabled"));
    }
    localLog(F("SSID: "));
    if(SSID != nullptr)
    {
      localLogLn(SSID);
    }
    else
    {
      localLogLn(F("<none>"));
    }
    localLog(F("PSK: "));
    if(PSK != nullptr)
    {
      localLogLn(F("<set>"));
    }
    else
    {
      localLogLn(F("<none>"));
    }
    localLog(F("wifiClientTimeout: ")); localLogLn(wifiClientTimeout);
    localLog(F("wiFiClientInactivityTimer: ")); localLogLn(wiFiClientInactivityTimer);
    localLog(F("startWiFiApOnBoot: "));
    if(startWiFiApOnBoot)
    {
      localLogLn(F("enabled"));
    }
    else
    {
      localLogLn(F("disabled"));
    }
    localLog(F("softApChannel: ")); localLogLn(softApChannel);
    localLog(F("AP SSID: "));
    if(APSSID != nullptr)
    {
      localLogLn(APSSID);
    }
    else
    {
      localLogLn(F("<none>"));
    }
    localLog(F("AP PSK: "));
    if(APPSK != nullptr)
    {
      localLogLn(F("<set>"));
    }
    else
    {
      localLogLn(F("<none>"));
    }
    #if defined(ENABLE_LOCAL_WEBSERVER)
      localLog(F("enableCaptivePortal: "));
      if(enableCaptivePortal)
      {
        localLogLn(F("enabled"));
      }
      else
      {
        localLogLn(F("disabled"));
      }
    #endif
    localLog(F("timeServer: "));
    if(timeServer != nullptr)
    {
      localLogLn(timeServer);
    }
    else
    {
      localLogLn(F("<none>"));
    }
    localLog(F("timeZone: "));
    if(timeZone != nullptr)
    {
      localLogLn(timeZone);
    }
    else
    {
      localLogLn(F("<none>"));
    }
  #endif
  #if defined(ENABLE_LOCAL_WEBSERVER)
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      localLog(F("basicAuthEnabled: "));
      if(basicAuthEnabled)
      {
        localLogLn(F("enabled"));
      }
      else
      {
        localLogLn(F("disabled"));
      }
      localLog(F("http_user: "));
      if(http_user != nullptr)
      {
        localLogLn(http_user);
      }
      else
      {
        localLogLn(F("<none>"));
      }
    #endif
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH) || defined(ENABLE_OTA_UPDATE)
      localLog(F("http_password: "));
      if(http_password != nullptr)
      {
        localLogLn(F("<set>"));
      }
      else
      {
        localLogLn(F("<not set>"));
      }
    #endif
  #endif
  #if defined(ENABLE_OTA_UPDATE)
    localLog(F("otaEnabled: ")); localLogLn(otaEnabled);
    localLog(F("otaAuthenticationEnabled: ")); localLogLn(otaAuthenticationEnabled);
  #endif
  #if defined(SUPPORT_GPS)
    localLog(F("useGpsForTimeSync: "));
    if(useGpsForTimeSync)
    {
      localLogLn(F("enabled"));
    }
    else
    {
      localLogLn(F("disabled"));
    }
    #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
      localLog(F("gpsStationaryTimeout: ")); localLogLn(gpsStationaryTimeout);
      localLog(F("gpsCheckInterval: ")); localLogLn(gpsCheckInterval);
    #endif
  #endif
  #if defined(ACT_AS_TRACKER)
    localLog(F("maximumEffectiveRange: ")); localLogLn(maximumEffectiveRange);
    localLog(F("trackingSensitivity: ")); localLogLn(sensitivityValues[trackingSensitivity]);
    localLog(F("trackerPriority: ")); localLogLn(trackerPriority);
  #elif defined(ACT_AS_BEACON)
    localLog(F("icName: ")); localLogLn(device[0].icName);
    localLog(F("icDescription: ")); localLogLn(device[0].icDescription);
    localLog(F("diameter: ")); localLogLn(device[0].diameter);
    localLog(F("height: ")); localLogLn(device[0].height);
  #endif
  #if defined(SUPPORT_FTM)
    localLog(F("ftmEnabled: ")); localLogLn(ftmEnabled);
    if(ftmSSID != nullptr)
    {
      localLog(F("ftmSSID: ")); localLogLn(ftmSSID);
    }
    localLog(F("ftmHideSSID: ")); localLogLn(ftmHideSSID);
    if(ftmPSK != nullptr)
    {
      localLog(F("ftmPSK: ")); localLogLn(ftmPSK);
    }
  #endif
  #if defined(SUPPORT_ESPNOW)
    localLog(F("espNowEnabled: ")); localLogLn(espNowEnabled);
    localLog(F("espNowPreferredChannel: ")); localLogLn(espNowPreferredChannel);
    localLog(F("espNowDeviceInfoInterval: ")); localLogLn(espNowDeviceInfoInterval);
    localLog(F("defaultEspNowLocationInterval: ")); localLogLn(defaultEspNowLocationInterval);
    localLog(F("espNowPerimiter1: ")); localLogLn(espNowPerimiter1);
    localLog(F("espNowLocationInterval1: ")); localLogLn(espNowLocationInterval1);
    localLog(F("espNowPerimiter2: ")); localLogLn(espNowPerimiter2);
    localLog(F("espNowLocationInterval2: ")); localLogLn(espNowLocationInterval2);
    localLog(F("espNowPerimiter3: ")); localLogLn(espNowPerimiter3);
    localLog(F("espNowLocationInterval3: ")); localLogLn(espNowLocationInterval3);
  #endif
  #if defined(SUPPORT_LORA)
    localLog(F("loRaEnabled: ")); localLogLn(loRaEnabled);
    localLog(F("loRaDeviceInfoInterval: ")); localLogLn(loRaDeviceInfoInterval);
    localLog(F("defaultLoRaLocationInterval: ")); localLogLn(defaultLoRaLocationInterval);
    localLog(F("loRaPerimiter1: ")); localLogLn(loRaPerimiter1);
    localLog(F("loRaLocationInterval1: ")); localLogLn(loRaLocationInterval1);
    localLog(F("loRaPerimiter2: ")); localLogLn(loRaPerimiter2);
    localLog(F("loRaLocationInterval2: ")); localLogLn(loRaLocationInterval2);
    localLog(F("loRaPerimiter3: ")); localLogLn(loRaPerimiter3);
    localLog(F("loRaLocationInterval3: ")); localLogLn(loRaLocationInterval3);
    #if defined(MEASURE_DISTANCE_WITH_LORA)
      localLog(F("rssiAttenuation: ")); localLogLn(rssiAttenuation);
      localLog(F("rssiAttenuationBaseline: ")); localLogLn(rssiAttenuationBaseline);
      localLog(F("rssiAttenuationPerimeter: ")); localLogLn(rssiAttenuationPerimeter);
    #endif
  #endif
  #if defined(SUPPORT_TREACLE)
    localLog(F("id: ")); localLogLn(device[0].id);
    localLog(F("espNowEnabled: ")); localLogLn(espNowEnabled);
    localLog(F("loRaEnabled: ")); localLogLn(loRaEnabled);
    localLog(F("loRaFrequency: ")); localLogLn(loRaFrequency/1e6);
    localLog(F("loRaTxPower: ")); localLogLn(loRaTxPower);
    localLog(F("loRaRxGain: ")); localLogLn(loRaRxGain);
    localLog(F("loRaSpreadingFactor: ")); localLogLn(loRaSpreadingFactor);
    localLog(F("loRaSignalBandwidth: ")); localLogLn(loRaSignalBandwidth);
  #endif
  #if defined(SUPPORT_BATTERY_METER)
    localLog(F("enableBatteryMonitor: ")); localLogLn(enableBatteryMonitor);
    localLog(F("topLadderResistor: ")); localLogLn(topLadderResistor);
    localLog(F("bottomLadderResistor: ")); localLogLn(bottomLadderResistor);
  #endif
  #if defined(SUPPORT_BEEPER)
    localLog(F("beeperEnabled: ")); localLogLn(beeperEnabled);
    #if defined(SUPPORT_BUTTON)
      localLog(F("beepOnPress: ")); localLogLn(beepOnPress);
    #endif
  #endif
  #if defined(SUPPORT_VIBRATION)
    localLog(F("vibrationEnabled: ")); localLogLn(vibrationEnabled);
    localLog(F("vibrationLevel: ")); localLogLn(vibrationLevel);
  #endif
  #if defined(SUPPORT_LVGL)
    #if defined(SUPPORT_TOUCHSCREEN)
      localLog(F("touchScreenMinimumX: ")); localLogLn(touchScreenMinimumX);
      localLog(F("touchScreenMaximumX: ")); localLogLn(touchScreenMaximumX);
      localLog(F("touchScreenMinimumY: ")); localLogLn(touchScreenMinimumY);
      localLog(F("touchScreenMaximumY: ")); localLogLn(touchScreenMaximumY);
    #endif
    localLog(F("units: ")); localLogLn(units);
    localLog(F("dateFormat: ")); localLogLn(dateFormat);
    localLog(F("displayTimeout: ")); localLogLn(displayTimeout);
    localLog(F("minimumBrightnessLevel: ")); localLogLn(minimumBrightnessLevel);
    localLog(F("maximumBrightnessLevel: ")); localLogLn(maximumBrightnessLevel);
    localLog(F("screenRotation: ")); localLogLn(screenRotation);
  #endif
  localLog(F("loggingBufferSize: ")); localLogLn(loggingBufferSize);
  localLog(F("logFlushThreshold: ")); localLogLn(logFlushThreshold);
  localLog(F("logFlushInterval: ")); localLogLn(logFlushInterval);
  #if defined(SUPPORT_HACKING)
    localLog(F("gameLength: "));
    localLogLn(gameLength);
    localLog(F("gameRetries: "));
    localLogLn(gameRetries);
    localLog(F("gameSpeedup: "));
    localLogLn(gameSpeedup);
  #endif
  localLogLn(F("========================="));
  #if defined(ACT_AS_SENSOR)
  showSensorConfiguration();
  #endif
}

String deviceFeatures(uint8_t featureFlags)
{
  String features;
  if((featureFlags & 0x01) == 0)
  {
    features = "GPS beacon";
  }
  else
  {
    features = "Tracker";
  }
  if((featureFlags & 0x02) == 2)
  {
    features += " Lasertag sensor";
  }
  if((featureFlags & 0x04) == 4)
  {
    features += " Lasertag emitter";
  }
  if((featureFlags & 0x08) == 8)
  {
    features += " Time-of-flight beacon";
  }
  return features;
}

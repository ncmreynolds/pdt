/*
 * 
 * This file contains functions related to storing/retrieving configuration from JSON files on the local flash filesystem
 * 
 */
const char string_colonSpace[] PROGMEM = ": ";
const char string_ltNoneGt[] PROGMEM = "<none>";
const char string_ltSetGt[] PROGMEM = "<set>";
const char string_enabled[] PROGMEM = "enabled";
const char string_disabled[] PROGMEM = "disabled";

bool saveConfiguration(const char* filename)  //Saves the configuration
{
  backupConfiguration(filename);
  localLog(F("Saving configuration to \""));
  localLog(filename);
  localLog(F("\": "));
  DynamicJsonDocument configuration(2048);
  configuration[string_deviceName] = device[0].name;
  configuration[string_deviceUsuallyStatic] = deviceUsuallyStatic;
  configuration[string_configurationComment] = configurationComment;
  #if defined(SUPPORT_WIFI)
    configuration[string_SSID] = SSID;
    configuration[string_PSK] = PSK;
    configuration[string_startWiFiClientOnBoot] = startWiFiClientOnBoot;
    configuration[string_APSSID] = APSSID;
    configuration[string_APPSK] = APPSK;
    configuration[string_startWiFiApOnBoot] = startWiFiApOnBoot;
    configuration[string_softApChannel] = softApChannel;
    #if defined(ENABLE_LOCAL_WEBSERVER)
      configuration[string_enableCaptivePortal] = enableCaptivePortal;
    #endif
    configuration[string_wiFiClientInactivityTimer]  = wiFiClientInactivityTimer;
    configuration[string_wifiClientTimeout]  = wifiClientTimeout;
  #endif
  #if defined(ENABLE_LOCAL_WEBSERVER)
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      configuration[string_http_user] = http_user;
      configuration[string_basicAuthEnabled] = basicAuthEnabled;
    #endif
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH) || defined(ENABLE_OTA_UPDATE)
      configuration[string_http_password] = http_password;
    #endif
  #endif
  #if defined(SUPPORT_HACKING)
    configuration[string_gameLength] = gameLength;
    configuration[string_gameRetries] = gameRetries;
    configuration[string_gameSpeedup] = gameSpeedup;
  #endif
  configuration[string_timeServer] = timeServer;
  configuration[string_timeZone] = timeZone;
  #if defined(SUPPORT_GPS)
    configuration[string_useGpsForTimeSync] = useGpsForTimeSync;
    configuration[string_movementThreshold] = movementThreshold;
    configuration[string_smoothingFactor] = smoothingFactor;
    #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
      configuration[string_gpsStationaryTimeout] = gpsStationaryTimeout;
      configuration[string_gpsCheckInterval] = gpsCheckInterval;
    #endif
  #endif
  #if defined(SUPPORT_BATTERY_METER)
    configuration[string_enableBatteryMonitor] = enableBatteryMonitor;
    configuration[string_topLadderResistor] = topLadderResistor;
    configuration[string_bottomLadderResistor] = bottomLadderResistor;
  #endif
  #if defined(SUPPORT_BEEPER)
    configuration[string_beeperEnabled] = beeperEnabled;
    #if defined(SUPPORT_BUTTON)
      configuration[string_beepOnPress] = beepOnPress;
    #endif
  #endif
  #if defined(SUPPORT_VIBRATION)
    configuration[string_vibrationEnabled] = vibrationEnabled;
    configuration[string_vibrationLevel] = vibrationLevel;
  #endif
  #if defined(ACT_AS_TRACKER)
    configuration[string_maximumEffectiveRange] = maximumEffectiveRange;
    configuration[string_trackingSensitivity] = trackingSensitivity;
    configuration[string_trackerPriority] = trackerPriority;
  #endif
  configuration[string_icName] = device[0].icName;
  configuration[string_icDescription] = device[0].icDescription;
  configuration[string_diameter] = device[0].diameter;
  configuration[string_diameter] = device[0].height;
  #if defined(SUPPORT_ESPNOW)
    configuration[string_espNowEnabled] = espNowEnabled;
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
    configuration[string_loRaEnabled] = loRaEnabled;
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
    configuration[string_id] = device[0].id;
    configuration[string_treacleEncryptionEnabled] = treacleEncryptionEnabled;
    configuration[string_espNowEnabled] = espNowEnabled;
    configuration[string_espNowTickInterval] = espNowTickInterval;
    configuration[string_loRaEnabled] = loRaEnabled;
    configuration[string_loRaTickInterval] = loRaTickInterval;
    configuration[string_loRaFrequency] = loRaFrequency;
    configuration[string_loRaTxPower] = loRaTxPower;
    configuration[string_loRaRxGain] = loRaRxGain;
    configuration[string_loRaSpreadingFactor] = loRaSpreadingFactor;
    configuration[string_loRaSignalBandwidth] = loRaSignalBandwidth;
  #endif
  #if defined(SUPPORT_LVGL)
    configuration[string_units] = units;
    configuration[string_dateFormat] = dateFormat;
    configuration[string_displayTimeout] = displayTimeout;
    configuration[string_minimumBrightnessLevel] = minimumBrightnessLevel;
    configuration[string_maximumBrightnessLevel] = maximumBrightnessLevel;
    configuration[string_screenRotation] = screenRotation;
    configuration[string_enableHomeTab] = enableHomeTab;
    configuration[string_enableMapTab] = enableMapTab;
    configuration[string_enableInfoTab] = enableInfoTab;
    configuration[string_enableGpsTab] = enableGpsTab;
    configuration[string_enableSettingsTab] = enableSettingsTab;
    #if defined(SUPPORT_TOUCHSCREEN)
      configuration[string_touchScreenMinimumX] = touchScreenMinimumX;
      configuration[string_touchScreenMaximumX] = touchScreenMaximumX;
      configuration[string_touchScreenMinimumY] = touchScreenMinimumY;
      configuration[string_touchScreenMaximumY] = touchScreenMaximumY;
    #endif
  #endif
  configuration[string_loggingBufferSize] = loggingBufferSize;
  configuration[string_logFlushThreshold] = logFlushThreshold;
  configuration[string_logFlushInterval] = logFlushInterval;
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
      useGpsForTimeSync = configuration[string_useGpsForTimeSync] | true;
      movementThreshold = configuration[string_movementThreshold] | 0.5;
      smoothingFactor = configuration[string_smoothingFactor] | 0.75;
      #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
        gpsStationaryTimeout = configuration[string_gpsStationaryTimeout] | 300E6;
        gpsCheckInterval = configuration[string_gpsCheckInterval] | 300E6;
      #endif
    #endif
    #if defined(ACT_AS_TRACKER)
      maximumEffectiveRange = configuration[string_maximumEffectiveRange] | 99;
      trackingSensitivity = configuration[string_trackingSensitivity] | 1;
      trackerPriority = configuration[string_trackerPriority];
    #endif
    if(configuration[string_icName])
    {
      device[0].icName = new char[strlen(configuration[string_icName]) + 1];
      strlcpy(device[0].icName,configuration[string_icName],strlen(configuration[string_icName]) + 1);
    }
    if(configuration[string_icDescription])
    {
      device[0].icDescription = new char[strlen(configuration[string_icDescription]) + 1];
      strlcpy(device[0].icDescription,configuration[string_icDescription],strlen(configuration[string_icDescription]) + 1);
    }
    device[0].diameter = configuration[string_diameter] | 1;
    device[0].height = configuration[string_height] | 2;
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
      espNowEnabled = configuration[string_espNowEnabled] | true;
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
      loRaEnabled = configuration[string_loRaEnabled] | true;
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
      device[0].id = configuration[string_id] | 0;
      treacleEncryptionEnabled = configuration[string_treacleEncryptionEnabled] | false;
      espNowEnabled = configuration[string_espNowEnabled] | true;
      espNowTickInterval = configuration[string_espNowTickInterval] | 10e3;
      loRaTickInterval = configuration[string_loRaTickInterval] | 45e3;
      loRaEnabled = configuration[string_loRaEnabled] | true;
      loRaFrequency = configuration[string_loRaFrequency] | 868E6;
      loRaTxPower = configuration[string_loRaTxPower] | 17;
      loRaRxGain = configuration[string_loRaRxGain] | 0;
      loRaSpreadingFactor = configuration[string_loRaSpreadingFactor] | 9;
      loRaSignalBandwidth = configuration[string_loRaSignalBandwidth] | 62.5E3;
    #endif
    #if defined(SUPPORT_BATTERY_METER)
      enableBatteryMonitor = configuration[string_enableBatteryMonitor] | true;
      topLadderResistor = configuration[string_topLadderResistor] | 330;
      bottomLadderResistor = configuration[string_bottomLadderResistor] | 90;
    #endif
    loggingBufferSize = configuration[string_loggingBufferSize] | 2048;
    logFlushThreshold = configuration[string_logFlushThreshold] | 2000;
    logFlushInterval = configuration[string_logFlushInterval] | 57600;
    if(configuration[string_deviceName])
    {
      device[0].name = new char[strlen(configuration[string_deviceName]) + 1];
      strlcpy(device[0].name,configuration[string_deviceName],strlen(configuration[string_deviceName]) + 1);
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
    deviceUsuallyStatic = configuration[string_deviceUsuallyStatic] | false;
    if(configuration[string_configurationComment])
    {
      configurationComment = new char[strlen(configuration[string_configurationComment]) + 1];
      strlcpy(configurationComment,configuration[string_configurationComment],strlen(configuration[string_configurationComment]) + 1);
    }
    else
    {
      configurationComment = new char[strlen(default_configurationComment) + 1];
      strlcpy(configurationComment,default_configurationComment,strlen(default_configurationComment) + 1);
    }
    #if defined(SUPPORT_WIFI)
      if(configuration[string_SSID] && strlen(configuration[string_SSID]) > 0)
      {
        SSID = new char[strlen(configuration[string_SSID]) + 1];
        strlcpy(SSID,configuration[string_SSID],strlen(configuration[string_SSID]) + 1);
      }
      else
      {
        SSID = new char[strlen(default_WiFi_SSID) + 1];
        strlcpy(SSID,default_WiFi_SSID,strlen(default_WiFi_SSID) + 1);
      }
      if(configuration[string_PSK])
      {
        PSK = new char[strlen(configuration[string_PSK]) + 1];
        strlcpy(PSK,configuration[string_PSK],strlen(configuration[string_PSK]) + 1);
      }
      else
      {
        PSK = new char[strlen(default_WiFi_PSK) + 1];
        strlcpy(PSK,default_WiFi_PSK,strlen(default_WiFi_PSK) + 1);
      }
      startWiFiClientOnBoot = configuration[string_startWiFiClientOnBoot] | true;
      startWiFiApOnBoot = configuration[string_startWiFiApOnBoot] | true;
      softApChannel = configuration[string_softApChannel] | 1;
      if(configuration[string_APSSID] && strlen(configuration[string_APSSID]) > 0)
      {
        APSSID = new char[strlen(configuration[string_APSSID]) + 1];
        strlcpy(APSSID,configuration[string_APSSID],strlen(configuration[string_APSSID]) + 1);
        if(configuration[string_APPSK] && strlen(configuration[string_APPSK]) > 0)
        {
          APPSK = new char[strlen(configuration[string_APPSK]) + 1];
          strlcpy(APPSK,configuration[string_APPSK],strlen(configuration[string_APPSK]) + 1);
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
      wiFiClientInactivityTimer = configuration[string_wiFiClientInactivityTimer] | 0;
      wifiClientTimeout = configuration[string_wifiClientTimeout] | 30;
    #endif
    #if defined(ENABLE_LOCAL_WEBSERVER)
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
        if(configuration[string_http_user])
        {
          http_user = new char[strlen(configuration[string_http_user]) + 1];
          strlcpy(http_user,configuration[string_http_user],strlen(configuration[string_http_user]) + 1);
        }
        else
        {
          http_user = new char[strlen(default_http_user) + 1];
          strlcpy(http_user,default_http_user,strlen(default_http_user) + 1);
        }
        basicAuthEnabled = configuration[string_basicAuthEnabled] | false;
      #endif
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH) || defined(ENABLE_OTA_UPDATE)
        if(configuration[string_http_password])
        {
          http_password = new char[strlen(configuration[string_http_password]) + 1];
          strlcpy(http_password,configuration[string_http_password],strlen(configuration[string_http_password]) + 1);
        }
        else
        {
          http_password = new char[strlen(default_http_password) + 1];
          strlcpy(http_password,default_http_password,strlen(default_http_password) + 1);
        }
      #endif
    #endif
    if(configuration[string_timeServer])
    {
      timeServer = new char[strlen(configuration[string_timeServer]) + 1];
      strlcpy(timeServer,configuration[string_timeServer],strlen(configuration[string_timeServer]) + 1);
    }
    else
    {
      timeServer = new char[strlen(default_timeServer) + 1];
      strlcpy(timeServer,default_timeServer,strlen(default_timeServer) + 1);
    }
    if(configuration[string_timeZone])
    {
      timeZone = new char[strlen(configuration[string_timeZone]) + 1];
      strlcpy(timeZone,configuration[string_timeZone],strlen(configuration[string_timeZone]) + 1);
    }
    else
    {
      timeZone = new char[strlen(default_timeZone) + 1];
      strlcpy(timeZone,default_timeZone,strlen(default_timeZone) + 1);
    }
    #if defined(SUPPORT_BEEPER)
      beeperEnabled = configuration[string_beeperEnabled] | false;
      #if defined(SUPPORT_BUTTON)
        beepOnPress = configuration[string_beepOnPress] | true;
      #endif
    #endif
    #if defined(SUPPORT_VIBRATION)
      vibrationEnabled = configuration[string_vibrationEnabled] | true;
      vibrationLevel = configuration[string_vibrationLevel] | 100;
    #endif
    #if defined(SUPPORT_LVGL)
      #if defined(SUPPORT_TOUCHSCREEN)
        touchScreenMinimumX = configuration[string_touchScreenMinimumX] | 0;
        touchScreenMaximumX = configuration[string_touchScreenMaximumX] | 0;
        touchScreenMinimumY = configuration[string_touchScreenMinimumY] | 0;
        touchScreenMaximumY = configuration[string_touchScreenMaximumY] | 0;
      #endif
      units = configuration[string_units] | 0;
      dateFormat = configuration[string_dateFormat] | 0;
      displayTimeout = configuration[string_displayTimeout] | 0;
      minimumBrightnessLevel = configuration[string_minimumBrightnessLevel] | absoluteMinimumBrightnessLevel;
      maximumBrightnessLevel = configuration[string_maximumBrightnessLevel] | absoluteMaximumBrightnessLevel;
      screenRotation = configuration[string_screenRotation];
      enableHomeTab = configuration[string_enableHomeTab] | true;
      enableMapTab = configuration[string_enableMapTab] | true;
      enableInfoTab = configuration[string_enableInfoTab] | true;
      enableGpsTab = configuration[string_enableGpsTab] | true;
      enableSettingsTab = configuration[string_enableSettingsTab] | true;
    #endif
    #if defined(SUPPORT_HACKING)
      gameLength = configuration[string_gameLength] | 10 ;
      gameRetries = configuration[string_gameRetries] | 0;
      gameSpeedup = configuration[string_gameSpeedup] | 500;
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
  localLog(string_deviceName); localLog(string_colonSpace);
  if(device[0].name != nullptr)
  {
    localLogLn(device[0].name);
  }
  else
  {
    localLogLn(F(string_ltNoneGt));
  }
  localLog(string_deviceUsuallyStatic); localLog(string_colonSpace); localLog(deviceUsuallyStatic);
  localLog(string_configurationComment); localLog(string_colonSpace);
  if(configurationComment != nullptr)
  {
    localLogLn(configurationComment);
  }
  else
  {
    localLogLn(F(string_ltNoneGt));
  }
  #if defined(SUPPORT_WIFI)
    localLog(string_startWiFiClientOnBoot); localLog(string_colonSpace);
    if(startWiFiClientOnBoot)
    {
      localLogLn(string_enabled);
    }
    else
    {
      localLogLn(string_disabled);
    }
    localLog(string_SSID); localLog(string_colonSpace);
    if(SSID != nullptr)
    {
      localLogLn(SSID);
    }
    else
    {
      localLogLn(F(string_ltNoneGt));
    }
    localLog(string_PSK); localLog(string_colonSpace);
    if(PSK != nullptr)
    {
      localLogLn(string_ltSetGt);
    }
    else
    {
      localLogLn(F(string_ltNoneGt));
    }
    localLog(string_wifiClientTimeout); localLog(string_colonSpace); localLogLn(wifiClientTimeout);
    localLog(string_wiFiClientInactivityTimer); localLog(string_colonSpace); localLogLn(wiFiClientInactivityTimer);
    localLog(string_startWiFiApOnBoot); localLog(string_colonSpace);
    if(startWiFiApOnBoot)
    {
      localLogLn(string_enabled);
    }
    else
    {
      localLogLn(string_disabled);
    }
    localLog(string_softApChannel); localLog(string_colonSpace); localLogLn(softApChannel);
    localLog(string_APSSID); localLog(string_colonSpace);
    if(APSSID != nullptr)
    {
      localLogLn(APSSID);
    }
    else
    {
      localLogLn(F(string_ltNoneGt));
    }
    localLog(string_APPSK); localLog(string_colonSpace);
    if(APPSK != nullptr)
    {
      localLogLn(string_ltSetGt);
    }
    else
    {
      localLogLn(F(string_ltNoneGt));
    }
    #if defined(ENABLE_LOCAL_WEBSERVER)
      localLog(string_enableCaptivePortal); localLog(string_colonSpace);
      if(enableCaptivePortal)
      {
        localLogLn(string_enabled);
      }
      else
      {
        localLogLn(string_disabled);
      }
    #endif
    localLog(string_timeServer); localLog(string_colonSpace);
    if(timeServer != nullptr)
    {
      localLogLn(timeServer);
    }
    else
    {
      localLogLn(F(string_ltNoneGt));
    }
    localLog(string_timeZone); localLog(string_colonSpace);
    if(timeZone != nullptr)
    {
      localLogLn(timeZone);
    }
    else
    {
      localLogLn(F(string_ltNoneGt));
    }
  #endif
  #if defined(ENABLE_LOCAL_WEBSERVER)
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      localLog(string_basicAuthEnabled); localLog(string_colonSpace);
      if(basicAuthEnabled)
      {
        localLogLn(string_enabled);
      }
      else
      {
        localLogLn(string_disabled);
      }
      localLog(string_http_user); localLog(string_colonSpace);
      if(http_user != nullptr)
      {
        localLogLn(http_user);
      }
      else
      {
        localLogLn(F(string_ltNoneGt));
      }
    #endif
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH) || defined(ENABLE_OTA_UPDATE)
      localLog(string_http_password); localLog(string_colonSpace);
      if(http_password != nullptr)
      {
        localLogLn(string_ltSetGt);
      }
      else
      {
        localLogLn(string_ltNoneGt);
      }
    #endif
  #endif
  #if defined(ENABLE_OTA_UPDATE)
    localLog(string_otaEnabled); localLog(string_colonSpace); localLogLn(otaEnabled);
    localLog(string_otaAuthenticationEnabled); localLog(string_colonSpace); localLogLn(otaAuthenticationEnabled);
  #endif
  #if defined(SUPPORT_GPS)
    localLog(string_useGpsForTimeSync); localLog(string_colonSpace);
    if(useGpsForTimeSync)
    {
      localLogLn(string_enabled);
    }
    else
    {
      localLogLn(string_disabled);
    }
    localLog(string_movementThreshold); localLog(string_colonSpace); localLogLn(movementThreshold);
    localLog(string_smoothingFactor); localLog(string_colonSpace); localLogLn(smoothingFactor);
    #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
      localLog(string_useGpsForTimeSync); localLog(string_colonSpace); localLogLn(gpsStationaryTimeout);
      localLog(string_gpsCheckInterval); localLog(string_colonSpace); localLogLn(gpsCheckInterval);
    #endif
  #endif
  #if defined(ACT_AS_TRACKER)
    localLog(string_maximumEffectiveRange); localLog(string_colonSpace); localLogLn(maximumEffectiveRange);
    localLog(string_trackingSensitivity); localLog(string_colonSpace); localLogLn(sensitivityValues[trackingSensitivity]);
    localLog(string_trackerPriority); localLog(string_colonSpace); localLogLn(trackerPriority);
  #endif
  localLog(string_icName); localLog(string_colonSpace); localLogLn(device[0].icName);
  localLog(string_icDescription); localLog(string_colonSpace); localLogLn(device[0].icDescription);
  localLog(string_diameter); localLog(string_colonSpace); localLogLn(device[0].diameter);
  localLog(string_height); localLog(string_colonSpace); localLogLn(device[0].height);
  #if defined(SUPPORT_FTM)
    localLog(string_ftmEnabled); localLog(string_colonSpace); localLogLn(ftmEnabled);
    if(ftmSSID != nullptr)
    {
      localLog(string_ftmSSID); localLog(string_colonSpace); localLogLn(ftmSSID);
    }
    localLog(string_ftmHideSSID); localLog(string_colonSpace); localLogLn(ftmHideSSID);
    if(ftmPSK != nullptr)
    {
      localLog(string_ftmPSK); localLog(string_colonSpace); localLogLn(ftmPSK);
    }
  #endif
  #if defined(SUPPORT_ESPNOW)
    localLog(string_espNowEnabled); localLog(string_colonSpace); localLogLn(espNowEnabled);
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
    localLog(string_id); localLog(string_colonSpace); localLogLn(device[0].id);
    localLog(string_treacleEncryptionEnabled); localLog(string_colonSpace); localLogLn(treacleEncryptionEnabled);
    localLog(string_espNowEnabled); localLog(string_colonSpace); localLogLn(espNowEnabled);
    localLog(string_espNowTickInterval); localLog(string_colonSpace); localLogLn(espNowTickInterval);
    localLog(string_loRaEnabled); localLog(string_colonSpace); localLogLn(loRaEnabled);
    localLog(string_loRaTickInterval); localLog(string_colonSpace); localLogLn(loRaTickInterval);
    localLog(string_loRaFrequency); localLog(string_colonSpace); localLogLn(loRaFrequency/1e6);
    localLog(string_loRaTxPower); localLog(string_colonSpace); localLogLn(loRaTxPower);
    localLog(string_loRaRxGain); localLog(string_colonSpace); localLogLn(loRaRxGain);
    localLog(string_loRaSpreadingFactor); localLog(string_colonSpace); localLogLn(loRaSpreadingFactor);
    localLog(string_loRaSignalBandwidth); localLog(string_colonSpace); localLogLn(loRaSignalBandwidth);
  #endif
  #if defined(SUPPORT_BATTERY_METER)
    localLog(string_enableBatteryMonitor); localLog(string_colonSpace); localLogLn(enableBatteryMonitor);
    localLog(string_topLadderResistor); localLog(string_colonSpace); localLogLn(topLadderResistor);
    localLog(string_bottomLadderResistor); localLog(string_colonSpace); localLogLn(bottomLadderResistor);
  #endif
  #if defined(SUPPORT_BEEPER)
    localLog(string_beeperEnabled); localLog(string_colonSpace); localLogLn(beeperEnabled);
    #if defined(SUPPORT_BUTTON)
      localLog(string_beepOnPress); localLog(string_colonSpace); localLogLn(beepOnPress);
    #endif
  #endif
  #if defined(SUPPORT_VIBRATION)
    localLog(string_vibrationEnabled); localLog(string_colonSpace); localLogLn(vibrationEnabled);
    localLog(string_vibrationLevel); localLog(string_colonSpace); localLogLn(vibrationLevel);
  #endif
  #if defined(SUPPORT_LVGL)
    #if defined(SUPPORT_TOUCHSCREEN)
      localLog(string_touchScreenMinimumX); localLog(string_colonSpace); localLogLn(touchScreenMinimumX);
      localLog(string_touchScreenMaximumX); localLog(string_colonSpace); localLogLn(touchScreenMaximumX);
      localLog(string_touchScreenMinimumY); localLog(string_colonSpace); localLogLn(touchScreenMinimumY);
      localLog(string_touchScreenMaximumY); localLog(string_colonSpace); localLogLn(touchScreenMaximumY);
    #endif
    localLog(string_units); localLog(string_colonSpace); localLogLn(units);
    localLog(string_dateFormat); localLog(string_colonSpace); localLogLn(dateFormat);
    localLog(string_displayTimeout); localLog(string_colonSpace); localLogLn(displayTimeout);
    localLog(string_minimumBrightnessLevel); localLog(string_colonSpace); localLogLn(minimumBrightnessLevel);
    localLog(string_maximumBrightnessLevel); localLog(string_colonSpace); localLogLn(maximumBrightnessLevel);
    localLog(string_screenRotation); localLog(string_colonSpace); localLogLn(screenRotation);
    localLog(string_enableHomeTab); localLog(string_colonSpace); localLogLn(enableHomeTab);
    localLog(string_enableMapTab); localLog(string_colonSpace); localLogLn(enableMapTab);
    localLog(string_enableInfoTab); localLog(string_colonSpace); localLogLn(enableInfoTab);
    localLog(string_enableGpsTab); localLog(string_colonSpace); localLogLn(enableGpsTab);
    localLog(string_enableSettingsTab); localLog(string_colonSpace); localLogLn(enableSettingsTab);
  #endif
  localLog(string_loggingBufferSize); localLog(string_colonSpace); localLogLn(loggingBufferSize);
  localLog(string_logFlushThreshold); localLog(string_colonSpace); localLogLn(logFlushThreshold);
  localLog(string_logFlushInterval); localLog(string_colonSpace); localLogLn(logFlushInterval);
  #if defined(SUPPORT_HACKING)
    localLog(string_gameLength); localLog(string_colonSpace);
    localLogLn(gameLength);
    localLog(string_gameRetries); localLog(string_colonSpace);
    localLogLn(gameRetries);
    localLog(string_gameSpeedup); localLog(string_colonSpace);
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
  if((featureFlags & 0x02) == 0x02)
  {
    features += " Lasertag sensor";
  }
  if((featureFlags & 0x04) == 0x04)
  {
    features += " Lasertag emitter";
  }
  if((featureFlags & 0x08) == 0x08)
  {
    features += " Time-of-flight beacon";
  }
  if((featureFlags & 0x80) == 0x80)
  {
    features += " Usually static";
  }
  return features;
}

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
    configuration["enableCaptivePortal"] = enableCaptivePortal;
    configuration["wiFiClientInactivityTimer"]  = wiFiClientInactivityTimer;
    configuration["wifiClientTimeout"]  = wifiClientTimeout;
  #endif
  #ifdef ENABLE_LOCAL_WEBSERVER
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      configuration["http_user"] = http_user;
      configuration["basicAuthEnabled"] = basicAuthEnabled;
    #endif
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH) || defined(ENABLE_OTA_UPDATE)
      configuration["http_password"] = http_password;
    #endif
  #endif
  #ifdef SUPPORT_HACKING
    configuration["gameLength"] = gameLength;
    configuration["gameRetries"] = gameRetries;
    configuration["gameSpeedup"] = gameSpeedup;
  #endif
  configuration["timeServer"] = timeServer;
  configuration["timeZone"] = timeZone;
  #ifdef SUPPORT_GPS
    configuration["useGpsForTimeSync"] = useGpsForTimeSync;
  #endif
  #ifdef SUPPORT_BATTERY_METER
    configuration["enableBatteryMonitor"] = enableBatteryMonitor;
    configuration["topLadderResistor"] = topLadderResistor;
    configuration["bottomLadderResistor"] = bottomLadderResistor;
  #endif
  #ifdef SUPPORT_BEEPER
    configuration["beeperEnabled"] = beeperEnabled;
    #ifdef SUPPORT_BUTTON
      configuration["beepOnPress"] = beepOnPress;
    #endif
  #endif
  #ifdef SUPPORT_VIBRATION
    configuration["vibrationEnabled"] = vibrationEnabled;
    configuration["vibrationLevel"] = vibrationLevel;
  #endif
  #if defined(ACT_AS_TRACKER)
    configuration["maximumEffectiveRange"] = maximumEffectiveRange;
  #endif
  #if defined(SUPPORT_LORA)
    configuration["loRaEnabled"] = loRaEnabled;
    configuration["deviceInfoSendInterval"] = deviceInfoSendInterval;
    configuration["defaultLocationSendInterval"] = defaultLocationSendInterval;
    configuration["locationSendInterval1"] = locationSendInterval1;
    configuration["loRaPerimiter1"] = loRaPerimiter1;
    configuration["locationSendInterval2"] = locationSendInterval2;
    configuration["loRaPerimiter2"] = loRaPerimiter2;
    configuration["locationSendInterval3"] = locationSendInterval3;
    configuration["loRaPerimiter3"] = loRaPerimiter3;
    configuration["rssiAttenuationPerimeter"] = rssiAttenuationPerimeter;
    configuration["rssiAttenuation"] = rssiAttenuation;
    configuration["rssiAttenuationBaseline"] = rssiAttenuationBaseline;
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
    #ifdef SUPPORT_GPS
      useGpsForTimeSync = configuration["useGpsForTimeSync"] | true;
    #endif
    #if defined(ACT_AS_TRACKER)
      maximumEffectiveRange = configuration["maximumEffectiveRange"] | 99;
    #endif
    #if defined(SUPPORT_LORA)
      loRaEnabled = configuration["loRaEnabled"] | true;
      deviceInfoSendInterval = configuration["deviceInfoSendInterval"] | 60000;
      defaultLocationSendInterval = configuration["defaultLocationSendInterval"] | 60000;
      locationSendInterval1 = configuration["locationSendInterval1"] | 5000;
      loRaPerimiter1 = configuration["loRaPerimiter1"] | 20;
      locationSendInterval2 = configuration["locationSendInterval2"] | 15000;
      loRaPerimiter2 = configuration["loRaPerimiter2"] | 50;
      locationSendInterval3 = configuration["locationSendInterval3"] | 30000;
      loRaPerimiter3 = configuration["loRaPerimiter3"] | 100;
      rssiAttenuationPerimeter = configuration["rssiAttenuationPerimeter"];
      rssiAttenuation = configuration["rssiAttenuation"];
      rssiAttenuationBaseline = configuration["rssiAttenuationBaseline"];
    #endif
    #ifdef SUPPORT_BATTERY_METER
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
      sprintf_P(device[0].name, PSTR("%s%02X%02X"), default_deviceName, device[0].id[4], device[0].id[5]);  //Add some hex from the MAC address on the end
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
      if(configuration["SSID"])
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
      if(configuration["APSSID"])
      {
        APSSID = new char[strlen(configuration["APSSID"]) + 1];
        strlcpy(APSSID,configuration["APSSID"],strlen(configuration["APSSID"]) + 1);
      }
      else
      {
        APSSID = new char[strlen(default_deviceName) + 5];
        sprintf_P(APSSID, PSTR("%s%02X%02X"), default_deviceName, device[0].id[4], device[0].id[5]);  //Add some hex from the MAC address on the end
      }
      if(configuration["APPSK"])
      {
        APPSK = new char[strlen(configuration["APPSK"]) + 1];
        strlcpy(APPSK,configuration["APPSK"],strlen(configuration["APPSK"]) + 1);
      }
      else
      {
        APPSK = new char[strlen(default_AP_PSK) + 1];
        strlcpy(APPSK,default_AP_PSK,strlen(default_AP_PSK) + 1);
      }
      wiFiClientInactivityTimer = configuration["wiFiClientInactivityTimer"] | 0;
      wifiClientTimeout = configuration["wifiClientTimeout"] | 30;
    #endif
    #ifdef ENABLE_LOCAL_WEBSERVER
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
    #ifdef SUPPORT_BEEPER
      beeperEnabled = configuration["beeperEnabled"] | true;
      #ifdef SUPPORT_BUTTON
        beepOnPress = configuration["beepOnPress"] | true;
      #endif
    #endif
    #ifdef SUPPORT_VIBRATION
      vibrationEnabled = configuration["vibrationEnabled"] | true;
      vibrationLevel = configuration["vibrationLevel"] | 100;
    #endif
    #ifdef SUPPORT_HACKING
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

bool loadDefaultConfiguration()
{
  localLogLn(F("Loading default configuration"));
  device[0].name = new char[strlen(default_deviceName) + 5];
  sprintf_P(device[0].name, PSTR("%s%02X%02X"), default_deviceName, device[0].id[4], device[0].id[5]);  //Add some hex from the MAC address on the end
  #ifdef ENABLE_LOCAL_WEBSERVER
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
    localLog(F("wifiClientTimeout: "));
    localLogLn(wifiClientTimeout);
    localLog(F("wiFiClientInactivityTimer: "));
    localLogLn(wiFiClientInactivityTimer);
    localLog(F("startWiFiApOnBoot: "));
    if(startWiFiApOnBoot)
    {
      localLogLn(F("enabled"));
    }
    else
    {
      localLogLn(F("disabled"));
    }
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
    localLog(F("enableCaptivePortal: "));
    if(enableCaptivePortal)
    {
      localLogLn(F("enabled"));
    }
    else
    {
      localLogLn(F("disabled"));
    }
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
  #ifdef ENABLE_LOCAL_WEBSERVER
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
    localLog(F("otaEnabled: "));
    localLogLn(otaEnabled);
    localLog(F("otaAuthenticationEnabled: "));
    localLogLn(otaAuthenticationEnabled);
  #endif
  #ifdef SUPPORT_GPS
    localLog(F("useGpsForTimeSync: "));
    if(useGpsForTimeSync)
    {
      localLogLn(F("enabled"));
    }
    else
    {
      localLogLn(F("disabled"));
    }
  #endif
  #if defined(ACT_AS_TRACKER)
    localLog(F("maximumEffectiveRange: "));
    localLogLn(maximumEffectiveRange);
  #endif
  #if defined(SUPPORT_LORA)
    localLog(F("loRaEnabled: "));
    localLogLn(loRaEnabled);
    localLog(F("deviceInfoSendInterval: "));
    localLogLn(deviceInfoSendInterval);
    localLog(F("defaultLocationSendInterval: "));
    localLogLn(defaultLocationSendInterval);
    localLog(F("loRaPerimiter1: "));
    localLogLn(loRaPerimiter1);
    localLog(F("locationSendInterval1: "));
    localLogLn(locationSendInterval1);
    localLog(F("loRaPerimiter2: "));
    localLogLn(loRaPerimiter2);
    localLog(F("locationSendInterval2: "));
    localLogLn(locationSendInterval2);
    localLog(F("loRaPerimiter3: "));
    localLogLn(loRaPerimiter3);
    localLog(F("locationSendInterval3: "));
    localLogLn(locationSendInterval3);
    localLog(F("rssiAttenuation: "));
    localLogLn(rssiAttenuation);
    localLog(F("rssiAttenuationBaseline: "));
    localLogLn(rssiAttenuationBaseline);
    localLog(F("rssiAttenuationPerimeter: "));
    localLogLn(rssiAttenuationPerimeter);
  #endif
  #ifdef SUPPORT_BATTERY_METER
    localLog(F("enableBatteryMonitor: "));
    localLogLn(enableBatteryMonitor);
    localLog(F("topLadderResistor: "));
    localLogLn(topLadderResistor);
    localLog(F("bottomLadderResistor: "));
    localLogLn(bottomLadderResistor);
  #endif
  #ifdef SUPPORT_BEEPER
    localLog(F("beeperEnabled: "));
    localLogLn(beeperEnabled);
    #ifdef SUPPORT_BUTTON
      localLog(F("beepOnPress: "));
      localLogLn(beepOnPress);
    #endif
  #endif
  #ifdef SUPPORT_VIBRATION
    localLog(F("vibrationEnabled: "));
    localLogLn(vibrationEnabled);
    localLog(F("vibrationLevel: "));
    localLogLn(vibrationLevel);
  #endif
  localLog(F("loggingBufferSize: "));
  localLogLn(loggingBufferSize);
  localLog(F("logFlushThreshold: "));
  localLogLn(logFlushThreshold);
  localLog(F("logFlushInterval: "));
  localLogLn(logFlushInterval);
  #ifdef SUPPORT_HACKING
    localLog(F("gameLength: "));
    localLogLn(gameLength);
    localLog(F("gameRetries: "));
    localLogLn(gameRetries);
    localLog(F("gameSpeedup: "));
    localLogLn(gameSpeedup);
  #endif
  localLogLn(F("========================="));
  #ifdef ACT_AS_SENSOR
  showSensorConfiguration();
  #endif
}

String deviceFeatures(uint8_t featureFlags)
{
  String features;
  if((featureFlags & 0x01) == 0)
  {
    features = "Location beacon";
  }
  else
  {
    features = "Beacon tracker";
  }
  if((featureFlags & 0x02) == 2)
  {
    features += " Lasertag sensor";
  }
  if((featureFlags & 0x04) == 4)
  {
    features += ", Lasertag emitter";
  }
  return features;
}

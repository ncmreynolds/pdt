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
  configuration["nodeName"] = nodeName;
  configuration["configurationComment"] = configurationComment;
  #if defined(SUPPORT_WIFI)
    configuration["SSID"] = SSID;
    configuration["PSK"] = PSK;
    configuration["startWiFiOnBoot"] = startWiFiOnBoot;
    configuration["wiFiInactivityTimer"]  = wiFiInactivityTimer;
  #endif
  #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
    configuration["http_user"] = http_user;
    configuration["basicAuthEnabled"] = basicAuthEnabled;
  #endif
  #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH) || defined(ENABLE_OTA_UPDATE)
    configuration["http_password"] = http_password;
  #endif
  configuration["timeServer"] = timeServer;
  configuration["timeZone"] = timeZone;
  #ifdef SUPPORT_GPS
    configuration["useGpsForTimeSync"] = useGpsForTimeSync;
  #endif
  #ifdef SUPPORT_BEEPER
    configuration["beeperEnabled"] = beeperEnabled;
  #endif
  #if defined(ACT_AS_TRACKER)
    configuration["maximumEffectiveRange"] = maximumEffectiveRange;
  #endif
  #if defined(SUPPORT_LORA)
    configuration["beaconInterval1"] = beaconInterval1;
    configuration["loRaPerimiter1"] = loRaPerimiter1;
    configuration["beaconInterval2"] = beaconInterval2;
    configuration["loRaPerimiter2"] = loRaPerimiter2;
    configuration["beaconInterval3"] = beaconInterval3;
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
      beaconInterval1 = configuration["beaconInterval1"] | 5000;
      loRaPerimiter1 = configuration["loRaPerimiter1"] | 20;
      beaconInterval2 = configuration["beaconInterval2"] | 5000;
      loRaPerimiter2 = configuration["loRaPerimiter2"] | 20;
      beaconInterval3 = configuration["beaconInterval3"] | 5000;
      loRaPerimiter3 = configuration["loRaPerimiter3"] | 20;
      rssiAttenuationPerimeter = configuration["rssiAttenuationPerimeter"];
      rssiAttenuation = configuration["rssiAttenuation"];
      rssiAttenuationBaseline = configuration["rssiAttenuationBaseline"];
    #endif
    loggingBufferSize = configuration["loggingBufferSize"] | 2048;
    logFlushThreshold = configuration["logFlushThreshold"] | 2000;
    logFlushInterval = configuration["logFlushInterval"] | 57600;
    if(configuration["nodeName"])
    {
      nodeName = new char[strlen(configuration["nodeName"]) + 1];
      strlcpy(nodeName,configuration["nodeName"],strlen(configuration["nodeName"]) + 1);
    }
    else
    {
      nodeName = new char[strlen(default_nodeName) + 1];
      strlcpy(nodeName,default_nodeName,strlen(default_nodeName) + 1);
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
      startWiFiOnBoot = configuration["startWiFiOnBoot"] | true;
      wiFiInactivityTimer = configuration["wiFiInactivityTimer"] | 0;
    #endif
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
  nodeName = new char[strlen(default_nodeName) + 1];  //Assign space on heap
  strlcpy(nodeName,default_nodeName,strlen(default_nodeName) + 1);  //Copy in default
  #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
    http_user = new char[strlen(default_http_user) + 1];
    strlcpy(http_user,default_http_user,strlen(default_http_user) + 1);
  #endif
  #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH) || defined(ENABLE_OTA_UPDATE)
    http_password = new char[strlen(default_http_password) + 1];
    strlcpy(http_password,default_http_password,strlen(default_http_password) + 1);
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
  localLog(F("nodeName: "));
  if(nodeName != nullptr)
  {
    localLogLn(nodeName);
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
    localLog(F("startWiFiOnBoot: "));
    if(startWiFiOnBoot)
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
    localLog(F("wiFiInactivityTimer: "));
    localLogLn(wiFiInactivityTimer);
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
    localLog(F("loRaPerimiter1: "));
    localLogLn(loRaPerimiter1);
    localLog(F("beaconInterval1: "));
    localLogLn(beaconInterval1);
    localLog(F("loRaPerimiter2: "));
    localLogLn(loRaPerimiter2);
    localLog(F("beaconInterval2: "));
    localLogLn(beaconInterval2);
    localLog(F("loRaPerimiter3: "));
    localLogLn(loRaPerimiter3);
    localLog(F("beaconInterval3: "));
    localLogLn(beaconInterval3);
    localLog(F("rssiAttenuation: "));
    localLogLn(rssiAttenuation);
    localLog(F("rssiAttenuationBaseline: "));
    localLogLn(rssiAttenuationBaseline);
    localLog(F("rssiAttenuationPerimeter: "));
    localLogLn(rssiAttenuationPerimeter);
  #endif
  #ifdef SUPPORT_BEEPER
    localLog(F("beeperEnabled: "));
    localLogLn(beeperEnabled);
  #endif
  localLog(F("loggingBufferSize: "));
  localLogLn(loggingBufferSize);
  localLog(F("logFlushThreshold: "));
  localLogLn(logFlushThreshold);
  localLog(F("logFlushInterval: "));
  localLogLn(logFlushInterval);
  localLogLn(F("========================="));
}

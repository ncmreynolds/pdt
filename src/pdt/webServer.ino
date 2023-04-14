/*
 * 
 * This file contains functions related to the local web server used for configuration, control and log access
 * 
 */
#if defined(ENABLE_LOCAL_WEBSERVER)
void addPageHeader(AsyncResponseStream *response, uint8_t refresh, const char* refreshTo)
{
  if(nodeName != nullptr)
  {
    response->printf_P(PSTR("<!DOCTYPE html><html><head><title>%s</title>"), nodeName);
  }
  else
  {
    response->print(F("<!DOCTYPE html><html><head>"));
  }
  if(refresh > 0)
  {
    if(refreshTo != nullptr)
    {
      response->printf_P(PSTR("<meta http-equiv=\"refresh\" http-equiv=\"refresh\" content=\"%u; url=%s\">"), refresh, refreshTo);
    }
    else
    {
      response->printf_P(PSTR("<meta http-equiv=\"refresh\" content=\"%u\">"), refresh);
    }
  }
  response->print(F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"));
  response->print(F("<link href=\"//fonts.googleapis.com/css?family=Raleway:400,300,600\" rel=\"stylesheet\" type=\"text/css\">"));
  response->print(F("<link rel=\"stylesheet\" href=\"/css/normalize.css\">"));
  response->print(F("<link rel=\"stylesheet\" href=\"/css/skeleton.css\">"));
  response->print(F("</head><body><div class=\"container\">"));
  if(nodeName != nullptr)
  {
    response->printf_P(PSTR("<h1>%s</h1>"),nodeName);
  }
}
void addPageFooter(AsyncResponseStream *response)
{
  response->print(F("</div></body></html>"));
}
void setupWebServer()
{
  localLog(F("Configuring web server callbacks: "));
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
    lastWifiActivity = millis();
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
      {
          return request->requestAuthentication();  //Force basic authentication
      }
    #endif
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    addPageHeader(response, 90, nullptr);
    response->print(F("<h2>General</h2>"));
    response->print(F("<ul>"));
    #if defined(ACT_AS_TRACKER)
      response->printf_P(PSTR("<li>PDT tracker firmware: %s %u.%u.%u</li>"), __BASE_FILE__, majorVersion, minorVersion, patchVersion);
    #elif defined(ACT_AS_BEACON)
      response->printf_P(PSTR("<li>PDT beacon firmware: %s %u.%u.%u</li>"), __BASE_FILE__, majorVersion, minorVersion, patchVersion);
    #endif
    response->printf_P(PSTR("<li>Built: %s %s</li>"), __TIME__, __DATE__);
    response->printf_P(PSTR("<li>Board: %s</li>"), ARDUINO_BOARD);
    #ifdef ESP_IDF_VERSION_MAJOR
      response->print(F("<li>ESP-IDF: v"));
      #ifdef ESP_IDF_VERSION_MINOR
        response->print(ESP_IDF_VERSION_MAJOR);
        response->print('.');
        response->print(ESP_IDF_VERSION_MINOR);
      #else
        response->print(ESP_IDF_VERSION_MAJOR);
      #endif
    #endif
    response->print(F("</li>"));
    if(filesystemMounted == true)
    {
      #if defined(USE_SPIFFS)
        FSInfo fsInfo;
        SPIFFS.info(fsInfo);
        response->printf_P(PSTR("<li>Filesystem: SPIFFS %u/%uKB used"), fsInfo.usedBytes/1024, fsInfo.totalBytes/1024);
        if(fsInfo.usedBytes > 0 && fsInfo.totalBytes > 0)
        {
          response->printf_P(PSTR(" %.1f%%</li>"),float(fsInfo.usedBytes) * 100/float(fsInfo.totalBytes));
        }
        else
        {
          response->print(F("</li>"));
        }
      #elif defined(USE_LITTLEFS)
        #if defined(ESP32)
          response->printf_P(PSTR("<li>Filesystem: LittleFS %u/%uKB used"), LittleFS.usedBytes()/1024, LittleFS.totalBytes()/1024);
          if(LittleFS.usedBytes() > 0 && LittleFS.totalBytes() > 0)
          {
            response->printf_P(PSTR(" %.1f%%</li>"),float(LittleFS.usedBytes()) * 100/float(LittleFS.totalBytes()));
          }
          else
          {
            response->print(F("</li>"));
          }
        #endif
      #endif
    }
    else
    {
      response->print(F("<li>Filesystem: not mounted</li>"));
    }
    //#if defined(ESP32)
      response->print(F("<li>Free heap: "));
      response->print(ESP.getFreeHeap()/1024);
      response->print(F("KB</li>"));
    //#endif
    #if defined(USE_WIEGAND_RFID_READER)
      response->print(F("<li>Wiegand RFID reader: "));
      if(wiegandRfidReaderConnected == true)
      {
        response->print(F("initialised</li>"));
      }
      else
      {
        response->print(F("failed</li>"));
      }
    #endif
    response->print(F("<li>USB serial logging: "));
    #if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
      if(debugPortAvailable)
      {
        response->print(F("enabled - connected</li>"));
      }
      else
      {
        response->print(F("enabled - disconnected</li>"));
      }
    #else
      response->print(F("disabled</li>"));
    #endif
    response->print(F("<li>Over-the-air software update (Arduino IDE): "));
    #if defined(ENABLE_OTA_UPDATE)
      response->print(F("enabled</li>"));
    #else
      response->print(F("disabled</li>"));
    #endif
    response->print(F("<li>Web UI software update: "));
    #if defined(ENABLE_LOCAL_WEBSERVER_FIRMWARE_UPDATE)
      response->print(F("enabled</li>"));
    #else
      response->print(F("disabled</li>"));
    #endif
    response->print(F("<li>Time: "));
    if(timeIsValid())
    {
      updateTimestamp();
      response->print(timestamp);
    }
    else
    {
      response->print(F("Not set"));
    }
    response->print(F("</li>"));
    response->print(F("<li>Uptime: "));
    response->print(printableUptime(millis()/1000));
    response->print(F("</li>"));
    #if defined(ESP32)
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          response->print(F("Restart reason core 0: "));
          response->print(es32ResetReason(0));
          response->print(F("Restart reason core 1: "));
          response->print(es32ResetReason(1));
        #elif CONFIG_IDF_TARGET_ESP32S2
          response->print(F("<li>Restart reason: "));
          response->print(es32ResetReason(0));
        #elif CONFIG_IDF_TARGET_ESP32C3
          response->print(F("<li>Restart reason: "));
          response->print(es32ResetReason(0));
        #else 
          #error Target CONFIG_IDF_TARGET is not supported
        #endif
      #else // ESP32 Before IDF 4.0
        response->print(F("Restart reason core 0: "));
        response->print(es32ResetReason(0));
        response->print(F("Restart reason core 1: "));
        response->print(es32ResetReason(1));
      #endif
    #endif
    response->print(F("</li>"));
    #if defined(SUPPORT_WIFI)
      if(wiFiInactivityTimer > 0)
      {
        response->print(F("<li>WiFi disconnecting after : "));
        if(wiFiInactivityTimer == 60000)
        {
          response->print(F("1 minute"));
        }
        else if(wiFiInactivityTimer == 180000)
        {
          response->print(F("3 minutes"));
        }
        else if(wiFiInactivityTimer == 300000)
        {
          response->print(F("f minutes"));
        }
        response->print(F(" inactivity</li>"));
      }
    #endif
    if(configurationComment != nullptr)
    {
      response->print(F("<li>Configuration comment: "));
      response->print(configurationComment);
      response->print(F("</li>"));
    }
    #if defined(SUPPORT_LORA)
    response->print(F("<li>LoRa radio: "));
    if(loRaConnected == true)
    {
      response->print(loRaRxPackets);
      response->print(F(" RX / "));
      response->print(loRaTxPackets);
      response->print(F(" TX"));
    }
    else
    {
      response->print(F("not connected"));
    }
    response->print(F("</li>"));
    #endif
    #ifdef SUPPORT_BATTERY_METER
      if(batteryVoltage > chargingVoltage)
      {
        response->print(F("<li>Battery voltage: USB power ("));
        response->print(batteryVoltage);
        response->print(F("v)</li>"));
      }
      else
      {
        response->print(F("<li>Battery voltage: "));
        response->print(batteryVoltage);
        response->print(F("v ("));
        response->print(batteryPercentage);
        response->print(F("% charge)</li>"));
      }
    #endif
    #ifdef SUPPORT_GPS
      #if defined(ACT_AS_TRACKER)
        if(tracker[0].hasFix)
      #else
        if(beacon[0].hasFix)
      #endif
      {
        
        response->print(F("<li>Latitude: "));
        #if defined(ACT_AS_TRACKER)
          response->print(tracker[0].latitude);
        #else
          response->print(beacon[0].latitude);
        #endif
        response->print(F("</li>"));
        response->print(F("<li>Longitude: "));
        #if defined(ACT_AS_TRACKER)
          response->print(tracker[0].longitude);
        #else
          response->print(beacon[0].longitude);
        #endif
        response->print(F("</li>"));
        /*
        response->print(F("<li>Altitude: "));
        response->print(gps.altitude.meters());
        response->print(F("m</li>"));
        if(gps.sentencesWithFix() > 0 || gps.failedChecksum() > 0)
        {
          response->print(F("<li>GPS errors: "));
          response->print(100.0*float(gps.failedChecksum())/float(gps.sentencesWithFix() + gps.failedChecksum()));
          response->print(F("%</li>"));
        }
        */
        response->print(F("<li>HDOP: "));
        #if defined(ACT_AS_TRACKER)
          response->print(tracker[0].hdop);
          if(tracker[0].hdop < 1)
        #else
          response->print(beacon[0].hdop);
          if(beacon[0].hdop < 1)
        #endif
        {
          response->print(F("(excellent)</li>"));
        }
        #if defined(ACT_AS_TRACKER)
          else if(tracker[0].hdop < 2)
        #else
          else if(beacon[0].hdop < 2)
        #endif
        {
          response->print(F("(good)</li>"));
        }
        #if defined(ACT_AS_TRACKER)
          else if(tracker[0].hdop < 3)
        #else
          else if(beacon[0].hdop < 3)
        #endif
        {
          response->print(F("(normal)</li>"));
        }
        #if defined(ACT_AS_TRACKER)
          else if(tracker[0].hdop < 4)
        #else
          else if(beacon[0].hdop < 4)
        #endif
        {
          response->print(F("(poor)</li>"));
        }
        else
        {
          response->print(F("(inaccurate)</li>"));
        }
        #if defined(ACT_AS_TRACKER)
          response->print(F("<li>Distance to beacon: "));
          if(numberOfBeacons > 0)
          {
            if(beacon[currentBeacon].hasFix)
            {
              response->print(beacon[currentBeacon].distanceTo);
              response->print(F("m</li>"));
              response->print(F("<li>Course to beacon: "));
              response->print(beacon[currentBeacon].courseTo);
            }
            else
            {
              response->print(F("Unknown"));
            }
          }
          else
          {
            response->print(F("no beacons"));
          }
        #elif defined(ACT_AS_BEACON)
          response->print(F("<li>Distance to tracker: "));
          if(tracker[0].hasFix)
          {
            response->print(tracker[0].distanceTo);
            response->print(F("m</li>"));
            response->print(F("<li>Course to tracker: "));
            response->print(tracker[0].courseTo);
          }
          else
          {
            response->print(F("Unknown"));
          }
        #endif
        response->print(F("</li>"));
      }
      else
      {
        response->print(F("<li>GPS: No fix</li>"));
      }
    #endif
    response->print(F("</ul>"));
    response->print(F("<a href =\"/listLogs\"><input class=\"button-primary\" type=\"button\" value=\"Logs\"></a> "));
    response->print(F("<a href =\"/configuration\"><input class=\"button-primary\" type=\"button\" value=\"Configuration\"></a> "));
    #if defined(ENABLE_LOCAL_WEBSERVER_FIRMWARE_UPDATE)
      response->print(F("<a href =\"/update\"><input class=\"button-primary\" type=\"button\" value=\"Software Update\"></a> "));
    #endif
    #if defined(ENABLE_REMOTE_RESTART)
      response->print(F("<a href =\"/restart\"><input class=\"button-primary\" type=\"button\" value=\"Restart\"></a> "));
    #endif
    addPageFooter(response);
    request->send(response);
  });
  webServer.on("/listLogs", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows a list of all the log files
    lastWifiActivity = millis();
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
      {
          return request->requestAuthentication();  //Force basic authentication
      }
    #endif
    if(loggingBuffer.length() > 0)  //Flush the log so it can be shown in full to a remote viewer if necessary
    {
      flushLogNow = true; //Force a log flush next time round the loop, so the current log is fresh
    }
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    addPageHeader(response, 0, nullptr);
    response->print(F("<h2>Log files</h2>"));
    response->print(F("<table class=\"u-full-width\"><thead><tr><th>File</th><th>Size</th><th colspan=\"2\"><a href =\"/\"><input class=\"button-primary\" type=\"button\" value=\"Back\"></a></th></tr></thead><tbody>"));
    //response->print(F("<a href =\"/\"><input class=\"button-primary\" type=\"button\" value=\"Back\"></a> "));
    #if defined(USE_SPIFFS)
      Dir dir = SPIFFS.openDir(logDirectory);
      while (dir.next ())
      {
        if(String(dir.fileName()).startsWith("log"))
        {
          #if defined(ENABLE_LOG_DELETION)
            response->printf_P(PSTR("<tr><td>%s</td><td>%.1fKB</td><td>[<a href=\"/logs/%s\">View</a>]</td><td>[<a href=\"/deleteLog?file=%s\">Delete</a>]</td></tr>"), dir.fileName(), float(dir.fileSize())/1024, dir.fileName(), dir.fileName());
          #else
            response->printf_P(PSTR("<tr><td>%s</td><td>%.1fKB</td><td>[<a href=\"/logs/%s\">View</a>]</td></tr>"), dir.fileName(), float(dir.fileSize())/1024, dir.fileName());
          #endif
        }
      }
    #elif defined(USE_LITTLEFS)
      #if defined(ESP32)
        File dir = LittleFS.open(logDirectory);
      #endif
      File file = dir.openNextFile();
      while(file)
      {
        if(!file.isDirectory())
        {
          if(String(file.name()).startsWith("log"))
          {
            #if defined(ENABLE_LOG_DELETION)
              response->printf_P(PSTR("<tr><td>%s</td><td>%.1fKB</td><td><a href=\"/logs/%s\"><input class=\"button-primary\" type=\"button\" value=\"View\"></a></td><td><a href=\"/deleteLog?file=%s\"><input class=\"button-primary\" type=\"button\" value=\"Delete\"></a></td></tr>"), file.name(), float(file.size())/1024, file.name(), file.name());
            #else
              response->printf_P(PSTR("<tr><td>%s</td><td>%.1fKB</td><td><a href=\"/logs/%s\"><input class=\"button-primary\" type=\"button\" value=\"View\"></a></td></tr>"), file.name(), float(file.size())/1024, file.name());
            #endif
          }
        }
        file = dir.openNextFile();
      }
    #endif
    response->print(F("</tbody></table>"));
    addPageFooter(response);
    request->send(response);
  });
  #if defined(ENABLE_LOG_DELETION)
    webServer.on("/deleteLog", HTTP_GET, [](AsyncWebServerRequest *request){
      lastWifiActivity = millis();
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
        if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
        {
            return request->requestAuthentication();  //Force basic authentication
        }
      #endif
      if(request->hasParam("file"))
      {
        AsyncWebParameter* file = request->getParam("file");
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        addPageHeader(response, 0, nullptr);
        response->print(F("<h2>Delete log file confirmation</h2>"));
        response->printf_P(PSTR("<p>Are you sure you want to delete the log file %s?</p>"),file->value().c_str());
        response->printf_P(PSTR("<p><a href =\"/deleteLogConfirmed?file=%s\"><input class=\"button-primary\" type=\"button\" value=\"Yes\"></a> <a href=\"/\"><input class=\"button-primary\" type=\"button\" value=\"No\"></a></p>"),file->value().c_str());
        addPageFooter(response);
        request->send(response);
      }
      else
      {
        request->send(500, "text/plain", request->url() + " file not found");
      }
    });
    webServer.on("/deleteLogConfirmed", HTTP_GET, [](AsyncWebServerRequest *request){
      lastWifiActivity = millis();
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
        if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
        {
            return request->requestAuthentication();  //Force basic authentication
        }
      #endif
      if(request->hasParam("file"))
      {
        AsyncWebParameter* file = request->getParam("file");
        #if defined(USE_LITTLEFS)
          String fileToRemove = "/" + file->value(); //LittleFS insists on a leading /
        #endif
        localLog(F("Web UI deleting log file \""));
        localLog(file->value());
        localLog(F("\", requested by "));
        localLogLn(request->client()->remoteIP().toString());
        char fileToDelete[strlen(logDirectory) + strlen(file->value().c_str()) + 2];
        sprintf_P(fileToDelete, PSTR("%s/%s"), logDirectory, file->value().c_str());
        if(deleteFile(fileToDelete))
        {
            request->redirect("/");
        }
        else
        {
          request->send(500, "text/plain", request->url() + " file not found");
        }
      }
      else
      {
        request->send(500, "text/plain", request->url() + " file not found");
      }
    });
  #endif
  webServer.on("/configuration", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
    lastWifiActivity = millis();
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
      {
          return request->requestAuthentication();  //Force basic authentication
      }
    #endif
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    addPageHeader(response, 0, nullptr);
    response->print(F("<h2>Configuration</h2>"));
    response->print(F("<form method=\"POST\">"));
    response->printf_P(PSTR("<div class=\"row\"><div class=\"twelve columns\"><label for=\"nodeName\">Node name</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"nodeName\" name=\"nodeName\"></div></div>"), nodeName);
    response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Networking</h3></div></div>"));
    #if defined(SUPPORT_WIFI)
      response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>WiFi</h3></div></div>"));
      response->print(F("<div class=\"row\"><div class=\"three columns\"><label for=\"startWiFiOnBoot\">Enable WiFi on boot</label><select class=\"u-full-width\" id=\"startWiFiOnBoot\" name=\"startWiFiOnBoot\">"));
      response->print(F("<option value=\"true\""));response->print(startWiFiOnBoot == true ? " selected>":">");response->print(F("Enabled</option>"));
      response->print(F("<option value=\"false\""));response->print(startWiFiOnBoot == false ? " selected>":">");response->print(F("Disabled</option>"));
      response->print(F("</select></div>"));
      response->print(F("<div class=\"three columns\"><label for=\"wiFiInactivityTimer\">WiFi inactivity timer</label><select class=\"u-full-width\" id=\"wiFiInactivityTimer\" name=\"wiFiInactivityTimer\">"));
      response->print(F("<option value=\"0\""));response->print(wiFiInactivityTimer == 0 ? " selected>":">");response->print(F("Never</option>"));
      response->print(F("<option value=\"60000\""));response->print(wiFiInactivityTimer == 60000 ? " selected>":">");response->print(F("1m</option>"));
      response->print(F("<option value=\"180000\""));response->print(wiFiInactivityTimer == 180000 ? " selected>":">");response->print(F("3m</option>"));
      response->print(F("<option value=\"300000\""));response->print(wiFiInactivityTimer == 300000 ? " selected>":">");response->print(F("5m</option>"));
      response->print(F("</select></div>"));
      response->printf_P(PSTR("<div class=\"three columns\"><label for=\"SSID\">WiFi SSID</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"SSID\" name=\"SSID\"></div>"), SSID);
      response->print(F("<div class=\"three columns\"><label for=\"PSK\">WiFi PSK</label><input class=\"u-full-width\" type=\"password\" placeholder=\"********\" id=\"PSK\" name=\"PSK\"></div></div>"));
    #endif
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      response->print(F("<div class=\"row\"><div class=\"four columns\"><label for=\"basicAuthEnabled\">Web UI login</label><select class=\"u-full-width\" id=\"basicAuthEnabled\" name=\"basicAuthEnabled\">"));
      response->print(F("<option value=\"true\""));response->print(basicAuthEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
      response->print(F("<option value=\"false\""));response->print(basicAuthEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
      response->print(F("</select></div>"));
      response->printf_P(PSTR("<div class=\"four columns\"><label for=\"http_user\">Web UI username</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"http_user\" name=\"http_user\"></div>"), http_user);
      response->print(F("<div class=\"four columns\"><label for=\"http_password\">Web UI/OTA password</label><input class=\"u-full-width\" type=\"password\" placeholder=\"********\" id=\"http_password\" name=\"http_password\"></div></div>"));
    #endif
    #if defined(SUPPORT_WIFI)
      //Time Server
      response->printf_P(PSTR("<div class=\"row\"><div class=\"six columns\"><label for=\"timeServer\">NTP host</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"timeServer\" name=\"timeServer\"></div>"), timeServer);
      response->printf_P(PSTR("<div class=\"six columns\"><label for=\"\">Timezone</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"timeZone\" name=\"timeZone\"></div></div>"), timeZone);
    #endif
    #if defined(ENABLE_OTA_UPDATE)
      response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Software Update</h3></div></div>"));
      response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"otaEnabled\">OTA enabled</label><select class=\"u-full-width\" id=\"otaEnabled\" name=\"otaEnabled\">"));
      response->print(F("<option value=\"true\""));response->print(otaEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
      response->print(F("<option value=\"false\""));response->print(otaEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
      response->print(F("</select></div>"));
      response->print(F("<div class=\"six columns\"><label for=\"otaAuthenticationEnabled\">OTA password enabled</label><select class=\"u-full-width\" id=\"otaAuthenticationEnabled\" name=\"otaAuthenticationEnabled\">"));
      response->print(F("<option value=\"true\""));response->print(otaAuthenticationEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
      response->print(F("<option value=\"false\""));response->print(otaAuthenticationEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
      response->print(F("</select></div>"));
    #endif
    //Logging
    response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Logging</h3></div></div>"));
    response->printf_P(PSTR("<div class=\"row\"><div class=\"four columns\"><label for=\"loggingBufferSize\">Logging buffer size</label><input class=\"u-full-width\" type=\"number\" value=\"%u\" id=\"loggingBufferSize\" name=\"loggingBufferSize\"></div>"), loggingBufferSize);
    response->printf_P(PSTR("<div class=\"four columns\"><label for=\"logFlushThreshold\">Logging buffer flush threshold</label><input class=\"u-full-width\" type=\"text\" value=\"%u\" id=\"logFlushThreshold\" name=\"logFlushThreshold\"></div>"), logFlushThreshold);
    response->print(F("<div class=\"four columns\"><label for=\"logFlushInterval\">Logging buffer flush frequency</label><select class=\"u-full-width\" id=\"logFlushInterval\" name=\"logFlushInterval\">"));
    response->print(F("<option value=\"7200\""));response->print(logFlushInterval == 7200 ? " selected>":">");response->print(F("2 hours</option>"));
    response->print(F("<option value=\"14400\""));response->print(logFlushInterval == 14400 ? " selected>":">");response->print(F("4 hours</option>"));
    response->print(F("<option value=\"28800\""));response->print(logFlushInterval == 28800 ? " selected>":">");response->print(F("8 hours</option>"));
    response->print(F("<option value=\"57600\""));response->print(logFlushInterval == 57600 ? " selected>":">");response->print(F("16 hours</option>"));
    response->print(F("<option value=\"86400\""));response->print(logFlushInterval == 86400 ? " selected>":">");response->print(F("24 hours</option>"));
    response->print(F("</select></div></div>"));
    //GPS
    #ifdef SUPPORT_GPS
      response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>GPS</h3></div></div>"));
      response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"useGpsForTimeSync\">Sync time with GPS</label><select class=\"u-full-width\" id=\"useGpsForTimeSync\" name=\"useGpsForTimeSync\">"));
      response->print(F("<option value=\"true\""));response->print(useGpsForTimeSync == true ? " selected>":">");response->print(F("Enabled</option>"));
      response->print(F("<option value=\"false\""));response->print(useGpsForTimeSync == false ? " selected>":">");response->print(F("Disabled</option>"));
      response->print(F("</select></div></div>"));
    #endif
    #if defined(ACT_AS_TRACKER)
      response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Tracker</h3></div></div>"));
      response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"maximumEffectiveRange\">Maximum effective range</label><select class=\"u-full-width\" id=\"maximumEffectiveRange\" name=\"maximumEffectiveRange\">"));
      response->print(F("<option value=\"50\""));response->print(maximumEffectiveRange == 50 ? " selected>":">");response->print(F("50m</option>"));
      response->print(F("<option value=\"75\""));response->print(maximumEffectiveRange == 75 ? " selected>":">");response->print(F("75m</option>"));
      response->print(F("<option value=\"99\""));response->print(maximumEffectiveRange == 99 ? " selected>":">");response->print(F("99m</option>"));
      response->print(F("<option value=\"150\""));response->print(maximumEffectiveRange == 150 ? " selected>":">");response->print(F("150m</option>"));
      response->print(F("<option value=\"250\""));response->print(maximumEffectiveRange == 250 ? " selected>":">");response->print(F("250m</option>"));
      response->print(F("<option value=\"750\""));response->print(maximumEffectiveRange == 750 ? " selected>":">");response->print(F("750m</option>"));
      response->print(F("<option value=\"1000\""));response->print(maximumEffectiveRange == 1000 ? " selected>":">");response->print(F("1000m</option>"));
      response->print(F("<option value=\"9999\""));response->print(maximumEffectiveRange == 9999 ? " selected>":">");response->print(F("9999m</option>"));
      response->print(F("</select></div>"));
      response->print(F("<div class=\"six columns\"><label for=\"beeperEnabled\">Beeper</label><select class=\"u-full-width\" id=\"beeperEnabled\" name=\"beeperEnabled\">"));
      response->print(F("<option value=\"true\""));response->print(beeperEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
      response->print(F("<option value=\"false\""));response->print(beeperEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
      response->print(F("</select></div></div>"));
    #endif
    #if defined(SUPPORT_LORA)
      //LoRa
      response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>LoRa</h3></div></div>"));
      //LoRa beacon interval 1
      response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"loRaPerimiter1\">LoRa perimiter</label><select class=\"u-full-width\" id=\"loRaPerimiter1\" name=\"loRaPerimiter1\">"));
      response->print(F("<option value=\"10\""));response->print(loRaPerimiter1 == 10 ? " selected>":">");response->print(F("10m</option>"));
      response->print(F("<option value=\"15\""));response->print(loRaPerimiter1 == 15 ? " selected>":">");response->print(F("15m</option>"));
      response->print(F("<option value=\"20\""));response->print(loRaPerimiter1 == 20 ? " selected>":">");response->print(F("20m</option>"));
      response->print(F("<option value=\"25\""));response->print(loRaPerimiter1 == 25 ? " selected>":">");response->print(F("25m</option>"));
      response->print(F("<option value=\"30\""));response->print(loRaPerimiter1 == 30 ? " selected>":">");response->print(F("30m</option>"));
      response->print(F("</select></div>"));
      response->print(F("<div class=\"six columns\"><label for=\"beaconInterval1\">LoRa beacon interval</label><select class=\"u-full-width\" id=\"beaconInterval1\" name=\"beaconInterval1\">"));
      response->print(F("<option value=\"1000\""));response->print(beaconInterval1 == 1000 ? " selected>":">");response->print(F("1s</option>"));
      response->print(F("<option value=\"2000\""));response->print(beaconInterval1 == 2000 ? " selected>":">");response->print(F("2s</option>"));
      response->print(F("<option value=\"5000\""));response->print(beaconInterval1 == 5000 ? " selected>":">");response->print(F("5s</option>"));
      response->print(F("<option value=\"10000\""));response->print(beaconInterval1 == 10000 ? " selected>":">");response->print(F("10s</option>"));
      response->print(F("<option value=\"30000\""));response->print(beaconInterval1 == 30000 ? " selected>":">");response->print(F("30s</option>"));
      response->print(F("</select></div></div>"));
      //LoRa beacon interval 2
      response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"loRaPerimiter2\">LoRa perimiter</label><select class=\"u-full-width\" id=\"loRaPerimiter2\" name=\"loRaPerimiter2\">"));
      response->print(F("<option value=\"10\""));response->print(loRaPerimiter2 == 10 ? " selected>":">");response->print(F("10m</option>"));
      response->print(F("<option value=\"15\""));response->print(loRaPerimiter2 == 15 ? " selected>":">");response->print(F("15m</option>"));
      response->print(F("<option value=\"20\""));response->print(loRaPerimiter2 == 20 ? " selected>":">");response->print(F("20m</option>"));
      response->print(F("<option value=\"25\""));response->print(loRaPerimiter2 == 25 ? " selected>":">");response->print(F("25m</option>"));
      response->print(F("<option value=\"30\""));response->print(loRaPerimiter2 == 30 ? " selected>":">");response->print(F("30m</option>"));
      response->print(F("</select></div>"));
      response->print(F("<div class=\"six columns\"><label for=\"beaconInterval2\">LoRa beacon interval</label><select class=\"u-full-width\" id=\"beaconInterval2\" name=\"beaconInterval2\">"));
      response->print(F("<option value=\"1000\""));response->print(beaconInterval2 == 1000 ? " selected>":">");response->print(F("1s</option>"));
      response->print(F("<option value=\"2000\""));response->print(beaconInterval2 == 2000 ? " selected>":">");response->print(F("2s</option>"));
      response->print(F("<option value=\"5000\""));response->print(beaconInterval2 == 5000 ? " selected>":">");response->print(F("5s</option>"));
      response->print(F("<option value=\"10000\""));response->print(beaconInterval2 == 10000 ? " selected>":">");response->print(F("10s</option>"));
      response->print(F("<option value=\"30000\""));response->print(beaconInterval2 == 30000 ? " selected>":">");response->print(F("30s</option>"));
      response->print(F("</select></div></div>"));
      //LoRa beacon interval 3
      response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"loRaPerimiter3\">LoRa perimiter</label><select class=\"u-full-width\" id=\"loRaPerimiter3\" name=\"loRaPerimiter3\">"));
      response->print(F("<option value=\"10\""));response->print(loRaPerimiter3 == 10 ? " selected>":">");response->print(F("10m</option>"));
      response->print(F("<option value=\"15\""));response->print(loRaPerimiter3 == 15 ? " selected>":">");response->print(F("15m</option>"));
      response->print(F("<option value=\"20\""));response->print(loRaPerimiter3 == 20 ? " selected>":">");response->print(F("20m</option>"));
      response->print(F("<option value=\"25\""));response->print(loRaPerimiter3 == 25 ? " selected>":">");response->print(F("25m</option>"));
      response->print(F("<option value=\"30\""));response->print(loRaPerimiter3 == 30 ? " selected>":">");response->print(F("30m</option>"));
      response->print(F("</select></div>"));
      response->print(F("<div class=\"six columns\"><label for=\"beaconInterval3\">LoRa beacon interval</label><select class=\"u-full-width\" id=\"beaconInterval3\" name=\"beaconInterval3\">"));
      response->print(F("<option value=\"1000\""));response->print(beaconInterval3 == 1000 ? " selected>":">");response->print(F("1s</option>"));
      response->print(F("<option value=\"2000\""));response->print(beaconInterval3 == 2000 ? " selected>":">");response->print(F("2s</option>"));
      response->print(F("<option value=\"5000\""));response->print(beaconInterval3 == 5000 ? " selected>":">");response->print(F("5s</option>"));
      response->print(F("<option value=\"10000\""));response->print(beaconInterval3 == 10000 ? " selected>":">");response->print(F("10s</option>"));
      response->print(F("<option value=\"30000\""));response->print(beaconInterval3 == 30000 ? " selected>":">");response->print(F("30s</option>"));
      response->print(F("</select></div></div>"));
      //RSSI range estimation
      response->printf_P(PSTR("<div class=\"row\"><div class=\"four columns\"><label for=\"rssiAttenuation\">RSSI attenuation (distance halfing rate)</label><input class=\"u-full-width\" type=\"number\" step=\"0.01\" value=\"%.2f\" id=\"rssiAttenuation\" name=\"rssiAttenuation\"></div>"), rssiAttenuation);
      response->printf_P(PSTR("<div class=\"four columns\"><label for=\"rssiAttenuationBaseline\">RSSI at 10m (approx)</label><input class=\"u-full-width\" type=\"number\" step=\"0.01\" value=\"%.2f\" id=\"rssiAttenuationBaseline\" name=\"rssiAttenuationBaseline\"></div>"), rssiAttenuationBaseline);
      response->print(F("<div class=\"four columns\"><label for=\"rssiAttenuationPerimeter\">Minimum distance for RSSI attenuation distance estimation</label><select class=\"u-full-width\" id=\"rssiAttenuationPerimeter\" name=\"rssiAttenuationPerimeter\">"));
      response->print(F("<option value=\"3\""));response->print(rssiAttenuationPerimeter == 3 ? " selected>":">");response->print(F("3m</option>"));
      response->print(F("<option value=\"5\""));response->print(rssiAttenuationPerimeter == 5 ? " selected>":">");response->print(F("5m</option>"));
      response->print(F("<option value=\"7\""));response->print(rssiAttenuationPerimeter == 7 ? " selected>":">");response->print(F("7m</option>"));
      response->print(F("<option value=\"10\""));response->print(rssiAttenuationPerimeter == 10 ? " selected>":">");response->print(F("10m</option>"));
      response->print(F("<option value=\"15\""));response->print(rssiAttenuationPerimeter == 15 ? " selected>":">");response->print(F("15m</option>"));
      response->print(F("</select></div></div>"));
    #endif
    //Comment
    response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Configuration</h3></div></div>"));
    response->printf_P(PSTR("<div class=\"row\"><div class=\"twelve columns\"><label for=\"configurationComment\">Comment (optional)</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"configurationComment\" name=\"configurationComment\"></div></div>"), configurationComment);
    response->print(F("<a href =\"/\"><input class=\"button-primary\" type=\"button\" value=\"Back\"></a> <input class=\"button-primary\" type=\"submit\" value=\"Save\"></form>"));
    addPageFooter(response);
    request->send(response);
  });
  webServer.on("/configuration", HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
    lastWifiActivity = millis();
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
      {
          return request->requestAuthentication();  //Force basic authentication
      }
    #endif
    int params = request->params();
    localLog(F("Submitted Configuration parameters: "));
    localLogLn(params);
    /*
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){ //p->isPost() is also true
        //SERIAL_DEBUG_PORT.printf("FILE[%s]: %s, size: %u\r\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        //SERIAL_DEBUG_PORT.printf("POST[%s]: %s\r\n", p->name().c_str(), p->value().c_str());
        localLog(F("POST["));
        localLog(p->name().c_str());
        localLog(F("]: "));
        localLogLn(p->value().c_str());
      } else {
        //SERIAL_DEBUG_PORT.printf("GET[%s]: %s\r\n", p->name().c_str(), p->value().c_str());
      }
    }
    */
    //Read the submitted configuration
    if(request->hasParam("nodeName", true))
    {
      if(nodeName != nullptr)
      {
        delete [] nodeName;
      }
      nodeName = new char[request->getParam("nodeName", true)->value().length() + 1];
      strlcpy(nodeName,request->getParam("nodeName", true)->value().c_str(),request->getParam("nodeName", true)->value().length() + 1);
      localLog(F("nodeName: "));
      localLogLn(nodeName);
    }
    if(request->hasParam("configurationComment", true))
    {
      if(configurationComment != nullptr)
      {
        delete [] configurationComment;
      }
      configurationComment = new char[request->getParam("configurationComment", true)->value().length() + 1];
      strlcpy(configurationComment,request->getParam("configurationComment", true)->value().c_str(),request->getParam("configurationComment", true)->value().length() + 1);
      localLog(F("configurationComment: "));
      localLogLn(configurationComment);
    }
    else
    {
      if(configurationComment != nullptr)
      {
        delete [] configurationComment;
      }
      configurationComment = new char[strlen(default_configurationComment) + 1];  //Assign space on heap
      strlcpy(configurationComment,default_configurationComment,strlen(default_configurationComment) + 1);  //Copy in default
    }
    #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
      if(request->hasParam("http_user", true))
      {
        if(http_user != nullptr)
        {
          delete [] http_user;
        }
        http_user = new char[request->getParam("http_user", true)->value().length() + 1];
        strlcpy(http_user,request->getParam("http_user", true)->value().c_str(),request->getParam("http_user", true)->value().length() + 1);
        localLog(F("http_user: "));
        localLogLn(http_user);
      }
      if(request->hasParam("http_password", true))
      {
        if(request->getParam("http_password", true)->value().length() > 0)
        {
          if(http_password != nullptr)
          {
            delete [] http_password;
          }
          http_password = new char[request->getParam("http_password", true)->value().length() + 1];
          strlcpy(http_password,request->getParam("http_password", true)->value().c_str(),request->getParam("http_password", true)->value().length() + 1);
          localLogLn(F("http_password: ********"));
        }
      }
      if(request->hasParam("basicAuthEnabled", true))
      {
        localLog(F("basicAuthEnabled: "));
        if(request->getParam("basicAuthEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
        {
          basicAuthEnabled = true;
          localLogLn(F("enabled"));
        }
        else
        {
          basicAuthEnabled = false;
          localLogLn(F("disabled"));
        }
      }
    #endif
    #if defined(ENABLE_OTA_UPDATE)
      if(request->hasParam("otaEnabled", true))
      {
        localLog(F("otaEnabled: "));
        if(request->getParam("otaEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
        {
          otaEnabled = true;
          localLogLn(F("enabled"));
        }
        else
        {
          otaEnabled = false;
          localLogLn(F("disabled"));
        }
      }
      if(request->hasParam("otaAuthenticationEnabled", true))
      {
        localLog(F("otaAuthenticationEnabled: "));
        if(request->getParam("otaAuthenticationEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
        {
          otaAuthenticationEnabled = true;
          localLogLn(F("enabled"));
        }
        else
        {
          otaAuthenticationEnabled = false;
          localLogLn(F("disabled"));
        }
      }
    #endif
    #if defined(SUPPORT_WIFI)
      if(request->hasParam("SSID", true))
      {
        if(SSID != nullptr)
        {
          delete [] SSID;
        }
        SSID = new char[request->getParam("SSID", true)->value().length() + 1];
        strlcpy(SSID,request->getParam("SSID", true)->value().c_str(),request->getParam("SSID", true)->value().length() + 1);
        localLog(F("SSID: "));
        localLogLn(SSID);
      }
      if(request->hasParam("PSK", true))
      {
        if(request->getParam("PSK", true)->value().length() > 0)
        {
          if(PSK != nullptr)
          {
            delete [] PSK;
          }
          PSK = new char[request->getParam("PSK", true)->value().length() + 1];
          strlcpy(PSK,request->getParam("PSK", true)->value().c_str(),request->getParam("PSK", true)->value().length() + 1);
          localLogLn(F("PSK: ********"));
        }
      }
      if(request->hasParam("startWiFiOnBoot", true))
      {
        localLog(F("startWiFiOnBoot: "));
        if(request->getParam("startWiFiOnBoot", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
        {
          startWiFiOnBoot = true;
          localLogLn(F("enabled"));
        }
        else
        {
          startWiFiOnBoot = false;
          localLogLn(F("disabled"));
        }
      }
      if(request->hasParam("wiFiInactivityTimer", true))
      {
        wiFiInactivityTimer = request->getParam("wiFiInactivityTimer", true)->value().toInt();
        localLog(F("wiFiInactivityTimer: "));
        localLogLn(wiFiInactivityTimer);
      }
      if(request->hasParam("timeServer", true))
      {
        if(timeServer != nullptr)
        {
          delete [] timeServer;
        }
        timeServer = new char[request->getParam("timeServer", true)->value().length() + 1];
        strlcpy(timeServer,request->getParam("timeServer", true)->value().c_str(),request->getParam("timeServer", true)->value().length() + 1);
        localLog(F("timeServer: "));
        localLogLn(timeServer);
      }
      if(request->hasParam("timeZone", true))
      {
        if(timeZone != nullptr)
        {
          delete [] timeZone;
        }
        timeZone = new char[request->getParam("timeZone", true)->value().length() + 1];
        strlcpy(timeZone,request->getParam("timeZone", true)->value().c_str(),request->getParam("timeZone", true)->value().length() + 1);
        localLog(F("timeZone: "));
        localLogLn(timeZone);
      }
    #endif
    #ifdef SUPPORT_GPS
      if(request->hasParam("useGpsForTimeSync", true))
      {
        localLog(F("useGpsForTimeSync: "));
        if(request->getParam("useGpsForTimeSync", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
        {
          useGpsForTimeSync = true;
          localLogLn(F("enabled"));
        }
        else
        {
          useGpsForTimeSync = false;
          localLogLn(F("disabled"));
        }
      }
    #endif
    #if defined(ACT_AS_TRACKER)    
      if(request->hasParam("maximumEffectiveRange", true))
      {
        maximumEffectiveRange = request->getParam("maximumEffectiveRange", true)->value().toInt();
        localLog(F("maximumEffectiveRange: "));
        localLogLn(maximumEffectiveRange);
      }
      if(request->hasParam("beeperEnabled", true))
      {
        localLog(F("beeperEnabled: "));
        if(request->getParam("beeperEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
        {
          beeperEnabled = true;
          localLogLn(F("enabled"));
        }
        else
        {
          beeperEnabled = false;
          localLogLn(F("disabled"));
        }
      }
    #endif
    #if defined(SUPPORT_LORA)
      if(request->hasParam("loRaPerimiter1", true))
      {
        loRaPerimiter1 = request->getParam("loRaPerimiter1", true)->value().toInt();
        localLog(F("loRaPerimiter1: "));
        localLogLn(loRaPerimiter1);
      }
      if(request->hasParam("beaconInterval1", true))
      {
        beaconInterval1 = request->getParam("beaconInterval1", true)->value().toInt();
        localLog(F("beaconInterval1: "));
        localLogLn(beaconInterval1);
      }
      if(request->hasParam("loRaPerimiter2", true))
      {
        loRaPerimiter2 = request->getParam("loRaPerimiter2", true)->value().toInt();
        localLog(F("loRaPerimiter2: "));
        localLogLn(loRaPerimiter2);
      }
      if(request->hasParam("beaconInterval2", true))
      {
        beaconInterval2 = request->getParam("beaconInterval2", true)->value().toInt();
        localLog(F("beaconInterval2: "));
        localLogLn(beaconInterval2);
      }
      if(request->hasParam("loRaPerimiter3", true))
      {
        loRaPerimiter3 = request->getParam("loRaPerimiter3", true)->value().toInt();
        localLog(F("loRaPerimiter3: "));
        localLogLn(loRaPerimiter3);
      }
      if(request->hasParam("beaconInterval3", true))
      {
        beaconInterval3 = request->getParam("beaconInterval3", true)->value().toInt();
        localLog(F("beaconInterval3: "));
        localLogLn(beaconInterval3);
      }
      if(request->hasParam("rssiAttenuation", true))
      {
        rssiAttenuation = request->getParam("rssiAttenuation", true)->value().toFloat();
        localLog(F("rssiAttenuation: "));
        localLogLn(rssiAttenuation);
      }
      if(request->hasParam("rssiAttenuationBaseline", true))
      {
        rssiAttenuationBaseline = request->getParam("rssiAttenuationBaseline", true)->value().toFloat();
        localLog(F("rssiAttenuationBaseline: "));
        localLogLn(rssiAttenuationBaseline);
      }
      if(request->hasParam("rssiAttenuationPerimeter", true))
      {
        rssiAttenuationPerimeter = request->getParam("rssiAttenuationPerimeter", true)->value().toFloat();
        localLog(F("rssiAttenuationPerimeter: "));
        localLogLn(rssiAttenuationPerimeter);
      }
    #endif
    if(request->hasParam("loggingBufferSize", true))
    {
      loggingBufferSize = request->getParam("loggingBufferSize", true)->value().toInt();
      localLog(F("loggingBufferSize: "));
      localLogLn(loggingBufferSize);
    }
    if(request->hasParam("logFlushThreshold", true))
    {
      logFlushThreshold = request->getParam("logFlushThreshold", true)->value().toInt();
      localLog(F("logFlushThreshold: "));
      localLogLn(logFlushThreshold);
    }
    if(request->hasParam("logFlushInterval", true))
    {
      logFlushInterval = request->getParam("logFlushInterval", true)->value().toInt();
      localLog(F("logFlushInterval: "));
      localLogLn(logFlushInterval);
    }
    if(configurationChanged())
    {
      saveConfigurationSoon = millis();
    }
    request->redirect("/");
  });
  #if defined(ENABLE_LOCAL_WEBSERVER_FIRMWARE_UPDATE)
    webServer.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
      lastWifiActivity = millis();
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
        if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
        {
            return request->requestAuthentication();  //Force basic authentication
        }
      #endif
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      addPageHeader(response, 0, nullptr);
      response->print(F("<h2>Software update</h2>"));
      response->print(F("<ul>"));
      response->printf_P(PSTR("<li>Current firmware: %u.%u.%u</li>"), majorVersion, minorVersion, patchVersion);
      response->printf_P(PSTR("<li>Built: %s %s</li>"), __TIME__, __DATE__);
      response->printf_P(PSTR("<li>Board: %s</li>"), ARDUINO_BOARD);
      #ifdef ESP_IDF_VERSION_MAJOR
        response->print(F("<li>ESP-IDF: v"));
        #ifdef ESP_IDF_VERSION_MINOR
          response->print(ESP_IDF_VERSION_MAJOR);
          response->print('.');
          response->print(ESP_IDF_VERSION_MINOR);
        #else
          response->print(ESP_IDF_VERSION_MAJOR);
        #endif
      #endif
      response->print(F("</ul>"));
      response->print(F("<p>Before uploading any pre-compiled binary software, please check it is the version you want and for the right board after checking the information above.</p>"));
      response->print(F("<form method=\"POST\" action=\"/update\" enctype=\"multipart/form-data\">"));
      response->print(F("<input class=\"button-primary\" type=\"file\" name=\"update\"><br /><a href =\"/\"><input class=\"button-primary\" type=\"button\" value=\"Back\"></a> <input class=\"button-primary\" type=\"submit\" value=\"Update\"></form>"));
      addPageFooter(response);
      request->send(response);
    });
    webServer.on("/update", HTTP_POST,
      [](AsyncWebServerRequest *request){ //This lambda function is called when the update is complete
        lastWifiActivity = millis();
        #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
          if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
          {
              return request->requestAuthentication();  //Force basic authentication
          }
        #endif
        bool updateSuccesful = !Update.hasError();
        if(updateSuccesful == true)
        {
          localLogLn(F("Web UI software update complete, restarting shortly"));
          restartTimer = millis();
        }
        if(updateSuccesful == true)
        {
          request->redirect("/postUpdateRestart");
        }
        else
        {
          AsyncResponseStream *response = request->beginResponseStream("text/html");
          addPageHeader(response, 0, nullptr);
          response->print(F("<h2>Update failed</h2>"));
          response->print(F("<p>The software update failed!</p>"));
          response->print(F("<a href =\"/\"><input class=\"button-primary\" type=\"button\" value=\"Back\"></a>"));
          addPageFooter(response);
          request->send(response);
        }
      },
      [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){ //This lambda function is called when the update starts/continues/fails
        if(!index)
        {
          localLog(F("Web UI software update \""));
          localLog(filename);
          localLog(F("\" uploaded by "));
          localLogLn(request->client()->remoteIP().toString());
          flushLog();
          if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
          {
            localLog(F("Update error: "));
            localLogLn(Update.getError());
          }
        }
        if(!Update.hasError())
        {
          if(Update.write(data, len) != len)
          {
            localLog(F("Update error: "));
            localLogLn(Update.getError());
          }
        }
        if(final)
        {
          if(Update.end(true))
          {
            localLog(F("Update Success, flashed: "));
            localLog((index+len)/1024);
            localLogLn(F("KB"));
          }
          else
          {
            localLog(F("Update error: "));
            localLogLn(Update.getError());
          }
        }
      });
    webServer.on("/postUpdateRestart", HTTP_GET, [](AsyncWebServerRequest *request){
      lastWifiActivity = millis();
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
        if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
        {
            return request->requestAuthentication();  //Force basic authentication
        }
      #endif
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      addPageHeader(response, 20, "/"); //This sends the page to / after 20s
      response->print(F("<h2>Software update successful</h2>"));
      response->print(F("<p>The software update was successful and this node will restart in roughly 10 seconds.</p>"));
      response->print(F("<a href =\"/\"><input class=\"button-primary\" type=\"button\" value=\"Back\"></a>"));
      addPageFooter(response);
      request->send(response);
      restartTimer = millis();
    });
  #endif
  #if defined ENABLE_REMOTE_RESTART
    webServer.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
      lastWifiActivity = millis();
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
        if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
        {
            return request->requestAuthentication();  //Force basic authentication
        }
      #endif
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
        if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
        {
            return request->requestAuthentication();  //Force basic authentication
        }
      #endif
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      addPageHeader(response, 0, nullptr);
      response->print(F("<h2>Restart confirmation</h2>"));
      response->printf_P(PSTR("<p>Are you sure you want to restart \"%s\"?</p>"),nodeName);
      response->print(F("<a href =\"/restartConfirmed\"><input class=\"button-primary\" type=\"button\" value=\"Yes\"></a> "));
      response->print(F("<a href =\"/\"><input class=\"button-primary\" type=\"button\" value=\"No\"></a> "));
      addPageFooter(response);
      request->send(response);
    });
    #endif
    #if defined(ENABLE_REMOTE_RESTART)
    webServer.on("/restartConfirmed", HTTP_GET, [](AsyncWebServerRequest *request){
      lastWifiActivity = millis();
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
        if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
        {
            return request->requestAuthentication();  //Force basic authentication
        }
      #endif
      localLog(F("Web UI restart requested from "));
      localLogLn(request->client()->remoteIP().toString());
      AsyncResponseStream *response = request->beginResponseStream("text/html");
      addPageHeader(response, 5, "/");
      response->print(F("<h2>Restart</h2>"));
      response->print(F("<p>This node is restarting in 10s</p>"));
      response->print(F("<a href =\"/\"><input class=\"button-primary\" type=\"button\" value=\"Back\"></a>"));
      //response->print(F("<p>[<a href=\"/\">Back</a>]</p>"));
      addPageFooter(response);
      request->send(response);
      restartTimer = millis();
    });
  #endif
  webServer.on("/css/normalize.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    lastWifiActivity = millis();
    request->send_P(200, "text/css", normalize);
  });
  webServer.on("/css/skeleton.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    lastWifiActivity = millis();
    request->send_P(200, "text/css", skeleton);
  });
  webServer.onNotFound([](AsyncWebServerRequest *request){  //This lambda function is a minimal 404 handler
    lastWifiActivity = millis();
    request->send(404, "text/plain", request->url() + " not found");
  });
  #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
    #if defined(USE_SPIFFS)
      if(basicAuthEnabled == true)
      {
        webServer
                  .serveStatic("/logs/", SPIFFS, logDirectory) //Serve the log files up statically
                  .setAuthentication(http_user, http_password); //Add a username and password
      }
      else
      {
        webServer.serveStatic("/logs/", SPIFFS, logDirectory); //Serve the log files up statically 
      }
      #if defined(SERVE_CONFIG_FILE)
        webServer.serveStatic("/configfile", SPIFFS, configurationFile);
      #endif
    #elif defined(USE_LITTLEFS)
      if(basicAuthEnabled == true)
      {
        webServer
                  .serveStatic("/logs/", LittleFS, logDirectory) //Serve the log files up statically
                  .setAuthentication(http_user, http_password); //Add a username and password
      }
      else
      {
        webServer.serveStatic("/logs/", LittleFS, logDirectory); //Serve the log files up statically
      }
      #if defined(SERVE_CONFIG_FILE)
        webServer.serveStatic("/configfile", LittleFS, configurationFile);
      #endif
    #endif
  #else
    #if defined(USE_SPIFFS)
      webServer.serveStatic("/logs/", SPIFFS, logDirectory); //Serve the log files up statically
    #elif defined(USE_LITTLEFS)
      webServer.serveStatic("/logs/", LittleFS, logDirectory); //Serve the log files up statically
    #endif
  #endif
  webServer.begin();
  localLogLn(F("OK"));
}
#endif

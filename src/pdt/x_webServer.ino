/*
 * 
 * This file contains functions related to the local web server used for configuration, control and log access
 * 
 */
#if defined(ENABLE_LOCAL_WEBSERVER)
  void addPageHeader(AsyncResponseStream *response, uint8_t refresh, const char* refreshTo)
  {
    lastWifiActivity = millis();
    if(device[0].name != nullptr)
    {
      response->printf_P(PSTR("<!DOCTYPE html><html><head><title>%s</title>"), device[0].name);
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
    if(device[0].name != nullptr)
    {
      response->printf_P(PSTR("<h1>%s</h1><hr>"),device[0].name);
    }
  }
  void addPageFooter(AsyncResponseStream *response)
  {
    response->print(F("</div></body></html>"));
  }
  void setupWebServer()
  {
    if(filesystemMounted == true)
    {
      #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
        webserverSemaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(webserverSemaphore);
      #endif
      #ifdef SUPPORT_HACKING
        if(sensorReset == false)
        {
          //Re-use the ESPUI object
          adminWebServer = ESPUI.server;
        }
        else
        {
          adminWebServer = new AsyncWebServer(80);
        }
      #else
        //Create the web server object
        adminWebServer = new AsyncWebServer(80);
      #endif
      localLog(F("Configuring web server callbacks: "));
      adminWebServer->on("/admin", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            AsyncResponseStream *response = request->beginResponseStream("text/html");
            addPageHeader(response, 90, nullptr);
            //Top of page buttons
            response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/devices\"><input class=\"button-primary\" type=\"button\" value=\"Devices\" style=\"width: 100%;\"></a></div>"));
            response->print(F("<div class=\"four columns\"><a href =\"/listLogs\"><input class=\"button-primary\" type=\"button\" value=\"Logs\" style=\"width: 100%;\"></a></div>"));
            #if defined(ENABLE_REMOTE_RESTART)
              response->print(F("<div class=\"four columns\"><a href =\"/restart\"><input class=\"button-primary\" type=\"button\" value=\"Restart\" style=\"width: 100%;\"></a></div>"));
            #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_FIRMWARE_UPDATE)
              response->print(F("<div class=\"four columns\"><a href =\"/update\"><input class=\"button-primary\" type=\"button\" value=\"Software Update\" style=\"width: 100%;\"></a></div>"));
            #endif
            response->print(F("</div>"));
            //Status information
            response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>General</h2></div></div>"));
            response->print(F("<div class=\"row\"><div class=\"twelve columns\"><ul>"));
            #if defined(ACT_AS_TRACKER)
              response->printf_P(PSTR("<li>PDT tracker firmware: <b>v%u.%u.%u</b>"), device[0].majorVersion, device[0].minorVersion, device[0].patchVersion);
            #elif defined(ACT_AS_BEACON)
              response->printf_P(PSTR("<li>PDT beacon firmware: <b>v%u.%u.%u</b>"), device[0].majorVersion, device[0].minorVersion, device[0].patchVersion);
            #endif
            response->print(F(" Features: <b>"));
            response->print(deviceFeatures(device[0].typeOfDevice));
            response->printf_P(PSTR("</b><li>Built: <b>%s %s</b>"), __TIME__, __DATE__);
            response->printf_P(PSTR(" Board: <b>%s</b></li><li>PCB Variant: <b>"), ARDUINO_BOARD);
            #if HARDWARE_VARIANT == C3PDT || HARDWARE_VARIANT == C3PDTasBeacon
              response->print(F("C3 PDT v1"));
            #elif HARDWARE_VARIANT == C3TrackedSensor || HARDWARE_VARIANT == C3TrackedSensorAsBeacon
              response->print(F("C3 Trackable sensor v1"));
            #elif HARDWARE_VARIANT == C3LoRaBeacon
              response->print(F("C3 LoRa beacon v1"));
            #elif HARDWARE_VARIANT == CYDTracker
              response->print(F("ESP32 CYD"));
            #endif
            #ifdef ESP_IDF_VERSION_MAJOR
              response->print(F("</b></li><li>ESP-IDF: <b>v"));
              #ifdef ESP_IDF_VERSION_MINOR
                response->print(ESP_IDF_VERSION_MAJOR);
                response->print('.');
                response->print(ESP_IDF_VERSION_MINOR);
              #else
                response->print(ESP_IDF_VERSION_MAJOR);
              #endif
            #endif
            response->print(F("</b></li>"));
            response->printf_P(PSTR("<li>MAC address: <b>%02x:%02x:%02x:%02x:%02x:%02x</b></li>"), device[0].id[0], device[0].id[1], device[0].id[2], device[0].id[3], device[0].id[4], device[0].id[5]);
            if(filesystemMounted == true)
            {
              #if defined(USE_SPIFFS)
                FSInfo fsInfo;
                SPIFFS.info(fsInfo);
                response->printf_P(PSTR("<li>Filesystem: <b>SPIFFS %u/%uKB used"), fsInfo.usedBytes/1024, fsInfo.totalBytes/1024);
                if(fsInfo.usedBytes > 0 && fsInfo.totalBytes > 0)
                {
                  response->printf_P(PSTR(" %.1f%% "),float(fsInfo.usedBytes) * 100/float(fsInfo.totalBytes));
                }
                else
                {
                  response->print(F("</b> "));
                }
              #elif defined(USE_LITTLEFS)
                #if defined(ESP32)
                  response->printf_P(PSTR("<li>Filesystem: <b>LittleFS %u/%uKB used"), LittleFS.usedBytes()/1024, LittleFS.totalBytes()/1024);
                  if(LittleFS.usedBytes() > 0 && LittleFS.totalBytes() > 0)
                  {
                    response->printf_P(PSTR(" %.1f%%</b> "),float(LittleFS.usedBytes()) * 100/float(LittleFS.totalBytes()));
                  }
                  else
                  {
                    response->print(F("</b> "));
                  }
                #endif
              #endif
            }
            else
            {
              response->print(F("<li>Filesystem: <b>not mounted</b> "));
            }
            response->print(F("</b></li>"));
            #if defined(ESP32)
              response->print(F("<li>Free heap: <b>"));
              response->print(ESP.getFreeHeap()/1024);
              response->print(F("KB</b></li>"));
            #endif
            response->print(F("<li>USB serial logging: "));
            #if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
              if(debugPortAvailable)
              {
                response->print(F("<b>enabled - connected</b></li>"));
              }
              else
              {
                response->print(F("<b>enabled - disconnected</b></li>"));
              }
            #else
              response->print(F("<b>disabled</b></li>"));
            #endif
            #if defined(ENABLE_OTA_UPDATE)
              response->print(F("<li>Over-the-air software update (Arduino IDE): <b>enabled</b></li>"));
            #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_FIRMWARE_UPDATE)
              response->print(F("<li>Web UI software update: <b>enabled</b></li>"));
            #endif
            response->print(F("<li>Time: <b>"));
            if(timeIsValid())
            {
              updateTimestamp();
              response->print(timestamp);
            }
            else
            {
              response->print(F("Not set"));
            }
            response->print(F("</b></li><li>Uptime: <b>"));
            response->print(printableUptime(millis()/1000));
            response->print(F("</b></li>"));
            #if defined(ESP32)
              #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
                #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
                  response->print(F("<li>Restart reason core 0: <b>"));
                  response->print(es32ResetReason(0));
                  response->print(F("</b> Restart reason core 1: <b>"));
                  response->print(es32ResetReason(1));
                #elif CONFIG_IDF_TARGET_ESP32S2
                  response->print(F("<li>Restart reason: <b>"));
                  response->print(es32ResetReason(0));
                #elif CONFIG_IDF_TARGET_ESP32C3
                  response->print(F("<li>Restart reason: <b>"));
                  response->print(es32ResetReason(0));
                #else 
                  #error Target CONFIG_IDF_TARGET is not supported
                #endif
              #else // ESP32 Before IDF 4.0
                response->print(F("<li>Restart reason core 0: <b>"));
                response->print(es32ResetReason(0));
                response->print(F("</b> Restart reason core 1: <b>"));
                response->print(es32ResetReason(1));
              #endif
            #endif
            response->print(F("</b></li>"));
            #ifdef SUPPORT_BATTERY_METER
              if(enableBatteryMonitor == true)
              {
                response->print(F("<li>Battery voltage: <b>"));
                response->print(device[0].supplyVoltage);
                response->print(F("v ("));
                response->print(batteryPercentage);
                response->print(F("% charge)</b></li>"));
              }
              else
              {
                response->print(F("<li>Battery monitoring: <b>disabled</b>"));
              }
            #endif
            #if defined(SUPPORT_WIFI)
              if(wiFiClientInactivityTimer > 0)
              {
                response->print(F("<li>WiFi disconnecting after : "));
                if(wiFiClientInactivityTimer == 60000)
                {
                  response->print(F("1 minute"));
                }
                else if(wiFiClientInactivityTimer == 180000)
                {
                  response->print(F("3 minutes"));
                }
                else if(wiFiClientInactivityTimer == 300000)
                {
                  response->print(F("f minutes"));
                }
                response->print(F(" inactivity</li>"));
              }
            #endif
            if(configurationComment != nullptr)
            {
              if(strlen(configurationComment) > 0)
              {
                response->print(F("<li>Configuration comment: <b>"));
                response->print(configurationComment);
                response->print(F("</b></li>"));
              }
            }
            response->print(F("</ul></div></div>"));
            response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/configuration\"><input class=\"button-primary\" type=\"button\" value=\"Config\" style=\"width: 100%;\"></a></div>"));
            response->print(F("<div class=\"four columns\"><a href =\"/wipe\"><input class=\"button-primary\" type=\"button\" value=\"Wipe\" style=\"width: 100%;\"></a></div></div>"));
            #ifdef ACT_AS_BEACON
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>In-game info</h2></div></div>"));
              response->print(F("<li>Name: <b>"));
              if(device[0].icName != nullptr)
              {
                response->print(device[0].icName);
              }
              else
              {
                response->print(F("not set"));
              }
              response->print(F("</b></li><li>Description: <b>"));
              if(device[0].icDescription != nullptr)
              {
                response->print(device[0].icDescription);
              }
              else
              {
                response->print(F("not set"));
              }
              response->print(F("</b></li>"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/icconfiguration\"><input class=\"button-primary\" type=\"button\" value=\"IC config\" style=\"width: 100%;\"></a></div></div>"));
            #endif
            #ifdef SUPPORT_ESPNOW
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>ESP-Now</h2></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><ul><li>ESP-Now radio: <b>"));
              if(espNowEnabled == true)
              {
                response->print(F("On"));
              }
              else
              {
                response->print(F("Off"));
              }
              response->print(F("</b></li>"));
              if(espNowEnabled == true)
              {
                if(espNowInitialised == true)
                {
                  response->print(F("<li>Channel: <b>"));
                  response->print(espNowChannel);
                  response->print(F("</b></li>"));
                  response->print(F("<li>Packets: <b>"));
                  response->print(espNowRxPackets);
                  response->print(F(" RX / "));
                  response->print(espNowTxPackets);
                  response->print(F(" TX"));
                  response->printf_P(PSTR(" Duty cycle: %.02f%%"),calculatedEspNowDutyCycle);
                  response->print(F(""));
                  response->print(F("</b></li><li>Dropped: <b>"));
                  response->print(espNowRxPacketsDropped);
                  response->print(F(" RX / "));
                  response->print(espNowTxPacketsDropped);
                  response->print(F(" TX</b></li>"));
                  response->print(F("<li>Update interval: <b>"));
                  response->print(device[0].nextEspNowLocationUpdate/1000);
                  response->print(F("s</b></li>"));
                }
                else
                {
                  response->print(F("<li><b>Not initialised</b></li>"));
                }
              }
              response->print(F("</ul></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/espnowconfiguration\"><input class=\"button-primary\" type=\"button\" value=\"ESP-Now config\" style=\"width: 100%;\"></a></div></div>"));
            #endif
            #if defined(SUPPORT_LORA)
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>LoRa</h2></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><ul><li>LoRa radio: <b>"));
              if(loRaEnabled == true)
              {
                response->print(F("On"));
              }
              else
              {
                response->print(F("Off"));
              }
              response->print(F("</b></li>"));
              if(loRaEnabled == true)
              {
                response->print(F("<li>Packets: <b>"));
                if(loRaInitialised == true)
                {                
                  response->print(loRaRxPackets);
                  response->print(F(" RX / "));
                  response->print(loRaTxPackets);
                  response->print(F(" TX"));
                  response->printf_P(PSTR(" Duty cycle: %.02f%%"),calculatedLoRaDutyCycle);
                  response->print(F("</b></li><li>Dropped: <b>"));
                  response->print(loRaRxPacketsDropped);
                  response->print(F(" RX / "));
                  response->print(loRaTxPacketsDropped);
                  response->print(F(" TX</b></li>"));
                  response->print(F("<li>Update interval: <b>"));
                  response->print(device[0].nextLoRaLocationUpdate/1000);
                  response->print(F("s</b></li>"));
                }
                else
                {
                  response->print(F("Not initialised</b></li>"));
                }
              }
              response->print(F("</ul></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/loraconfiguration\"><input class=\"button-primary\" type=\"button\" value=\"LoRa config\" style=\"width: 100%;\"></a></div></div>"));
            #endif
            #ifdef SUPPORT_FTM
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>FTM (time-of-flight) measurements</h2></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><ul><li>FTM beacon: <b>"));
              if(ftmEnabled == true)
              {
                response->print(F("On"));
              }
              else
              {
                response->print(F("Off"));
              }
              response->print(F("</b></li>"));
              response->print(F("</ul></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/ftmconfiguration\"><input class=\"button-primary\" type=\"button\" value=\"FTM configuration\" style=\"width: 100%;\"></a></div></div>"));
            #endif
            #ifdef SUPPORT_GPS
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>GPS</h2></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><ul>"));
              #ifdef SUPPORT_SOFT_PERIPHERAL_POWER_OFF
                response->print(F("<li>Power: <b>"));
                if(peripheralsEnabled == true)
                {
                  response->print(F("On"));
                }
                else
                {
                  response->print(F("Off"));
                }
                response->print(F("</b></li>"));
              #endif
              if(device[0].hasGpsFix)
              {
                #if defined(SUPPORT_LORA) || defined(SUPPORT_ESPNOW)
                  response->print(F("<li>Last update: <b>"));
                  #if defined(SUPPORT_LORA)
                    response->print(float(millis() - device[0].lastLoRaLocationUpdate)/1000);
                  #elif defined(SUPPORT_ESPNOW)
                    response->print(float(millis() - device[0].lastEspNowLocationUpdate)/1000);
                  #endif
                  response->print(F("s</b> ago</li>"));
                #endif
                response->print(F("<li>Latitude: <b>"));
                response->print(device[0].latitude);
                response->print(F("</b> Longitude: <b>"));
                response->print(device[0].longitude);
                response->print(F("</b> Speed: <b>"));
                response->print(device[0].speed);
                response->print(F("</b> HDOP: <b>"));
                response->print(device[0].hdop);
                response->print('(');
                response->print(hdopDescription(device[0].hdop));
                response->print(F(")</b></li>"));
                response->print(F("<li>Chars: <b>"));
                response->print(gpsChars);
                response->print(F("</b></li>"));
                response->print(F("<li>Sentences: <b>"));
                response->print(gpsSentences);
                response->print(F("</b> Errors: <b>"));
                response->print(gpsErrors);
                if(gpsSentences > 0 && gpsErrors > 0)
                {
                  response->print('(');
                  response->print(float(100) * float(gpsErrors)/float(gpsSentences));
                  response->print(F("%)"));
                }
                response->print(F("</b></li>"));
                #if defined(ACT_AS_TRACKER)
                  response->print(F("<li>Distance to tracked beacon: <b>"));
                  if(currentlyTrackedBeacon < maximumNumberOfDevices)
                  {
                    if(device[currentlyTrackedBeacon].hasGpsFix)
                    {
                      response->print(device[currentlyTrackedBeacon].distanceTo);
                      response->print(F("m</b></li>"));
                      response->print(F("<li>Course to tracked beacon: <b>"));
                      response->print(device[currentlyTrackedBeacon].courseTo);
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
                  response->print(F("<li>Distance to nearest tracker: <b>"));
                  if(closestTracker < maximumNumberOfDevices)
                  {
                    if(device[closestTracker].hasGpsFix)
                    {
                      response->print(device[closestTracker].distanceTo);
                      response->print(F("m</b></li>"));
                      response->print(F("<li>Course to nearest tracker: <b>"));
                      response->print(device[closestTracker].courseTo);
                    }
                    else
                    {
                      response->print(F("Unknown"));
                    }
                  }
                  else
                  {
                    response->print(F("no trackers"));
                  }
                #endif
                response->print(F("</b></li>"));
              }
              else
              {
                response->print(F("<li>Chars: <b>"));
                response->print(gpsChars);
                response->print(F("</b> - no fix yet</li>"));
              }
              response->print(F("</ul></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/gpsconfiguration\"><input class=\"button-primary\" type=\"button\" value=\"GPS config\" style=\"width: 100%;\"></a></div></div>"));
            #endif
            //response->print(F("</b>"));
            #if defined(ACT_AS_SENSOR)
              response->print(F("<h2>Sensor</h2>"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/sensorConfiguration\"><input class=\"button-primary\" type=\"button\" value=\"Sensor config\" style=\"width: 100%;\"></a></div>"));
              response->print(F("<div class=\"four columns\"><a href =\"/sensorReset\"><input class=\"button-primary\" type=\"button\" value=\"Reset sensor\" style=\"width: 100%;\"></a></div></div>"));
              response->printf_P(PSTR("<li>Current hits: <b>%u/%u</b> stun: <b>%u/%u</b></li>"), device[0].currentNumberOfHits, device[0].numberOfStartingHits, device[0].currentNumberOfStunHits, device[0].numberOfStartingStunHits);
              if(armourValue > 0)
              {
                response->printf_P(PSTR("<li>Armour value: <b>%u</b></li>"), armourValue);
              }
              if(EP_flag == true)
              {
                response->print(F("<li>Require powerup to hit: <b>true</b></li>"));
              }
              if(ig_healing_flag == true)
              {
                response->print(F("<li>Ignore healing: <b>true</b></li>"));
              }
              if(ig_stun_flag == true)
              {
                response->print(F("<li>Ignore stun: <b>true</b></li>"));
              }
              if(ig_ongoing_flag == true)
              {
                response->print(F("<li>Ignore ongoing: <b>true</b></li>"));
              }
              if(regen_while_zero == true)
              {
                response->print(F("<li>Regen from zero: <b>true</b></li>"));
              }
              if(treat_as_one == true)
              {
                response->print(F("<li>Treat hits as one: <b>true</b></li>"));
              }
              if(treat_stun_as_one == true)
              {
                response->print(F("<li>Treat stun as one: <b>true</b></li>"));
              }
              if(ongoing_is_cumulative == true)
              {
                response->print(F("<li>Ongoing is cumlative: <b>true</b></li>"));
              }
              if(ig_non_dot == true)
              {
                response->print(F("<li>Ignore non-DOT: <b>true</b></li>"));
              }
            #endif
            response->print(F("</ul>"));
            #ifdef SUPPORT_HACKING
              response->print(F("<h2>Game</h2>"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/gameConfiguration\"><input class=\"button-primary\" type=\"button\" value=\"Configure hacking game\" style=\"width: 100%;\"></a></div></div>"));
              response->print(F("<ul>"));
              response->printf_P(PSTR("<li>Game length: <b>%u</b></li><li>Game retries: <b>%u</b> (0=infinite)</li><li>Game speedup: <b>%u</b>(ms)</li>"), gameLength, gameRetries, gameSpeedup);
              response->print(F("</ul>"));
            #endif
            addPageFooter(response);
            //Send response
            request->send(response);
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
      });
      adminWebServer->on("/listLogs", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows a list of all the log files
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
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
            response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a></div></div>"));
            response->print(F("<h2>Log files</h2>"));
            response->print(F("<table class=\"u-full-width\"><thead><tr><th>File</th><th>Size</th></tr></thead><tbody>"));
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
                      response->printf_P(PSTR("<tr><td>%s</td><td>%.1fKB</td><td><a href=\"/logs/%s\"><input class=\"button-primary\" type=\"button\" value=\"View\" style=\"width: 100%;\"></a></td><td><a href=\"/deleteLog?file=%s\"><input class=\"button-primary\" type=\"button\" value=\"Delete\" style=\"width: 100%;\"></a></td></tr>"), file.name(), float(file.size())/1024, file.name(), file.name());
                    #else
                      response->printf_P(PSTR("<tr><td>%s</td><td>%.1fKB</td><td><a href=\"/logs/%s\"><input class=\"button-primary\" type=\"button\" value=\"View\" style=\"width: 100%;\"></a></td></tr>"), file.name(), float(file.size())/1024, file.name());
                    #endif
                  }
                }
                file = dir.openNextFile();
              }
            #endif
            response->print(F("</tbody></table>"));
            addPageFooter(response);
            //Send response
            request->send(response);
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
      });
      #if defined(ENABLE_LOG_DELETION)
        adminWebServer->on("/deleteLog", HTTP_GET, [](AsyncWebServerRequest *request){
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
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
                response->printf_P(PSTR("<p><a href =\"/deleteLogConfirmed?file=%s\"><input class=\"button-primary\" type=\"button\" value=\"Yes\" style=\"width: 100%;\"></a> <a href=\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"No\" style=\"width: 100%;\"></a></p>"),file->value().c_str());
                addPageFooter(response);
                //Send response
                request->send(response);
              }
              else
              {
                request->send(500, "text/plain", request->url() + " file not found");
              }
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
        });
        adminWebServer->on("/deleteLogConfirmed", HTTP_GET, [](AsyncWebServerRequest *request){
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
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
                if(pdtDeleteFile(fileToDelete))
                {
                    request->redirect("/listLogs");
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
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
        });
      #endif
      adminWebServer->on("/configuration", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            AsyncResponseStream *response = request->beginResponseStream("text/html");
            addPageHeader(response, 0, nullptr);
            //Start of form
            response->print(F("<form method=\"POST\">"));
            response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a></div><div class=\"four columns\"><input class=\"button-primary\" type=\"submit\" value=\"Save\" style=\"width: 100%;\"></div></div>"));
            response->print(F("<h2>Configuration</h2>"));
            if(device[0].name != nullptr)
            {
              response->printf_P(PSTR("<div class=\"row\"><div class=\"twelve columns\"><label for=\"deviceName\">Node name</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"deviceName\" name=\"deviceName\"></div></div>"), device[0].name);
            }
            else
            {
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"deviceName\">Node name</label><input class=\"u-full-width\" type=\"text\" value=\"\" id=\"deviceName\" name=\"deviceName\"></div></div>"));
            }
            response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Networking</h3></div></div>"));
            #if defined(SUPPORT_WIFI)
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>WiFi</h3></div></div>"));
              //WiFi client
              response->print(F("<div class=\"row\"><div class=\"three columns\"><label for=\"startWiFiClientOnBoot\">Enable WiFi client on boot</label><select class=\"u-full-width\" id=\"startWiFiClientOnBoot\" name=\"startWiFiClientOnBoot\">"));
              response->print(F("<option value=\"true\""));response->print(startWiFiClientOnBoot == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(startWiFiClientOnBoot == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div>"));
              response->print(F("<div class=\"three columns\"><label for=\"wiFiClientInactivityTimer\">WiFi inactivity timer</label><select class=\"u-full-width\" id=\"wiFiClientInactivityTimer\" name=\"wiFiClientInactivityTimer\">"));
              response->print(F("<option value=\"0\""));response->print(wiFiClientInactivityTimer == 0 ? " selected>":">");response->print(F("Never</option>"));
              response->print(F("<option value=\"60000\""));response->print(wiFiClientInactivityTimer == 60000 ? " selected>":">");response->print(F("1m</option>"));
              response->print(F("<option value=\"180000\""));response->print(wiFiClientInactivityTimer == 180000 ? " selected>":">");response->print(F("3m</option>"));
              response->print(F("<option value=\"300000\""));response->print(wiFiClientInactivityTimer == 300000 ? " selected>":">");response->print(F("5m</option>"));
              response->print(F("</select></div>"));
              if(SSID != nullptr)
              {
                response->printf_P(PSTR("<div class=\"three columns\"><label for=\"SSID\">WiFi SSID</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"SSID\" name=\"SSID\"></div>"), SSID);
              }
              else
              {
                response->print(F("<div class=\"three columns\"><label for=\"SSID\">WiFi SSID</label><input class=\"u-full-width\" type=\"text\" value=\"\" id=\"SSID\" name=\"SSID\"></div>"));
              }
              response->print(F("<div class=\"three columns\"><label for=\"PSK\">WiFi PSK</label><input class=\"u-full-width\" type=\"password\" placeholder=\"********\" id=\"PSK\" name=\"PSK\"></div></div>"));
              //WiFi AP        
              response->print(F("<div class=\"row\"><div class=\"four columns\"><label for=\"startWiFiApOnBoot\">Enable WiFi AP on boot</label><select class=\"u-full-width\" id=\"startWiFiApOnBoot\" name=\"startWiFiApOnBoot\">"));
              response->print(F("<option value=\"true\""));response->print(startWiFiApOnBoot == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(startWiFiApOnBoot == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div>"));
              response->print(F("<div class=\"four columns\"><label for=\"enableCaptivePortal\">Enable captive portal</label><select class=\"u-full-width\" id=\"enableCaptivePortal\" name=\"enableCaptivePortal\">"));
              response->print(F("<option value=\"true\""));response->print(enableCaptivePortal == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(enableCaptivePortal == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div>"));
              response->print(F("<div class=\"four columns\"><label for=\"softApChannel\">Preferred channel</label><select class=\"u-full-width\" id=\"softApChannel\" name=\"softApChannel\">"));
              response->print(F("<option value=\"1\""));response->print(softApChannel == 1 ? " selected>":">");response->print(F("1</option>"));
              response->print(F("<option value=\"6\""));response->print(softApChannel == 6 ? " selected>":">");response->print(F("6</option>"));
              response->print(F("<option value=\"11\""));response->print(softApChannel == 11 ? " selected>":">");response->print(F("11</option>"));
              response->print(F("</select></div></div>"));
              if(APSSID != nullptr)
              {
                response->printf_P(PSTR("<div class=\"row\"><div class=\"six columns\"><label for=\"APSSID\">AP SSID</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"APSSID\" name=\"APSSID\"></div>"), APSSID);
              }
              else
              {
                response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"APSSID\">AP SSID</label><input class=\"u-full-width\" type=\"text\" value=\"\" id=\"APSSID\" name=\"APSSID\"></div>"));
              }
              response->print(F("<div class=\"six columns\"><label for=\"APPSK\">AP PSK</label><input class=\"u-full-width\" type=\"password\" placeholder=\"********\" id=\"APPSK\" name=\"APPSK\"></div></div>"));
            #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              response->print(F("<div class=\"row\"><div class=\"four columns\"><label for=\"basicAuthEnabled\">Web UI login</label><select class=\"u-full-width\" id=\"basicAuthEnabled\" name=\"basicAuthEnabled\">"));
              response->print(F("<option value=\"true\""));response->print(basicAuthEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(basicAuthEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div>"));
              if(http_user != nullptr)
              {
                response->printf_P(PSTR("<div class=\"four columns\"><label for=\"http_user\">Web UI username</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"http_user\" name=\"http_user\"></div>"), http_user);
              }
              else
              {
                response->print(F("<div class=\"four columns\"><label for=\"http_user\">Web UI username</label><input class=\"u-full-width\" type=\"text\" value=\"\" id=\"http_user\" name=\"http_user\"></div>"));
              }
              response->print(F("<div class=\"four columns\"><label for=\"http_password\">Web UI/OTA password</label><input class=\"u-full-width\" type=\"password\" placeholder=\"********\" id=\"http_password\" name=\"http_password\"></div></div>"));
            #endif
            #if defined(SUPPORT_WIFI)
              //Time Server
              if(timeServer != nullptr)
              {
                response->printf_P(PSTR("<div class=\"row\"><div class=\"six columns\"><label for=\"timeServer\">NTP host</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"timeServer\" name=\"timeServer\"></div>"), timeServer);
              }
              else
              {
                response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"timeServer\">NTP host</label><input class=\"u-full-width\" type=\"text\" value=\"\" id=\"timeServer\" name=\"timeServer\"></div>"));
              }
              if(timeZone != nullptr)
              {
                response->printf_P(PSTR("<div class=\"six columns\"><label for=\"\">Timezone</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"timeZone\" name=\"timeZone\"></div></div>"), timeZone);
              }
              else
              {
                response->print(F("<div class=\"six columns\"><label for=\"\">Timezone</label><input class=\"u-full-width\" type=\"text\" value=\"\" id=\"timeZone\" name=\"timeZone\"></div></div>"));
              }
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
            response->print(F("<option value=\"30\""));response->print(logFlushInterval == 30 ? " selected>":">");response->print(F("30s</option>"));
            response->print(F("<option value=\"60\""));response->print(logFlushInterval == 60 ? " selected>":">");response->print(F("60s</option>"));
            response->print(F("<option value=\"90\""));response->print(logFlushInterval == 90 ? " selected>":">");response->print(F("90s</option>"));
            response->print(F("<option value=\"180\""));response->print(logFlushInterval == 180 ? " selected>":">");response->print(F("3 minutes</option>"));
            response->print(F("<option value=\"300\""));response->print(logFlushInterval == 300 ? " selected>":">");response->print(F("5 minutes</option>"));
            //response->print(F("<option value=\"7200\""));response->print(logFlushInterval == 7200 ? " selected>":">");response->print(F("2 hours</option>"));
            //response->print(F("<option value=\"14400\""));response->print(logFlushInterval == 14400 ? " selected>":">");response->print(F("4 hours</option>"));
            //response->print(F("<option value=\"28800\""));response->print(logFlushInterval == 28800 ? " selected>":">");response->print(F("8 hours</option>"));
            //response->print(F("<option value=\"57600\""));response->print(logFlushInterval == 57600 ? " selected>":">");response->print(F("16 hours</option>"));
            //response->print(F("<option value=\"86400\""));response->print(logFlushInterval == 86400 ? " selected>":">");response->print(F("24 hours</option>"));
            response->print(F("</select></div></div>"));
            //Battery
            #ifdef SUPPORT_BATTERY_METER
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Battery monitor</h3></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"enableBatteryMonitor\">Enable battery monitor</label><select class=\"u-full-width\" id=\"enableBatteryMonitor\" name=\"enableBatteryMonitor\">"));
              response->print(F("<option value=\"true\""));response->print(enableBatteryMonitor == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(enableBatteryMonitor == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div><div class=\"six columns\">Defaults 330/104kOhm</div></div>"));
              response->printf_P(PSTR("<div class=\"row\"><div class=\"six columns\"><label for=\"topLadderResistor\">Top ladder resistor (Kohms)</label><input class=\"u-full-width\" type=\"number\" step=\"0.1\" value=\"%.1f\" id=\"topLadderResistor\" name=\"topLadderResistor\"></div>"), topLadderResistor);
              response->printf_P(PSTR("<div class=\"six columns\"><label for=\"bottomLadderResistor\">Bottom ladder resistor (Kohms)</label><input class=\"u-full-width\" type=\"number\" step=\"0.1\" value=\"%.1f\" id=\"bottomLadderResistor\" name=\"bottomLadderResistor\"></div></div>"), bottomLadderResistor);
            #endif
            #if defined(ACT_AS_TRACKER)
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Tracking</h3></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"maximumEffectiveRange\">Maximum effective range</label><select class=\"u-full-width\" id=\"maximumEffectiveRange\" name=\"maximumEffectiveRange\">"));
              response->print(F("<option value=\"50\""));response->print(maximumEffectiveRange == 50 ? " selected>":">");response->print(F("50m</option>"));
              response->print(F("<option value=\"75\""));response->print(maximumEffectiveRange == 75 ? " selected>":">");response->print(F("75m</option>"));
              response->print(F("<option value=\"99\""));response->print(maximumEffectiveRange == 99 ? " selected>":">");response->print(F("99m</option>"));
              response->print(F("<option value=\"150\""));response->print(maximumEffectiveRange == 150 ? " selected>":">");response->print(F("150m</option>"));
              response->print(F("<option value=\"250\""));response->print(maximumEffectiveRange == 250 ? " selected>":">");response->print(F("250m</option>"));
              response->print(F("<option value=\"750\""));response->print(maximumEffectiveRange == 750 ? " selected>":">");response->print(F("750m</option>"));
              response->print(F("<option value=\"1000\""));response->print(maximumEffectiveRange == 1000 ? " selected>":">");response->print(F("1000m</option>"));
              response->print(F("<option value=\"9999\""));response->print(maximumEffectiveRange == 9999 ? " selected>":">");response->print(F("9999m</option>"));
              response->print(F("<option value=\""));response->print(effectivelyUnreachable);response->print('"');response->print(maximumEffectiveRange == effectivelyUnreachable ? " selected>":">");response->print(F("Unlimited</option>"));
              response->print(F("</select></div></div>"));
            #endif
            #if defined(ACT_AS_BEACON)
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Location tracking</h3></div></div>"));
            #endif
            response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"trackingSensitivity\">Sensitivity</label><select class=\"u-full-width\" id=\"trackingSensitivity\" name=\"trackingSensitivity\">"));
            response->print(F("<option value=\"0\""));response->print(trackingSensitivity == 0 ? " selected>":">");response->print(F("Low</option>"));
            response->print(F("<option value=\"1\""));response->print(trackingSensitivity == 1 ? " selected>":">");response->print(F("Medium</option>"));
            response->print(F("<option value=\"2\""));response->print(trackingSensitivity == 2 ? " selected>":">");response->print(F("High</option>"));
            response->print(F("</select></div></div>"));
            #ifdef SUPPORT_BEEPER
              #if defined(ACT_AS_TRACKER)
                response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Beeper</h3></div></div>"));
                response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"beeperEnabled\">Beeper enabled</label><select class=\"u-full-width\" id=\"beeperEnabled\" name=\"beeperEnabled\">"));
                response->print(F("<option value=\"true\""));response->print(beeperEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
                response->print(F("<option value=\"false\""));response->print(beeperEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
                response->print(F("</select></div>"));
                #ifdef SUPPORT_BUTTON
                  response->print(F("<div class=\"six columns\"><label for=\"beepOnPress\">Button beep</label><select class=\"u-full-width\" id=\"beepOnPress\" name=\"beepOnPress\">"));
                  response->print(F("<option value=\"true\""));response->print(beepOnPress == true ? " selected>":">");response->print(F("Enabled</option>"));
                  response->print(F("<option value=\"false\""));response->print(beepOnPress == false ? " selected>":">");response->print(F("Disabled</option>"));
                  response->print(F("</select></div></div>"));
                #endif
              #else
                response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Beeper</h3></div></div>"));
                response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"beeperEnabled\">Beeper enabled</label><select class=\"u-full-width\" id=\"beeperEnabled\" name=\"beeperEnabled\">"));
                response->print(F("<option value=\"true\""));response->print(beeperEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
                response->print(F("<option value=\"false\""));response->print(beeperEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
                response->print(F("</select></div></div>"));
              #endif
            #endif
            #ifdef SUPPORT_VIBRATION
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Vibration motor</h3></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"vibrationEnabled\">Haptic feedback enabled</label><select class=\"u-full-width\" id=\"vibrationEnabled\" name=\"vibrationEnabled\">"));
              response->print(F("<option value=\"true\""));response->print(vibrationEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(vibrationEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"vibrationLevel\">Vibration level</label><select class=\"u-full-width\" id=\"vibrationLevel\" name=\"vibrationLevel\">"));
              response->print(F("<option value=\"10\""));response->print(vibrationLevel == 10 ? " selected>":">");response->print(F("10%</option>"));
              response->print(F("<option value=\"25\""));response->print(vibrationLevel == 25 ? " selected>":">");response->print(F("25%</option>"));
              response->print(F("<option value=\"50\""));response->print(vibrationLevel == 50 ? " selected>":">");response->print(F("50%</option>"));
              response->print(F("<option value=\"75\""));response->print(vibrationLevel == 75 ? " selected>":">");response->print(F("75%</option>"));
              response->print(F("<option value=\"100\""));response->print(vibrationLevel == 100 ? " selected>":">");response->print(F("100%</option>"));
              response->print(F("</select></div></div>"));
            #endif
            //Comment
            response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Configuration</h3></div></div>"));
            if(configurationComment != nullptr)
            {
              response->printf_P(PSTR("<div class=\"row\"><div class=\"twelve columns\"><label for=\"configurationComment\">Comment (optional)</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"configurationComment\" name=\"configurationComment\"></div></div>"), configurationComment);
            }
            else
            {
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"configurationComment\">Comment (optional)</label><input class=\"u-full-width\" type=\"text\" value=\"\" id=\"configurationComment\" name=\"configurationComment\"></div></div>"));
            }
            //End of form
            response->print(F("</form>"));
            addPageFooter(response);
            //Send response
            request->send(response);
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
        });
        adminWebServer->on("/configuration", HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            #ifdef DEBUG_FORM_SUBMISSION
              int params = request->params();
              localLog(F("Submitted Configuration parameters: "));
              localLogLn(params);
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
            #endif
            //Read the submitted configuration
            if(request->hasParam("deviceName", true))
            {
              if(device[0].name != nullptr)
              {
                delete [] device[0].name;
              }
              device[0].name = new char[request->getParam("deviceName", true)->value().length() + 1];
              strlcpy(device[0].name,request->getParam("deviceName", true)->value().c_str(),request->getParam("deviceName", true)->value().length() + 1);
              //localLog(F("deviceName: "));
              //localLogLn(device[0].name);
            }
            if(request->hasParam("configurationComment", true))
            {
              if(configurationComment != nullptr)
              {
                delete [] configurationComment;
              }
              configurationComment = new char[request->getParam("configurationComment", true)->value().length() + 1];
              strlcpy(configurationComment,request->getParam("configurationComment", true)->value().c_str(),request->getParam("configurationComment", true)->value().length() + 1);
              //localLog(F("configurationComment: "));
              //localLogLn(configurationComment);
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
                //localLog(F("http_user: "));
                //localLogLn(http_user);
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
                  //localLogLn(F("http_password: ********"));
                }
              }
              if(request->hasParam("basicAuthEnabled", true))
              {
                //localLog(F("basicAuthEnabled: "));
                if(request->getParam("basicAuthEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  basicAuthEnabled = true;
                  //localLogLn(F("enabled"));
                }
                else
                {
                  basicAuthEnabled = false;
                  //localLogLn(F("disabled"));
                }
              }
            #endif
            #if defined(ENABLE_OTA_UPDATE)
              if(request->hasParam("otaEnabled", true))
              {
                //localLog(F("otaEnabled: "));
                if(request->getParam("otaEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  otaEnabled = true;
                  //localLogLn(F("enabled"));
                }
                else
                {
                  otaEnabled = false;
                  //localLogLn(F("disabled"));
                }
              }
              if(request->hasParam("otaAuthenticationEnabled", true))
              {
                //localLog(F("otaAuthenticationEnabled: "));
                if(request->getParam("otaAuthenticationEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  otaAuthenticationEnabled = true;
                  //localLogLn(F("enabled"));
                }
                else
                {
                  otaAuthenticationEnabled = false;
                  //localLogLn(F("disabled"));
                }
              }
            #endif
            #ifdef SUPPORT_BATTERY_METER
              if(request->hasParam("enableBatteryMonitor", true))
              {
                if(request->getParam("enableBatteryMonitor", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  enableBatteryMonitor = true;
                }
                else
                {
                  enableBatteryMonitor = false;
                }
              }
              if(request->hasParam("topLadderResistor", true))
              {
                topLadderResistor = request->getParam("topLadderResistor", true)->value().toFloat();
              }
              if(request->hasParam("bottomLadderResistor", true))
              {
                bottomLadderResistor = request->getParam("bottomLadderResistor", true)->value().toFloat();
              }
            #endif
            #if defined(SUPPORT_BEEPER)
              if(request->hasParam("beeperEnabled", true))
              {
                //localLog(F("beeperEnabled: "));
                if(request->getParam("beeperEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  beeperEnabled = true;
                  //localLogLn(F("enabled"));
                }
                else
                {
                  beeperEnabled = false;
                  //localLogLn(F("disabled"));
                }
              }
              #ifdef SUPPORT_BUTTON
                if(request->hasParam("beepOnPress", true))
                {
                  if(request->getParam("beepOnPress", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                  {
                    beepOnPress = true;
                  }
                  else
                  {
                    beepOnPress = false;
                  }
                }
              #endif
            #endif
            #ifdef SUPPORT_VIBRATION
              if(request->hasParam("vibrationEnabled", true))
              {
                if(request->getParam("vibrationEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  vibrationEnabled = true;
                }
                else
                {
                  vibrationEnabled = false;
                }
              }
              if(request->hasParam("vibrationLevel", true))
              {
                vibrationLevel = request->getParam("vibrationLevel", true)->value().toInt();
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
                }
              }
              if(request->hasParam("startWiFiClientOnBoot", true))
              {
                //localLog(F("startWiFiClientOnBoot: "));
                if(request->getParam("startWiFiClientOnBoot", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  startWiFiClientOnBoot = true;
                  //localLogLn(F("enabled"));
                }
                else
                {
                  startWiFiClientOnBoot = false;
                  //localLogLn(F("disabled"));
                }
              }
              if(request->hasParam("wiFiClientInactivityTimer", true))
              {
                wiFiClientInactivityTimer = request->getParam("wiFiClientInactivityTimer", true)->value().toInt();
                //localLog(F("wiFiClientInactivityTimer: "));
                //localLogLn(wiFiClientInactivityTimer);
              }
              if(request->hasParam("timeServer", true))
              {
                if(timeServer != nullptr)
                {
                  delete [] timeServer;
                }
                timeServer = new char[request->getParam("timeServer", true)->value().length() + 1];
                strlcpy(timeServer,request->getParam("timeServer", true)->value().c_str(),request->getParam("timeServer", true)->value().length() + 1);
                //localLog(F("timeServer: "));
                //localLogLn(timeServer);
              }
              if(request->hasParam("timeZone", true))
              {
                if(timeZone != nullptr)
                {
                  delete [] timeZone;
                }
                timeZone = new char[request->getParam("timeZone", true)->value().length() + 1];
                strlcpy(timeZone,request->getParam("timeZone", true)->value().c_str(),request->getParam("timeZone", true)->value().length() + 1);
                //localLog(F("timeZone: "));
                //localLogLn(timeZone);
              }
              if(request->hasParam("APSSID", true))
              {
                if(APSSID != nullptr)
                {
                  delete [] APSSID;
                }
                APSSID = new char[request->getParam("APSSID", true)->value().length() + 1];
                strlcpy(APSSID,request->getParam("APSSID", true)->value().c_str(),request->getParam("APSSID", true)->value().length() + 1);
              }
              if(request->hasParam("APPSK", true))
              {
                if(request->getParam("APPSK", true)->value().length() > 0)
                {
                  if(APPSK != nullptr)
                  {
                    delete [] APPSK;
                  }
                  APPSK = new char[request->getParam("APPSK", true)->value().length() + 1];
                  strlcpy(APPSK,request->getParam("APPSK", true)->value().c_str(),request->getParam("APPSK", true)->value().length() + 1);
                }
              }
              if(request->hasParam("startWiFiApOnBoot", true))
              {
                if(request->getParam("startWiFiApOnBoot", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  startWiFiApOnBoot = true;
                }
                else
                {
                  startWiFiApOnBoot = false;
                }
              }
              if(request->hasParam("enableCaptivePortal", true))
              {
                if(request->getParam("enableCaptivePortal", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  enableCaptivePortal = true;
                }
                else
                {
                  enableCaptivePortal = false;
                }
              }
              if(request->hasParam("softApChannel", true))
              {
                softApChannel = request->getParam("softApChannel", true)->value().toInt();
              }
            #endif
            #if defined(ACT_AS_TRACKER)    
              if(request->hasParam("trackingSensitivity", true))
              {
                trackingSensitivity = request->getParam("trackingSensitivity", true)->value().toInt();
                //localLog(F("maximumEffectiveRange: "));
                //localLogLn(maximumEffectiveRange);
              }
              if(request->hasParam("maximumEffectiveRange", true))
              {
                maximumEffectiveRange = request->getParam("maximumEffectiveRange", true)->value().toInt();
                //localLog(F("maximumEffectiveRange: "));
                //localLogLn(maximumEffectiveRange);
              }
            #endif
            #if defined(SUPPORT_BEEPER)
              if(request->hasParam("beeperEnabled", true))
              {
                //localLog(F("beeperEnabled: "));
                if(request->getParam("beeperEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  beeperEnabled = true;
                  //localLogLn(F("enabled"));
                }
                else
                {
                  beeperEnabled = false;
                  //localLogLn(F("disabled"));
                }
              }
            #endif
            if(request->hasParam("loggingBufferSize", true))
            {
              if(loggingBufferSize != request->getParam("loggingBufferSize", true)->value().toInt())
              {
                loggingBufferSize = request->getParam("loggingBufferSize", true)->value().toInt();
                //localLog(F("loggingBufferSize: "));
                //localLogLn(loggingBufferSize);
              }
            }
            if(request->hasParam("logFlushThreshold", true))
            {
              if(logFlushThreshold != request->getParam("logFlushThreshold", true)->value().toInt())
              {
                logFlushThreshold = request->getParam("logFlushThreshold", true)->value().toInt();
                //localLog(F("logFlushThreshold: "));
                //localLogLn(logFlushThreshold);
              }
            }
            if(request->hasParam("logFlushInterval", true))
            {
              if(logFlushInterval != request->getParam("logFlushInterval", true)->value().toInt())
              {
                logFlushInterval = request->getParam("logFlushInterval", true)->value().toInt();
                //localLog(F("logFlushInterval: "));
                //localLogLn(logFlushInterval);
              }
            }
            if(configurationChanged())
            {
              saveConfigurationSoon = millis();
            }
            request->redirect("/admin");
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
      });
      #ifdef ACT_AS_BEACON
        adminWebServer->on("/icconfiguration", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              AsyncResponseStream *response = request->beginResponseStream("text/html");
              addPageHeader(response, 0, nullptr);
              //Start of form
              response->print(F("<form method=\"POST\">"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a></div><div class=\"four columns\"><input class=\"button-primary\" type=\"submit\" value=\"Save\" style=\"width: 100%;\"></div></div>"));
              response->print(F("<h2>In-game configuration</h2>"));
              if(device[0].icName != nullptr)
              {
                response->printf_P(PSTR("<div class=\"row\"><div class=\"twelve columns\"><label for=\"icName\">IC name</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"icName\" name=\"icName\"></div></div>"), device[0].icName);
              }
              else
              {
                response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"icName\">IC name</label><input class=\"u-full-width\" type=\"text\" value=\"\" id=\"icName\" name=\"icName\"></div></div>"));
              }
              if(device[0].icDescription != nullptr)
              {
                response->printf_P(PSTR("<div class=\"row\"><div class=\"twelve columns\"><label for=\"icDescription\">IC description</label><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"icDescription\" name=\"icDescription\"></div></div>"), device[0].icDescription);
              }
              else
              {
                response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"icDescription\">IC description</label><input class=\"u-full-width\" type=\"text\" value=\"\" id=\"icDescription\" name=\"icDescription\"></div></div>"));
              }
              response->printf_P(PSTR("<div class=\"row\"><div class=\"twelve columns\"><label for=\"diameter\">Diameter(m)</label><input class=\"u-full-width\" type=\"text\" value=\"%u\" id=\"diameter\" name=\"diameter\"></div>"), diameter);
              response->print(F("</form>"));
              addPageFooter(response);
              //Send response
              request->send(response);
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
          });
          adminWebServer->on("/icconfiguration", HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
            #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
              {
            #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              #ifdef DEBUG_FORM_SUBMISSION
                int params = request->params();
                localLog(F("Submitted Configuration parameters: "));
                localLogLn(params);
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
              #endif
              //Read the submitted configuration
              if(request->hasParam("icName", true))
              {
                if(device[0].icName != nullptr)
                {
                  delete [] device[0].icName;
                }
                device[0].icName = new char[request->getParam("icName", true)->value().length() + 1];
                strlcpy(device[0].icName,request->getParam("icName", true)->value().c_str(),request->getParam("icName", true)->value().length() + 1);
              }
              if(request->hasParam("icName", true))
              {
                if(device[0].icDescription != nullptr)
                {
                  delete [] device[0].icDescription;
                }
                device[0].icDescription = new char[request->getParam("icDescription", true)->value().length() + 1];
                strlcpy(device[0].icDescription,request->getParam("icDescription", true)->value().c_str(),request->getParam("icDescription", true)->value().length() + 1);
              }
              if(request->hasParam("diameter", true))
              {
                if(logFlushThreshold != request->getParam("diameter", true)->value().toInt())
                {
                  device[0].diameter = request->getParam("diameter", true)->value().toInt();
                }
              }
              saveConfigurationSoon = millis();
              request->redirect("/admin");
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
        });
      #endif
      #ifdef SUPPORT_ESPNOW
          adminWebServer->on("/espnowconfiguration", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              AsyncResponseStream *response = request->beginResponseStream("text/html");
              addPageHeader(response, 0, nullptr);
              //Start of form
              response->print(F("<form method=\"POST\">"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a></div><div class=\"four columns\"><input class=\"button-primary\" type=\"submit\" value=\"Save\" style=\"width: 100%;\"></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>ESP-Now configuration</h2></div></div>"));
              //ESP-Now
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"espNowEnabled\">ESP-Now radio enabled</label><select class=\"u-full-width\" id=\"espNowEnabled\" name=\"espNowEnabled\">"));
              response->print(F("<option value=\"true\""));response->print(espNowEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(espNowEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //Channel
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"espNowPreferredChannel\">Preferred channel</label><select class=\"u-full-width\" id=\"espNowPreferredChannel\" name=\"espNowPreferredChannel\">"));
              response->print(F("<option value=\"1\""));response->print(espNowPreferredChannel == 1 ? " selected>":">");response->print(F("1</option>"));
              response->print(F("<option value=\"6\""));response->print(espNowPreferredChannel == 6 ? " selected>":">");response->print(F("6</option>"));
              response->print(F("<option value=\"11\""));response->print(espNowPreferredChannel == 11 ? " selected>":">");response->print(F("11</option>"));
              response->print(F("</select></div></div>"));
              //espNow beacon interval 1
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"espNowPerimiter1\">ESP-Now perimiter 1</label><select class=\"u-full-width\" id=\"espNowPerimiter1\" name=\"espNowPerimiter1\">"));
              response->print(F("<option value=\"10\""));response->print(espNowPerimiter1 == 10 ? " selected>":">");response->print(F("10m</option>"));
              response->print(F("<option value=\"15\""));response->print(espNowPerimiter1 == 15 ? " selected>":">");response->print(F("15m</option>"));
              response->print(F("<option value=\"20\""));response->print(espNowPerimiter1 == 20 ? " selected>":">");response->print(F("20m</option>"));
              response->print(F("<option value=\"25\""));response->print(espNowPerimiter1 == 25 ? " selected>":">");response->print(F("25m</option>"));
              response->print(F("<option value=\"30\""));response->print(espNowPerimiter1 == 30 ? " selected>":">");response->print(F("30m</option>"));
              response->print(F("</select></div>"));
              response->print(F("<div class=\"six columns\"><label for=\"espNowLocationInterval1\">ESP-Now beacon interval 1</label><select class=\"u-full-width\" id=\"espNowLocationInterval1\" name=\"espNowLocationInterval1\">"));
              response->print(F("<option value=\"1000\""));response->print(espNowLocationInterval1 == 1000 ? " selected>":">");response->print(F("1s</option>"));
              response->print(F("<option value=\"5000\""));response->print(espNowLocationInterval1 == 5000 ? " selected>":">");response->print(F("5s</option>"));
              response->print(F("<option value=\"10000\""));response->print(espNowLocationInterval1 == 10000 ? " selected>":">");response->print(F("10s</option>"));
              response->print(F("<option value=\"15000\""));response->print(espNowLocationInterval1 == 15000 ? " selected>":">");response->print(F("15s</option>"));
              response->print(F("<option value=\"30000\""));response->print(espNowLocationInterval1 == 30000 ? " selected>":">");response->print(F("30s</option>"));
              response->print(F("<option value=\"45000\""));response->print(espNowLocationInterval1 == 45000 ? " selected>":">");response->print(F("45s</option>"));
              response->print(F("<option value=\"60000\""));response->print(espNowLocationInterval1 == 60000 ? " selected>":">");response->print(F("60s</option>"));
              response->print(F("<option value=\"90000\""));response->print(espNowLocationInterval1 == 90000 ? " selected>":">");response->print(F("90s</option>"));
              response->print(F("</select></div></div>"));
              //espNow beacon interval 2
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"espNowPerimiter2\">ESP-Now perimiter 2</label><select class=\"u-full-width\" id=\"espNowPerimiter2\" name=\"espNowPerimiter2\">"));
              response->print(F("<option value=\"25\""));response->print(espNowPerimiter2 == 25 ? " selected>":">");response->print(F("25m</option>"));
              response->print(F("<option value=\"30\""));response->print(espNowPerimiter2 == 30 ? " selected>":">");response->print(F("30m</option>"));
              response->print(F("<option value=\"35\""));response->print(espNowPerimiter2 == 35 ? " selected>":">");response->print(F("35m</option>"));
              response->print(F("<option value=\"40\""));response->print(espNowPerimiter2 == 40 ? " selected>":">");response->print(F("40m</option>"));
              response->print(F("<option value=\"45\""));response->print(espNowPerimiter2 == 45 ? " selected>":">");response->print(F("45m</option>"));
              response->print(F("<option value=\"50\""));response->print(espNowPerimiter2 == 50 ? " selected>":">");response->print(F("50m</option>"));
              response->print(F("</select></div>"));
              response->print(F("<div class=\"six columns\"><label for=\"espNowLocationInterval2\">ESP-Now beacon interval 2</label><select class=\"u-full-width\" id=\"espNowLocationInterval2\" name=\"espNowLocationInterval2\">"));
              response->print(F("<option value=\"1000\""));response->print(espNowLocationInterval2 == 1000 ? " selected>":">");response->print(F("1s</option>"));
              response->print(F("<option value=\"5000\""));response->print(espNowLocationInterval2 == 5000 ? " selected>":">");response->print(F("5s</option>"));
              response->print(F("<option value=\"10000\""));response->print(espNowLocationInterval2 == 10000 ? " selected>":">");response->print(F("10s</option>"));
              response->print(F("<option value=\"15000\""));response->print(espNowLocationInterval2 == 15000 ? " selected>":">");response->print(F("15s</option>"));
              response->print(F("<option value=\"30000\""));response->print(espNowLocationInterval2 == 30000 ? " selected>":">");response->print(F("30s</option>"));
              response->print(F("<option value=\"45000\""));response->print(espNowLocationInterval2 == 45000 ? " selected>":">");response->print(F("45s</option>"));
              response->print(F("<option value=\"60000\""));response->print(espNowLocationInterval2 == 60000 ? " selected>":">");response->print(F("60s</option>"));
              response->print(F("<option value=\"90000\""));response->print(espNowLocationInterval2 == 90000 ? " selected>":">");response->print(F("90s</option>"));
              response->print(F("</select></div></div>"));
              //espNow beacon interval 3
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"espNowPerimiter3\">ESP-Now perimiter 3</label><select class=\"u-full-width\" id=\"espNowPerimiter3\" name=\"espNowPerimiter3\">"));
              response->print(F("<option value=\"50\""));response->print(espNowPerimiter3 == 50 ? " selected>":">");response->print(F("50m</option>"));
              response->print(F("<option value=\"75\""));response->print(espNowPerimiter3 == 75 ? " selected>":">");response->print(F("75m</option>"));
              response->print(F("<option value=\"100\""));response->print(espNowPerimiter3 == 100 ? " selected>":">");response->print(F("100m</option>"));
              response->print(F("<option value=\"150\""));response->print(espNowPerimiter3 == 150 ? " selected>":">");response->print(F("150m</option>"));
              response->print(F("</select></div>"));
              response->print(F("<div class=\"six columns\"><label for=\"espNowLocationInterval3\">ESP-Now beacon interval 3</label><select class=\"u-full-width\" id=\"espNowLocationInterval3\" name=\"espNowLocationInterval3\">"));
              response->print(F("<option value=\"1000\""));response->print(espNowLocationInterval3 == 1000 ? " selected>":">");response->print(F("1s</option>"));
              response->print(F("<option value=\"5000\""));response->print(espNowLocationInterval3 == 5000 ? " selected>":">");response->print(F("5s</option>"));
              response->print(F("<option value=\"10000\""));response->print(espNowLocationInterval3 == 10000 ? " selected>":">");response->print(F("10s</option>"));
              response->print(F("<option value=\"15000\""));response->print(espNowLocationInterval3 == 15000 ? " selected>":">");response->print(F("15s</option>"));
              response->print(F("<option value=\"30000\""));response->print(espNowLocationInterval3 == 30000 ? " selected>":">");response->print(F("30s</option>"));
              response->print(F("<option value=\"45000\""));response->print(espNowLocationInterval3 == 45000 ? " selected>":">");response->print(F("45s</option>"));
              response->print(F("<option value=\"60000\""));response->print(espNowLocationInterval3 == 60000 ? " selected>":">");response->print(F("60s</option>"));
              response->print(F("<option value=\"90000\""));response->print(espNowLocationInterval3 == 90000 ? " selected>":">");response->print(F("90s</option>"));
              response->print(F("</select></div></div>"));
              //Default beacon interval
              response->print(F("<div class=\"row\"><div class=\"six columns\">Default ESP-Now beacon interval</div>"));
              //response->print(F("<div class=\"six columns\"><label for=\"defaultEspNowLocationInterval\">ESP-Now beacon interval</label><select class=\"u-full-width\" id=\"defaultEspNowLocationInterval\" name=\"defaultEspNowLocationInterval\">"));
              response->print(F("<div class=\"six columns\"><select class=\"u-full-width\" id=\"defaultEspNowLocationInterval\" name=\"defaultEspNowLocationInterval\">"));
              response->print(F("<option value=\"30000\""));response->print(defaultEspNowLocationInterval == 30000 ? " selected>":">");response->print(F("30s</option>"));
              response->print(F("<option value=\"60000\""));response->print(defaultEspNowLocationInterval == 60000 ? " selected>":">");response->print(F("60s</option>"));
              response->print(F("<option value=\"90000\""));response->print(defaultEspNowLocationInterval == 90000 ? " selected>":">");response->print(F("90s</option>"));
              response->print(F("<option value=\"120000\""));response->print(defaultEspNowLocationInterval == 120000 ? " selected>":">");response->print(F("2m</option>"));
              response->print(F("<option value=\"180000\""));response->print(defaultEspNowLocationInterval == 180000 ? " selected>":">");response->print(F("3m</option>"));
              response->print(F("</select></div></div>"));
              //Config sync
              response->print(F("<div class=\"row\"><div class=\"six columns\">Configuration sync interval</div>"));
              response->print(F("<div class=\"six columns\"><select class=\"u-full-width\" id=\"espNowDeviceInfoInterval\" name=\"espNowDeviceInfoInterval\">"));
              response->print(F("<option value=\"60000\""));response->print(espNowDeviceInfoInterval == 60000 ? " selected>":">");response->print(F("60s</option>"));
              response->print(F("<option value=\"90000\""));response->print(espNowDeviceInfoInterval == 90000 ? " selected>":">");response->print(F("90s</option>"));
              response->print(F("<option value=\"180000\""));response->print(espNowDeviceInfoInterval == 180000 ? " selected>":">");response->print(F("3m</option>"));
              response->print(F("<option value=\"120000\""));response->print(espNowDeviceInfoInterval == 300000 ? " selected>":">");response->print(F("5m</option>"));
              response->print(F("</select></div></div>"));
              //End of form
              response->print(F("</form>"));
              addPageFooter(response);
              //Send response
              request->send(response);
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
          });
        adminWebServer->on("/espnowconfiguration", HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            #ifdef DEBUG_FORM_SUBMISSION
              int params = request->params();
              localLog(F("Submitted Configuration parameters: "));
              localLogLn(params);
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
            #endif
            //Read the submitted configuration
            bool espNowConfigurationChanged = false;
            if(request->hasParam("espNowEnabled", true))
            {
              if(request->getParam("espNowEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                if(espNowEnabled != true)
                {
                  espNowEnabled = true;
                  espNowConfigurationChanged = true;
                  localLog(F("espNowEnabled: "));
                  localLogLn(espNowEnabled);
                }
              }
              else
              {
                if(espNowEnabled != false)
                {
                  espNowEnabled = false;
                  espNowConfigurationChanged = true;
                  localLog(F("espNowEnabled: "));
                  localLogLn(espNowEnabled);
                }
              }
            }
            if(request->hasParam("espNowPreferredChannel", true))
            {
              if(espNowPreferredChannel != request->getParam("espNowPreferredChannel", true)->value().toInt())
              {
                espNowPreferredChannel = request->getParam("espNowPreferredChannel", true)->value().toInt();
                localLog(F("espNowPreferredChannel: "));
                localLogLn(espNowPreferredChannel);
                espNowConfigurationChanged = true;
              }
            }
            if(request->hasParam("espNowDeviceInfoInterval", true))
            {
              if(espNowDeviceInfoInterval != request->getParam("espNowDeviceInfoInterval", true)->value().toInt())
              {
                espNowDeviceInfoInterval = request->getParam("espNowDeviceInfoInterval", true)->value().toInt();
                localLog(F("espNowDeviceInfoInterval: "));
                localLogLn(espNowDeviceInfoInterval);
                espNowConfigurationChanged = true;
              }
            }
            if(request->hasParam("defaultEspNowLocationInterval", true))
            {
              if(defaultEspNowLocationInterval != request->getParam("defaultEspNowLocationInterval", true)->value().toInt())
              {
                defaultEspNowLocationInterval = request->getParam("defaultEspNowLocationInterval", true)->value().toInt();
                localLog(F("defaultEspNowLocationInterval: "));
                localLogLn(defaultEspNowLocationInterval);
                espNowConfigurationChanged = true;
              }
            }
            if(request->hasParam("espNowPerimiter1", true))
            {
              if(espNowPerimiter1 != request->getParam("espNowPerimiter1", true)->value().toInt())
              {
                espNowPerimiter1 = request->getParam("espNowPerimiter1", true)->value().toInt();
                localLog(F("espNowPerimiter1: "));
                localLogLn(espNowPerimiter1);
                espNowConfigurationChanged = true;
              }
            }
            if(request->hasParam("espNowPerimiter2", true))
            {
              if(espNowPerimiter2 != request->getParam("espNowPerimiter2", true)->value().toInt())
              {
                espNowPerimiter2 = request->getParam("espNowPerimiter2", true)->value().toInt();
                localLog(F("espNowPerimiter2: "));
                localLogLn(espNowPerimiter2);
                espNowConfigurationChanged = true;
              }
            }
            if(request->hasParam("espNowPerimiter3", true))
            {
              if(espNowPerimiter3 != request->getParam("espNowPerimiter3", true)->value().toInt())
              {
                espNowPerimiter3 = request->getParam("espNowPerimiter3", true)->value().toInt();
                localLog(F("espNowPerimiter3: "));
                localLogLn(espNowPerimiter3);
                espNowConfigurationChanged = true;
              }
            }
            if(request->hasParam("espNowLocationInterval1", true))
            {
              if(espNowLocationInterval1 != request->getParam("espNowLocationInterval1", true)->value().toInt())
              {
                espNowLocationInterval1 = request->getParam("espNowLocationInterval1", true)->value().toInt();
                localLog(F("espNowLocationInterval1: "));
                localLogLn(espNowLocationInterval1);
                espNowConfigurationChanged = true;
              }
            }
            if(request->hasParam("espNowLocationInterval2", true))
            {
              if(espNowLocationInterval2 != request->getParam("espNowLocationInterval2", true)->value().toInt())
              {
                espNowLocationInterval2 = request->getParam("espNowLocationInterval2", true)->value().toInt();
                localLog(F("espNowLocationInterval2: "));
                localLogLn(espNowLocationInterval2);
                espNowConfigurationChanged = true;
              }
            }
            if(request->hasParam("espNowLocationInterval3", true))
            {
              if(espNowLocationInterval3 != request->getParam("espNowLocationInterval3", true)->value().toInt())
              {
                espNowLocationInterval3 = request->getParam("espNowLocationInterval3", true)->value().toInt();
                localLog(F("espNowLocationInterval3: "));
                localLogLn(espNowLocationInterval3);
                espNowConfigurationChanged = true;
              }
            }
            if(espNowConfigurationChanged == true)
            {
              saveConfigurationSoon = millis();
            }
            request->redirect("/admin");
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
      });
      #endif
      #ifdef SUPPORT_LORA
        adminWebServer->on("/loraconfiguration", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              AsyncResponseStream *response = request->beginResponseStream("text/html");
              addPageHeader(response, 0, nullptr);
              //Start of form
              response->print(F("<form method=\"POST\">"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a></div><div class=\"four columns\"><input class=\"button-primary\" type=\"submit\" value=\"Save\" style=\"width: 100%;\"></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>LoRa configuration</h2></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><label for=\"loRaEnabled\">LoRa radio enabled</label><select class=\"u-full-width\" id=\"loRaEnabled\" name=\"loRaEnabled\">"));
              response->print(F("<option value=\"true\""));response->print(loRaEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(loRaEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //LoRa beacon interval 1
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"loRaPerimiter1\">LoRa perimiter 1</label><select class=\"u-full-width\" id=\"loRaPerimiter1\" name=\"loRaPerimiter1\">"));
              response->print(F("<option value=\"10\""));response->print(loRaPerimiter1 == 10 ? " selected>":">");response->print(F("10m</option>"));
              response->print(F("<option value=\"15\""));response->print(loRaPerimiter1 == 15 ? " selected>":">");response->print(F("15m</option>"));
              response->print(F("<option value=\"20\""));response->print(loRaPerimiter1 == 20 ? " selected>":">");response->print(F("20m</option>"));
              response->print(F("<option value=\"25\""));response->print(loRaPerimiter1 == 25 ? " selected>":">");response->print(F("25m</option>"));
              response->print(F("<option value=\"30\""));response->print(loRaPerimiter1 == 30 ? " selected>":">");response->print(F("30m</option>"));
              response->print(F("</select></div>"));
              response->print(F("<div class=\"six columns\"><label for=\"loRaLocationInterval1\">LoRa beacon interval 1</label><select class=\"u-full-width\" id=\"loRaLocationInterval1\" name=\"loRaLocationInterval1\">"));
              response->print(F("<option value=\"10000\""));response->print(loRaLocationInterval1 == 10000 ? " selected>":">");response->print(F("10s</option>"));
              response->print(F("<option value=\"15000\""));response->print(loRaLocationInterval1 == 15000 ? " selected>":">");response->print(F("15s</option>"));
              response->print(F("<option value=\"30000\""));response->print(loRaLocationInterval1 == 30000 ? " selected>":">");response->print(F("30s</option>"));
              response->print(F("<option value=\"45000\""));response->print(loRaLocationInterval1 == 45000 ? " selected>":">");response->print(F("45s</option>"));
              response->print(F("<option value=\"60000\""));response->print(loRaLocationInterval1 == 60000 ? " selected>":">");response->print(F("60s</option>"));
              response->print(F("<option value=\"90000\""));response->print(loRaLocationInterval1 == 90000 ? " selected>":">");response->print(F("90s</option>"));
              response->print(F("</select></div></div>"));
              //LoRa beacon interval 2
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"loRaPerimiter2\">LoRa perimiter 2</label><select class=\"u-full-width\" id=\"loRaPerimiter2\" name=\"loRaPerimiter2\">"));
              response->print(F("<option value=\"25\""));response->print(loRaPerimiter2 == 25 ? " selected>":">");response->print(F("25m</option>"));
              response->print(F("<option value=\"30\""));response->print(loRaPerimiter2 == 30 ? " selected>":">");response->print(F("30m</option>"));
              response->print(F("<option value=\"35\""));response->print(loRaPerimiter2 == 35 ? " selected>":">");response->print(F("35m</option>"));
              response->print(F("<option value=\"40\""));response->print(loRaPerimiter2 == 40 ? " selected>":">");response->print(F("40m</option>"));
              response->print(F("<option value=\"45\""));response->print(loRaPerimiter2 == 45 ? " selected>":">");response->print(F("45m</option>"));
              response->print(F("<option value=\"50\""));response->print(loRaPerimiter2 == 50 ? " selected>":">");response->print(F("50m</option>"));
              response->print(F("</select></div>"));
              response->print(F("<div class=\"six columns\"><label for=\"loRaLocationInterval2\">LoRa beacon interval 2</label><select class=\"u-full-width\" id=\"loRaLocationInterval2\" name=\"loRaLocationInterval2\">"));
              response->print(F("<option value=\"10000\""));response->print(loRaLocationInterval2 == 10000 ? " selected>":">");response->print(F("10s</option>"));
              response->print(F("<option value=\"15000\""));response->print(loRaLocationInterval2 == 15000 ? " selected>":">");response->print(F("15s</option>"));
              response->print(F("<option value=\"30000\""));response->print(loRaLocationInterval2 == 30000 ? " selected>":">");response->print(F("30s</option>"));
              response->print(F("<option value=\"45000\""));response->print(loRaLocationInterval2 == 45000 ? " selected>":">");response->print(F("45s</option>"));
              response->print(F("<option value=\"60000\""));response->print(loRaLocationInterval2 == 60000 ? " selected>":">");response->print(F("60s</option>"));
              response->print(F("<option value=\"90000\""));response->print(loRaLocationInterval2 == 90000 ? " selected>":">");response->print(F("90s</option>"));
              response->print(F("</select></div></div>"));
              //LoRa beacon interval 3
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"loRaPerimiter3\">LoRa perimiter 3</label><select class=\"u-full-width\" id=\"loRaPerimiter3\" name=\"loRaPerimiter3\">"));
              response->print(F("<option value=\"50\""));response->print(loRaPerimiter3 == 50 ? " selected>":">");response->print(F("50m</option>"));
              response->print(F("<option value=\"75\""));response->print(loRaPerimiter3 == 75 ? " selected>":">");response->print(F("75m</option>"));
              response->print(F("<option value=\"100\""));response->print(loRaPerimiter3 == 100 ? " selected>":">");response->print(F("100m</option>"));
              response->print(F("<option value=\"150\""));response->print(loRaPerimiter3 == 150 ? " selected>":">");response->print(F("150m</option>"));
              response->print(F("</select></div>"));
              response->print(F("<div class=\"six columns\"><label for=\"loRaLocationInterval3\">LoRa beacon interval 3</label><select class=\"u-full-width\" id=\"loRaLocationInterval3\" name=\"loRaLocationInterval3\">"));
              response->print(F("<option value=\"10000\""));response->print(loRaLocationInterval3 == 10000 ? " selected>":">");response->print(F("10s</option>"));
              response->print(F("<option value=\"15000\""));response->print(loRaLocationInterval3 == 15000 ? " selected>":">");response->print(F("15s</option>"));
              response->print(F("<option value=\"30000\""));response->print(loRaLocationInterval3 == 30000 ? " selected>":">");response->print(F("30s</option>"));
              response->print(F("<option value=\"45000\""));response->print(loRaLocationInterval3 == 45000 ? " selected>":">");response->print(F("45s</option>"));
              response->print(F("<option value=\"60000\""));response->print(loRaLocationInterval3 == 60000 ? " selected>":">");response->print(F("60s</option>"));
              response->print(F("<option value=\"90000\""));response->print(loRaLocationInterval3 == 90000 ? " selected>":">");response->print(F("90s</option>"));
              response->print(F("</select></div></div>"));
              //Default beacon interval
              response->print(F("<div class=\"row\"><div class=\"six columns\">Default LoRa beacon interval</div>"));
              response->print(F("<div class=\"six columns\"><select class=\"u-full-width\" id=\"defaultLoRaLocationInterval\" name=\"defaultLoRaLocationInterval\">"));
              response->print(F("<option value=\"30000\""));response->print(defaultLoRaLocationInterval == 30000 ? " selected>":">");response->print(F("30s</option>"));
              response->print(F("<option value=\"60000\""));response->print(defaultLoRaLocationInterval == 60000 ? " selected>":">");response->print(F("60s</option>"));
              response->print(F("<option value=\"90000\""));response->print(defaultLoRaLocationInterval == 90000 ? " selected>":">");response->print(F("90s</option>"));
              response->print(F("<option value=\"120000\""));response->print(defaultLoRaLocationInterval == 120000 ? " selected>":">");response->print(F("2m</option>"));
              response->print(F("<option value=\"180000\""));response->print(defaultLoRaLocationInterval == 180000 ? " selected>":">");response->print(F("3m</option>"));
              response->print(F("</select></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"six columns\">Configuration sync interval</div>"));
              response->print(F("<div class=\"six columns\"><select class=\"u-full-width\" id=\"loRaDeviceInfoInterval\" name=\"loRaDeviceInfoInterval\">"));
              response->print(F("<option value=\"60000\""));response->print(loRaDeviceInfoInterval == 60000 ? " selected>":">");response->print(F("60s</option>"));
              response->print(F("<option value=\"90000\""));response->print(loRaDeviceInfoInterval == 90000 ? " selected>":">");response->print(F("90s</option>"));
              response->print(F("<option value=\"180000\""));response->print(loRaDeviceInfoInterval == 180000 ? " selected>":">");response->print(F("3m</option>"));
              response->print(F("<option value=\"120000\""));response->print(loRaDeviceInfoInterval == 300000 ? " selected>":">");response->print(F("5m</option>"));
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
              //End of form
              response->print(F("</form>"));
              addPageFooter(response);
              //Send response
              request->send(response);
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
          });
          adminWebServer->on("/loraconfiguration", HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
            #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
              {
            #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              #ifdef DEBUG_FORM_SUBMISSION
                int params = request->params();
                localLog(F("Submitted Configuration parameters: "));
                localLogLn(params);
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
              #endif
              //Read the submitted configuration
              bool loRaConfigurationChanged = false;
              if(request->hasParam("loRaEnabled", true))
              {
                if(request->getParam("loRaEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  if(loRaEnabled == false)
                  {
                    loRaEnabled = true;
                    localLog(F("loRaEnabled: "));
                    localLogLn(loRaEnabled);
                    loRaConfigurationChanged = true;
                  }
                }
                else
                {
                  if(loRaEnabled == true)
                  {
                    loRaEnabled = false;
                    localLog(F("loRaEnabled: "));
                    localLogLn(loRaEnabled);
                    loRaConfigurationChanged = true;
                  }
                }
              }
              if(request->hasParam("loRaDeviceInfoInterval", true))
              {
                if(loRaDeviceInfoInterval != request->getParam("loRaDeviceInfoInterval", true)->value().toInt())
                {
                  loRaDeviceInfoInterval = request->getParam("loRaDeviceInfoInterval", true)->value().toInt();
                  localLog(F("loRaDeviceInfoInterval: "));
                  localLogLn(loRaDeviceInfoInterval);
                  loRaConfigurationChanged = true;
                }
              }
              if(request->hasParam("defaultLoRaLocationInterval", true))
              {
                if(defaultLoRaLocationInterval != request->getParam("defaultLoRaLocationInterval", true)->value().toInt())
                {
                  defaultLoRaLocationInterval = request->getParam("defaultLoRaLocationInterval", true)->value().toInt();
                  localLog(F("defaultLoRaLocationInterval: "));
                  localLogLn(defaultLoRaLocationInterval);
                  loRaConfigurationChanged = true;
                }
              }
              if(request->hasParam("loRaPerimiter1", true))
              {
                if(loRaPerimiter1 != request->getParam("loRaPerimiter1", true)->value().toInt())
                {
                  loRaPerimiter1 = request->getParam("loRaPerimiter1", true)->value().toInt();
                  localLog(F("loRaPerimiter1: "));
                  localLogLn(loRaPerimiter1);
                  loRaConfigurationChanged = true;
                }
              }
              if(request->hasParam("loRaPerimiter2", true))
              {
                if(loRaPerimiter2 != request->getParam("loRaPerimiter2", true)->value().toInt())
                {
                  loRaPerimiter2 = request->getParam("loRaPerimiter2", true)->value().toInt();
                  localLog(F("loRaPerimiter2: "));
                  localLogLn(loRaPerimiter2);
                  loRaConfigurationChanged = true;
                }
              }
              if(request->hasParam("loRaPerimiter3", true))
              {
                if(loRaPerimiter3 != request->getParam("loRaPerimiter3", true)->value().toInt())
                {
                  loRaPerimiter3 = request->getParam("loRaPerimiter3", true)->value().toInt();
                  localLog(F("loRaPerimiter3: "));
                  localLogLn(loRaPerimiter3);
                  loRaConfigurationChanged = true;
                }
              }
              if(request->hasParam("loRaLocationInterval1", true))
              {
                if(loRaLocationInterval1 != request->getParam("loRaLocationInterval1", true)->value().toInt())
                {
                  loRaLocationInterval1 = request->getParam("loRaLocationInterval1", true)->value().toInt();
                  localLog(F("loRaLocationInterval1: "));
                  localLogLn(loRaLocationInterval1);
                  loRaConfigurationChanged = true;
                }
              }
              if(request->hasParam("loRaLocationInterval2", true))
              {
                if(loRaLocationInterval2 != request->getParam("loRaLocationInterval2", true)->value().toInt())
                {
                  loRaLocationInterval2 = request->getParam("loRaLocationInterval2", true)->value().toInt();
                  localLog(F("loRaLocationInterval2: "));
                  localLogLn(loRaLocationInterval2);
                  loRaConfigurationChanged = true;
                }
              }
              if(request->hasParam("loRaLocationInterval3", true))
              {
                if(loRaLocationInterval3 != request->getParam("loRaLocationInterval3", true)->value().toInt())
                {
                  loRaLocationInterval3 = request->getParam("loRaLocationInterval3", true)->value().toInt();
                  localLog(F("loRaLocationInterval3: "));
                  localLogLn(loRaLocationInterval3);
                  loRaConfigurationChanged = true;
                }
              }
              if(request->hasParam("rssiAttenuation", true))
              {
                if(rssiAttenuation != request->getParam("rssiAttenuation", true)->value().toFloat())
                {
                  rssiAttenuation = request->getParam("rssiAttenuation", true)->value().toFloat();
                  localLog(F("rssiAttenuation: "));
                  localLogLn(rssiAttenuation);
                  loRaConfigurationChanged = true;
                }
              }
              if(request->hasParam("rssiAttenuationBaseline", true))
              {
                if(rssiAttenuationBaseline != request->getParam("rssiAttenuationBaseline", true)->value().toFloat())
                {
                  rssiAttenuationBaseline = request->getParam("rssiAttenuationBaseline", true)->value().toFloat();
                  localLog(F("rssiAttenuationBaseline: "));
                  localLogLn(rssiAttenuationBaseline);
                  loRaConfigurationChanged = true;
                }
              }
              if(request->hasParam("rssiAttenuationPerimeter", true))
              {
                if(rssiAttenuationPerimeter != request->getParam("rssiAttenuationPerimeter", true)->value().toFloat())
                {
                  rssiAttenuationPerimeter = request->getParam("rssiAttenuationPerimeter", true)->value().toFloat();
                  localLog(F("rssiAttenuationPerimeter: "));
                  localLogLn(rssiAttenuationPerimeter);
                  loRaConfigurationChanged = true;
                }
              }
              if(loRaConfigurationChanged == true)
              {
                saveConfigurationSoon = millis();
              }
              request->redirect("/admin");
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
        });
      #endif
      #ifdef SUPPORT_FTM
        adminWebServer->on("/ftmconfiguration", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              AsyncResponseStream *response = request->beginResponseStream("text/html");
              addPageHeader(response, 0, nullptr);
              //Start of form
              response->print(F("<form method=\"POST\">"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a></div><div class=\"four columns\"><input class=\"button-primary\" type=\"submit\" value=\"Save\" style=\"width: 100%;\"></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>FTM configuration</h2></div></div>"));
              //FTM
              #if defined(ACT_AS_TRACKER)
                //State
                response->print(F("<div class=\"row\"><div class=\"six columns\">Time-of-flight probe</div>"));
                response->print(F("<div class=\"six columns\"><select class=\"u-full-width\" id=\"ftmEnabled\" name=\"ftmEnabled\">"));
                response->print(F("<option value=\"true\""));response->print(ftmEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
                response->print(F("<option value=\"false\""));response->print(ftmEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
                response->print(F("</select></div></div>"));
                //SSID/PSK
                response->print(F("<div class=\"row\"><div class=\"six columns\">SSID template (simple pattern search)</div>"));
                if(ftmSSID != nullptr)
                {
                  response->printf_P(PSTR("<div class=\"six columns\"><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"ftmSSID\" name=\"ftmSSID\"></div>"), ftmSSID);
                }
                else
                {
                  response->print(F("<div class=\"six columns\"><input class=\"u-full-width\" type=\"text\" value=\"\" id=\"ftmSSID\" name=\"ftmSSID\"></div>"));
                }
                response->print(F("<div class=\"row\"><div class=\"six columns\">Pre-shared key to try</div>"));
                response->print(F("<div class=\"six columns\"><input class=\"u-full-width\" type=\"password\" placeholder=\"********\" id=\"ftmPSK\" name=\"ftmPSK\"></div></div>"));
              #elif defined(ACT_AS_BEACON)
                //State
                response->print(F("<div class=\"row\"><div class=\"six columns\">Time-of-flight beacon</div>"));
                response->print(F("<div class=\"six columns\"><select class=\"u-full-width\" id=\"ftmEnabled\" name=\"ftmEnabled\">"));
                response->print(F("<option value=\"true\""));response->print(ftmEnabled == true ? " selected>":">");response->print(F("Enabled</option>"));
                response->print(F("<option value=\"false\""));response->print(ftmEnabled == false ? " selected>":">");response->print(F("Disabled</option>"));
                response->print(F("</select></div></div>"));
                response->print(F("<div class=\"row\"><div class=\"six columns\">SSID suffix (appended to any configured SSID)</div>"));
                if(ftmSSID != nullptr)
                {
                  response->printf_P(PSTR("<div class=\"six columns\"><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"ftmSSID\" name=\"ftmSSID\"></div>"), ftmSSID);
                }
                else
                {
                  response->print(F("<div class=\"six columns\"><input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"ftmSSID\" name=\"ftmSSID\"></div>"));
                }
                response->print(F("<div class=\"row\"><div class=\"six columns\">Hide FTM SSID (unless otherwise enabled)</div>"));
                response->print(F("<div class=\"six columns\"><select class=\"u-full-width\" id=\"ftmHideSSID\" name=\"ftmHideSSID\">"));
                response->print(F("<option value=\"true\""));response->print(ftmHideSSID == true ? " selected>":">");response->print(F("Enabled</option>"));
                response->print(F("<option value=\"false\""));response->print(ftmHideSSID == false ? " selected>":">");response->print(F("Disabled</option>"));
                response->print(F("</select></div></div>"));
                response->print(F("<div class=\"row\"><div class=\"six columns\">Pre-shared key (if not otherwise set)</div>"));
                response->print(F("<div class=\"six columns\"><input class=\"u-full-width\" type=\"password\" placeholder=\"********\" id=\"ftmPSK\" name=\"ftmPSK\"></div></div>"));
              #endif
              //End of form
              response->print(F("</form>"));
              addPageFooter(response);
              //Send response
              request->send(response);
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
          });
          adminWebServer->on("/ftmconfiguration", HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
            #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
              {
            #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              #ifdef DEBUG_FORM_SUBMISSION
                int params = request->params();
                localLog(F("Submitted Configuration parameters: "));
                localLogLn(params);
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
              #endif
              bool ftmConfigurationChanged = false;
              if(request->hasParam("ftmEnabled", true))
              {
                if(request->getParam("ftmEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  if(ftmEnabled != true)
                  {
                    ftmEnabled = true;
                    ftmConfigurationChanged = true;
                    localLog(F("ftmEnabled: "));
                    localLogLn(ftmEnabled);
                  }
                }
                else
                {
                  if(ftmEnabled != false)
                  {
                    ftmEnabled = false;
                    ftmConfigurationChanged = true;
                    localLog(F("ftmEnabled: "));
                    localLogLn(ftmEnabled);
                  }
                }
              }
              if(request->hasParam("ftmSSID", true))
              {
                if(request->getParam("ftmSSID", true)->value().equals(String(ftmSSID)) == false)
                {
                  if(ftmSSID != nullptr)
                  {
                    delete [] ftmSSID;
                  }
                  ftmSSID = new char[request->getParam("ftmSSID", true)->value().length() + 1];
                  strlcpy(ftmSSID,request->getParam("ftmSSID", true)->value().c_str(),request->getParam("ftmSSID", true)->value().length() + 1);
                  localLog(F("ftmSSID: "));
                  localLogLn(ftmSSID);
                  ftmConfigurationChanged = true;
                }
              }
              if(request->hasParam("ftmHideSSID", true))
              {
                if(request->getParam("ftmHideSSID", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  if(ftmHideSSID != true)
                  {
                    ftmHideSSID = true;
                    ftmConfigurationChanged = true;
                    localLog(F("ftmHideSSID: "));
                    localLogLn(ftmHideSSID);
                  }
                }
                else
                {
                  if(ftmHideSSID != false)
                  {
                    ftmHideSSID = false;
                    ftmConfigurationChanged = true;
                    localLog(F("ftmHideSSID: "));
                    localLogLn(ftmHideSSID);
                  }
                }
              }
              if(request->hasParam("ftmPSK", true))
              {
                if(request->getParam("ftmPSK", true)->value().length() > 0)
                {
                  if(ftmPSK != nullptr)
                  {
                    delete [] ftmPSK;
                  }
                  ftmPSK = new char[request->getParam("ftmPSK", true)->value().length() + 1];
                  strlcpy(ftmPSK,request->getParam("ftmPSK", true)->value().c_str(),request->getParam("ftmPSK", true)->value().length() + 1);
                  localLog(F("ftmPSK: "));
                  localLogLn(ftmPSK);
                  ftmConfigurationChanged = true;
                }
              }
              if(ftmConfigurationChanged == true)
              {
                saveConfigurationSoon = millis();
              }
              request->redirect("/admin");
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
        });
      #endif
      #ifdef SUPPORT_GPS
        adminWebServer->on("/gpsconfiguration", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              AsyncResponseStream *response = request->beginResponseStream("text/html");
              addPageHeader(response, 0, nullptr);
              //Start of form
              response->print(F("<form method=\"POST\">"));
              response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a></div><div class=\"four columns\"><input class=\"button-primary\" type=\"submit\" value=\"Save\" style=\"width: 100%;\"></div></div>"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>GPS configuration</h2></div></div>"));
              #ifdef SUPPORT_SOFT_PERIPHERAL_POWER_OFF
                response->print(F("<div class=\"row\"><div class=\"six columns\">Switch off GPS when stationary after</div>"));
                response->print(F("<div class=\"six columns\"><select class=\"u-full-width\" id=\"gpsStationaryTimeout\" name=\"gpsStationaryTimeout\">"));
                response->print(F("<option value=\"0\""));response->print(gpsStationaryTimeout == 0 ? " selected>":">");response->print(F("Never</option>"));
                response->print(F("<option value=\"60000\""));response->print(gpsStationaryTimeout == 60000 ? " selected>":">");response->print(F("1m</option>"));
                response->print(F("<option value=\"180000\""));response->print(gpsStationaryTimeout == 180000 ? " selected>":">");response->print(F("3m</option>"));
                response->print(F("<option value=\"120000\""));response->print(gpsStationaryTimeout == 300000 ? " selected>":">");response->print(F("5m</option>"));
                response->print(F("</select></div></div>"));
                response->print(F("<div class=\"row\"><div class=\"six columns\">Check for movement after</div>"));
                response->print(F("<div class=\"six columns\"><select class=\"u-full-width\" id=\"gpsCheckInterval\" name=\"gpsCheckInterval\">"));
                response->print(F("<option value=\"0\""));response->print(gpsCheckInterval == 0 ? " selected>":">");response->print(F("Never (dangerous)</option>"));
                response->print(F("<option value=\"180000\""));response->print(gpsCheckInterval == 180000 ? " selected>":">");response->print(F("3m</option>"));
                response->print(F("<option value=\"120000\""));response->print(gpsCheckInterval == 300000 ? " selected>":">");response->print(F("5m</option>"));
                response->print(F("<option value=\"900000\""));response->print(gpsCheckInterval == 900000 ? " selected>":">");response->print(F("15m</option>"));
                response->print(F("<option value=\"1800000\""));response->print(gpsCheckInterval == 1800000 ? " selected>":">");response->print(F("30m</option>"));
                response->print(F("<option value=\"3600000\""));response->print(gpsCheckInterval == 3600000 ? " selected>":">");response->print(F("60m</option>"));
                response->print(F("</select></div></div>"));
              #endif
              response->print(F("<div class=\"row\"><div class=\"six columns\">Sync time with GPS</div>"));
              response->print(F("<div class=\"six columns\"><select class=\"u-full-width\" id=\"useGpsForTimeSync\" name=\"useGpsForTimeSync\">"));
              response->print(F("<option value=\"true\""));response->print(useGpsForTimeSync == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(useGpsForTimeSync == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //End of form
              response->print(F("</form>"));
              addPageFooter(response);
              //Send response
              request->send(response);
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
          });
          adminWebServer->on("/gpsconfiguration", HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
            #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
              {
            #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              #ifdef DEBUG_FORM_SUBMISSION
                int params = request->params();
                localLog(F("Submitted Configuration parameters: "));
                localLogLn(params);
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
              #endif
              //Read the submitted configuration
              bool gpsConfigurationChanged = false;
              if(request->hasParam("useGpsForTimeSync", true))
              {
                if(request->getParam("useGpsForTimeSync", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  if(useGpsForTimeSync != true)
                  {
                    useGpsForTimeSync = true;
                    localLog(F("useGpsForTimeSync: "));
                    localLogLn(useGpsForTimeSync);
                    gpsConfigurationChanged = true;
                  }
                }
                else
                {
                  if(useGpsForTimeSync != false)
                  {
                    useGpsForTimeSync = false;
                    localLog(F("useGpsForTimeSync: "));
                    localLogLn(useGpsForTimeSync);
                    gpsConfigurationChanged = true;
                  }
                }
              }
              #ifdef SUPPORT_SOFT_POWER_OFF
                if(request->hasParam("gpsStationaryTimeout", true))
                {
                  if(gpsStationaryTimeout != request->getParam("gpsStationaryTimeout", true)->value().toInt())
                  {
                    gpsStationaryTimeout = request->getParam("gpsStationaryTimeout", true)->value().toInt();
                    localLog(F("gpsStationaryTimeout: "));
                    localLogLn(gpsStationaryTimeout);
                    gpsConfigurationChanged = true;
                  }
                }
                if(request->hasParam("gpsCheckInterval", true))
                {
                  if(gpsCheckInterval != request->getParam("gpsCheckInterval", true)->value().toInt())
                  {
                    gpsCheckInterval = request->getParam("gpsCheckInterval", true)->value().toInt();
                    localLog(F("gpsCheckInterval: "));
                    localLogLn(gpsCheckInterval);
                    gpsConfigurationChanged = true;
                  }
                }
              #endif
              if(gpsConfigurationChanged == true)
              {
                saveConfigurationSoon = millis();
              }
              request->redirect("/admin");
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
        });
      #endif
      
      #if defined(ENABLE_LOCAL_WEBSERVER_FIRMWARE_UPDATE)
        adminWebServer->on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
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
              response->printf_P(PSTR("<li>Current firmware: %u.%u.%u</li>"), device[0].majorVersion, device[0].minorVersion, device[0].patchVersion);
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
              response->print(F("<input class=\"button-primary\" type=\"file\" name=\"update\"><br /><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a> <input class=\"button-primary\" type=\"submit\" value=\"Update\" style=\"width: 100%;\"></form>"));
              addPageFooter(response);
              //Send response
              request->send(response);
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
        });
        adminWebServer->on("/update", HTTP_POST,[](AsyncWebServerRequest *request)
          { //This lambda function is called when the update is complete
            #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
              {
            #endif
                #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                  if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                  {
                      return request->requestAuthentication();  //Force basic authentication
                  }
                #endif
                bool updateSuccesful = !Update.hasError();
                if(updateSuccesful == true)
                {
                  #if defined(SERIAL_DEBUG)
                    SERIAL_DEBUG_PORT.println(F("Web UI software update complete, restarting shortly"));
                  #endif
                  restartTimer = millis();
                  otaInProgress = false;
                  request->redirect("/postUpdateRestart");
                }
                else
                {
                  #if defined(SERIAL_DEBUG)
                    SERIAL_DEBUG_PORT.println(F("Web UI software update failed"));
                  #endif
                  AsyncResponseStream *response = request->beginResponseStream("text/html");
                  addPageHeader(response, 0, nullptr);
                  response->print(F("<h2>Update failed</h2>"));
                  response->print(F("<p>The software update failed!</p>"));
                  response->print(F("<a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a>"));
                  addPageFooter(response);
                  //Send response
                  request->send(response);
                  otaInProgress = false;
                }
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          }
          #endif
          ,[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
          { //This lambda function is called when the update starts/continues/fails
            if(!index)  //Start of upload process
            {
              #if defined(SERIAL_DEBUG)
                SERIAL_DEBUG_PORT.print(F("Web UI software update \""));
                SERIAL_DEBUG_PORT.print(filename);
                SERIAL_DEBUG_PORT.print(F("\" uploading from "));
                SERIAL_DEBUG_PORT.println(request->client()->remoteIP().toString());
              #endif
              otaInProgress = true;
              vTaskDelay(100 / portTICK_PERIOD_MS); //Allow all the tasks to exit if still active
              killAllTasks(); //Hard kill all tasks if they didn't exit in 100ms
              if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
              {
                #if defined(SERIAL_DEBUG)
                  SERIAL_DEBUG_PORT.print(F("Update error: "));
                  SERIAL_DEBUG_PORT.println(Update.getError());
                #endif
              }
            }
            if(Update.hasError() == false)  //Received a chunk
            {
              if(Update.write(data, len) != len)  //Write to flash failed
              {
                #if defined(SERIAL_DEBUG)
                  SERIAL_DEBUG_PORT.print(F("Update error: "));
                  SERIAL_DEBUG_PORT.println(Update.getError());
                #endif
              }
              else  //Write OK
              {
                otaInProgress = true;
                #if defined(SERIAL_DEBUG)
                  SERIAL_DEBUG_PORT.print('.');
                #endif
              }
            }
            if(final) //Upload complete
            {
              #if defined(SERIAL_DEBUG)
                SERIAL_DEBUG_PORT.println(F("Done"));
              #endif
              if(Update.end(true))
              {
                #if defined(SERIAL_DEBUG)
                  SERIAL_DEBUG_PORT.print(F("Update Success, flashed: "));
                  SERIAL_DEBUG_PORT.print((index+len)/1024);
                  SERIAL_DEBUG_PORT.println(F("KB"));
                #endif
              }
              else
              {
                #if defined(SERIAL_DEBUG)
                  SERIAL_DEBUG_PORT.print(F("Update error: "));
                  SERIAL_DEBUG_PORT.println(Update.getError());
                #endif
              }
              otaInProgress = false;
              restartAllTasks();
            }
          });
        adminWebServer->on("/postUpdateRestart", HTTP_GET, [](AsyncWebServerRequest *request){
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            AsyncResponseStream *response = request->beginResponseStream("text/html");
            addPageHeader(response, 20, "/admin"); //This sends the page to / after 20s
            response->print(F("<h2>Software update successful</h2>"));
            response->print(F("<p>The software update was successful and this node will restart in roughly 10 seconds.</p>"));
            response->print(F("<a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a>"));
            addPageFooter(response);
            //Send response
            request->send(response);
            restartTimer = millis();
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
        });
      #endif
      #if defined(ACT_AS_SENSOR)
        adminWebServer->on("/sensorConfiguration", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              AsyncResponseStream *response = request->beginResponseStream("text/html");
              addPageHeader(response, 0, nullptr);
              response->print(F("<h2>Sensor configuration</h2>"));
              //Start of form
              response->print(F("<form method=\"POST\">"));
              response->print(F("<a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a> <input class=\"button-primary\" type=\"submit\" value=\"Save\" style=\"width: 100%;\">"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Starting values</h3></div></div>"));
              //Starting hits
              response->printf_P(PSTR("<div class=\"row\"><div class=\"six columns\"><label for=\"numberOfStartingHits\">Starting hits</label><input class=\"u-full-width\" type=\"number\" min=\"1\" max=\"99\" step=\"1\" value=\"%u\" id=\"numberOfStartingHits\" name=\"numberOfStartingHits\"></div></div>"), device[0].numberOfStartingHits);
              response->printf_P(PSTR("<div class=\"row\"><div class=\"six columns\"><label for=\"numberOfStartingStunHits\">Starting stun hits</label><input class=\"u-full-width\" type=\"number\" min=\"1\" max=\"99\" step=\"1\" value=\"%u\" id=\"numberOfStartingStunHits\" name=\"numberOfStartingStunHits\"></div></div>"), device[0].numberOfStartingStunHits);
              response->printf_P(PSTR("<div class=\"row\"><div class=\"six columns\"><label for=\"armourValue\">Armour value</label><input class=\"u-full-width\" type=\"number\" min=\"0\" max=\"99\" step=\"1\" value=\"%u\" id=\"armourValue\" name=\"armourValue\"></div></div>"), armourValue);
              //Flags
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Sensor flags</h3></div></div>"));
              //Require EP
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"EP_flag\">Require EP to hit</label><select class=\"u-full-width\" id=\"EP_flag\" name=\"EP_flag\">"));
              response->print(F("<option value=\"true\""));response->print(EP_flag == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(EP_flag == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //Ignore healing
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"ig_healing_flag\">Ignore healing</label><select class=\"u-full-width\" id=\"ig_healing_flag\" name=\"ig_healing_flag\">"));
              response->print(F("<option value=\"true\""));response->print(ig_healing_flag == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(ig_healing_flag == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //Ignore stun
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"ig_stun_flag\">Ignore stun</label><select class=\"u-full-width\" id=\"ig_stun_flag\" name=\"ig_stun_flag\">"));
              response->print(F("<option value=\"true\""));response->print(ig_stun_flag == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(ig_stun_flag == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //Ignore ongoing
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"ig_ongoing_flag\">Ignore ongoing</label><select class=\"u-full-width\" id=\"ig_ongoing_flag\" name=\"ig_ongoing_flag\">"));
              response->print(F("<option value=\"true\""));response->print(ig_ongoing_flag == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(ig_ongoing_flag == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //Regen while zero
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"regen_while_zero\">Regen while zero</label><select class=\"u-full-width\" id=\"regen_while_zero\" name=\"regen_while_zero\">"));
              response->print(F("<option value=\"true\""));response->print(regen_while_zero == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(regen_while_zero == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //Treat damage as one
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"treat_as_one\">Treat damage as one hit</label><select class=\"u-full-width\" id=\"treat_as_one\" name=\"treat_as_one\">"));
              response->print(F("<option value=\"true\""));response->print(treat_as_one == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(treat_as_one == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //Treat stun damage as one
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"treat_stun_as_one\">Treat stun as one hit</label><select class=\"u-full-width\" id=\"treat_stun_as_one\" name=\"treat_stun_as_one\">"));
              response->print(F("<option value=\"true\""));response->print(treat_stun_as_one == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(treat_stun_as_one == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //Ongoing is cumulative
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"ongoing_is_cumulative\">Ongoing is cumulative</label><select class=\"u-full-width\" id=\"ongoing_is_cumulative\" name=\"ongoing_is_cumulative\">"));
              response->print(F("<option value=\"true\""));response->print(ongoing_is_cumulative == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(ongoing_is_cumulative == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //Ignore non-DOT
              response->print(F("<div class=\"row\"><div class=\"six columns\"><label for=\"ig_non_dot\">Ignore non-DOT signals</label><select class=\"u-full-width\" id=\"ig_non_dot\" name=\"ig_non_dot\">"));
              response->print(F("<option value=\"true\""));response->print(ig_non_dot == true ? " selected>":">");response->print(F("Enabled</option>"));
              response->print(F("<option value=\"false\""));response->print(ig_non_dot == false ? " selected>":">");response->print(F("Disabled</option>"));
              response->print(F("</select></div></div>"));
              //End of form
              response->print(F("</form>"));
              addPageFooter(response);
              //Send response
              request->send(response);
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
          });
          adminWebServer->on("/sensorConfiguration", HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
            lastWifiActivity = millis();
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            #ifdef DEBUG_FORM_SUBMISSION
              int params = request->params();
              localLog(F("Submitted Configuration parameters: "));
              localLogLn(params);
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
            #endif
            //Read the submitted configuration
            //Starting values
            if(request->hasParam("numberOfStartingHits", true))
            {
              device[0].numberOfStartingHits = request->getParam("numberOfStartingHits", true)->value().toInt();
              device[0].currentNumberOfHits = device[0].numberOfStartingHits;
            }
            if(request->hasParam("numberOfStartingStunHits", true))
            {
              device[0].numberOfStartingStunHits = request->getParam("numberOfStartingStunHits", true)->value().toInt();
              device[0].currentNumberOfStunHits = device[0].numberOfStartingStunHits ;
            }
            if(request->hasParam("armourValue", true))
            {
              armourValue = request->getParam("armourValue", true)->value().toInt();
            }
            //Sensor flags
            if(request->hasParam("EP_flag", true))
            {
              if(request->getParam("EP_flag", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                EP_flag = true;
              }
              else
              {
                EP_flag = false;
              }
            }
            if(request->hasParam("ig_healing_flag", true))
            {
              if(request->getParam("ig_healing_flag", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                ig_healing_flag = true;
              }
              else
              {
                ig_healing_flag = false;
              }
            }
            if(request->hasParam("EP_flag", true))
            {
              if(request->getParam("ig_stun_flag", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                ig_stun_flag = true;
              }
              else
              {
                ig_stun_flag = false;
              }
            }
            if(request->hasParam("ig_ongoing_flag", true))
            {
              if(request->getParam("ig_ongoing_flag", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                ig_ongoing_flag = true;
              }
              else
              {
                ig_ongoing_flag = false;
              }
            }
            if(request->hasParam("regen_while_zero", true))
            {
              if(request->getParam("regen_while_zero", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                regen_while_zero = true;
              }
              else
              {
                regen_while_zero = false;
              }
            }
            if(request->hasParam("treat_as_one", true))
            {
              if(request->getParam("treat_as_one", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                treat_as_one = true;
              }
              else
              {
                treat_as_one = false;
              }
            }
            if(request->hasParam("treat_stun_as_one", true))
            {
              if(request->getParam("treat_stun_as_one", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                treat_stun_as_one = true;
              }
              else
              {
                treat_stun_as_one = false;
              }
            }
            if(request->hasParam("ongoing_is_cumulative", true))
            {
              if(request->getParam("ongoing_is_cumulative", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                ongoing_is_cumulative = true;
              }
              else
              {
                ongoing_is_cumulative = false;
              }
            }
            if(request->hasParam("ig_non_dot", true))
            {
              if(request->getParam("ig_non_dot", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                ig_non_dot = true;
              }
              else
              {
                ig_non_dot = false;
              }
            }
            saveSensorConfigurationSoon = millis();
            request->redirect("/admin");
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
      });
        adminWebServer->on("/sensorReset", HTTP_GET, [](AsyncWebServerRequest *request){
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            lastSensorStateChange = millis();
            currentSensorState = sensorState::resetting;
            request->redirect("/admin");
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
        });
      #endif
      #if defined ENABLE_REMOTE_RESTART
        adminWebServer->on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
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
            response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>Restart confirmation</h2></div></div>"));
            response->printf_P(PSTR("<div class=\"row\"><div class=\"twelve columns\"><p>Are you sure you want to restart \"%s\"?</p></div></div>"),device[0].name);
            response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/restartConfirmed\"><input class=\"button-primary\" type=\"button\" value=\"Yes\" style=\"width: 100%;\"></a></div>"));
            response->print(F("<div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"No\" style=\"width: 100%;\"></a></div></div>"));
            addPageFooter(response);
            //Send response
            request->send(response);
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
        });
        adminWebServer->on("/restartConfirmed", HTTP_GET, [](AsyncWebServerRequest *request){
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            localLog(F("Web UI restart requested from "));
            localLogLn(request->client()->remoteIP().toString());
            AsyncResponseStream *response = request->beginResponseStream("text/html");
            addPageHeader(response, 5, "/admin");
            response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>Restart</h2></div></div>"));
            //Top of page buttons
            response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a></div></div>"));
            response->print(F("<div class=\"row\"><div class=\"twelve columns\"><p>This node is restarting in 10s</p></div></div>"));
            addPageFooter(response);
            //Send response
            request->send(response);
            restartTimer = millis();
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            request->send(response);
          }
        #endif
        });
      #endif
      adminWebServer->on("/wipe", HTTP_GET, [](AsyncWebServerRequest *request){
      #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
        if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
        {
      #endif
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
          response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>Setting wipe</h2></div></div>"));
          response->printf_P(PSTR("<div class=\"row\"><div class=\"twelve columns\"><p>Are you sure you want to wipe all configuration of \"%s\"?</p></div></div>"),device[0].name);
          response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/wipeconfirmed\"><input class=\"button-primary\" type=\"button\" value=\"Yes\" style=\"width: 100%;\"></a></div>"));
          response->print(F("<div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"No\" style=\"width: 100%;\"></a></div></div>"));
          addPageFooter(response);
          //Send response
          request->send(response);
      #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          xSemaphoreGive(webserverSemaphore);
        }
        else
        {
          AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
          response->addHeader("Retry-After","5"); //Ask it to wait 5s
          //Send response
          request->send(response);
        }
      #endif
      });
      adminWebServer->on("/wipeconfirmed", HTTP_GET, [](AsyncWebServerRequest *request){
      #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
        if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
        {
      #endif
          #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
            if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
            {
                return request->requestAuthentication();  //Force basic authentication
            }
          #endif
          localLog(F("Web UI wipe requested from "));
          localLogLn(request->client()->remoteIP().toString());
          AsyncResponseStream *response = request->beginResponseStream("text/html");
          addPageHeader(response, 5, "/admin");
          response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>Wipe</h2></div></div>"));
          //Top of page buttons
          response->print(F("<div class=\"row\"><div class=\"twelve columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a></div></div>"));
          response->print(F("<div class=\"row\"><div class=\"twelve columns\"><p>This node is restarting in 10s</p></div></div>"));
          addPageFooter(response);
          //Send response
          request->send(response);
          wipeTimer = millis();
      #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          xSemaphoreGive(webserverSemaphore);
        }
        else
        {
          AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
          response->addHeader("Retry-After","5"); //Ask it to wait 5s
          request->send(response);
        }
      #endif
      });
      adminWebServer->on("/devices", HTTP_GET, [](AsyncWebServerRequest *request){
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
          #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
            if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
            {
                return request->requestAuthentication();  //Force basic authentication
            }
          #endif
          AsyncResponseStream *response = request->beginResponseStream("text/html");
          addPageHeader(response, 90, "/devices");
          //Top of page buttons
          response->print(F("<div class=\"row\"><div class=\"four columns\"><a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a></div>"));
          #ifdef ACT_AS_TRACKER
            response->print(F("<div class=\"four columns\"><a href =\"/nearest\"><input class=\"button-primary\" type=\"button\" value=\"Track nearest\" style=\"width: 100%;\"></a></div>"));
            response->print(F("<div class=\"four columns\"><a href =\"/furthest\"><input class=\"button-primary\" type=\"button\" value=\"Track furthest\" style=\"width: 100%;\"></a></div></div>"));
            response->print(F("<div class=\"row\"><div class=\"twelve columns\">Tracking mode: "));
            if(currentTrackingMode == trackingMode::nearest)
            {
              response->print(F("nearest"));
            }
            else if(currentTrackingMode == trackingMode::furthest)
            {
              response->print(F("furthest, waiting to select"));
            }
            else if(currentTrackingMode == trackingMode::fixed)
            {
              response->print(F("specific device"));
            }
            if(currentTrackingMode == trackingMode::nearest || currentTrackingMode == trackingMode::fixed)
            {
              if(currentlyTrackedBeacon != maximumNumberOfDevices)
              {
                if(device[currentlyTrackedBeacon].name != nullptr)
                {
                  response->print(F(" - "));
                  response->print(device[currentlyTrackedBeacon].name);
                }
                else
                {
                  response->printf_P(PSTR(" - device %u"),currentlyTrackedBeacon);
                }
              }
            }
            response->print(F("</div></div>"));
          #else
            response->print(F("</div>"));
          #endif
          if(numberOfDevices == maximumNumberOfDevices)
          {
            response->print(F("<div class=\"row\"><div class=\"twelve columns\"><H3>Warning: maximum number of devices reached!</H3></div></div>"));
          }
          response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h2>Devices</h2></div></div>"));
          response->print(F("<div class=\"row\"><div class=\"twelve columns\">"));
          #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
          response->print(F("<table><thead><tr><th>Name</th><th>MAC address</th><th>Features</th><th>Version</th><th>Uptime</th><th>Battery</th><th>Fix</th><th>Lat</th><th>Lon</th><th>Distance</th><th>Course</th><th>ESP-Now signal quality</th><th>LoRa signal quality</th><th>Info</th></tr></thead><tbody>"));
          #elif defined(SUPPORT_ESPNOW)
          response->print(F("<table><thead><tr><th>Name</th><th>MAC address</th><th>Features</th><th>Version</th><th>Uptime</th><th>Battery</th><th>Fix</th><th>Lat</th><th>Lon</th><th>Distance</th><th>Course</th><th>ESP-Now signal quality</th><th>Info</th></tr></thead><tbody>"));
          #elif defined(SUPPORT_LORA)
          response->print(F("<table><thead><tr><th>Name</th><th>MAC address</th><th>Features</th><th>Version</th><th>Uptime</th><th>Battery</th><th>Fix</th><th>Lat</th><th>Lon</th><th>Distance</th><th>Course</th><th>LoRa signal quality</th><th>Info</th></tr></thead><tbody>"));
          #endif
          for(uint8_t index = 0; index < numberOfDevices; index++)
          {
            if(index > 0)
            {
              #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
                response->printf_P(PSTR("<tr><td>%s %s</td><td>%02x:%02x:%02x:%02x:%02x:%02x</td><td>%s</td><td>v%u.%u.%u</td><td>%s</td><td>%.1fv</td><td>%s</td><td>%f</td><td>%f</td><td>%.1f</td><td>%.1f</td><td>%04x</td><td>%04x</td><td>"),
              #elif defined(SUPPORT_ESPNOW) || defined(SUPPORT_LORA)
                response->printf_P(PSTR("<tr><td>%s %s</td><td>%02x:%02x:%02x:%02x:%02x:%02x</td><td>%s</td><td>v%u.%u.%u</td><td>%s</td><td>%.1fv</td><td>%s</td><td>%f</td><td>%f</td><td>%.1f</td><td>%.1f</td><td>%04x</td><td>"),
              #else
                response->printf_P(PSTR("<tr><td>%s %s</td><td>%02x:%02x:%02x:%02x:%02x:%02x</td><td>%s</td><td>v%u.%u.%u</td><td>%s</td><td>%.1fv</td><td>%s</td><td>%f</td><td>%f</td><td>%.1f</td><td>%.1f</td><td></td><td>"),
              #endif
                (device[index].name == nullptr) ? "n/a" : device[index].name,
                (device[index].icName == nullptr) ? "" : device[index].icName,
                device[index].id[0],device[index].id[1],device[index].id[2],device[index].id[3],device[index].id[4],device[index].id[5],
                deviceFeatures(device[index].typeOfDevice).c_str(),
                device[index].majorVersion,device[index].minorVersion,device[index].patchVersion,
                (index == 0) ? printableUptime(millis()/1000).c_str() : printableUptime(device[index].uptime/1000).c_str(),
                device[index].supplyVoltage,
                (device[index].hasGpsFix == true) ? PSTR("Yes") : PSTR("No"),
                device[index].latitude,
                device[index].longitude,
                device[index].distanceTo,
                device[index].courseTo,
                #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
                  device[index].espNowUpdateHistory,
                  device[index].loRaUpdateHistory);
                #elif  defined(SUPPORT_ESPNOW)
                  device[index].espNowUpdateHistory);
                #elif  defined(SUPPORT_LORA)
                  device[index].loRaUpdateHistory);
                #endif
            }
            else
            {
              #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
              response->printf_P(PSTR("<tr><td>%s</td><td>%02x:%02x:%02x:%02x:%02x:%02x</td><td>%s</td><td>v%u.%u.%u</td><td>%s</td><td>%.1fv</td><td>%s</td><td>%f</td><td>%f</td><td>--</td><td>--</td><td>----</td><td>----</td><td>This device"),
              #elif defined(SUPPORT_ESPNOW) || defined(SUPPORT_LORA)
              response->printf_P(PSTR("<tr><td>%s</td><td>%02x:%02x:%02x:%02x:%02x:%02x</td><td>%s</td><td>v%u.%u.%u</td><td>%s</td><td>%.1fv</td><td>%s</td><td>%f</td><td>%f</td><td>--</td><td>--</td><td>----</td><td>This device"),
              #endif
              #if defined(SUPPORT_ESPNOW) || defined(SUPPORT_LORA)
                (device[index].name == nullptr) ? "n/a" : device[index].name,
                device[index].id[0],device[index].id[1],device[index].id[2],device[index].id[3],device[index].id[4],device[index].id[5],
                deviceFeatures(device[index].typeOfDevice).c_str(),
                device[index].majorVersion,device[index].minorVersion,device[index].patchVersion,
                (index == 0) ? printableUptime(millis()/1000).c_str() : printableUptime(device[index].uptime/1000).c_str(),
                device[index].supplyVoltage,
                (device[index].hasGpsFix == true) ? PSTR("Yes") : PSTR("No"),
                device[index].latitude,
                device[index].longitude
                );
              #endif
            }
              #ifdef ACT_AS_TRACKER
                if(index == currentlyTrackedBeacon)
                {
                  response->print(F("Tracked"));
                }
                #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
                  else if(index != 0 && (device[index].loRaOnline == false && device[index].espNowOnline == false))
                #elif defined(SUPPORT_ESPNOW)
                  else if(index != 0 && device[index].espNowOnline == false)
                #elif defined(SUPPORT_LORA)
                  else if(index != 0 && device[index].loRaOnline == false)
                #endif
                {
                  response->print(F("Offline"));
                }
                else if(index != 0) 
                {
                  response->printf_P(PSTR("<a href =\"/track?index=%u\"><input class=\"button-primary\" type=\"button\" value=\"Track\" style=\"width: 100%;\"></a>"),index);
                }
              #else
                if(index == closestTracker)
                {
                  response->print(F("Closest tracker"));
                }
                #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
                else if(index != 0 && (device[index].loRaOnline == false && device[index].espNowOnline == false))
                #elif defined(SUPPORT_ESPNOW)
                else if(index != 0 && device[index].espNowOnline == false)
                #elif defined(SUPPORT_LORA)
                else if(index != 0 && device[index].loRaOnline == false)
                #endif
                {
                  response->print(F("Offline"));
                }
              #endif
              response->print(F("</td></tr>"));
          }
          response->print(F("</tbody></table></div></div>"));
          addPageFooter(response);
          //Send response
          request->send(response);
      #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          xSemaphoreGive(webserverSemaphore);
        }
        else
        {
          AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
          response->addHeader("Retry-After","5"); //Ask it to wait 5s
          //Send response
          request->send(response);
        }
      #endif
      });
      #ifdef ACT_AS_TRACKER
        adminWebServer->on("/nearest", HTTP_GET, [](AsyncWebServerRequest *request){
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              #if defined(SERIAL_DEBUG)
                if(waitForBufferSpace(75))
                {
                  SERIAL_DEBUG_PORT.print(F("Web UI chose track nearest\r\n"));
                }
              #endif
              currentTrackingMode = trackingMode::nearest;
              request->redirect("/devices");
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
        });
        adminWebServer->on("/furthest", HTTP_GET, [](AsyncWebServerRequest *request){
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              #if defined(SERIAL_DEBUG)
                if(waitForBufferSpace(75))
                {
                  SERIAL_DEBUG_PORT.print(F("Web UI chose track furthest\r\n"));
                }
              #endif
              currentTrackingMode = trackingMode::furthest;
              request->redirect("/devices");
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
        });
        adminWebServer->on("/track", HTTP_GET, [](AsyncWebServerRequest *request){
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              if(request->hasParam("index"))
              {
                uint8_t selectedBeacon = request->getParam("index")->value().toInt();;
                if(selectedBeacon > 0 && selectedBeacon < maximumNumberOfDevices)
                {
                  currentlyTrackedBeacon = selectedBeacon;
                  currentTrackingMode = trackingMode::fixed;
                  #if defined(SERIAL_DEBUG)
                    if(waitForBufferSpace(75))
                    {
                      SERIAL_DEBUG_PORT.printf_P(PSTR("Web UI chose device %u to track\r\n"),currentlyTrackedBeacon);
                    }
                  #endif
                }
                else
                {
                  #if defined(SERIAL_DEBUG)
                    if(waitForBufferSpace(75))
                    {
                      SERIAL_DEBUG_PORT.printf_P(PSTR("Web UI chose invalid beacon\r\n"),selectedBeacon);
                    }
                  #endif
                }
              }
              request->redirect("/devices");
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
        });
      #endif
      #ifdef SUPPORT_HACKING
        adminWebServer->on("/gameConfiguration", HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
              #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
                if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
                {
                    return request->requestAuthentication();  //Force basic authentication
                }
              #endif
              AsyncResponseStream *response = request->beginResponseStream("text/html");
              addPageHeader(response, 0, nullptr);
              response->print(F("<h2>Game configuration</h2>"));
              //Start of form
              response->print(F("<form method=\"POST\">"));
              response->print(F("<a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\" style=\"width: 100%;\"></a> <input class=\"button-primary\" type=\"submit\" value=\"Save\" style=\"width: 100%;\">"));
              response->print(F("<div class=\"row\"><div class=\"twelve columns\"><h3>Starting values</h3></div></div>"));
              //Starting hits
              response->printf_P(PSTR("<div class=\"row\"><div class=\"six columns\"><label for=\"gameLength\">Game length/win threshold</label><input class=\"u-full-width\" type=\"number\" min=\"1\" max=\"99\" step=\"1\" value=\"%u\" id=\"gameLength\" name=\"gameLength\"></div></div>"), gameLength);
              response->printf_P(PSTR("<div class=\"row\"><div class=\"six columns\"><label for=\"gameRetries\">Game retries</label><input class=\"u-full-width\" type=\"number\" min=\"0\" max=\"99\" step=\"1\" value=\"%u\" id=\"gameRetries\" name=\"gameRetries\"></div></div>"), gameRetries);
              response->printf_P(PSTR("<div class=\"row\"><div class=\"six columns\"><label for=\"gameSpeedup\">Game speedup(ms)</label><input class=\"u-full-width\" type=\"number\" min=\"100\" max=\"2000\" step=\"1\" value=\"%u\" id=\"gameSpeedup\" name=\"gameSpeedup\"></div></div>"), gameSpeedup);
              //End of form
              response->print(F("</form>"));
              addPageFooter(response);
              //Send response
              request->send(response);
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
              xSemaphoreGive(webserverSemaphore);
            }
            else
            {
              AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
              response->addHeader("Retry-After","5"); //Ask it to wait 5s
              //Send response
              request->send(response);
            }
          #endif
          });
          adminWebServer->on("/gameConfiguration", HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
            lastWifiActivity = millis();
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            #ifdef DEBUG_FORM_SUBMISSION
              int params = request->params();
              localLog(F("Submitted Configuration parameters: "));
              localLogLn(params);
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
            #endif
            //Read the submitted configuration
            if(request->hasParam("gameLength", true))
            {
              gameLength = request->getParam("gameLength", true)->value().toInt();
            }
            if(request->hasParam("gameRetries", true))
            {
              gameRetries = request->getParam("gameRetries", true)->value().toInt();
            }
            if(request->hasParam("gameSpeedup", true))
            {
              gameSpeedup = request->getParam("gameSpeedup", true)->value().toInt();
            }
            if(configurationChanged())
            {
              saveConfigurationSoon = millis();
            }
            request->redirect("/admin");
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
      });
      #endif
      adminWebServer->on("/css/normalize.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
            lastWifiActivity = millis();
            request->send_P(200, "text/css", normalize);
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            AsyncWebServerResponse *response = request->beginResponse(503); //Sends 503 as the server is busy
            response->addHeader("Retry-After","5"); //Ask it to wait 5s
            //Send response
            request->send(response);
          }
        #endif
      });
      adminWebServer->on("/css/skeleton.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
            lastWifiActivity = millis();
            request->send_P(200, "text/css", skeleton);
        #ifdef ENABLE_LOCAL_WEBSERVER_SEMAPHORE
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            request->send(500); //Sends 500 if the server is busy
          }
        #endif
      });
        adminWebServer->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
          request->send(404); //There is no favicon!
        });
        #ifdef SUPPORT_HACKING //ESPUI already does a redirect
        if(sensorReset == true)
        {
        #endif
          adminWebServer->onNotFound([](AsyncWebServerRequest *request){  //This lambda function is a minimal 404 handler
            if(enableCaptivePortal)
            {
              request->redirect("/admin"); //Needed for captive portal
            }
            else
            {
              request->send(404, "text/plain", request->url() + " not found");
            }
          });
        #ifdef SUPPORT_HACKING
        }
        #endif
      #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
        #if defined(USE_SPIFFS)
          if(basicAuthEnabled == true)
          {
            adminWebServer
                      .serveStatic("/logs/", SPIFFS, logDirectory) //Serve the log files up statically
                      .setAuthentication(http_user, http_password); //Add a username and password
          }
          else
          {
            adminWebServer->serveStatic("/logs/", SPIFFS, logDirectory); //Serve the log files up statically 
          }
          #if defined(SERVE_CONFIG_FILE)
            adminWebServer->serveStatic("/configfile", SPIFFS, configurationFile);  //Serve the configuration file statically
          #endif
        #elif defined(USE_LITTLEFS)
          if(basicAuthEnabled == true)
          {
            adminWebServer->serveStatic("/logs/", LittleFS, logDirectory) //Serve the log files up statically
                      setAuthentication(http_user, http_password); //Add a username and password
          }
          else
          {
            adminWebServer->serveStatic("/logs/", LittleFS, logDirectory); //Serve the log files up statically
          }
          #if defined(SERVE_CONFIG_FILE)
            adminWebServer->serveStatic("/configfile", LittleFS, configurationFile);  //Serve the configuration file statically
          #endif
        #endif
      #else
        #if defined(USE_SPIFFS)
          adminWebServer->serveStatic("/logs/", SPIFFS, logDirectory); //Serve the log files up statically
          #if defined(SERVE_CONFIG_FILE)
            adminWebServer->serveStatic("/configfile", SPIFFS, configurationFile);  //Serve the configuration file statically
          #endif
        #elif defined(USE_LITTLEFS)
          adminWebServer->serveStatic("/logs/", LittleFS, logDirectory); //Serve the log files up statically
          #if defined(SERVE_CONFIG_FILE)
            adminWebServer->serveStatic("/configfile", LittleFS, configurationFile);  //Serve the configuration file statically
          #endif
        #endif
      #endif
      #ifdef SUPPORT_HACKING
        if(sensorReset == true)
        {
          adminWebServer->begin();
        }
      #else
        adminWebServer->begin();
      #endif
      localLogLn(F("OK"));
    }
  }
#endif

/*
 * 
 * This file contains functions related to the local web server used for configuration, control and log access
 * 
 */
#if defined(ENABLE_LOCAL_WEBSERVER)
  const char h2printfFormatString[] PROGMEM =                 "<div class=\"row\"><div class=\"twelve columns\"><h2>%s</h2></div></div>";
  const char h2printfEightColumnsFormatString[] PROGMEM =     "<div class=\"row\"><div class=\"eight columns\"><h2>%s</h2></div>";
  const char h3printfFormatString[] PROGMEM =                 "<div class=\"row\"><div class=\"twelve columns\"><h3>%s</h3></div></div>";
  const char hr[] PROGMEM =                                   "<hr style=\"width:100%;text-align:left;margin-left:0\">";
  const char startRow[] PROGMEM =                             "<div class=\"row\">";
  const char ulStart[] PROGMEM =                              "<div class=\"row\"><div class=\"eight columns\"><ul>";
  const char ulEnd[] PROGMEM =                                "</ul></div>";
  const char ulEndRow[] PROGMEM =                             "</ul></div></div>";
  const char liIntegerPrintfFormatString[] PROGMEM =          "<li>%s: <b>%u</b></li>";
  const char liIntegerWithUnitsPrintfFormatString[] PROGMEM = "<li>%s: <b>%u%s</b></li>";
  const char liFloatPrintfFormatString[] PROGMEM =            "<li>%s: <b>%.2f</b></li>";
  const char liFloatWithUnitsPrintfFormatString[] PROGMEM =   "<li>%s: <b>%.2f%s</b></li>";
  const char liStringPrintfFormatString[] PROGMEM =           "<li>%s: <b>%s</b></li>";
  const char formStart[] PROGMEM =                            "<form method=\"POST\">";
  const char formEnd[] PROGMEM =                              "</form>";
  const char labelForPrintfFormatString[] PROGMEM =           "<label for=\"%s\">%s</label>";
  const char selectPrintfFormatString[] PROGMEM =             "<select class=\"u-full-width\" id=\"%s\" name=\"%s\">";
  const char inputTextPrintfFormatString[] PROGMEM =          "<input class=\"u-full-width\" type=\"text\" id=\"%s\" name=\"%s\" value=\"%s\">";
  const char inputPasswordPrintfFormatString[] PROGMEM =      "<input class=\"u-full-width\" type=\"password\" placeholder=\"********\" id=\"%s\" name=\"%s\">";
  const char inputNumberPrintfFormatString[] PROGMEM =        "<input class=\"u-full-width\" type=\"number\" id=\"%s\" name=\"%s\" value=\"%u\">";
  const char string_empty[] PROGMEM =                         "";
  const char divEnd[] PROGMEM =                               "</div>";
  const char* endRow = divEnd;
  const char buttonPrintfFormatString[] PROGMEM =             "<a href =\"/%s\"><input class=\"button-primary\" type=\"button\" value=\"%s\"></a>";
  const char saveButton[] PROGMEM =                           "<input class=\"button-primary\" type=\"submit\" value=\"Save\">";
  const char backButton[] PROGMEM =                           "<a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"Back\"></a>";
  const char startTwelveColumns[] PROGMEM =                   "<div class=\"twelve columns\">";
  const char startEightColumns[] PROGMEM =                    "<div class=\"eight columns\">";
  const char startSixColumns[] PROGMEM =                      "<div class=\"six columns\">";
  const char startFourColumns[] PROGMEM =                     "<div class=\"four columns\">";
  const char startThreeColumns[] PROGMEM =                    "<div class=\"three columns\">";
  const char* endColumn = divEnd;
  const char selectValueTrue[] PROGMEM =                      "<option value=\"true\"";
  const char selectValueFalse[] PROGMEM =                     "<option value=\"false\"";
  const char selectValueSelected[] PROGMEM =                  " selected>";
  const char selectValueNotSelected[] PROGMEM =               ">";
  const char selectValueEnabled[] PROGMEM =                   "Enabled</option>";
  const char selectValueDisabled[] PROGMEM =                  "Disabled</option>";
  const char endSelect[] PROGMEM =                            "</select>";
  const char labelNotSet[] PROGMEM =                          "Not set";
  const char labelOn[] PROGMEM =                              "On";
  const char labelOff[] PROGMEM =                             "Off";
  const char labelConfigure[] PROGMEM =                       "Configure";
  const char labelBack[] PROGMEM =                            "Back";
  //Default header addition
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
      response->printf_P(PSTR("<h1>%s</h1>"),device[0].name);
      response->print(hr);
    }
  }
  //Default footer addition
  void addPageFooter(AsyncResponseStream *response)
  {
    response->print(F("</div></body></html>"));
  }
  //Add all the many many callback functions
  void setupWebServer()
  {
    if(filesystemMounted == true)
    {
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
        webserverSemaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(webserverSemaphore);
      #endif
      #if defined(SUPPORT_HACKING)
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
      adminWebServer->on(PSTR("/admin"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            //Top of page buttons
            // ROW 1
            response->printf_P(h2printfFormatString, PSTR("Info"));
            response->print(startRow);
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("listDevices"), PSTR("Devices"));
            response->print(endColumn);
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("listLogs"), PSTR("Logs"));
            response->print(endColumn);
            response->print(endRow);
            // ROW 2
            response->print(hr);
            response->printf_P(h2printfFormatString, PSTR("Configuration"));
            response->print(startRow);
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("icInfo"), PSTR("IC Info"));
            response->print(endColumn);
            #if defined(SUPPORT_LVGL)
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("gui"), PSTR("Gui"));
              response->print(endColumn);
            #endif
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("tracking"), PSTR("Tracking"));
            response->print(endColumn);
            response->print(endRow);
            // ROW 3
            response->print(hr);
            response->printf_P(h2printfFormatString, PSTR("Hardware"));
            response->print(startRow);
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("hardware"), PSTR("Hardware"));
            response->print(endColumn);
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("treacle"), PSTR("Treacle"));
            response->print(endColumn);
            #if defined(SUPPORT_WIFI)
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("wifi"), PSTR("Wi-Fi"));
              response->print(endColumn);
            #endif
            response->print(endRow);
            // ROW 4
            response->print(startRow);
            #if defined(SUPPORT_FTM)
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("ftm"), PSTR("FTM"));
              response->print(endColumn);
            #endif
            #if defined(SUPPORT_GPS)
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("gps"), PSTR("GPS"));
              response->print(endColumn);
            #endif
            #if defined(ACT_AS_SENSOR)
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("sensor"), PSTR("Sensor"));
              response->print(endColumn);
            #endif
            #if defined(SUPPORT_HACKING)
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("game"), PSTR("Game"));
              response->print(endColumn);
            #endif
            response->print(endRow);
            // ROW 5
            response->print(hr);
            response->printf_P(h2printfFormatString, PSTR("Logs"));
            response->print(startRow);
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("logging"), PSTR("Log settings"));
            response->print(endColumn);
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("listLogs"), PSTR("List logs"));
            response->print(endColumn);
            response->print(endRow);
            // ROW 6
            response->print(hr);
            response->printf_P(h2printfFormatString, PSTR("Admin"));
            response->print(startRow);
            #if defined(ENABLE_LOCAL_WEBSERVER_FIRMWARE_UPDATE)
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("update"), PSTR("Software Update"));
              response->print(endColumn);
            #endif
            #if defined(ENABLE_REMOTE_RESTART)
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("restart"), PSTR("Restart"));
              response->print(endColumn);
            #endif
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("wipe"), PSTR("Wipe"));
            response->print(endColumn);
            response->print(endRow);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/hardware"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            //Status information
            response->printf_P(h2printfEightColumnsFormatString, PSTR("Hardware"));
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("configureHardware"), labelConfigure);
            response->print(endColumn);
            response->print(ulStart);
            #if defined(ACT_AS_TRACKER)
              response->printf_P(PSTR("<li>PDT tracker firmware: <b>v%u.%u.%u</b></li>"), device[0].majorVersion, device[0].minorVersion, device[0].patchVersion);
            #elif defined(ACT_AS_BEACON)
              response->printf_P(PSTR("<li>PDT beacon firmware: <b>v%u.%u.%u</b></li>"), device[0].majorVersion, device[0].minorVersion, device[0].patchVersion);
            #endif
            response->printf_P(liStringPrintfFormatString, PSTR("Features"), deviceFeatures(device[0].typeOfDevice).c_str());
            response->printf_P(PSTR("<li>Built: <b>%s %s</b></li>"), __TIME__, __DATE__);
            response->printf_P(PSTR("<li>Board: <b>%s</b></li><li>PCB Variant: <b>"), ARDUINO_BOARD);
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
              response->print(F("</b></li><li>ESP&#8209;IDF: <b>v"));
              #ifdef ESP_IDF_VERSION_MINOR
                response->print(ESP_IDF_VERSION_MAJOR);
                response->print('.');
                response->print(ESP_IDF_VERSION_MINOR);
              #else
                response->print(ESP_IDF_VERSION_MAJOR);
              #endif
            #endif
            response->print(F("</b></li>"));
            response->printf_P(PSTR("<li>MAC address: <b>%02x:%02x:%02x:%02x:%02x:%02x</b></li>"), localMacAddress[0], localMacAddress[1], localMacAddress[2], localMacAddress[3], localMacAddress[4], localMacAddress[5]);            
            if(filesystemMounted == true)
            {
              #if defined(USE_SPIFFS)
                FSInfo fsInfo;
                SPIFFS.info(fsInfo);
                response->printf_P(PSTR("<li>SPIFFS: <b>%u/%uKB used"), fsInfo.usedBytes/1024, fsInfo.totalBytes/1024);
                if(fsInfo.usedBytes > 0 && fsInfo.totalBytes > 0)
                {
                  response->printf_P(PSTR(" %.1f%% "),float(fsInfo.usedBytes) * 100/float(fsInfo.totalBytes));
                }
              #elif defined(USE_LITTLEFS)
                #if defined(ESP32)
                  response->printf_P(PSTR("<li>LittleFS: <b>%u/%uKB used"), LittleFS.usedBytes()/1024, LittleFS.totalBytes()/1024);
                  if(LittleFS.usedBytes() > 0 && LittleFS.totalBytes() > 0)
                  {
                    response->printf_P(PSTR(" %.1f%%"),float(LittleFS.usedBytes()) * 100/float(LittleFS.totalBytes()));
                  }
                #endif
              #endif
              response->print(F("</b></li>"));
            }
            #if defined(ESP32)
              response->printf_P(liIntegerWithUnitsPrintfFormatString, PSTR("Free heap"), ESP.getFreeHeap()/1024, PSTR("KB"));
            #endif
            response->print(F("<li>USB serial logging: <b>"));
            #if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
              if(debugPortAvailable)
              {
                response->print(F("enabled - connected"));
              }
              else
              {
                response->print(F("enabled - disconnected"));
              }
            #else
              response->print(F("disabled"));
            #endif
            response->print(F("</b></li>"));
            #if defined(ENABLE_OTA_UPDATE)
              response->print(F("<li>Over-the-air software update (Arduino IDE): <b>enabled</b></li>"));
            #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_FIRMWARE_UPDATE)
              response->print(F("<li>Web UI software update: <b>enabled</b></li>"));
            #endif
            response->print(F("<li>Time(UTC): <b>"));
            if(timeIsValid())
            {
              updateTimestamp();
              response->print(timestamp);
            }
            else
            {
              response->print(labelNotSet);
            }
            response->print(F("</b></li><li>Uptime: <b>"));
            response->print(printableDuration(millis()/1000));
            response->print(F("</b></li>"));
            #if defined(ESP32)
              #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
                #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
                  response->print(F("<li>Restart reason core 0: <b>"));
                  response->print(es32ResetReason(0));
                  response->print(F("</li><li></b>Restart reason core 1: <b>"));
                  response->print(es32ResetReason(1));
                  response->print(F("</b></li>"));
                #elif CONFIG_IDF_TARGET_ESP32S2
                  response->print(F("<li>Restart reason: <b>"));
                  response->print(es32ResetReason(0));
                  response->print(F("</b></li>"));
                #elif CONFIG_IDF_TARGET_ESP32C3
                  response->print(F("<li>Restart reason: <b>"));
                  response->print(es32ResetReason(0));
                  response->print(F("</b></li>"));
                #else 
                  #error Target CONFIG_IDF_TARGET is not supported
                #endif
              #else // ESP32 Before IDF 4.0
                response->print(F("<li>Restart reason core 0: <b>"));
                response->print(es32ResetReason(0));
                response->print(F("</b> Restart reason core 1: <b>"));
                response->print(es32ResetReason(1));
                response->print(F("</b></li>"));
              #endif
            #endif
            #if defined(SUPPORT_BATTERY_METER)
              if(enableBatteryMonitor == true)
              {
                response->printf_P(liFloatWithUnitsPrintfFormatString, PSTR("Battery monitoring"), device[0].supplyVoltage, PSTR("v"));
              }
              else
              {
                response->printf_P(liStringPrintfFormatString, PSTR("Battery monitoring"), PSTR("disabled"));
              }
            #endif
            if(configurationComment != nullptr)
            {
              if(strlen(configurationComment) > 0)
              {
                response->printf_P(liStringPrintfFormatString, PSTR("Configuration comment"), configurationComment);
              }
            }
            response->print(ulEndRow);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/icInfo"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            //General
            response->printf_P(h2printfEightColumnsFormatString, PSTR("IC Info"));
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("configureIcInfo"), labelConfigure);
            response->print(endColumn);
            response->print(ulStart);
            response->printf_P(liStringPrintfFormatString, PSTR("Name"), (device[0].name != nullptr) ? device[0].name : labelNotSet);
            if(device[0].icName != nullptr)
            {
              response->printf_P(liStringPrintfFormatString, PSTR("IC Name"), device[0].icName);
            }
            else
            {
              response->printf_P(liStringPrintfFormatString, PSTR("IC Name"), labelNotSet);
            }
            #if defined(ACT_AS_BEACON)
              response->printf_P(liStringPrintfFormatString, PSTR("Usually static"), (deviceUsuallyStatic) ? labelOn : labelOff);
            #endif
            if(device[0].icDescription != nullptr)
            {
              response->printf_P(liStringPrintfFormatString, PSTR("IC Description"), device[0].icDescription);
            }
            else
            {
              response->printf_P(liStringPrintfFormatString, PSTR("IC Description"), labelNotSet);
            }
            response->printf_P(liIntegerWithUnitsPrintfFormatString, PSTR("IC Diameter"), device[0].diameter, PSTR("m"));
            response->printf_P(liIntegerWithUnitsPrintfFormatString, PSTR("IC Height"), device[0].height, PSTR("m"));
            response->print(ulEndRow);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(SUPPORT_WIFI)
        adminWebServer->on(PSTR("/wifi"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->printf_P(h2printfEightColumnsFormatString, PSTR("WiFi"));
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("configureWifi"), labelConfigure);
              response->print(endColumn);
              response->print(ulStart);
              response->printf_P(liStringPrintfFormatString, PSTR("WiFi client"), (startWiFiClientOnBoot) ? labelOn : labelOff);
              if(startWiFiClientOnBoot)
              {
                response->print(F("<ul>"));
                response->printf_P(liStringPrintfFormatString, PSTR("WiFi SSID"), (SSID != nullptr) ? SSID : labelNotSet);
                if(wiFiClientInactivityTimer > 0)
                {
                  response->print(F("<li>WiFi disconnecting after: <b>"));
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
                    response->print(F("5 minutes"));
                  }
                  response->print(F(" inactivity</b></li>"));
                }
                response->print(F("</ul>"));
              }
              response->printf_P(liStringPrintfFormatString, PSTR("WiFi AP"), (startWiFiApOnBoot) ? labelOn : labelOff);
              if(startWiFiApOnBoot)
              {
                response->print(F("<ul>"));
                response->printf_P(liStringPrintfFormatString, PSTR("SSID"), (APSSID != nullptr) ? APSSID : labelNotSet);
                response->printf_P(liStringPrintfFormatString, PSTR("PSK"), (APPSK != nullptr) ? APPSK : labelNotSet);
                response->printf_P(liIntegerPrintfFormatString, PSTR("Preferred Channel"), softApChannel);
                #if defined(ENABLE_LOCAL_WEBSERVER)
                  response->printf_P(liStringPrintfFormatString, PSTR("Captive portal"), (enableCaptivePortal) ? labelOn : labelOff);
                #endif
                response->print(F("</ul>"));
              }
              response->print(ulEndRow);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      //Tracking information
      adminWebServer->on(PSTR("/tracking"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->printf_P(h2printfEightColumnsFormatString, PSTR("Tracking"));
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("configureTracking"), labelConfigure);
            response->print(endColumn);
            response->print(ulStart);
            response->printf_P(liIntegerWithUnitsPrintfFormatString, PSTR("Effective range"), maximumEffectiveRange, PSTR("m"));
            #if defined(ACT_AS_TRACKER)
              if(trackerPriority == 0)
              {
                response->printf_P(liStringPrintfFormatString, PSTR("Indicate range to"), PSTR("Centre"));
              }
              else
              {
                response->printf_P(liStringPrintfFormatString, PSTR("Indicate range to"), PSTR("Edge"));
              }
            #endif
            response->print(F("<li>Sensitivity: <b>"));
            if(trackingSensitivity == 2)
            {
              response->print(F("High"));
            }
            else if(trackingSensitivity == 1)
            {
              response->print(F("Medium"));
            }
            else
            {
              response->print(F("Low"));
            }
            response->print(F("</b></li>"));
            response->print(ulEndRow);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(SUPPORT_LVGL)
        adminWebServer->on(PSTR("/gui"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->printf_P(h2printfEightColumnsFormatString, PSTR("GUI"));
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("configureGui"), labelConfigure);
              response->print(endColumn);
              response->print(endRow);
              response->print(ulStart);
              #if defined(LVGL_SUPPORT_HOME_TAB)
                response->printf_P(liStringPrintfFormatString, PSTR("Home tab"), (enableHomeTab) ? labelOn : labelOff);
              #endif
              #if defined(LVGL_SUPPORT_MAP_TAB)
                response->printf_P(liStringPrintfFormatString, PSTR("Map tab"), (enableMapTab) ? labelOn : labelOff);
              #endif
              #if defined(LVGL_SUPPORT_GPS_TAB)
                response->printf_P(liStringPrintfFormatString, PSTR("GPS info tab"), (enableGpsTab) ? labelOn : labelOff);
              #endif
              #if defined(LVGL_SUPPORT_SCAN_INFO_TAB)
                response->printf_P(liStringPrintfFormatString, PSTR("Scan Info tab"), (enableInfoTab) ? labelOn : labelOff);
              #endif
              #if defined(LVGL_SUPPORT_SETTINGS_TAB)
                response->printf_P(liStringPrintfFormatString, PSTR("Settings tab"), (enableSettingsTab) ? labelOn : labelOff);
              #endif
              response->print(ulEndRow);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/treacle"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->printf_P(h2printfEightColumnsFormatString, PSTR("Treacle"));
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("configureTreacle"), labelConfigure);
            response->print(endColumn);
            response->print(endRow);
            response->print(ulStart);
            if(treacleIntialised == true)
            {
              response->printf_P(liIntegerPrintfFormatString, PSTR("Node ID"), treacle.getNodeId());
              response->printf_P(liIntegerPrintfFormatString, PSTR("Number of nodes"), treacle.nodes());
              response->printf_P(liIntegerWithUnitsPrintfFormatString, PSTR("Location/Info updates"), treacleDeviceInfoInterval/1000, PSTR("s"));
              response->printf_P(liStringPrintfFormatString, PSTR("ESP&#8209;Now radio"), (treacle.espNowEnabled()) ? labelOn : labelOff);
              if(treacle.espNowEnabled() == true)
              {
                response->print(F("<ul>"));
                if(espNowInitialised == true)
                {
                  if(espNowPhyMode == 1)
                  {
                    response->printf_P(liStringPrintfFormatString, PSTR("PHY mode"), PSTR("B"));
                  }
                  else if(espNowPhyMode == 2)
                  {
                    response->printf_P(liStringPrintfFormatString, PSTR("PHY mode"), PSTR("LR"));
                  }
                  else
                  {
                    response->printf_P(liStringPrintfFormatString, PSTR("PHY mode"), PSTR("BGN"));
                  }
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Current channel"), treacle.getEspNowChannel());
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Packets Rx"), treacle.getEspNowRxPackets());
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Packets Tx"), treacle.getEspNowTxPackets());
                  response->printf_P(liFloatWithUnitsPrintfFormatString, PSTR("Duty cycle"), treacle.getEspNowDutyCycle(), PSTR("%"));
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Duty cycle exceptions"), treacle.getEspNowDutyCycleExceptions());
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Packets Rx dropped"), treacle.getEspNowRxPacketsDropped());
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Packets Tx dropped"), treacle.getEspNowTxPacketsDropped());
                  response->printf_P(liIntegerWithUnitsPrintfFormatString, PSTR("Update interval"), treacle.getEspNowTickInterval()/1000, PSTR("s"));
                }
                else
                {
                  response->print(F("<li><b>Not initialised</b></li>"));
                }
                response->print(F("</ul>"));
              }
              response->printf_P(liStringPrintfFormatString, PSTR("LoRa radio"), (treacle.loRaEnabled()) ? labelOn : labelOff);
              if(treacle.loRaEnabled() == true)
              {
                response->print(F("<ul>"));
                if(treacle.loRaInitialised() == true)
                {
                  response->printf_P(liIntegerWithUnitsPrintfFormatString, PSTR("Frequency"), uint16_t(loRaFrequency/1E6), PSTR("MHz"));
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Spreading factor"), treacle.getLoRaSpreadingFactor());
                  response->printf_P(liFloatWithUnitsPrintfFormatString, PSTR("Signal bandwidth"), float(treacle.getLoRaSignalBandwidth())/1000.0, PSTR("kBaud"));
                  response->printf_P(liIntegerWithUnitsPrintfFormatString, PSTR("Transmit power"), treacle.getLoRaTxPower(), PSTR("dBm"));
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Packets Rx"), treacle.getLoRaRxPackets());
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Packets Tx"), treacle.getLoRaTxPackets());
                  response->printf_P(liFloatWithUnitsPrintfFormatString, PSTR("Duty cycle"), treacle.getLoRaDutyCycle(), PSTR("%"));
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Duty cycle exceptions"), treacle.getLoRaDutyCycleExceptions());
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Packets Rx dropped"), treacle.getLoRaRxPacketsDropped());
                  response->printf_P(liIntegerPrintfFormatString, PSTR("Packets Tx dropped"), treacle.getLoRaTxPacketsDropped());
                  response->printf_P(liIntegerWithUnitsPrintfFormatString, PSTR("Update interval"), treacle.getLoRaTickInterval()/1000, PSTR("s"));
                }
                else
                {
                  response->print(F("<li><b>Not initialised</b></li>"));
                }
                response->print(F("</ul>"));
              }
            }
            else
            {
              response->print(F("<li><b>Not initialised</b></li>"));
            }
            response->print(ulEndRow);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(SUPPORT_FTM)
        adminWebServer->on(PSTR("/ftm"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->printf_P(h2printfFormatString, PSTR("FTM (time-of-flight) measurements"));
              response->print(ulStart);
              response->printf_P(liStringPrintfFormatString, PSTR("FTM beacon"), (ftmEnabled) ? labelOn : labelOff);
              response->print(ulEndRow);
              response->print(startRow);
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("configureFtm"), labelConfigure);
              response->print(endColumn);
              response->print(endRow);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(SUPPORT_GPS)
        adminWebServer->on(PSTR("/gps"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->printf_P(h2printfEightColumnsFormatString, PSTR("GPS"));
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("configureGps"), labelConfigure);
              response->print(endColumn);
              response->print(endRow);
              response->print(ulStart);
              #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
                response->printf_P(liStringPrintfFormatString, PSTR("Power"), (peripheralsEnabled) ? labelOn : labelOff);
              #endif
              if(device[0].hasGpsFix)
              {
                response->printf_P(liStringPrintfFormatString, PSTR("Fix"), PSTR("Yes"));
                response->printf_P(liFloatPrintfFormatString, PSTR("Latitude"), device[0].latitude);
                response->printf_P(liFloatPrintfFormatString, PSTR("Longitude"), device[0].longitude);
                response->printf_P(liFloatPrintfFormatString, PSTR("Speed"), device[0].speed);
                response->printf_P(liFloatPrintfFormatString, PSTR("Speed(smoothed)"), device[0].smoothedSpeed);
                response->printf_P(liFloatWithUnitsPrintfFormatString, PSTR("Movement threshold"), movementThreshold, PSTR("m/s"));
                response->printf_P(liFloatWithUnitsPrintfFormatString, PSTR("HDOP"), device[0].hdop, hdopDescription(device[0].hdop));
                #if defined(ACT_AS_TRACKER)
                  if(currentlyTrackedDevice < maximumNumberOfDevices)
                  {
                    if(device[currentlyTrackedDevice].hasGpsFix)
                    {
                      response->printf_P(liFloatPrintfFormatString, PSTR("Distance to tracked beacon"), device[currentlyTrackedDevice].distanceTo);
                      response->printf_P(liFloatPrintfFormatString, PSTR("Course to tracked beacon: "), device[currentlyTrackedDevice].courseTo);
                    }
                  }
                #elif defined(ACT_AS_BEACON)
                  if(currentlyTrackedDevice < maximumNumberOfDevices)
                  {
                    if(device[currentlyTrackedDevice].hasGpsFix)
                    {
                      response->printf_P(liFloatPrintfFormatString, PSTR("Distance to nearest tracker"), device[currentlyTrackedDevice].distanceTo);
                      response->printf_P(liFloatPrintfFormatString, PSTR("Course to nearest tracker"), device[currentlyTrackedDevice].courseTo);
                    }
                  }
                #endif
              }
              else
              {
                response->printf_P(liStringPrintfFormatString, PSTR("Fix"), PSTR("No"));
              }
              response->printf_P(liIntegerPrintfFormatString, PSTR("Chars"), gpsChars);
              response->printf_P(liIntegerPrintfFormatString, PSTR("Sentences"), gpsSentences);
              response->printf_P(liIntegerPrintfFormatString, PSTR("Errors"), gpsErrors);
              response->print(ulEndRow);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/sensor"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->printf_P(h2printfEightColumnsFormatString, PSTR("Sensor"));
              response->printf_P(buttonPrintfFormatString, PSTR("configureSensor"), labelConfigure);
              response->print(endColumn);
              response->print(endRow);
              response->print(startRow);
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("sensorReset"), PSTR("Reset sensor"));
              response->print(endColumn);
              response->print(endRow);
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
              response->print(ulEndRow);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(SUPPORT_HACKING)
        adminWebServer->on(PSTR("/game"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->printf_P(h2printfFormatString, PSTR("Game"));
              response->print(startRow);
              response->print(startFourColumns);
              response->printf_P(buttonPrintfFormatString, PSTR("configureGame"), labelConfigure);
              response->print(endColumn);
              response->print(endRow);
              response->print(ulStart);
              response->printf_P(PSTR("<li>Game length: <b>%u</b></li><li>Game retries: <b>%u</b> (0=infinite)</li><li>Game speedup: <b>%u</b>(ms)</li>"), gameLength, gameRetries, gameSpeedup);
              response->print(ulEndRow);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      //Logging
      adminWebServer->on(PSTR("/logging"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function is a mimimal default response that shows some info and lists the log files
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->printf_P(h2printfEightColumnsFormatString, PSTR("Logging"));
            response->print(startFourColumns);
            response->printf_P(buttonPrintfFormatString, PSTR("configureLogging"), labelConfigure);
            response->print(endColumn);
            response->print(ulStart);
            response->printf_P(liIntegerPrintfFormatString, PSTR("Buffer size"), loggingBufferSize);
            response->printf_P(liIntegerWithUnitsPrintfFormatString, PSTR("Flush interval"), logFlushInterval, PSTR("s"));
            response->printf_P(liIntegerPrintfFormatString, PSTR("Flush threshold"), logFlushThreshold);
            response->print(ulEndRow);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/listLogs"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows a list of all the log files
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            /*
            response->print(startRow);
            response->print(startFourColumns);
            response->print(backButton);
            response->print(endColumn);
            response->print(endRow);
            */
            response->printf_P(h2printfFormatString, PSTR("Log files"));
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
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/deleteLog"), HTTP_GET, [](AsyncWebServerRequest *request){
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
                response->printf_P(h2printfFormatString, PSTR("Delete log file confirmation"));
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
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/deleteLogConfirmed"), HTTP_GET, [](AsyncWebServerRequest *request){
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/configureHardware"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->print(formStart);
            response->printf_P(h2printfEightColumnsFormatString, PSTR("Hardware configuration"));
            response->print(startFourColumns);
            response->print(saveButton);
            response->print(endColumn);
            response->print(endRow);
            #if defined(ENABLE_OTA_UPDATE)
              response->printf_P(h3printfFormatString, PSTR("Software Update"));
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_otaEnabled, PSTR("OTA enabled"));
              response->printf_P(selectPrintfFormatString, string_otaEnabled, string_otaEnabled);
              response->print(selectValueTrue);response->print(otaEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(otaEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_otaAuthenticationEnabled, PSTR("OTA password enabled"));
              response->printf_P(selectPrintfFormatString, string_otaAuthenticationEnabled, string_otaAuthenticationEnabled);
              response->print(selectValueTrue);response->print(otaAuthenticationEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(otaAuthenticationEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
            #endif
            //Battery
            #if defined(SUPPORT_BATTERY_METER)
              response->printf_P(h3printfFormatString, PSTR("Battery monitor"));
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_enableBatteryMonitor, PSTR("Enable battery monitor"));
              response->printf_P(selectPrintfFormatString, string_enableBatteryMonitor, string_enableBatteryMonitor);
              response->print(selectValueTrue);response->print(enableBatteryMonitor == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(enableBatteryMonitor == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(startSixColumns);
              response->print(F("Defaults 330/104kOhm"));
              response->print(endColumn);
              response->print(endRow);
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_topLadderResistor, PSTR("Top ladder resistor (Kohms)"));
              response->printf_P(PSTR("<input class=\"u-full-width\" type=\"number\" step=\"0.1\" value=\"%.1f\" id=\"topLadderResistor\" name=\"topLadderResistor\">"), topLadderResistor);
              response->print(endColumn);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_bottomLadderResistor, PSTR("Bottom ladder resistor (Kohms)"));
              response->printf_P(PSTR("<input class=\"u-full-width\" type=\"number\" step=\"0.1\" value=\"%.1f\" id=\"bottomLadderResistor\" name=\"bottomLadderResistor\">"), bottomLadderResistor);
              response->print(endColumn);
              response->print(endRow);
            #endif
            #if defined(SUPPORT_BEEPER)
              #if defined(ACT_AS_TRACKER)
                response->printf_P(h3printfFormatString, PSTR("Beeper"));
                response->print(startRow);
                response->print(startSixColumns);
                response->printf_P(labelForPrintfFormatString, string_beeperEnabled, PSTR("Beeper enabled"));
                response->printf_P(selectPrintfFormatString, string_beeperEnabled, string_beeperEnabled);
                response->print(selectValueTrue);response->print(beeperEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                response->print(selectValueFalse);response->print(beeperEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                response->print(endSelect);
                response->print(endColumn);
                #if defined(SUPPORT_BUTTON)
                  response->print(startSixColumns);
                  response->printf_P(labelForPrintfFormatString, string_beepOnPress, PSTR("Button beep"));
                  response->printf_P(selectPrintfFormatString, string_beepOnPress, string_beepOnPress);
                  response->print(selectValueTrue);response->print(beepOnPress == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                  response->print(selectValueFalse);response->print(beepOnPress == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                  response->print(endSelect);
                  response->print(endColumn);
                  response->print(endRow);
                #endif
              #else
                response->printf_P(h3printfFormatString, PSTR("Beeper"));
                response->print(startRow);
                response->print(startTwelveColumns);
                response->printf_P(labelForPrintfFormatString, string_beeperEnabled, PSTR("Beeper enabled"));
                response->printf_P(selectPrintfFormatString, string_beeperEnabled, string_beeperEnabled);
                response->print(selectValueTrue);response->print(beeperEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                response->print(selectValueFalse);response->print(beeperEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
              #endif
            #endif
            #if defined(SUPPORT_VIBRATION)
              response->printf_P(h3printfFormatString, PSTR("Vibration motor"));
              response->print(startRow);
              response->print(startTwelveColumns);
              response->printf_P(labelForPrintfFormatString, string_vibrationEnabled, PSTR("Haptic feedback enabled"));
              response->printf_P(selectPrintfFormatString, string_vibrationEnabled, string_vibrationEnabled);
              response->print(selectValueTrue);response->print(vibrationEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(vibrationEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              response->print(startRow);
              response->print(startTwelveColumns);
              response->printf_P(labelForPrintfFormatString, string_vibrationLevel, PSTR("Vibration level"));
              response->printf_P(selectPrintfFormatString, string_vibrationLevel, string_vibrationLevel);
              response->print(F("<option value=\"10\""));response->print(vibrationLevel == 10 ? selectValueSelected:selectValueNotSelected);response->print(F("10%</option>"));
              response->print(F("<option value=\"25\""));response->print(vibrationLevel == 25 ? selectValueSelected:selectValueNotSelected);response->print(F("25%</option>"));
              response->print(F("<option value=\"50\""));response->print(vibrationLevel == 50 ? selectValueSelected:selectValueNotSelected);response->print(F("50%</option>"));
              response->print(F("<option value=\"75\""));response->print(vibrationLevel == 75 ? selectValueSelected:selectValueNotSelected);response->print(F("75%</option>"));
              response->print(F("<option value=\"100\""));response->print(vibrationLevel == 100 ? selectValueSelected:selectValueNotSelected);response->print(F("100%</option>"));
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
            #endif
            //End of form
            response->print(formEnd);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/configureHardware"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            //Read the submitted configuration
            #if defined(ENABLE_OTA_UPDATE)
              if(request->hasParam("otaEnabled", true))
              {
                if(request->getParam("otaEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  otaEnabled = true;
                }
                else
                {
                  otaEnabled = false;
                }
              }
              if(request->hasParam("otaAuthenticationEnabled", true))
              {
                if(request->getParam("otaAuthenticationEnabled", true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  otaAuthenticationEnabled = true;
                }
                else
                {
                  otaAuthenticationEnabled = false;
                }
              }
            #endif
            #if defined(SUPPORT_BATTERY_METER)
              if(request->hasParam(string_enableBatteryMonitor, true))
              {
                if(request->getParam(string_enableBatteryMonitor, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  enableBatteryMonitor = true;
                }
                else
                {
                  enableBatteryMonitor = false;
                }
              }
              if(request->hasParam(string_topLadderResistor, true))
              {
                topLadderResistor = request->getParam(string_topLadderResistor, true)->value().toFloat();
              }
              if(request->hasParam(string_bottomLadderResistor, true))
              {
                bottomLadderResistor = request->getParam(string_bottomLadderResistor, true)->value().toFloat();
              }
            #endif
            #if defined(SUPPORT_BEEPER)
              if(request->hasParam(string_beeperEnabled, true))
              {
                if(request->getParam(string_beeperEnabled, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  beeperEnabled = true;
                }
                else
                {
                  beeperEnabled = false;
                }
              }
              #if defined(SUPPORT_BUTTON)
                if(request->hasParam(string_beepOnPress, true))
                {
                  if(request->getParam(string_beepOnPress, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
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
            #if defined(SUPPORT_VIBRATION)
              if(request->hasParam(string_vibrationEnabled, true))
              {
                if(request->getParam(string_vibrationEnabled, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  vibrationEnabled = true;
                }
                else
                {
                  vibrationEnabled = false;
                }
              }
              if(request->hasParam(string_vibrationLevel, true))
              {
                vibrationLevel = request->getParam(string_vibrationLevel, true)->value().toInt();
              }
            #endif
            #if defined(SUPPORT_BEEPER)
              if(request->hasParam(string_beeperEnabled, true))
              {
                if(request->getParam(string_beeperEnabled, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  beeperEnabled = true;
                }
                else
                {
                  beeperEnabled = false;
                }
              }
            #endif
            if(request->hasParam(string_loggingBufferSize, true))
            {
              if(loggingBufferSize != request->getParam(string_loggingBufferSize, true)->value().toInt())
              {
                loggingBufferSize = request->getParam(string_loggingBufferSize, true)->value().toInt();
              }
            }
            if(request->hasParam(string_logFlushThreshold, true))
            {
              if(logFlushThreshold != request->getParam(string_logFlushThreshold, true)->value().toInt())
              {
                logFlushThreshold = request->getParam(string_logFlushThreshold, true)->value().toInt();
              }
            }
            if(request->hasParam(string_logFlushInterval, true))
            {
              if(logFlushInterval != request->getParam(string_logFlushInterval, true)->value().toInt())
              {
                logFlushInterval = request->getParam(string_logFlushInterval, true)->value().toInt();
              }
            }
            saveConfigurationSoon = millis();
            request->redirect("/admin");
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      //WiFi configuration
      #if defined(SUPPORT_WIFI)
        adminWebServer->on(PSTR("/configureWifi"), HTTP_GET, [](AsyncWebServerRequest *request){
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->print(formStart);
            response->printf_P(h2printfEightColumnsFormatString, PSTR("Wi-Fi configuration"));
            response->print(startFourColumns);
            response->print(saveButton);
            response->print(endColumn);
            response->print(endRow);
            //WiFi client
            response->print(startRow);
            response->print(startThreeColumns);
            response->printf_P(labelForPrintfFormatString, string_startWiFiClientOnBoot, PSTR("Enable Wi-Fi client on boot"));
            response->printf_P(selectPrintfFormatString, string_startWiFiClientOnBoot, string_startWiFiClientOnBoot);
            response->print(selectValueTrue);response->print(startWiFiClientOnBoot == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
            response->print(selectValueFalse);response->print(startWiFiClientOnBoot == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
            response->print(endSelect);
            response->print(endColumn);
            response->print(startThreeColumns);
            response->printf_P(labelForPrintfFormatString, string_wiFiClientInactivityTimer, PSTR("Wi-Fi inactivity timer"));
            response->printf_P(selectPrintfFormatString, string_wiFiClientInactivityTimer, string_wiFiClientInactivityTimer);
            response->print(F("<option value=\"0\""));response->print(wiFiClientInactivityTimer == 0 ? selectValueSelected:selectValueNotSelected);response->print(F("Never</option>"));
            response->print(F("<option value=\"60000\""));response->print(wiFiClientInactivityTimer == 60000 ? selectValueSelected:selectValueNotSelected);response->print(F("1m</option>"));
            response->print(F("<option value=\"180000\""));response->print(wiFiClientInactivityTimer == 180000 ? selectValueSelected:selectValueNotSelected);response->print(F("3m</option>"));
            response->print(F("<option value=\"300000\""));response->print(wiFiClientInactivityTimer == 300000 ? selectValueSelected:selectValueNotSelected);response->print(F("5m</option>"));
            response->print(endSelect);
            response->print(endColumn);
            response->print(startThreeColumns);
            response->printf_P(labelForPrintfFormatString, string_SSID, PSTR("Wi-Fi SSID"));
            response->printf_P(inputTextPrintfFormatString, string_SSID, string_SSID, (SSID != nullptr) ? SSID : string_empty);
            response->print(endColumn);
            response->print(startThreeColumns);
            response->printf_P(labelForPrintfFormatString, string_PSK, PSTR("Wi-Fi PSK"));
            response->printf_P(inputPasswordPrintfFormatString, string_PSK, string_PSK);
            response->print(endColumn);
            response->print(endRow);
            //WiFi AP        
            response->print(startRow);
            response->print(startFourColumns);
            response->printf_P(labelForPrintfFormatString, string_startWiFiApOnBoot, PSTR("Enable Wi-Fi AP on boot"));
            response->printf_P(selectPrintfFormatString, string_startWiFiApOnBoot, string_startWiFiApOnBoot);
            response->print(selectValueTrue);response->print(startWiFiApOnBoot == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
            response->print(selectValueFalse);response->print(startWiFiApOnBoot == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
            response->print(endSelect);
            response->print(endColumn);
            response->print(startFourColumns);
            response->printf_P(labelForPrintfFormatString, string_enableCaptivePortal, PSTR("Enable captive portal"));
            response->printf_P(selectPrintfFormatString, string_enableCaptivePortal, string_enableCaptivePortal);
            response->print(selectValueTrue);response->print(enableCaptivePortal == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
            response->print(selectValueFalse);response->print(enableCaptivePortal == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
            response->print(endSelect);
            response->print(endColumn);
            response->print(startFourColumns);
            response->printf_P(labelForPrintfFormatString, string_softApChannel, PSTR("Preferred channel"));
            response->printf_P(selectPrintfFormatString, string_softApChannel, string_softApChannel);
            response->print(F("<option value=\"1\""));response->print(softApChannel == 1 ? selectValueSelected:selectValueNotSelected);response->print(F("1</option>"));
            response->print(F("<option value=\"6\""));response->print(softApChannel == 6 ? selectValueSelected:selectValueNotSelected);response->print(F("6</option>"));
            response->print(F("<option value=\"11\""));response->print(softApChannel == 11 ? selectValueSelected:selectValueNotSelected);response->print(F("11</option>"));
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            response->print(startRow);
            response->print(startSixColumns);
            response->printf_P(labelForPrintfFormatString, string_APSSID, PSTR("AP SSID"));
            response->printf_P(inputTextPrintfFormatString, string_APSSID, string_APSSID, (APSSID != nullptr) ? APSSID : string_empty);
            response->print(endColumn);
            response->print(startSixColumns);
            response->printf_P(labelForPrintfFormatString, string_APPSK, PSTR("AP PSK"));
            response->printf_P(inputPasswordPrintfFormatString, string_APPSK, string_APPSK);
            response->print(endColumn);
            response->print(endRow);
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              response->print(startRow);
              response->print(startFourColumns);
              response->printf_P(labelForPrintfFormatString, string_basicAuthEnabled, PSTR("Web UI login"));
              response->printf_P(selectPrintfFormatString, string_basicAuthEnabled, string_basicAuthEnabled);
              response->print(selectValueTrue);response->print(basicAuthEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(basicAuthEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(startFourColumns);
              response->printf_P(labelForPrintfFormatString, string_http_user, PSTR("Web UI username"));
              response->printf_P(inputTextPrintfFormatString, string_http_user, string_http_user, (http_user != nullptr) ? http_user : string_empty);
              response->print(endColumn);
              response->print(startFourColumns);
              response->printf_P(labelForPrintfFormatString, string_http_password, PSTR("Web UI/OTA password"));
              response->printf_P(inputPasswordPrintfFormatString, string_http_password, string_http_password);
              response->print(endColumn);
              response->print(endRow);
            #endif
            //Time Server
            response->print(startRow);
            response->print(startSixColumns);
            response->printf_P(labelForPrintfFormatString, string_timeServer, PSTR("NTP host"));
            response->printf_P(inputTextPrintfFormatString, string_timeServer, string_timeServer, (timeServer != nullptr) ? timeServer : string_empty);
            response->print(endColumn);
            response->print(startSixColumns);
            response->printf_P(labelForPrintfFormatString, string_timeZone, PSTR("Timezone"));
            response->printf_P(inputTextPrintfFormatString, string_timeZone, string_timeZone, (timeZone != nullptr) ? timeZone : string_empty);
            response->print(endColumn);
            response->print(endRow);
            //End of form
            response->print(formEnd);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/configureWifi"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            //Read the submitted configuration
            if(request->hasParam(string_SSID, true))
            {
              if(SSID != nullptr)
              {
                delete [] SSID;
              }
              SSID = new char[request->getParam(string_SSID, true)->value().length() + 1];
              strlcpy(SSID,request->getParam(string_SSID, true)->value().c_str(),request->getParam(string_SSID, true)->value().length() + 1);
            }
            if(request->hasParam(string_PSK, true))
            {
              if(request->getParam(string_PSK, true)->value().length() > 0)
              {
                if(PSK != nullptr)
                {
                  delete [] PSK;
                }
                PSK = new char[request->getParam(string_PSK, true)->value().length() + 1];
                strlcpy(PSK,request->getParam(string_PSK, true)->value().c_str(),request->getParam(string_PSK, true)->value().length() + 1);
              }
            }
            if(request->hasParam(string_startWiFiClientOnBoot, true))
            {
              if(request->getParam(string_startWiFiClientOnBoot, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                startWiFiClientOnBoot = true;
              }
              else
              {
                startWiFiClientOnBoot = false;
              }
            }
            if(request->hasParam(string_wiFiClientInactivityTimer, true))
            {
              wiFiClientInactivityTimer = request->getParam(string_wiFiClientInactivityTimer, true)->value().toInt();
            }
            if(request->hasParam(string_timeServer, true))
            {
              if(timeServer != nullptr)
              {
                delete [] timeServer;
              }
              timeServer = new char[request->getParam(string_timeServer, true)->value().length() + 1];
              strlcpy(timeServer,request->getParam(string_timeServer, true)->value().c_str(),request->getParam(string_timeServer, true)->value().length() + 1);
            }
            if(request->hasParam(string_timeZone, true))
            {
              if(timeZone != nullptr)
              {
                delete [] timeZone;
              }
              timeZone = new char[request->getParam(string_timeZone, true)->value().length() + 1];
              strlcpy(timeZone,request->getParam(string_timeZone, true)->value().c_str(),request->getParam(string_timeZone, true)->value().length() + 1);
            }
            if(request->hasParam(string_APSSID, true))
            {
              if(APSSID != nullptr)
              {
                delete [] APSSID;
              }
              APSSID = new char[request->getParam(string_APSSID, true)->value().length() + 1];
              strlcpy(APSSID,request->getParam(string_APSSID, true)->value().c_str(),request->getParam(string_APSSID, true)->value().length() + 1);
            }
            if(request->hasParam(string_APPSK, true))
            {
              if(request->getParam(string_APPSK, true)->value().length() > 0)
              {
                if(APPSK != nullptr)
                {
                  delete [] APPSK;
                }
                APPSK = new char[request->getParam(string_APPSK, true)->value().length() + 1];
                strlcpy(APPSK,request->getParam(string_APPSK, true)->value().c_str(),request->getParam(string_APPSK, true)->value().length() + 1);
              }
            }
            if(request->hasParam(string_startWiFiApOnBoot, true))
            {
              if(request->getParam(string_startWiFiApOnBoot, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                startWiFiApOnBoot = true;
              }
              else
              {
                startWiFiApOnBoot = false;
              }
            }
            if(request->hasParam(string_enableCaptivePortal, true))
            {
              if(request->getParam(string_enableCaptivePortal, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
              {
                enableCaptivePortal = true;
              }
              else
              {
                enableCaptivePortal = false;
              }
            }
            if(request->hasParam(string_softApChannel, true))
            {
              softApChannel = request->getParam(string_softApChannel, true)->value().toInt();
            }
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(request->hasParam(string_http_user, true))
              {
                if(http_user != nullptr)
                {
                  delete [] http_user;
                }
                http_user = new char[request->getParam(string_http_user, true)->value().length() + 1];
                strlcpy(http_user,request->getParam(string_http_user, true)->value().c_str(),request->getParam(string_http_user, true)->value().length() + 1);
              }
              if(request->hasParam(string_http_password, true))
              {
                if(request->getParam(string_http_password, true)->value().length() > 0)
                {
                  if(http_password != nullptr)
                  {
                    delete [] http_password;
                  }
                  http_password = new char[request->getParam(string_http_password, true)->value().length() + 1];
                  strlcpy(http_password,request->getParam(string_http_password, true)->value().c_str(),request->getParam(string_http_password, true)->value().length() + 1);
                }
              }
              if(request->hasParam(string_basicAuthEnabled, true))
              {
                if(request->getParam(string_basicAuthEnabled, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  basicAuthEnabled = true;
                }
                else
                {
                  basicAuthEnabled = false;
                }
              }
            #endif
            saveConfigurationSoon = millis();
            request->redirect("/admin");
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/configureIcInfo"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->print(formStart);
            response->printf_P(h2printfEightColumnsFormatString, PSTR("General configuration"));
            response->print(startFourColumns);
            response->print(saveButton);
            response->print(endColumn);
            response->print(endRow);
            //Name
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_deviceName, PSTR("Node name"));
            response->printf_P(inputTextPrintfFormatString, string_deviceName, string_deviceName, (device[0].name != nullptr) ? device[0].name : string_empty);
            response->print(endColumn);
            response->print(endRow);
            #if defined(ACT_AS_BEACON)
              //Static
              response->print(startRow);
              response->print(startTwelveColumns);
              response->printf_P(labelForPrintfFormatString, string_deviceUsuallyStatic, PSTR("Usually static"));
              response->printf_P(selectPrintfFormatString, string_deviceUsuallyStatic, string_deviceUsuallyStatic);
              response->print(selectValueTrue);response->print(deviceUsuallyStatic == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(deviceUsuallyStatic == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
            #endif
            //IC Name
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_icName, PSTR("IC name"));
            response->printf_P(inputTextPrintfFormatString, string_icName, string_icName, (device[0].icName != nullptr) ? device[0].icName : string_empty);
            response->print(endColumn);
            response->print(endRow);
            //IC Description
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_icDescription, PSTR("IC description"));
            response->printf_P(inputTextPrintfFormatString, string_icDescription, string_icDescription, (device[0].icDescription != nullptr) ? device[0].icDescription : string_empty);
            response->print(endColumn);
            response->print(endRow);
            //Diameter
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_diameter, PSTR("Diameter(m)"));
            response->printf_P(inputNumberPrintfFormatString, string_diameter, string_diameter, device[0].diameter);
            response->print(endColumn);
            //Height
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_height, PSTR("Height(m)"));
            response->printf_P(inputNumberPrintfFormatString, string_height, string_height, device[0].height);
            response->print(endColumn);
            //Comment
            response->printf_P(h3printfFormatString, PSTR("Configuration"));
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_configurationComment, PSTR("Comment (optional)"));
            response->printf_P(inputTextPrintfFormatString, string_configurationComment, string_configurationComment, (configurationComment != nullptr) ? configurationComment : string_empty);
            response->print(endColumn);
            response->print(endRow);
            response->print(formEnd);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/configureIcInfo"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
          #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
            if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
            {
                return request->requestAuthentication();  //Force basic authentication
            }
          #endif
          //Read the submitted configuration
          if(request->hasParam(string_deviceName, true))
          {
            if(device[0].name != nullptr)
            {
              delete [] device[0].name;
            }
            device[0].name = new char[request->getParam(string_deviceName, true)->value().length() + 1];
            strlcpy(device[0].name,request->getParam(string_deviceName, true)->value().c_str(),request->getParam(string_deviceName, true)->value().length() + 1);
          }
          if(request->hasParam(string_deviceUsuallyStatic, true))
          {
            if(request->getParam(string_deviceUsuallyStatic, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
            {
              if(deviceUsuallyStatic != true)
              {
                deviceUsuallyStatic = true;
              }
            }
            else
            {
              if(deviceUsuallyStatic != false)
              {
                deviceUsuallyStatic = false;
              }
            }
          }
          if(request->hasParam(string_configurationComment, true))
          {
            if(configurationComment != nullptr)
            {
              delete [] configurationComment;
            }
            configurationComment = new char[request->getParam(string_configurationComment, true)->value().length() + 1];
            strlcpy(configurationComment,request->getParam(string_configurationComment, true)->value().c_str(),request->getParam(string_configurationComment, true)->value().length() + 1);
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
          if(request->hasParam(string_icName, true))
          {
            if(device[0].icName != nullptr)
            {
              delete [] device[0].icName;
            }
            device[0].icName = new char[request->getParam(string_icName, true)->value().length() + 1];
            strlcpy(device[0].icName,request->getParam(string_icName, true)->value().c_str(),request->getParam(string_icName, true)->value().length() + 1);
          }
          if(request->hasParam(string_icName, true))
          {
            if(device[0].icDescription != nullptr)
            {
              delete [] device[0].icDescription;
            }
            device[0].icDescription = new char[request->getParam(string_icDescription, true)->value().length() + 1];
            strlcpy(device[0].icDescription,request->getParam(string_icDescription, true)->value().c_str(),request->getParam(string_icDescription, true)->value().length() + 1);
          }
          if(request->hasParam(string_diameter, true))
          {
            if(device[0].diameter != request->getParam(string_diameter, true)->value().toInt())
            {
              device[0].diameter = request->getParam(string_diameter, true)->value().toInt();
            }
          }
          if(request->hasParam(string_height, true))
          {
            if(device[0].height != request->getParam(string_height, true)->value().toInt())
            {
              device[0].height = request->getParam(string_height, true)->value().toInt();
            }
          }
          saveConfigurationSoon = millis();
          request->redirect("/admin");
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/configureTracking"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->print(formStart);
            response->printf_P(h2printfEightColumnsFormatString, PSTR("General configuration"));
            response->print(startFourColumns);
            response->print(saveButton);
            response->print(endColumn);
            response->print(endRow);
            //Name
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_maximumEffectiveRange, PSTR("Maximum effective range"));
            response->printf_P(selectPrintfFormatString, string_maximumEffectiveRange, string_maximumEffectiveRange);
            response->print(F("<option value=\"50\""));response->print(maximumEffectiveRange == 50 ? selectValueSelected:selectValueNotSelected);response->print(F("50m</option>"));
            response->print(F("<option value=\"75\""));response->print(maximumEffectiveRange == 75 ? selectValueSelected:selectValueNotSelected);response->print(F("75m</option>"));
            response->print(F("<option value=\"99\""));response->print(maximumEffectiveRange == 99 ? selectValueSelected:selectValueNotSelected);response->print(F("99m</option>"));
            response->print(F("<option value=\"100\""));response->print(maximumEffectiveRange == 100 ? selectValueSelected:selectValueNotSelected);response->print(F("100m</option>"));
            response->print(F("<option value=\"150\""));response->print(maximumEffectiveRange == 150 ? selectValueSelected:selectValueNotSelected);response->print(F("150m</option>"));
            response->print(F("<option value=\"250\""));response->print(maximumEffectiveRange == 250 ? selectValueSelected:selectValueNotSelected);response->print(F("250m</option>"));
            response->print(F("<option value=\"500\""));response->print(maximumEffectiveRange == 500 ? selectValueSelected:selectValueNotSelected);response->print(F("500m</option>"));
            response->print(F("<option value=\"750\""));response->print(maximumEffectiveRange == 750 ? selectValueSelected:selectValueNotSelected);response->print(F("750m</option>"));
            response->print(F("<option value=\"1000\""));response->print(maximumEffectiveRange == 1000 ? selectValueSelected:selectValueNotSelected);response->print(F("1000m</option>"));
            response->print(F("<option value=\"9999\""));response->print(maximumEffectiveRange == 9999 ? selectValueSelected:selectValueNotSelected);response->print(F("9999m</option>"));
            response->print(F("<option value=\"10000\""));response->print(maximumEffectiveRange == 10000 ? selectValueSelected:selectValueNotSelected);response->print(F("10000m</option>"));
            response->print(F("<option value=\""));response->print(effectivelyUnreachable);response->print('"');response->print(maximumEffectiveRange == effectivelyUnreachable ? selectValueSelected:selectValueNotSelected);response->print(F("Unlimited</option>"));
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            #if defined(ACT_AS_TRACKER)
              response->print(startRow);
              response->print(startTwelveColumns);
              response->printf_P(labelForPrintfFormatString, string_trackerPriority, PSTR("Show distance to"));
              response->printf_P(selectPrintfFormatString, string_trackerPriority, string_trackerPriority);
              response->print(F("<option value=\"0\""));response->print(trackerPriority == 0 ? selectValueSelected:selectValueNotSelected);response->print(F("Centre</option>"));
              response->print(F("<option value=\"1\""));response->print(trackerPriority == 1 ? selectValueSelected:selectValueNotSelected);response->print(F("Edge</option>"));
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
            #endif
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_trackingSensitivity, PSTR("Sensitivity"));
            response->printf_P(selectPrintfFormatString, string_trackingSensitivity, string_trackingSensitivity);
            response->print(F("<option value=\"0\""));response->print(trackingSensitivity == 0 ? selectValueSelected:selectValueNotSelected);response->print(F("Low</option>"));
            response->print(F("<option value=\"1\""));response->print(trackingSensitivity == 1 ? selectValueSelected:selectValueNotSelected);response->print(F("Medium</option>"));
            response->print(F("<option value=\"2\""));response->print(trackingSensitivity == 2 ? selectValueSelected:selectValueNotSelected);response->print(F("High</option>"));
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            response->print(formEnd);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/configureTracking"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
          #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
            if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
            {
                return request->requestAuthentication();  //Force basic authentication
            }
          #endif
          //Read the submitted configuration
          if(request->hasParam(string_trackingSensitivity, true))
          {
            trackingSensitivity = request->getParam(string_trackingSensitivity, true)->value().toInt();
          }
          #if defined(ACT_AS_TRACKER)
            if(request->hasParam(string_trackerPriority, true))
            {
              trackerPriority = request->getParam(string_trackerPriority, true)->value().toInt();
            }
          #endif
          if(request->hasParam(string_maximumEffectiveRange, true))
          {
            maximumEffectiveRange = request->getParam(string_maximumEffectiveRange, true)->value().toInt();
          }
          saveConfigurationSoon = millis();
          request->redirect("/admin");
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/configureTreacle"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->print(formStart);
            response->printf_P(h2printfEightColumnsFormatString, PSTR("Treacle configuration"));
            response->print(startFourColumns);
            response->print(saveButton);
            response->print(endColumn);
            response->print(endRow);
            //Treacle encryption
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_treacleEncryptionEnabled, PSTR("Encryption enabled"));
            response->printf_P(selectPrintfFormatString, string_treacleEncryptionEnabled, string_treacleEncryptionEnabled);
            response->print(selectValueTrue);response->print(treacleEncryptionEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
            response->print(selectValueFalse);response->print(treacleEncryptionEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            //ESP-Now
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_espNowEnabled, PSTR("ESP&#8209;Now radio enabled"));
            response->printf_P(selectPrintfFormatString, string_espNowEnabled, string_espNowEnabled);
            response->print(selectValueTrue);response->print(espNowEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
            response->print(selectValueFalse);response->print(espNowEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            //LR mode
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_espNowPhyMode, PSTR("ESP&#8209;Now PHY"));
            response->printf_P(selectPrintfFormatString, string_espNowPhyMode, string_espNowPhyMode);
            response->print(F("<option value=\"0\""));response->print(espNowPhyMode == 0 ? selectValueSelected:selectValueNotSelected);response->print(F("BGN(default)</option>"));
            response->print(F("<option value=\"1\""));response->print(espNowPhyMode == 1 ? selectValueSelected:selectValueNotSelected);response->print(F("B</option>"));
            response->print(F("<option value=\"2\""));response->print(espNowPhyMode == 2 ? selectValueSelected:selectValueNotSelected);response->print(F("LR(long range)</option>"));
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            //ESP-Now tick interval
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_espNowTickInterval, PSTR("ESP&#8209;Now tick interval"));
            response->printf_P(selectPrintfFormatString, string_espNowTickInterval, string_espNowTickInterval);
            for(uint32_t value = 5000; value <= 55000; value+=5000)
            {
              response->printf_P(PSTR("<option value=\"%u\""), value);
              response->print(espNowTickInterval == value ? selectValueSelected:selectValueNotSelected);
              response->print(value/1000);
              response->print(F("s</option>"));
            }
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_loRaEnabled, PSTR("LoRa radio enabled"));
            response->printf_P(selectPrintfFormatString, string_loRaEnabled, string_loRaEnabled);
            response->print(selectValueTrue);response->print(loRaEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
            response->print(selectValueFalse);response->print(loRaEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            //ESP-Now tick interval
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_loRaTickInterval, PSTR("LoRa tick interval"));
            response->printf_P(selectPrintfFormatString, string_loRaTickInterval, string_loRaTickInterval);
            for(uint32_t value = 5000; value <= 55000; value+=5000)
            {
              response->printf_P(PSTR("<option value=\"%u\""), value);
              response->print(loRaTickInterval == value ? selectValueSelected:selectValueNotSelected);
              response->print(value/1000);
              response->print(F("s</option>"));
            }
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            //TX power
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_loRaTxPower, PSTR("LoRa Tx power"));
            response->printf_P(selectPrintfFormatString, string_loRaTxPower, string_loRaTxPower);
            for(uint8_t value = 2; value <= 20; value++)
            {
              response->printf_P(PSTR("<option value=\"%u\""), value);
              response->print(loRaTxPower == value ? selectValueSelected:selectValueNotSelected);
              response->print(value);
              response->print(F("dBm</option>"));
            }
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            //RX gain
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_loRaRxGain, PSTR("LoRa Rx gain"));
            response->printf_P(selectPrintfFormatString, string_loRaRxGain, string_loRaRxGain);
            response->printf_P(PSTR("<option value=\"%u\""), 0);
            response->print(loRaRxGain == 0 ? selectValueSelected:selectValueNotSelected);
            response->print(F("Auto</option>"));
            for(uint8_t value = 1; value <= 6; value++)
            {
              response->printf_P(PSTR("<option value=\"%u\""), value);
              response->print(loRaRxGain == value ? selectValueSelected:selectValueNotSelected);
              response->print(value);
              response->print(F("</option>"));
            }
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            //Spreading factor
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_loRaSpreadingFactor, PSTR("LoRa spreading factor"));
            response->printf_P(selectPrintfFormatString, string_loRaSpreadingFactor, string_loRaSpreadingFactor);
            for(uint8_t value = 7; value <= 12; value++)
            {
              response->printf_P(PSTR("<option value=\"%u\""), value);
              response->print(loRaSpreadingFactor == value ? selectValueSelected:selectValueNotSelected);
              response->print(value);
              response->print(F("</option>"));
            }
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            //Signal bandwidth
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(labelForPrintfFormatString, string_loRaSignalBandwidth, PSTR("LoRa signal bandwidth"));
            response->printf_P(selectPrintfFormatString, string_loRaSignalBandwidth, string_loRaSignalBandwidth);
            for(uint8_t index = 0; index < 10; index++)
            {
              response->printf_P(PSTR("<option value=\"%u\""), validLoRaSignalBandwidth[index]);
              response->print(loRaSignalBandwidth == validLoRaSignalBandwidth[index] ? selectValueSelected:selectValueNotSelected);
              response->print(validLoRaSignalBandwidth[index]/1000.0);
              response->print(F("baud</option>"));
            }
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            //End of form
            response->print(formEnd);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/configureTreacle"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
          #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
            if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
            {
                return request->requestAuthentication();  //Force basic authentication
            }
          #endif
          //Read the submitted configuration
          bool treacleConfigurationChanged = false;
          if(request->hasParam(string_espNowEnabled, true))
          {
            if(request->getParam(string_espNowEnabled, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
            {
              if(espNowEnabled == false)
              {
                espNowEnabled = true;
                treacleConfigurationChanged = true;
              }
            }
            else
            {
              if(espNowEnabled == true)
              {
                espNowEnabled = false;
                treacleConfigurationChanged = true;
              }
            }
          }
          if(request->hasParam(string_espNowPhyMode, true))
          {
            if(espNowPhyMode != request->getParam(string_espNowPhyMode, true)->value().toInt())
            {
              espNowPhyMode = request->getParam(string_espNowPhyMode, true)->value().toInt();
              treacleConfigurationChanged = true;
            }
          }
          if(request->hasParam(string_espNowTickInterval, true))
          {
            if(espNowTickInterval != request->getParam(string_espNowTickInterval, true)->value().toInt())
            {
              espNowTickInterval = request->getParam(string_espNowTickInterval, true)->value().toInt();
              treacle.setEspNowTickInterval(espNowTickInterval);
              treacleConfigurationChanged = true;
            }
          }
          if(request->hasParam(string_loRaEnabled, true))
          {
            if(request->getParam(string_loRaEnabled, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
            {
              if(loRaEnabled == false)
              {
                loRaEnabled = true;
                treacleConfigurationChanged = true;
              }
            }
            else
            {
              if(loRaEnabled == true)
              {
                loRaEnabled = false;
                treacleConfigurationChanged = true;
              }
            }
          }
          if(request->hasParam(string_loRaTickInterval, true))
          {
            if(loRaTickInterval != request->getParam(string_loRaTickInterval, true)->value().toInt())
            {
              loRaTickInterval = request->getParam(string_loRaTickInterval, true)->value().toInt();
              treacle.setLoRaTickInterval(loRaTickInterval);
              treacleConfigurationChanged = true;
            }
          }
          if(request->hasParam(string_loRaTxPower, true))
          {
            if(loRaTxPower != request->getParam(string_loRaTxPower, true)->value().toInt())
            {
              loRaTxPower = request->getParam(string_loRaTxPower, true)->value().toInt();
              treacleConfigurationChanged = true;
            }
          }
          if(request->hasParam(string_loRaRxGain, true))
          {
            if(loRaRxGain != request->getParam(string_loRaRxGain, true)->value().toInt())
            {
              loRaRxGain = request->getParam(string_loRaRxGain, true)->value().toInt();
              treacleConfigurationChanged = true;
            }
          }
          if(request->hasParam(string_loRaSpreadingFactor, true))
          {
            if(loRaSpreadingFactor != request->getParam(string_loRaSpreadingFactor, true)->value().toInt())
            {
              loRaSpreadingFactor = request->getParam(string_loRaSpreadingFactor, true)->value().toInt();
              treacleConfigurationChanged = true;
            }
          }
          if(request->hasParam(string_loRaSignalBandwidth, true))
          {
            if(loRaSignalBandwidth != request->getParam(string_loRaSignalBandwidth, true)->value().toInt())
            {
              loRaSignalBandwidth = request->getParam(string_loRaSignalBandwidth, true)->value().toInt();
              treacleConfigurationChanged = true;
            }
          }
          if(treacleConfigurationChanged == true)
          {
            saveConfigurationSoon = millis();
          }
          request->redirect("/admin");
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(SUPPORT_LVGL)
        adminWebServer->on(PSTR("/configureGui"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->print(formStart);
              response->printf_P(h2printfEightColumnsFormatString, PSTR("GUI configuration"));
              response->print(startFourColumns);
              response->print(saveButton);
              response->print(endColumn);
              response->print(endRow);
              response->print(startRow);
              response->print(startEightColumns);
              response->print(F("&nbsp;"));
              response->print(endColumn);
              #if defined(SUPPORT_TOUCHSCREEN)
                response->print(startFourColumns);
                response->printf_P(buttonPrintfFormatString, PSTR("touchscreen"), PSTR("Reset touchscreen"));
                response->print(endColumn);
              #endif
              response->print(endRow);
              response->print(startRow);
              #if defined(LVGL_SUPPORT_HOME_TAB)
                response->print(startTwelveColumns);
                response->printf_P(labelForPrintfFormatString, string_enableHomeTab, PSTR("Home tab"));
                response->printf_P(selectPrintfFormatString, string_enableHomeTab, string_enableHomeTab);
                response->print(selectValueTrue);response->print(enableHomeTab == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                response->print(selectValueFalse);response->print(enableHomeTab == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
              #endif
              #if defined(LVGL_SUPPORT_MAP_TAB)
                response->print(startRow);
                response->print(startTwelveColumns);
                response->printf_P(labelForPrintfFormatString, string_enableMapTab, PSTR("Map tab"));
                response->printf_P(selectPrintfFormatString, string_enableMapTab, string_enableMapTab);
                response->print(selectValueTrue);response->print(enableMapTab == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                response->print(selectValueFalse);response->print(enableMapTab == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
              #endif
              #if defined(LVGL_SUPPORT_SCAN_INFO_TAB)
                response->print(startRow);
                response->print(startTwelveColumns);
                response->printf_P(labelForPrintfFormatString, string_enableInfoTab, PSTR("Info tab"));
                response->printf_P(selectPrintfFormatString, string_enableInfoTab, string_enableInfoTab);
                response->print(selectValueTrue);response->print(enableInfoTab == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                response->print(selectValueFalse);response->print(enableInfoTab == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
              #endif
              #if defined(LVGL_SUPPORT_GPS_TAB)
                response->print(startRow);
                response->print(startTwelveColumns);
                response->printf_P(labelForPrintfFormatString, string_enableGpsTab, PSTR("GPS tab"));
                response->printf_P(selectPrintfFormatString, string_enableGpsTab, string_enableGpsTab);
                response->print(selectValueTrue);response->print(enableGpsTab == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                response->print(selectValueFalse);response->print(enableGpsTab == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
              #endif
              #if defined(LVGL_SUPPORT_SETTINGS_TAB)
                response->print(startRow);
                response->print(startTwelveColumns);
                response->printf_P(labelForPrintfFormatString, string_enableSettingsTab, PSTR("Settings tab"));
                response->printf_P(selectPrintfFormatString, string_enableSettingsTab, string_enableSettingsTab);
                response->print(selectValueTrue);response->print(enableSettingsTab == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                response->print(selectValueFalse);response->print(enableSettingsTab == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
              #endif
              //End of form
              response->print(formEnd);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/configureGui"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            //Read the submitted configuration
            bool lvglConfigurationChanged = false;
            #if defined(LVGL_SUPPORT_HOME_TAB)
              if(request->hasParam(string_enableHomeTab, true))
              {
                if(request->getParam(string_enableHomeTab, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  if(enableHomeTab == false)
                  {
                    enableHomeTab = true;
                    lvglConfigurationChanged = true;
                  }
                }
                else
                {
                  if(enableHomeTab == true)
                  {
                    enableHomeTab = false;
                    lvglConfigurationChanged = true;
                  }
                }
              }
            #endif
            #if defined(LVGL_SUPPORT_MAP_TAB)
              if(request->hasParam(string_enableMapTab, true))
              {
                if(request->getParam(string_enableMapTab, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  if(enableMapTab == false)
                  {
                    enableMapTab = true;
                    lvglConfigurationChanged = true;
                  }
                }
                else
                {
                  if(enableMapTab == true)
                  {
                    enableMapTab = false;
                    lvglConfigurationChanged = true;
                  }
                }
              }
            #endif
            #if defined(LVGL_SUPPORT_SCAN_INFO_TAB)
              if(request->hasParam(string_enableInfoTab, true))
              {
                if(request->getParam(string_enableInfoTab, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  if(enableInfoTab == false)
                  {
                    enableInfoTab = true;
                    lvglConfigurationChanged = true;
                  }
                }
                else
                {
                  if(enableInfoTab == true)
                  {
                    enableInfoTab = false;
                    lvglConfigurationChanged = true;
                  }
                }
              }
            #endif
            #if defined(LVGL_SUPPORT_GPS_TAB)
              if(request->hasParam(string_enableGpsTab, true))
              {
                if(request->getParam(string_enableGpsTab, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  if(enableGpsTab == false)
                  {
                    enableGpsTab = true;
                    lvglConfigurationChanged = true;
                  }
                }
                else
                {
                  if(enableGpsTab == true)
                  {
                    enableGpsTab = false;
                    lvglConfigurationChanged = true;
                  }
                }
              }
            #endif
            #if defined(LVGL_SUPPORT_SETTINGS_TAB)
              if(request->hasParam(string_enableSettingsTab, true))
              {
                if(request->getParam(string_enableSettingsTab, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
                {
                  if(enableSettingsTab == false)
                  {
                    enableSettingsTab = true;
                    lvglConfigurationChanged = true;
                  }
                }
                else
                {
                  if(enableSettingsTab == true)
                  {
                    enableSettingsTab = false;
                    lvglConfigurationChanged = true;
                  }
                }
              }
            #endif
            if(lvglConfigurationChanged == true)
            {
              saveConfigurationSoon = millis();
            }
            request->redirect("/admin");
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(SUPPORT_FTM)
        adminWebServer->on(PSTR("/configureFtm"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->print(formStart);
              response->printf_P(h2printfEightColumnsFormatString, PSTR("FTM configuration"));
              response->print(startFourColumns);
              response->print(saveButton);
              response->print(endColumn);
              response->print(endRow);
              //FTM
              #if defined(ACT_AS_TRACKER)
                //State
                response->print(startRow);
                response->print(startSixColumns);
                response->print(F("Time-of-flight probe"));
                response->print(endColumn);
                response->print(startSixColumns);
                response->print(F("<select class=\"u-full-width\" id=\"ftmEnabled\" name=\"ftmEnabled\">"));
                response->print(selectValueTrue);response->print(ftmEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                response->print(selectValueFalse);response->print(ftmEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
                //SSID/PSK
                response->print(startRow);
                response->print(startSixColumns);
                response->print(F("SSID template (simple pattern search)"));
                response->print(endColumn);
                if(ftmSSID != nullptr)
                {
                  response->print(startSixColumns);
                  response->printf_P(PSTR("<input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"ftmSSID\" name=\"ftmSSID\">"), ftmSSID);
                  response->print(endColumn);
                }
                else
                {
                  response->print(startSixColumns);
                  response->print(F("<input class=\"u-full-width\" type=\"text\" value=\"\" id=\"ftmSSID\" name=\"ftmSSID\">"));
                  response->print(endColumn);
                }
                response->print(startRow);
                response->print(startSixColumns);
                response->print(F("Pre-shared key to try"));
                response->print(endColumn);
                response->print(startSixColumns);
                response->printf_P(inputPasswordPrintfFormatString, string_ftmPSK, string_ftmPSK);
                response->print(endColumn);
                response->print(endRow);
              #elif defined(ACT_AS_BEACON)
                //State
                response->print(startRow);
                response->print(startSixColumns);
                response->print(F("Time-of-flight beacon"));
                response->print(endColumn);
                response->print(startSixColumns);
                response->print(F("<select class=\"u-full-width\" id=\"ftmEnabled\" name=\"ftmEnabled\">"));
                response->print(selectValueTrue);response->print(ftmEnabled == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                response->print(selectValueFalse);response->print(ftmEnabled == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
                response->print(startRow);
                response->print(startSixColumns);
                response->print(F("SSID suffix (appended to any configured SSID)"));
                response->print(endColumn);
                if(ftmSSID != nullptr)
                {
                  response->print(startSixColumns);
                  response->printf_P(PSTR("<input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"ftmSSID\" name=\"ftmSSID\">"), ftmSSID);
                  response->print(endColumn);
                }
                else
                {
                  response->print(startSixColumns);
                  response->print(F("<input class=\"u-full-width\" type=\"text\" value=\"%s\" id=\"ftmSSID\" name=\"ftmSSID\">"));
                  response->print(endColumn);
                }
                response->print(startRow);
                response->print(startSixColumns);
                response->print(F("Hide FTM SSID (unless otherwise enabled)"));
                response->print(endColumn);
                response->print(startSixColumns);
                response->print(F("<select class=\"u-full-width\" id=\"ftmHideSSID\" name=\"ftmHideSSID\">"));
                response->print(selectValueTrue);response->print(ftmHideSSID == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
                response->print(selectValueFalse);response->print(ftmHideSSID == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
                response->print(startRow);
                response->print(startSixColumns);
                response->print(F("Pre-shared key (if not otherwise set)"));
                response->print(endColumn);
                response->print(startSixColumns);
                response->printf_P(inputPasswordPrintfFormatString, string_ftmPSK, string_ftmPSK);
                response->print(endColumn);
                response->print(endRow);
              #endif
              //End of form
              response->print(formEnd);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/configureFtm"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
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
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(SUPPORT_GPS)
        adminWebServer->on(PSTR("/configureGps"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->print(formStart);
              response->printf_P(h2printfEightColumnsFormatString, PSTR("GPS configuration"));
              response->print(startFourColumns);
              response->print(saveButton);
              response->print(endRow);
              response->print(startRow);
              response->print(startTwelveColumns);
              response->printf_P(labelForPrintfFormatString, string_useGpsForTimeSync, PSTR("Sync time with GPS"));
              response->printf_P(selectPrintfFormatString, string_useGpsForTimeSync, string_useGpsForTimeSync);
              response->print(selectValueTrue);response->print(useGpsForTimeSync == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(useGpsForTimeSync == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              response->print(startRow);
              response->print(startTwelveColumns);
              response->printf_P(labelForPrintfFormatString, string_movementThreshold, PSTR("Movement threshold(m/s)"));
              response->printf_P(PSTR("<input type=\"number\" id=\"movementThreshold\" name=\"movementThreshold\" class=\"u-full-width\" min=\"0.1\" max=\"5.0\" step=\"0.1\" value=\"%.1f\">"), movementThreshold);
              response->printf_P(labelForPrintfFormatString, string_smoothingFactor, PSTR("Movement smoothing factor"));
              response->printf_P(PSTR("<input type=\"number\" id=\"smoothingFactor\" name=\"smoothingFactor\" class=\"u-full-width\" min=\"0.05\" max=\"0.95\" step=\"0.05\" value=\"%.2f\">"), smoothingFactor);
              response->print(endColumn);
              response->print(endRow);
              #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
                response->print(startRow);
                response->print(startTwelveColumns);
                response->printf_P(labelForPrintfFormatString, string_gpsStationaryTimeout, PSTR("Switch off GPS when stationary after"));
                response->printf_P(selectPrintfFormatString, string_gpsStationaryTimeout, string_gpsStationaryTimeout);
                response->print(F("<option value=\"0\""));response->print(gpsStationaryTimeout == 0 ? selectValueSelected:selectValueNotSelected);response->print(F("Never</option>"));
                response->print(F("<option value=\"60000\""));response->print(gpsStationaryTimeout == 60000 ? selectValueSelected:selectValueNotSelected);response->print(F("1m</option>"));
                response->print(F("<option value=\"90000\""));response->print(gpsStationaryTimeout == 60000 ? selectValueSelected:selectValueNotSelected);response->print(F("90s</option>"));
                response->print(F("<option value=\"180000\""));response->print(gpsStationaryTimeout == 180000 ? selectValueSelected:selectValueNotSelected);response->print(F("3m</option>"));
                response->print(F("<option value=\"300000\""));response->print(gpsStationaryTimeout == 300000 ? selectValueSelected:selectValueNotSelected);response->print(F("5m</option>"));
                response->print(F("<option value=\"900000\""));response->print(gpsStationaryTimeout == 900000 ? selectValueSelected:selectValueNotSelected);response->print(F("15m</option>"));
                response->print(F("<option value=\"1800000\""));response->print(gpsStationaryTimeout == 1800000 ? selectValueSelected:selectValueNotSelected);response->print(F("30m</option>"));
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
                response->print(startRow);
                response->print(startTwelveColumns);
                response->printf_P(labelForPrintfFormatString, string_gpsCheckInterval, PSTR("Check for movement after"));
                response->printf_P(selectPrintfFormatString, string_gpsCheckInterval, string_gpsCheckInterval);
                response->print(F("<option value=\"0\""));response->print(gpsCheckInterval == 0 ? selectValueSelected:selectValueNotSelected);response->print(F("Never (dangerous)</option>"));
                response->print(F("<option value=\"180000\""));response->print(gpsCheckInterval == 180000 ? selectValueSelected:selectValueNotSelected);response->print(F("3m</option>"));
                response->print(F("<option value=\"120000\""));response->print(gpsCheckInterval == 300000 ? selectValueSelected:selectValueNotSelected);response->print(F("5m</option>"));
                response->print(F("<option value=\"900000\""));response->print(gpsCheckInterval == 900000 ? selectValueSelected:selectValueNotSelected);response->print(F("15m</option>"));
                response->print(F("<option value=\"1800000\""));response->print(gpsCheckInterval == 1800000 ? selectValueSelected:selectValueNotSelected);response->print(F("30m</option>"));
                response->print(F("<option value=\"3600000\""));response->print(gpsCheckInterval == 3600000 ? selectValueSelected:selectValueNotSelected);response->print(F("60m</option>"));
                response->print(endSelect);
                response->print(endColumn);
                response->print(endRow);
              #endif
              //End of form
              response->print(formEnd);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/configureGps"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
            if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
            {
          #endif
            #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
              if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
              {
                  return request->requestAuthentication();  //Force basic authentication
              }
            #endif
            //Read the submitted configuration
            bool gpsConfigurationChanged = false;
            if(request->hasParam(string_useGpsForTimeSync, true))
            {
              if(request->getParam(string_useGpsForTimeSync, true)->value().length() == 4) //Length 4 implies 'true' rather than 'false'
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
            if(request->hasParam(string_movementThreshold, true))
            {
              if(movementThreshold != request->getParam(string_movementThreshold, true)->value().toFloat())
              {
                movementThreshold = request->getParam(string_movementThreshold, true)->value().toFloat();
                localLog(F("movementThreshold: "));
                localLogLn(movementThreshold);
                gpsConfigurationChanged = true;
              }
            }
            if(request->hasParam(string_smoothingFactor, true))
            {
              if(smoothingFactor != request->getParam(string_smoothingFactor, true)->value().toFloat())
              {
                smoothingFactor = request->getParam(string_smoothingFactor, true)->value().toFloat();
                localLog(F("smoothingFactor: "));
                localLogLn(smoothingFactor);
                gpsConfigurationChanged = true;
              }
            }
            #if defined(SUPPORT_SOFT_POWER_OFF)
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
              if(request->hasParam(string_gpsCheckInterval, true))
              {
                if(gpsCheckInterval != request->getParam(string_gpsCheckInterval, true)->value().toInt())
                {
                  gpsCheckInterval = request->getParam(string_gpsCheckInterval, true)->value().toInt();
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
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/update"), HTTP_GET, [](AsyncWebServerRequest *request){
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->printf_P(h2printfFormatString, PSTR("Software update"));
              response->print(ulStart);
              response->printf_P(PSTR("<li>Current firmware: %u.%u.%u</li>"), device[0].majorVersion, device[0].minorVersion, device[0].patchVersion);
              response->printf_P(PSTR("<li>Built: %s %s</li>"), __TIME__, __DATE__);
              response->printf_P(PSTR("<li>Board: %s</li>"), ARDUINO_BOARD);
              #ifdef ESP_IDF_VERSION_MAJOR
                response->print(F("<li>ESP&#8209;IDF: v"));
                #ifdef ESP_IDF_VERSION_MINOR
                  response->print(ESP_IDF_VERSION_MAJOR);
                  response->print('.');
                  response->print(ESP_IDF_VERSION_MINOR);
                #else
                  response->print(ESP_IDF_VERSION_MAJOR);
                #endif
              #endif
              response->print(ulEndRow);
              response->print(F("<p>Before uploading any pre-compiled binary software, please check it is the version you want and for the right board after checking the information above.</p>"));
              response->print(F("<form method=\"POST\" action=\"/update\" enctype=\"multipart/form-data\">"));
              response->print(F("<input class=\"button-primary\" type=\"file\" name=\"update\"><br />"));
              response->print(startRow);
              /*
              response->print(startFourColumns);
              response->print(backButton);
              response->print(endColumn);
              */
              response->print(startFourColumns);
              repsonse->print(F("<input class=\"button-primary\" type=\"submit\" value=\"Update\" style=\"width: 100%;\">"));
              response->print(endColumn);
              response->print(endRow);
              response->print(formEnd);
              addPageFooter(response);
              //Send response
              request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/update"), HTTP_POST,[](AsyncWebServerRequest *request)
          { //This lambda function is called when the update is complete
            #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
                  response->printf_P(h2printfFormatString, PSTR("Update failed"));
                  response->print(startRow);
                  response->print(F("<p>The software update failed!</p>"));
                  response->print(endRow);
                  /*
                  response->print(startRow);
                  response->print(startFourColumns);
                  response->print(backButton);
                  response->print(endColumn);
                  response->print(endRow);
                  */
                  addPageFooter(response);
                  //Send response
                  request->send(response);
                  otaInProgress = false;
                }
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/postUpdateRestart"), HTTP_GET, [](AsyncWebServerRequest *request){
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->printf_P(h2printfFormatString, PSTR("Software update successful"));
            response->print(F("<p>The software update was successful and this node will restart in roughly 10 seconds.</p>"));
            /*
            response->print(startRow);
            response->print(startFourColumns);
            response->print(backButton);
            response->print(endColumn);
            response->print(endRow);
            */
            addPageFooter(response);
            //Send response
            request->send(response);
            restartTimer = millis();
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/configureLogging"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->print(formStart);
            response->printf_P(h2printfEightColumnsFormatString, PSTR("Logging configuration"));
            response->print(startFourColumns);
            response->print(saveButton);
            response->print(endColumn);
            response->print(endRow);
            //Start of form
            response->print(startRow);
            response->print(startFourColumns);
            response->printf_P(labelForPrintfFormatString, string_loggingBufferSize, PSTR("Buffer size"));
            response->printf_P(inputNumberPrintfFormatString, string_loggingBufferSize, string_loggingBufferSize, loggingBufferSize);
            response->print(endColumn);
            response->print(startFourColumns);
            response->printf_P(labelForPrintfFormatString, string_logFlushThreshold, PSTR("Buffer flush threshold"));
            response->printf_P(inputNumberPrintfFormatString, string_logFlushThreshold, string_logFlushThreshold, logFlushThreshold);
            response->print(endColumn);
            response->print(startFourColumns);
            response->printf_P(labelForPrintfFormatString, string_logFlushInterval, PSTR("Buffer flush frequency"));
            response->printf_P(selectPrintfFormatString, string_logFlushInterval, string_logFlushInterval);
            response->print(F("<option value=\"30\""));response->print(logFlushInterval == 30 ? selectValueSelected:selectValueNotSelected);response->print(F("30s</option>"));
            response->print(F("<option value=\"60\""));response->print(logFlushInterval == 60 ? selectValueSelected:selectValueNotSelected);response->print(F("60s</option>"));
            response->print(F("<option value=\"90\""));response->print(logFlushInterval == 90 ? selectValueSelected:selectValueNotSelected);response->print(F("90s</option>"));
            response->print(F("<option value=\"180\""));response->print(logFlushInterval == 180 ? selectValueSelected:selectValueNotSelected);response->print(F("3 minutes</option>"));
            response->print(F("<option value=\"300\""));response->print(logFlushInterval == 300 ? selectValueSelected:selectValueNotSelected);response->print(F("5 minutes</option>"));
            response->print(endSelect);
            response->print(endColumn);
            response->print(endRow);
            //End of form
            response->print(formEnd);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/configureLogging"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
          #if defined(ENABLE_LOCAL_WEBSERVER_BASIC_AUTH)
            if(basicAuthEnabled == true && request->authenticate(http_user, http_password) == false)
            {
                return request->requestAuthentication();  //Force basic authentication
            }
          #endif
          //Read the submitted configuration
          if(request->hasParam(string_loggingBufferSize, true))
          {
            if(loggingBufferSize != request->getParam(string_loggingBufferSize, true)->value().toInt())
            {
              loggingBufferSize = request->getParam(string_loggingBufferSize, true)->value().toInt();
            }
          }
          if(request->hasParam(string_logFlushThreshold, true))
          {
            if(logFlushThreshold != request->getParam(string_logFlushThreshold, true)->value().toInt())
            {
              logFlushThreshold = request->getParam(string_logFlushThreshold, true)->value().toInt();
            }
          }
          if(request->hasParam(string_logFlushInterval, true))
          {
            if(logFlushInterval != request->getParam(string_logFlushInterval, true)->value().toInt())
            {
              logFlushInterval = request->getParam(string_logFlushInterval, true)->value().toInt();
            }
          }
          saveConfigurationSoon = millis();
          request->redirect("/admin");
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(ACT_AS_SENSOR)
        adminWebServer->on(PSTR("/configureSensor"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->print(formStart);
              response->printf_P(h2printfEightColumnsFormatString, PSTR("Sensor configuration"));
              response->print(startFourColumns);
              response->print(saveButton);
              response->print(endColumn);
              response->print(endRow);
              response->printf_P(h3printfFormatString, PSTR("Starting values"));
              //Starting hits
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_numberOfStartingHits, PSTR("Starting hits"));
              response->printf_P(PSTR("<input class=\"u-full-width\" type=\"number\" min=\"1\" max=\"99\" step=\"1\" value=\"%u\" id=\"numberOfStartingHits\" name=\"numberOfStartingHits\">"), device[0].numberOfStartingHits);
              response->print(endColumn);
              response->print(endRow);
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_numberOfStartingStunHits, PSTR("Starting stun hits"));
              response->printf_P(PSTR("<input class=\"u-full-width\" type=\"number\" min=\"1\" max=\"99\" step=\"1\" value=\"%u\" id=\"numberOfStartingStunHits\" name=\"numberOfStartingStunHits\">"), device[0].numberOfStartingStunHits);
              response->print(endColumn);
              response->print(endRow);
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_armourValue, PSTR("Armour value"));
              response->printf_P(PSTR("<input class=\"u-full-width\" type=\"number\" min=\"0\" max=\"99\" step=\"1\" value=\"%u\" id=\"armourValue\" name=\"armourValue\">"), armourValue);
              response->print(endColumn);
              response->print(endRow);
              //Flags
              response->printf_P(h3printfFormatString, PSTR("Sensor flags"));
              //Require EP
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_EP_flag, PSTR("Require EP to hit"));
              response->printf_P(selectPrintfFormatString, string_EP_flag, string_EP_flag);
              response->print(selectValueTrue);response->print(EP_flag == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(EP_flag == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              //Ignore healing
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_ig_healing_flag, PSTR("Ignore healing"));
              response->printf_P(selectPrintfFormatString, string_ig_healing_flag, string_ig_healing_flag);
              response->print(selectValueTrue);response->print(ig_healing_flag == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(ig_healing_flag == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              //Ignore stun
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_ig_stun_flag, PSTR("Ignore stun"));
              response->printf_P(selectPrintfFormatString, string_ig_stun_flag, string_ig_stun_flag);
              response->print(selectValueTrue);response->print(ig_stun_flag == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(ig_stun_flag == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              //Ignore ongoing
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_ig_ongoing_flag, PSTR("Ignore ongoing"));
              response->printf_P(selectPrintfFormatString, string_ig_ongoing_flag, string_ig_ongoing_flag);
              response->print(selectValueTrue);response->print(ig_ongoing_flag == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(ig_ongoing_flag == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              //Regen while zero
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_regen_while_zero, PSTR("Regen while zero"));
              response->printf_P(selectPrintfFormatString, string_regen_while_zero, string_regen_while_zero);
              response->print(selectValueTrue);response->print(regen_while_zero == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(regen_while_zero == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              //Treat damage as one
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_treat_as_one, PSTR("Treat damage as one hit"));
              response->printf_P(selectPrintfFormatString, string_treat_as_one, string_treat_as_one);
              response->print(selectValueTrue);response->print(treat_as_one == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(treat_as_one == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              //Treat stun damage as one
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_treat_stun_as_one, PSTR("Treat stun as one hit"));
              response->printf_P(selectPrintfFormatString, string_treat_stun_as_one, string_treat_stun_as_one);
              response->print(selectValueTrue);response->print(treat_stun_as_one == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(treat_stun_as_one == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              //Ongoing is cumulative
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_ongoing_is_cumulative, PSTR("Ongoing is cumulative"));
              response->printf_P(selectPrintfFormatString, string_ongoing_is_cumulative, string_ongoing_is_cumulative);
              response->print(selectValueTrue);response->print(ongoing_is_cumulative == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(ongoing_is_cumulative == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              //Ignore non-DOT
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_ig_non_dot, PSTR("Ignore non-DOT signals"));
              response->printf_P(selectPrintfFormatString, string_ig_non_dot, string_ig_non_dot);
              response->print(selectValueTrue);response->print(ig_non_dot == true ? selectValueSelected:selectValueNotSelected);response->print(selectValueEnabled);
              response->print(selectValueFalse);response->print(ig_non_dot == false ? selectValueSelected:selectValueNotSelected);response->print(selectValueDisabled);
              response->print(endSelect);
              response->print(endColumn);
              response->print(endRow);
              //End of form
              response->print(formEnd);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/configureSensor"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
          //Read the submitted configuration
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
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/sensorReset"), HTTP_GET, [](AsyncWebServerRequest *request){
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(SUPPORT_LVGL)
        adminWebServer->on(PSTR("/touchscreen"), HTTP_GET, [](AsyncWebServerRequest *request){
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            addPageHeader(response, 5, "/admin");
            response->printf_P(h2printfFormatString, PSTR("Touchscreen"));
            //Top of page buttons
            /*
            response->print(startRow);
            response->print(startFourColumns);
            response->print(backButton);
            response->print(endColumn);
            response->print(endRow);
            */
            response->print(startRow);
            response->print(startTwelveColumns);
            response->print(F("Touchscreen settings wiped"));
            response->print(endColumn);
            response->print(endRow);
            addPageFooter(response);
            //Send response
            request->send(response);
            touchScreenMinimumX = 0;
            touchScreenMaximumX = 0;
            touchScreenMinimumY = 0;
            touchScreenMaximumY = 0;
            saveConfigurationSoon = millis();
            restartTimer = millis();
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined ENABLE_REMOTE_RESTART
        adminWebServer->on(PSTR("/restart"), HTTP_GET, [](AsyncWebServerRequest *request){
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->printf_P(h2printfFormatString, PSTR("Restart confirmation"));
            response->print(startRow);
            response->print(startTwelveColumns);
            response->printf_P(PSTR("<p>Are you sure you want to restart \"%s\"?</p>"),device[0].name);
            response->print(endColumn);
            response->print(endRow);
            response->print(startRow);
            response->print(startFourColumns);
            response->print(F("<a href =\"/restartConfirmed\"><input class=\"button-primary\" type=\"button\" value=\"Yes\" style=\"width: 100%;\"></a>"));
            response->print(endColumn);
            response->print(startFourColumns);
            response->print(F("<a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"No\" style=\"width: 100%;\"></a>"));
            response->print(endColumn);
            response->print(endRow);
            addPageFooter(response);
            //Send response
            request->send(response);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/restartConfirmed"), HTTP_GET, [](AsyncWebServerRequest *request){
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
            response->printf_P(h2printfFormatString, PSTR("Restart"));
            //Top of page buttons
            /*
            response->print(startRow);
            response->print(startFourColumns);
            response->print(backButton);
            response->print(endColumn);
            response->print(endRow);
            */
            response->print(startRow);
            response->print(startTwelveColumns);
            response->print(F("<p>This node is restarting in 10s</p>"));
            response->print(endColumn);
            response->print(endRow);
            addPageFooter(response);
            //Send response
            request->send(response);
            restartTimer = millis();
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/wipe"), HTTP_GET, [](AsyncWebServerRequest *request){
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
          response->printf_P(h2printfFormatString, PSTR("Setting wipe"));
          response->print(startRow);
          response->print(startTwelveColumns);
          response->printf_P(PSTR("<p>Are you sure you want to wipe all configuration of \"%s\"?</p>"),device[0].name);
          response->print(endColumn);
          response->print(endRow);
          response->print(startRow);
          response->print(startFourColumns);
          response->print(F("<a href =\"/wipeconfirmed\"><input class=\"button-primary\" type=\"button\" value=\"Yes\" style=\"width: 100%;\"></a>"));
          response->print(endColumn);
          response->print(startFourColumns);
          response->print(F("<a href =\"/admin\"><input class=\"button-primary\" type=\"button\" value=\"No\" style=\"width: 100%;\"></a>"));
          response->print(endColumn);
          response->print(endRow);
          addPageFooter(response);
          //Send response
          request->send(response);
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/wipeconfirmed"), HTTP_GET, [](AsyncWebServerRequest *request){
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
          response->printf_P(h2printfFormatString, PSTR("Wipe"));
          //Top of page buttons
          /*
          response->print(startRow);
          response->print(startTwelveColumns);
          response->print(backButton);
          response->print(endColumn);
          response->print(endRow);
          */
          response->print(startRow);
          response->print(startTwelveColumns);
          response->print(F("<p>This node is restarting in 10s</p>"));
          response->print(endColumn);
          response->print(endRow);
          addPageFooter(response);
          //Send response
          request->send(response);
          wipeTimer = millis();
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/listDevices"), HTTP_GET, [](AsyncWebServerRequest *request){
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
          addPageHeader(response, 90, "/listDevices");
          //Top of page buttons
          /*
          response->print(startRow);
          response->print(startFourColumns);
          response->print(backButton);
          response->print(endColumn);
          */
          #if defined(ACT_AS_TRACKER)
            response->print(startFourColumns);
            response->print(F("<a href =\"/nearest\"><input class=\"button-primary\" type=\"button\" value=\"Track nearest\" style=\"width: 100%;\"></a>"));
            response->print(endColumn);
            response->print(startFourColumns);
            response->print(F("<a href =\"/furthest\"><input class=\"button-primary\" type=\"button\" value=\"Track furthest\" style=\"width: 100%;\"></a>"));
            response->print(endColumn);
            response->print(endRow);
            response->print(startRow);
            response->print(startTwelveColumns);
            response->print(F("Tracking mode: "));
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
              if(currentlyTrackedDevice != maximumNumberOfDevices)
              {
                if(device[currentlyTrackedDevice].name != nullptr)
                {
                  response->print(F(" - "));
                  response->print(device[currentlyTrackedDevice].name);
                }
                else
                {
                  response->printf_P(PSTR(" - device %u"),currentlyTrackedDevice);
                }
              }
            }
            response->print(endColumn);
            response->print(endRow);
          #else
            response->print(divEnd);
          #endif
          if(numberOfDevices == maximumNumberOfDevices)
          {
            response->printf_P(h3printfFormatString, PSTR("Warning: maximum number of devices reached!"));
          }
          response->printf_P(h2printfFormatString, PSTR("Devices"));
          response->print(startRow);
          response->print(startTwelveColumns);
          response->print(F("<table><thead><tr><th>Name</th><th>IC</th><th>ID</th><th>Features</th><th>Version</th><th>Uptime</th><th>Battery</th><th>GPS</th><th>ESP&#8209;Now</th><th>LoRa</th><th>Last seen</th><th>Info</th></tr></thead><tbody>"));
          for(uint8_t index = 0; index < numberOfDevices; index++)
          {
            if(index > 0)
            {
              response->printf_P(PSTR("<tr><td>%s</td><td>%s</td><td>%02x</td><td>%s</td><td>v%u.%u.%u</td><td>%s</td><td>%s</td><td>Fix:%s Lat:%.3f Lon:%.3f Distance:%.1f Course:%.1f Speed:%s</td><td>RX&#8209;%04x TX&#8209;%04x</td><td>RX&#8209;%04x TX&#8209;%04x RSSI&nbsp;%idBm SNR&nbsp;%.2fdB</td><td>%s&nbsp;ago</td><td>"),
              (device[index].name == nullptr) ? "n/a" : device[index].name,
              (device[index].icName == nullptr) ? "" : device[index].icName,
              device[index].id,
              deviceFeatures(device[index].typeOfDevice).c_str(),
              device[index].majorVersion,device[index].minorVersion,device[index].patchVersion,
              (index == 0) ? printableDuration(millis()/1000).c_str() : printableDuration(device[index].uptime/1000).c_str(),
              (device[index].typeOfDevice & 0x40) ? String(device[index].supplyVoltage).c_str() : PSTR("n/a"),
              (device[index].hasGpsFix == true) ? PSTR("Yes") : PSTR("No"),
              device[index].latitude,
              device[index].longitude,
              device[index].distanceTo,
              device[index].courseTo,
              ((device[index].moving == true) ? String(device[index].smoothedSpeed).c_str() : PSTR("Stationary")),
              treacle.espNowRxReliability(device[index].id),
              treacle.espNowTxReliability(device[index].id),
              treacle.loRaRxReliability(device[index].id),
              treacle.loRaTxReliability(device[index].id),
              treacle.loRaRSSI(device[index].id),
              treacle.loRaSNR(device[index].id),
              printableDuration(treacle.rxAge(device[index].id)/1000.0).c_str()
              );
            }
            else
            {
              response->printf_P(PSTR("<tr><td>%s</td><td>%s</td><td>%02x</td><td>%s</td><td>v%u.%u.%u</td><td>%s</td><td>%s</td><td>Fix:%s Lat:%.3f Lon:%.3f Speed:%s</td><td></td><td></td><td></td><td>This device"),
              (device[index].name == nullptr) ? "n/a" : device[index].name,
              (device[index].icName == nullptr) ? "" : device[index].icName,
              device[index].id,
              deviceFeatures(device[index].typeOfDevice).c_str(),
              device[index].majorVersion,device[index].minorVersion,device[index].patchVersion,
              (index == 0) ? printableDuration(millis()/1000).c_str() : printableDuration(device[index].uptime/1000).c_str(),
              (device[index].typeOfDevice & 0x40) ? String(device[index].supplyVoltage).c_str() : PSTR("n/a"),
              (device[index].hasGpsFix == true) ? PSTR("Yes") : PSTR("No"),
              device[index].latitude,
              device[index].longitude,
              ((device[index].moving == true) ? String(device[index].smoothedSpeed).c_str() : PSTR("Stationary"))
              );
            }
              #if defined(ACT_AS_TRACKER)
                if(index == currentlyTrackedDevice)
                {
                  response->print(F("Tracked "));
                }
                else if(index != 0 && treacle.online(device[index].id) == false)
                {
                  response->print(F("Offline"));
                }
                else if(index != 0) 
                {
                  response->printf_P(PSTR("<a href =\"/track?index=%u\"><input class=\"button-primary\" type=\"button\" value=\"Track\" style=\"width: 100%;\"></a>"),index);
                }
              #else
                if(index == currentlyTrackedDevice)
                {
                  response->print(F("Closest tracker"));
                }
                else if(index != 0 && treacle.online(device[index].id) == false)
                {
                  response->print(F("Offline"));
                }
              #endif
              response->print(F("</td></tr>"));
          }
          response->print(F("</tbody></table>"));
          response->print(endColumn);
          response->print(endRow);
          addPageFooter(response);
          //Send response
          request->send(response);
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(ACT_AS_TRACKER)
        adminWebServer->on(PSTR("/nearest"), HTTP_GET, [](AsyncWebServerRequest *request){
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/furthest"), HTTP_GET, [](AsyncWebServerRequest *request){
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/track"), HTTP_GET, [](AsyncWebServerRequest *request){
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
                  currentlyTrackedDevice = selectedBeacon;
                  currentTrackingMode = trackingMode::fixed;
                  #if defined(SERIAL_DEBUG)
                    if(waitForBufferSpace(75))
                    {
                      SERIAL_DEBUG_PORT.printf_P(PSTR("Web UI chose device %u to track\r\n"),currentlyTrackedDevice);
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
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      #if defined(SUPPORT_HACKING)
        adminWebServer->on(PSTR("/configureGame"), HTTP_GET, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
              response->printf_P(h2printfFormatString, PSTR("Game configuration"));
              //Start of form
              response->print(formStart);
              response->printf_P(h2printfEightColumnsFormatString, PSTR("Starting values"));
              response->print(startFourColumns);
              response->print(saveButton);
              response->print(endColumn);
              response->print(endRow);
              //Starting hits
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_gameLength, PSTR("Game length/win threshold"));
              response->printf_P(PSTR("<input class=\"u-full-width\" type=\"number\" min=\"1\" max=\"99\" step=\"1\" value=\"%u\" id=\"gameLength\" name=\"gameLength\">"), gameLength);
              response->print(endColumn);
              response->print(endRow);
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_gameRetries, PSTR("Game retries"));
              response->printf_P(PSTR("<input class=\"u-full-width\" type=\"number\" min=\"0\" max=\"99\" step=\"1\" value=\"%u\" id=\"gameRetries\" name=\"gameRetries\">"), gameRetries);
              response->print(endColumn);
              response->print(endRow);
              response->print(startRow);
              response->print(startSixColumns);
              response->printf_P(labelForPrintfFormatString, string_gameSpeedup, PSTR("Game speedup(ms)"));
              response->printf_P(PSTR("<input class=\"u-full-width\" type=\"number\" min=\"100\" max=\"2000\" step=\"1\" value=\"%u\" id=\"gameSpeedup\" name=\"gameSpeedup\">"), gameSpeedup);
              response->print(endColumn);
              response->print(endRow);
              //End of form
              response->print(formEnd);
              addPageFooter(response);
              //Send response
              request->send(response);
          #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
        adminWebServer->on(PSTR("/configureGame"), HTTP_POST, [](AsyncWebServerRequest *request){ //This lambda function shows the configuration for editing
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
          //Read the submitted configuration
          if(request->hasParam(string_gameLength, true))
          {
            gameLength = request->getParam(string_gameLength, true)->value().toInt();
          }
          if(request->hasParam(string_gameRetries, true))
          {
            gameRetries = request->getParam(string_gameRetries, true)->value().toInt();
          }
          if(request->hasParam(string_gameSpeedup, true))
          {
            gameSpeedup = request->getParam(string_gameSpeedup, true)->value().toInt();
          }
          saveConfigurationSoon = millis();
          request->redirect("/admin");
      #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/css/normalize.css"), HTTP_GET, [](AsyncWebServerRequest *request) {
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
            lastWifiActivity = millis();
            request->send_P(200, "text/css", normalize);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
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
      adminWebServer->on(PSTR("/css/skeleton.css"), HTTP_GET, [](AsyncWebServerRequest *request) {
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
          if(xSemaphoreTake(webserverSemaphore, webserverSemaphoreTimeout) == pdTRUE)
          {
        #endif
            lastWifiActivity = millis();
            request->send_P(200, "text/css", skeleton);
        #if defined(ENABLE_LOCAL_WEBSERVER_SEMAPHORE)
            xSemaphoreGive(webserverSemaphore);
          }
          else
          {
            request->send(500); //Sends 500 if the server is busy
          }
        #endif
      });
      adminWebServer->on(PSTR("/favicon.ico"), HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(404); //There is no favicon!
      });
      #if defined(SUPPORT_HACKING) //ESPUI already does a redirect
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
      #if defined(SUPPORT_HACKING)
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
      #if defined(SUPPORT_HACKING)
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

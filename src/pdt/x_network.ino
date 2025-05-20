#if defined(SUPPORT_WIFI)
  void setupNetwork()
  {
    #if defined(SUPPORT_BUTTON)
      if(digitalRead(buttonPin) == false) //Start WiFi if button pushed
      {
        startWiFiApOnBoot = true;
      }
    #endif
    #if defined(SERIAL_DEBUG)
      espBoilerplate.setOutputStream(SERIAL_DEBUG_PORT);
    #endif
    espBoilerplate.setRetries(wifiClientTimeout);
    #if defined(SUPPORT_FTM)
      if(ftmEnabled == true)
      {
        espBoilerplate.enableFtm();
      }
    #endif
    //Get the local MAC address
    //esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
    if(startWiFiClientOnBoot == true || startWiFiApOnBoot == true)
    {
      if(startWiFiClientOnBoot == true)
      {
        wifiClientConnected = espBoilerplate.begin(SSID, PSK, startWiFiApOnBoot == false); //Last parameter means it only shows IP info if no AP
      }
      if(startWiFiApOnBoot == true)
      {
        if(wifiClientConnected == false)
        {
          espBoilerplate.setApChannel(softApChannel);
          wifiApStarted = espBoilerplate.beginAp(APSSID, APPSK);
        }
        else
        {
          wifiApStarted = espBoilerplate.beginAp(APSSID, APPSK);
        }
      }
      #if defined(ENABLE_LOCAL_WEBSERVER)
        if(wifiApStarted == true && enableCaptivePortal == true)
        {
          localLog(F("Starting captive portal DNS server: "));
          dnsServer = new DNSServer;
          dnsServer->start(53, "*", WiFi.softAPIP());  //DNS server is required for captive portal to work
          localLogLn(F("OK"));
        }
      #endif
      if(wifiApStarted == true || wifiClientConnected == true)
      {
        espBoilerplate.setHostname(device[0].name); //Set mDNS name
      }
      if(wifiClientConnected == true && timeServer != nullptr)
      {
        configureTimeServer();  //Set up the time server
      }
      #if defined(SUPPORT_HACKING)
        if(sensorReset == false)
        {
          setupHacking();
        }
      #endif
      #if defined(ENABLE_LOCAL_WEBSERVER)
        #if defined(ACT_AS_SENSOR)
          #if defined(SUPPORT_HACKING)
            if(sensorReset == true && (wifiApStarted == true || wifiClientConnected == true))
            {
              setupWebServer();
              #if defined(SUPPORT_OTA)
                if(otaEnabled == true)
                {
                    configureOTA();
                }
              #endif
            }
          #else
            if(wifiApStarted == true || wifiClientConnected == true)
            {
              setupWebServer();
              #if defined(SUPPORT_OTA)
                if(otaEnabled == true)
                {
                    configureOTA();
                }
              #endif
            }
          #endif
        #else
          if(wifiApStarted == true || wifiClientConnected == true)
          {
            setupWebServer();
            #if defined(SUPPORT_OTA)
              if(otaEnabled == true)
              {
                  configureOTA();
              }
            #endif
          }
        #endif
      #endif
    }
    else
    {
      localLogLn(F("Disabling WiFi"));
      WiFi.mode(WIFI_OFF);
    }
  }
  void manageNetwork()
  {
    #if defined(ENABLE_LOCAL_WEBSERVER) || defined(SUPPORT_HACKING)
      if(enableCaptivePortal == true && dnsServer != nullptr) //Must process inbound DNS requests
      {
        dnsServer->processNextRequest();
      }
    #endif
    #if defined(SUPPORT_WIFI)
      if(wifiClientConnected == true && wiFiClientInactivityTimer > 0 && millis() - lastWifiActivity > wiFiClientInactivityTimer)
      {
        lastWifiActivity = millis();
        localLogLn(F("WiFi inactive, disconnecting"));
        WiFi.disconnect();
        //WiFi.mode(WIFI_OFF);
        wifiClientConnected = false;
      }
      #if defined(ACT_AS_SENSOR)
        if(sensorReset == true && restartTimer == 0 && millis() - lastWifiActivity > 60000) //Admin mode timeout
        {
          localLogLn(F("Admin web portal idle, restarting!"));
          restartTimer = millis();
        }
      #endif
    #endif
  }
#endif

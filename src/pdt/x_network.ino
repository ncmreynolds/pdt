#if defined(SUPPORT_WIFI)
  void setupNetwork()
  {
    #ifdef ACT_AS_TRACKER
      if(digitalRead(buttonPin) == false) //Start WiFi if button pushed
      {
        startWiFiApOnBoot = true;
      }
    #endif
    #if defined(SERIAL_DEBUG)
      espBoilerplate.setOutputStream(Serial);
    #endif
    espBoilerplate.setRetries(wifiClientTimeout);
    if(startWiFiClientOnBoot == true || startWiFiApOnBoot == true)
    {
      if(startWiFiClientOnBoot == true)
      {
        wifiClientConnected = espBoilerplate.begin(SSID, PSK, startWiFiApOnBoot == false); //Last parameter means it only shows IP info if no AP
      }
      if(startWiFiApOnBoot == true)
      {
        wifiApStarted = espBoilerplate.beginAp(APSSID, APPSK);
      }
      #ifdef ENABLE_LOCAL_WEBSERVER
      if(wifiClientConnected == true)
      {
        configureTimeServer();  //Set up the time server
      }
      if(wifiApStarted == true || wifiClientConnected == true)
      {
        espBoilerplate.setHostname(device[0].name); //Set mDNS name
        setupWebServer();
        if(otaEnabled == true)
        {
          #ifdef SUPPORT_OTA
            configureOTA();
          #endif
        }
      }
      if(wifiApStarted == true && enableCaptivePortal == true)
      {
        localLog(F("Starting captive portal DNS server: "));
        dnsServer = new DNSServer;
        dnsServer->start(53, "*", WiFi.softAPIP());  //DNS server is required for captive portal to work
        localLogLn(F("OK"));
      }
      #endif
    }
    else
    {
      localLogLn(F("Disabling WiFi"));
      //WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
    }
  }
  void manageNetwork()
  {
    #ifdef ENABLE_LOCAL_WEBSERVER
      if(enableCaptivePortal == true && dnsServer != nullptr) //Must process inbound DNS requests
      {
        dnsServer->processNextRequest();
      }
      #ifdef SUPPORT_OTA
        if(otaEnabled == true)
        {
          ArduinoOTA.handle();  //Handle software updates
          if(otaInProgress == true)
          {
            return; //Pause the usual behaviour
          }
        }
      #endif
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
    #endif
  }
#endif

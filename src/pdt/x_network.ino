#if defined(SUPPORT_ETHERNET) || defined(SUPPORT_WIFI)
  void setupNetwork()
  {
    #ifdef SUPPORT_WIFI
    #ifdef ACT_AS_TRACKER
      if(digitalRead(buttonPin) == false) //Start WiFi if button pushed
      {
        startWiFiOnBoot = true;
      }
    #endif
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
      #ifdef ENABLE_LOCAL_WEBSERVER
      if(networkConnected == true && otaEnabled == true)
      {
        #ifdef SUPPORT_OTA
          configureOTA();
        #endif
      }
      #endif
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
  }
  void manageNetwork()
  {
    #ifdef ENABLE_LOCAL_WEBSERVER
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
      if(networkConnected == true && wiFiInactivityTimer > 0 && millis() - lastWifiActivity > wiFiInactivityTimer)
      {
        lastWifiActivity = millis();
        localLogLn(F("WiFi inactive, disconnecting"));
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        networkConnected = false;
      }
    #endif
  }
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

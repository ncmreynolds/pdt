/*
 *
 * This file contains the main loop, which runs endlessly 
 * 
 */
void loop()
{
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
  #if defined(SUPPORT_LORA) && defined(SUPPORT_GPS)
    if(loRaBufferSize > 0) //There's something in the buffer to process
    {
      processLoRaPacket();
      loRaBufferSize = 0;
    }
    #if defined(ACT_AS_TRACKER)
      if(beacon[currentBeacon].hasFix == true && millis() - beacon[currentBeacon].lastReceive > beacon[currentBeacon].timeout)
      {
        beacon[currentBeacon].hasFix = false;
        localLogLn(F("Lost beacon"));
        #ifdef SUPPORT_DISPLAY
          if(currentDisplayState == displayState::distance && millis() - lastDisplayUpdate > 1000) //Clear distance if showing
          {
            displayDistanceToBeacon();
          }
        #endif
      }
    #endif
    if(millis() - lastBeaconSendTime > beaconInterval1 && gps.location.isValid())
    {
      #if defined(ACT_AS_TRACKER)
        calculateDistanceToBeacon(currentBeacon);
      #elif defined(ACT_AS_BEACON)
        calculateDistanceToTracker(currentTracker);
      #endif
      if(loRaConnected)
      {
        shareLocation();
      }
      lastBeaconSendTime = millis();
    }
  #endif
  #ifdef SUPPORT_GPS
    #ifndef USE_RTOS
      smartDelay(250);
    #endif
    if(millis() > 60000 && timeIsValid() == false && gps.location.isValid() == true)
    {
      localLogLn(F("Using GPS for time"));
      setTimeFromGps();
    }
    /*
    while(GPS_PORT.available())
    {
      gps.encode(GPS_PORT.read());
    }
    */
    #if defined(ACT_AS_TRACKER)
      if(tracker[0].hasFix == false && gps.location.isValid() == true)
    #elif defined(ACT_AS_BEACON)
      if(beacon[0].hasFix == false && gps.location.isValid() == true)
    #endif
    {
      #if defined(ACT_AS_TRACKER)
        tracker[0].hasFix = true;
      #elif defined(ACT_AS_BEACON)
        beacon[0].hasFix = true;
      #endif
      #if defined(ARDUINO_ESP32C3_DEV)
        #ifdef SUPPORT_DOSTAR
          dotStar.setPixelColor(0, dotStar.Color(255, 0, 0, 0));
          dotStar.show();
        #endif
      #endif
      if(timeIsValid())
      {
        localLogLn(F("GPS got fix"));
      }
      else
      {
        localLogLn(F("GPS got fix, using for time"));
        setTimeFromGps();
      }
    }
    #if defined(ACT_AS_TRACKER)
      if(tracker[0].hasFix == true && gps.location.isValid() == false)
    #elif defined(ACT_AS_BEACON)
      if(beacon[0].hasFix == true && gps.location.isValid() == false)
    #endif
    {
      #if defined(ACT_AS_TRACKER)
        tracker[0].hasFix = false;
      #elif defined(ACT_AS_BEACON)
        beacon[0].hasFix = false;
      #endif
      localLogLn(F("GPS lost fix"));
    }
    #ifdef SERIAL_DEBUG
      if(millis() - lastGPSstatus > 10000)
      {
        lastGPSstatus = millis();
        showGPSstatus();
      }
    #endif
    #ifdef SUPPORT_DISPLAY
      if(tracker[0].hasFix == true && beacon[currentBeacon].hasFix == true && currentDisplayState == displayState::distance && distanceToCurrentBeaconChanged == true && millis() - lastDisplayUpdate > 1000) //Show distance if it changes
      {
        displayDistanceToBeacon();
        distanceToCurrentBeaconChanged = false;
      }
      else if(currentDisplayState != displayState::blank && millis() - lastDisplayUpdate > displayTimeout) //Time out the display
      {
        if(currentDisplayState == displayState::distance)
        {
          blankDisplay(); //Blank the display
        }
        else if(currentDisplayState == displayState::trackingMode && currentTrackingMode == trackingMode::nearest)  //Find nearest then show user
        {
          if(selectNearestBeacon())
          {
            currentTrackingMode = trackingMode::fixed;
            displayBeaconMode();
          }
          else
          {
            displayDistanceToBeacon();  //Drop back to the range display
          }
        }
        else if(currentDisplayState == displayState::trackingMode && currentTrackingMode == trackingMode::furthest) //Find furthest then show user
        {
          if(selectFurthestBeacon())
          {
            currentTrackingMode = trackingMode::fixed;
            displayBeaconMode();
          }
          else
          {
            displayDistanceToBeacon();  //Drop back to the range display
          }
        }
        else
        {
          displayDistanceToBeacon();  //Drop back to the range display
        }
      }
      checkButton();
    #endif
    #ifdef SUPPORT_LED
      if(ledState == true && millis() - ledLastStateChange > ledOnTime)
      {
        ledLastStateChange = millis();
        ledState = false;
        #if defined(ARDUINO_ESP32C3_DEV)
          #ifdef SUPPORT_DOSTAR
            dotStar.setPixelColor(0, dotStar.Color(0, 0, 0, 0));
            dotStar.show();
          #endif
        #endif
      }
      else if(ledState == false && millis() - ledLastStateChange > ledOffTime)
      {
        ledLastStateChange = millis();
        ledState = true;
        #if defined(ARDUINO_ESP32C3_DEV)
          #ifdef SUPPORT_DOSTAR
            dotStar.setPixelColor(0, dotStar.Color(0, 255, 0, 0));
            dotStar.show();
          #endif
        #endif
      }
    #endif
  #endif
  #ifdef SUPPORT_BEEPER
    if(beeperEnabled == true)
    {
      manageBeeper();
    }
  #endif
  #ifdef SUPPORT_BATTERY_METER
    manageBattery();
  #endif
  if(bootTime == 0)
  {
    if(timeIsValid())
    {
      recordBootTime();
      localLogLn(F("Time synced"));
    }
  }
  if(flushLogNow == true || ((millis() - logLastFlushed) >= (logFlushInterval * 1000)))
  {
    logLastFlushed = millis();
    flushLogNow = false;
    #ifdef SERIAL_LOG
      SERIAL_DEBUG_PORT.println(F("PERIODIC LOG FLUSH"));
    #endif
    if(loggingBuffer.length() > 0)
    {
      flushLog();
    }
    else
    {
      #ifdef SERIAL_LOG
        SERIAL_DEBUG_PORT.println(F("LOG BUFFER SUSPICIOUSLY EMPTY"));
      #endif
    }
  }
  if(saveConfigurationSoon != 0 && millis() - saveConfigurationSoon > 500) //Save configuration after a delay to avoid AsyncWebserver doing it in a callback
  {
    saveConfigurationSoon = 0;
    saveConfiguration(configurationFile);
  }
  #if defined(ENABLE_REMOTE_RESTART)
    if(restartTimer !=0 && millis() - restartTimer > 7000)  //Restart the ESP based off a request in the Web UI
    {
      localLogLn(F("Restarting now"));
      flushLog();
      delay(3000);
      #ifdef ESP32
        ESP.restart();
      #else
        ESP.reset();
      #endif
    }
  #endif
}

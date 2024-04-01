/*
 * 
 * This file contains functions related to the GPS module, for both location and time
 * 
 */
#if defined(SUPPORT_GPS)
  void setupGps()
  {
    GPS_PORT.setRxBufferSize(256);  //Set the largest possible buffer for GPS data so it can buffer then be ingested quickly. 9600 baud sucks
    GPS_PORT.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);  //RX only hardware serial port
    localLog(F("Configuring GPS on hardware serial RX: pin "));
    //Use a semaphore to control access to global variables and data structures
    gpsSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(gpsSemaphore);
    //Start processing GPS data as an RTOS task
    xTaskCreate(processGpsSentences, "processGpsSentences", 10000, NULL, 1, &gpsManagementTask );
    localLogLn(RXPin);
  }
  void manageGps()
  {
    if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout) == pdTRUE) //Take the semaphore to exclude the sentence processing task and udate the data structures
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
        if(millis() - lastGPSstatus > 10000)
        {
          lastGPSstatus = millis();
          #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
          if(peripheralsEnabled)
          {
          #endif
            showGPSstatus();
          #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
          }
          #endif
        }
      #endif
      if(locationUpdated())  //Get the latest GPS location stored in device[0], if there's a fix
      {
        if(millis() - lastDistanceCalculation > distanceCalculationInterval)  //Recalculate distances on a short interval
        {
          lastDistanceCalculation = millis();
          #if defined(ACT_AS_TRACKER)
            selectDeviceToTrack();  //May need to change tracked device due to movement
          #elif defined(ACT_AS_BEACON)
            calculateDistanceToTrackers();  //May need to change update frequency due to movement
          #endif
        }
        #if defined(SUPPORT_BEEPER)
          #if defined(ACT_AS_TRACKER)
            if(currentlyTrackedBeacon != maximumNumberOfDevices && device[currentlyTrackedBeacon].hasGpsFix == true && (distanceToCurrentBeaconChanged == true || millis() - lastDistanceChangeUpdate > 5000))  //Set beeper urgency based on current distance, if it has changed
            {
              //distanceToCurrentBeaconChanged 
              setBeeperUrgency();
              #ifndef SUPPORT_DISPLAY
                distanceToCurrentBeaconChanged = false; //If it's beeper only, acknowledge the change
                lastDistanceChangeUpdate = millis();
              #endif
            }
          #endif
        #endif
        #if defined(SUPPORT_LVGL)
        /*
        if(currentTrackingMode == trackingMode::furthest)
        {
          if(selectFurthestBeacon())
          {
            currentTrackingMode = trackingMode::fixed;
            displayTrackingMode();
          }
        }*/
        #endif
        #if defined(SUPPORT_DISPLAY)
          if(currentlyTrackedBeacon != maximumNumberOfDevices && device[currentlyTrackedBeacon].hasGpsFix == true && (distanceToCurrentBeaconChanged == true || millis() - lastDistanceChangeUpdate > 5000)) //Show distance if it changes
          {
            distanceToCurrentBeaconChanged = false;
            lastDistanceChangeUpdate = millis();
            if(currentDisplayState == displayState::distance && millis() - lastDisplayUpdate > 100)
            {
              displayDistanceToBeacon();
            }
          }
          if(currentDisplayState != displayState::blank && millis() - lastDisplayUpdate > displayTimeout) //Time out the display
          {
            if(currentDisplayState == displayState::distance)
            {
              blankDisplay(); //Blank the display
            }
            else if(currentDisplayState == displayState::trackingMode && currentTrackingMode == trackingMode::nearest)  //Find nearest then show user
            {
              displayDistanceToBeacon();  //Drop back to the range display
            }
            else if(currentDisplayState == displayState::trackingMode && currentTrackingMode == trackingMode::furthest) //Find furthest then show user
            {
              if(selectFurthestBeacon())
              {
                currentTrackingMode = trackingMode::fixed;
                displayTrackingMode();
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
        #endif
      }
      #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
      if(peripheralsEnabled)
      {
      #endif
        if(millis() - lastGpsTimeCheck > gpsTimeCheckInterval)  //Maintain system time using GPS which may be possible even without a full fix
        {
          lastGpsTimeCheck = millis();
          updateTimeFromGps();
        }
      #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
      }
      #endif
      xSemaphoreGive(gpsSemaphore);
    }
  }
  void updateTimeFromGps()
  {
    if(gps.time.isValid() == true)
    {
      if(gpsTimeCheckInterval != 1800E3)
      {
        gpsTimeCheckInterval = 1800E3;  //Reduce the check interval to half an hour
      }
      if(timeIsValid() == false)  //Get an initial time at startup
      {
        localLog(F("Attempting to use GPS for time: "));
        setTimeFromGps();
        if(timeIsValid() == true)
        {
          localLogLn(F("OK"));
        }
        else
        {
          localLogLn(F("failed"));
        }
      }
      else
      {
        localLogLn(F("Updating time from GPS"));
        setTimeFromGps();
      }
    }
    else
    {
      if(gpsTimeCheckInterval != 30E3)
      {
        gpsTimeCheckInterval = 30E3;  //Reduce the check interval to 30s
      }
    }
  }
  bool locationUpdated() //True implies there's a GPS fix and it changed
  {
    #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
    if(peripheralsEnabled == true)
    {
    #endif
      if(gps.location.isValid() == true)
      {
        bool updateOccured = false;
        if(device[0].hasGpsFix == false)
        {
          device[0].hasGpsFix = true;
          lastGPSstateChange = millis();
          localLogLn(F("GPS got fix"));
          #if defined(SUPPORT_LED)
            ledOff(0);
          #endif
        }
        #if defined(SUPPORT_LORA)
          device[0].lastLoRaLocationUpdate = millis(); //Record when the last location update happened, so GPS updates are more resilient than pure isValid test
        #endif
        #if defined(SUPPORT_ESPNOW)
          device[0].lastEspNowLocationUpdate = millis(); //Record when the last location update happened, so GPS updates are more resilient than pure isValid test
        #endif
        if(device[0].latitude != gps.location.lat())
        {
          device[0].latitude = gps.location.lat();
          updateOccured = true;
        }
        if(device[0].longitude != gps.location.lng())
        {
          device[0].longitude = gps.location.lng();
          updateOccured = true;
        }
        if(device[0].course != gps.course.deg())
        {
          device[0].course = gps.course.deg();
          updateOccured = true;
        }
        if(device[0].speed != gps.speed.mps())
        {
          device[0].speed = gps.speed.mps();
          updateOccured = true;
        }
        if(device[0].hdop != gps.hdop.hdop())
        {
          device[0].hdop = gps.hdop.hdop();
          updateOccured = true;
        }
        gpsSentences = gps.passedChecksum();
        gpsErrors = gps.failedChecksum();
        #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
          if(peripheralsEnabled == true)
          {
            if(device[0].hdop < normalHdopThreshold)
            {
              device[0].smoothedSpeed = (device[0].smoothedSpeed * 0.75) + (device[0].speed *0.25);
              if(device[0].smoothedSpeed < stationaryThreshold)
              {
                if(device[0].moving == true)
                {
                  device[0].moving = false;
                  lastGPSstateChange = millis();
                  #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
                    SERIAL_DEBUG_PORT.println(F("Device stationary"));
                  #endif
                }
              }
              else
              {
                if(device[0].moving == false)
                {
                  device[0].moving = true;
                  lastGPSstateChange = millis();
                  #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
                    SERIAL_DEBUG_PORT.println(F("Device moving"));
                  #endif
                }
              }
            }
          }
        #endif
        return updateOccured;
      }
      else if(device[0].hasGpsFix == true)
      {
        device[0].hasGpsFix = false;
        lastGPSstateChange = millis();
        #if defined(SUPPORT_LVGL) && defined(LVGL_SUPPORT_SCAN_INFO_TAB)
          findableDevicesChanged = true;
        #endif
        localLogLn(F("GPS lost fix"));
        #if defined(SUPPORT_LED)
          ledSlowBlink();
        #endif
      }
      return false;
    #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
    }
    else
    {
      return false;
    }
    #endif
    return false;
  }
  void processGpsSentences(void * parameter)
  {
    char character;
    #if defined(ENABLE_OTA_UPDATE)
    while(otaInProgress == false)
    #else
    while(true)
    #endif
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout) == pdTRUE)
      {
        while(GPS_PORT.available()) //Check for incoming GPS data
        {
          character = GPS_PORT.read();
          gps.encode(character);  //Process the data
          //SERIAL_DEBUG_PORT.print(character);
          gpsChars = gps.charsProcessed();
        }
        xSemaphoreGive(gpsSemaphore);
      }
      vTaskDelay(gpsYieldTime / portTICK_PERIOD_MS); //Hand back for 100ms
    }
    vTaskDelete(NULL);  //Kill this task
  }
  void setTimeFromGps()
  {
    struct tm tm;
    tm.tm_year = gps.date.year() - 1900;
    tm.tm_mon = gps.date.month() - 1;
    tm.tm_mday = gps.date.day();
    tm.tm_hour = gps.time.hour();
    tm.tm_min = gps.time.minute();
    tm.tm_sec = gps.time.second();
    time_t t = mktime(&tm);
    //printf("Setting time: %s", asctime(&tm));
    struct timeval now = { .tv_sec = t };
    settimeofday(&now, NULL);
  }
  uint8_t countBits(uint16_t thingToCount)
  {
    uint8_t total = 0;
    for(uint8_t bit = 0; bit < 16; bit++)
    {
      if(thingToCount & (uint16_t)(pow(2, bit)))
      {
        total++;
      }
    }
    return total;
  }
  #if defined(ACT_AS_TRACKER)
    uint8_t numberOfBeacons()
    {
      uint8_t count = 0;
      if(numberOfDevices == 1)
      {
        return 0;
      }
      for(uint8_t beaconIndex = 1; beaconIndex < numberOfDevices; beaconIndex++)
      {
        if((device[beaconIndex].typeOfDevice & 0x01) == 0x00 && device[beaconIndex].hasGpsFix == true)
        {
          count++;
        }
      }
      return count;
    }
    void calculateDistanceToBeacons()
    {
      for(uint8_t beaconIndex = 1; beaconIndex < numberOfDevices; beaconIndex++)
      {
        if((device[beaconIndex].typeOfDevice & 0x01) == 0x00 && device[beaconIndex].hasGpsFix == true)
        {
          device[beaconIndex].distanceTo = TinyGPSPlus::distanceBetween(device[0].latitude, device[0].longitude, device[beaconIndex].latitude, device[beaconIndex].longitude);
          device[beaconIndex].courseTo = TinyGPSPlus::courseTo(device[0].latitude, device[0].longitude, device[beaconIndex].latitude, device[beaconIndex].longitude);
          #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
            SERIAL_DEBUG_PORT.printf_P(PSTR("Beacon %u - distance:%01.1f(m) course:%03.1f(deg)"), beaconIndex, device[beaconIndex].distanceTo, device[beaconIndex].courseTo);
          #endif
          if(beaconIndex == currentlyTrackedBeacon)
          {
            #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
              SERIAL_DEBUG_PORT.println(F(" - tracking"));
            #endif
            if(distanceToCurrentBeacon != (uint32_t)device[beaconIndex].distanceTo)
            {
              distanceToCurrentBeacon = (uint32_t)device[beaconIndex].distanceTo;
              distanceToCurrentBeaconChanged = true;
              /*
              if(distanceToCurrentBeacon < loRaPerimiter1)
              {
                device[beaconIndex].timeout = loRaLocationInterval1 * 2.5;
              }
              else if(distanceToCurrentBeacon < loRaPerimiter2)
              {
                device[beaconIndex].timeout = loRaLocationInterval2 * 2.5;
              }
              else if(distanceToCurrentBeacon < loRaPerimiter3)
              {
                device[beaconIndex].timeout = loRaLocationInterval3 * 2.5;
              }
              else
              {
                device[beaconIndex].timeout = 60000;
              }
              */
            }
          }
          else
          {
            #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
              SERIAL_DEBUG_PORT.println();
            #endif
          }
        }
      }
    }
    void selectDeviceToTrack()
    {
      if(numberOfDevices > 1)
      {
        calculateDistanceToBeacons();
        if(currentTrackingMode == trackingMode::nearest)
        {
          if(selectNearestBeacon())
          {
            #if defined(SERIAL_DEBUG) && (defined(DEBUG_LORA) || defined(DEBUG_ESPNOW))
              SERIAL_DEBUG_PORT.print(F("Tracking nearest beacon: "));
              SERIAL_DEBUG_PORT.println(currentlyTrackedBeacon);
            #endif
            #if defined(SUPPORT_LVGL) && defined(LVGL_SUPPORT_SCAN_INFO_TAB)
              findableDevicesChanged = true;
            #endif
          }
        }
        else if(currentTrackingMode == trackingMode::furthest)
        {
          if(selectFurthestBeacon())
          {
            #if defined(SERIAL_DEBUG) && (defined(DEBUG_LORA) || defined(DEBUG_ESPNOW))
              SERIAL_DEBUG_PORT.print(F("Tracking furthest beacon: "));
              SERIAL_DEBUG_PORT.println(currentlyTrackedBeacon);
            #endif
            currentTrackingMode = trackingMode::fixed;  //Switch to fixed as 'furthest' needs to fix once chose
            #if defined(SUPPORT_LVGL) && defined(LVGL_SUPPORT_SCAN_INFO_TAB)
              findableDevicesChanged = true;
            #endif
          }
        }
        else if(currentTrackingMode == trackingMode::fixed)
        {
          #if defined(SERIAL_DEBUG) && (defined(DEBUG_LORA) || defined(DEBUG_ESPNOW))
            SERIAL_DEBUG_PORT.print(F("Tracking specific beacon: "));
            SERIAL_DEBUG_PORT.println(currentlyTrackedBeacon);
          #endif
        }
      }
    }
    void updateDistanceToBeacon(const uint8_t index)
    {
      if(uint32_t(device[index].distanceTo) != distanceToCurrentBeacon)
      {
        distanceToCurrentBeacon = uint32_t(device[index].distanceTo);
        distanceToCurrentBeaconChanged = true;
        #if defined(SUPPORT_DISPLAY)
          if(currentDisplayState == displayState::distance) //Clear distance if showing
          {
            displayDistanceToBeacon();
          }
        #endif
      }
    }
    bool selectNearestBeacon()  //True only implies it has changed!
    {
      if(numberOfDevices == 0)
      {
        return false;
      }
      #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
      else if(numberOfDevices == 2 && device[1].hasGpsFix == true && (device[1].espNowOnline || device[1].loRaOnline) && rangeToIndicate(1) < maximumEffectiveRange)
      #elif defined(SUPPORT_ESPNOW)
      else if(numberOfDevices == 2 && device[1].hasGpsFix == true && device[1].espNowOnline && rangeToIndicate(1) < maximumEffectiveRange)
      #elif defined(SUPPORT_LORA)
      else if(numberOfDevices == 2 && device[1].hasGpsFix == true && device[1].loRaOnline && rangeToIndicate(1) < maximumEffectiveRange)
      #elif defined(SUPPORT_TREACLE)
      else if(numberOfDevices == 2 && device[1].hasGpsFix == true && treacle.online(device[1].id) && rangeToIndicate(1) < maximumEffectiveRange)
      #endif
      {
        if(currentlyTrackedBeacon != 1)  //Only assign this once
        {
          currentlyTrackedBeacon = 1;
          updateDistanceToBeacon(currentlyTrackedBeacon);
          return true;
        }
        updateDistanceToBeacon(currentlyTrackedBeacon);
        return false;
      }
      else
      {
        uint8_t nearestBeacon = maximumNumberOfDevices; //Determine this anew every time
        for(uint8_t index = 1; index < numberOfDevices; index++)
        {
          #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
          if((device[index].typeOfDevice & 0x01) == 0 && device[index].hasGpsFix && (device[index].espNowOnline || device[index].loRaOnline) && rangeToIndicate(index) < maximumEffectiveRange && (nearestBeacon == maximumNumberOfDevices || device[index].distanceTo < device[nearestBeacon].distanceTo))
          #elif defined(SUPPORT_ESPNOW)
          if((device[index].typeOfDevice & 0x01) == 0 && device[index].hasGpsFix && device[index].espNowOnline && rangeToIndicate(index) < maximumEffectiveRange && (nearestBeacon == maximumNumberOfDevices || device[index].distanceTo < device[nearestBeacon].distanceTo))
          #elif defined(SUPPORT_LORA)
          if((device[index].typeOfDevice & 0x01) == 0 && device[index].hasGpsFix && device[index].loRaOnline && rangeToIndicate(index) < maximumEffectiveRange && (nearestBeacon == maximumNumberOfDevices || device[index].distanceTo < device[nearestBeacon].distanceTo))
          #endif
          {
            nearestBeacon = index;
          }
        }
        if(nearestBeacon != maximumNumberOfDevices && currentlyTrackedBeacon != nearestBeacon) //Choose a new nearest beacon
        {
          currentlyTrackedBeacon = nearestBeacon;
          updateDistanceToBeacon(currentlyTrackedBeacon);
          return true;
        }
        updateDistanceToBeacon(currentlyTrackedBeacon);
        return false;
      }
      return false;
    }
    bool selectFurthestBeacon() //True implies this has changed!
    {
      if(numberOfDevices == 1)
      {
        return false;
      }
      #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
      else if(numberOfDevices == 2 && device[1].hasGpsFix && (device[1].espNowOnline || device[1].loRaOnline) && rangeToIndicate(1) < maximumEffectiveRange)
      #elif defined(SUPPORT_ESPNOW)
      else if(numberOfDevices == 2 && device[1].hasGpsFix && device[1].espNowOnline && rangeToIndicate(1) < maximumEffectiveRange)
      #elif defined(SUPPORT_LORA)
      else if(numberOfDevices == 2 && device[1].hasGpsFix && device[1].loRaOnline && rangeToIndicate(1) < maximumEffectiveRange)
      #elif defined(SUPPORT_TREACLE)
      else if(numberOfDevices == 2 && device[1].hasGpsFix && treacle.online(device[1].id) && rangeToIndicate(1) < maximumEffectiveRange)
      #endif
      {
        if(currentlyTrackedBeacon != 1)
        {
          currentlyTrackedBeacon = 1;
          updateDistanceToBeacon(currentlyTrackedBeacon);
          return true;
        }
        updateDistanceToBeacon(currentlyTrackedBeacon);
        return false;
      }
      else
      {
        uint8_t furthestBeacon = maximumNumberOfDevices;
        for(uint8_t index = 1; index < numberOfDevices; index++)
        {
          #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
          if((device[index].typeOfDevice & 0x01) == 0 && device[index].hasGpsFix && (device[index].espNowOnline || device[index].loRaOnline) && rangeToIndicate(index) < maximumEffectiveRange && (furthestBeacon == maximumNumberOfDevices || device[index].distanceTo > device[furthestBeacon].distanceTo))
          #elif defined(SUPPORT_ESPNOW)
          if((device[index].typeOfDevice & 0x01) == 0 && device[index].hasGpsFix && device[index].espNowOnline && rangeToIndicate(index) < maximumEffectiveRange && (furthestBeacon == maximumNumberOfDevices || device[index].distanceTo > device[furthestBeacon].distanceTo))
          #elif defined(SUPPORT_LORA)
          if((device[index].typeOfDevice & 0x01) == 0 && device[index].hasGpsFix && device[index].loRaOnline && rangeToIndicate(index) < maximumEffectiveRange && (furthestBeacon == maximumNumberOfDevices || device[index].distanceTo > device[furthestBeacon].distanceTo))
          #endif
          {
            furthestBeacon = index;
          }
        }
        if(furthestBeacon != maximumNumberOfDevices && currentlyTrackedBeacon != furthestBeacon)
        {
          currentlyTrackedBeacon = furthestBeacon;
          updateDistanceToBeacon(currentlyTrackedBeacon);
          return true;
        }
        updateDistanceToBeacon(currentlyTrackedBeacon);
        return false;
      }
      return false;
    }
    float rangeToIndicate(uint8_t deviceIndex)
    {
      if(trackerPriority == 0)
      {
        return device[deviceIndex].distanceTo;
      }
      else
      {
        if(device[deviceIndex].distanceTo > device[deviceIndex].diameter/2)
        {
          return device[deviceIndex].distanceTo - device[deviceIndex].diameter/2;
        }
        else
        {
          return 0;
        }
      }
      return effectivelyUnreachable;
    }
  #elif defined(ACT_AS_BEACON)
    uint8_t numberOfTrackers()
    {
      uint8_t count = 0;
      if(numberOfDevices == 1)
      {
        return 0;
      }
      for(uint8_t index = 1; index < numberOfDevices; index++)
      {
        if((device[index].typeOfDevice & 0x01) == 0x01) //Tracker doesn't need to have a fix
        {
          count++;
        }
      }
      return count;
    }
    void calculateDistanceToTrackers()
    {
      distanceToClosestTracker = effectivelyUnreachable;
      for(uint8_t index = 1; index < numberOfDevices; index++)
      {
        #if defined(SUPPORT_ESPNOW) && defined(SUPPORT_LORA)
          if((device[index].typeOfDevice & 0x01) == 0x01 && device[index].hasGpsFix == true && (device[index].espNowOnline == true || device[index].loRaOnline == true))
        #elif defined(SUPPORT_ESPNOW)
          if((device[index].typeOfDevice & 0x01) == 0x01 && device[index].hasGpsFix == true && device[index].espNowOnline == true)
        #elif defined(SUPPORT_LORA)
          if((device[index].typeOfDevice & 0x01) == 0x01 && device[index].hasGpsFix == true && device[index].loRaOnline == true)
        #endif
        {
          device[index].distanceTo = TinyGPSPlus::distanceBetween(device[0].latitude, device[0].longitude, device[index].latitude, device[index].longitude);
          device[index].courseTo = TinyGPSPlus::courseTo(device[0].latitude, device[0].longitude, device[index].latitude, device[index].longitude);
          if(device[index].distanceTo < distanceToClosestTracker)
          {
            distanceToClosestTracker = device[index].distanceTo;
            closestTracker = index;
          }
          #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
            SERIAL_DEBUG_PORT.printf_P(PSTR("Found tracker %u: distance %.1f course %.1f\r\n"), index, device[index].distanceTo, device[index].courseTo);
          #endif
        }
      }
    }
  #endif
  char* hdopDescription(float hdop)
  {
    if(hdop < excellentHdopThreshold)
    {
       return PSTR("excellent");
    }
    else if(hdop < goodHdopThreshold)
    {
      return PSTR("good");
    }
    else if(hdop < normalHdopThreshold)
    {
      return PSTR("normal");
    }
    else if(hdop < poorHdopThreshold)
    {
      return PSTR("poor");
    }
    else
    {
      return PSTR("inaccurate");
    }
  }
  #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
    void showGPSstatus()
    {
      //if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout) == pdTRUE)
      {
        SERIAL_DEBUG_PORT.print(F("GPS Chars:"));
        SERIAL_DEBUG_PORT.print(gps.charsProcessed());
        if(gps.passedChecksum() > 0)
        {
          SERIAL_DEBUG_PORT.print(F(" Valid:"));
          SERIAL_DEBUG_PORT.print(gps.passedChecksum());
        }
        if(gps.failedChecksum() > 0)
        {
          SERIAL_DEBUG_PORT.print(F(" Errors:"));
          SERIAL_DEBUG_PORT.print(gps.failedChecksum());
        }
        if(gps.satellites.value() > 0)
        {
          SERIAL_DEBUG_PORT.print(F(" Sat:"));
          SERIAL_DEBUG_PORT.print(gps.satellites.value());
        }
        if(gps.sentencesWithFix() > 0)
        {
          SERIAL_DEBUG_PORT.print(F(" Fix:"));
          SERIAL_DEBUG_PORT.print(gps.sentencesWithFix());
        }
        if(gps.location.isValid())
        {
          SERIAL_DEBUG_PORT.print(F(" Age(ms):"));
          SERIAL_DEBUG_PORT.print(gps.location.age());
          SERIAL_DEBUG_PORT.print(F(" Lat:"));
          SERIAL_DEBUG_PORT.print(gps.location.lat());
          SERIAL_DEBUG_PORT.print(F(" Lon: "));
          SERIAL_DEBUG_PORT.print(gps.location.lng());
          SERIAL_DEBUG_PORT.print(F(" Alt(m):"));
          SERIAL_DEBUG_PORT.print(gps.altitude.meters());
          SERIAL_DEBUG_PORT.print(F(" HDOP:"));
          SERIAL_DEBUG_PORT.print(gps.hdop.hdop());
          SERIAL_DEBUG_PORT.print(F(" Course(deg):"));
          SERIAL_DEBUG_PORT.print(gps.course.deg());
          SERIAL_DEBUG_PORT.print(F(" Speed(m/s):"));
          SERIAL_DEBUG_PORT.print(gps.speed.mps());
        }
        /*
        else
        {
          SERIAL_DEBUG_PORT.print(gps.satellites.value());
          SERIAL_DEBUG_PORT.println(F(" - no fix"));
        }
        */
        SERIAL_DEBUG_PORT.println();
      }
    }
  #endif
#endif

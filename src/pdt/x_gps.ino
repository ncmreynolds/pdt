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
          calculateDistanceToAllDevices();
          #if defined(SUPPORT_BEEPER)
            #if defined(SUPPORT_DISPLAY)
              if(beeperEnabled)
              {
                setBeeperUrgency();
              }
            #elif defined(SUPPORT_LVGL)
              if(beeperEnabled && currentLvglUiState == deviceState::tracking)
              {
                setBeeperUrgency();
              }
            #endif
          #endif
          #if defined(ACT_AS_TRACKER)
            selectDeviceToTrack();          //May need to change tracked device due to movement and finding/losing signal
          #endif
        }
        #if defined(ACT_AS_TRACKER)
        if(currentlyTrackedDevice != maximumNumberOfDevices && device[currentlyTrackedDevice].hasGpsFix == true && (distanceToCurrentlyTrackedDeviceChanged == true || millis() - lastDistanceChangeUpdate > 5000))  //Set beeper urgency based on current distance, if it has changed
        {
          distanceToCurrentlyTrackedDeviceChanged = false;
          lastDistanceChangeUpdate = millis();
          #if defined(SUPPORT_DISPLAY)
            if(currentDisplayState == displayState::distance && millis() - lastDisplayUpdate > 100)
            {
              displayDistanceToBeacon();
            }
            else if(currentDisplayState != displayState::blank && millis() - lastDisplayUpdate > displayTimeout) //Time out the display
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
        else if(currentlyTrackedDevice == maximumNumberOfDevices) //Nothing being tracked
        {
          if(currentDisplayState != displayState::blank && millis() - lastDisplayUpdate > displayTimeout) //Time out the display
          {
            blankDisplay(); //Blank the display
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
        if(device[0].hdop < normalHdopThreshold)
        {
          device[0].smoothedSpeed = (device[0].smoothedSpeed * smoothingFactor) + (device[0].speed * (1-smoothingFactor));
          if(device[0].smoothedSpeed < movementThreshold)
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

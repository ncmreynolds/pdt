/*
 * 
 * This file contains functions related to the GPS module, for both location and time
 * 
 */
#ifdef SUPPORT_GPS
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
      for(uint8_t index = 0; index < numberOfDevices; index++)  //Degrade quality if location updates missed, this INCLUDES this device!
      {
        if(device[index].hasFix == true && //Thing has fix!
            device[index].nextLocationUpdate != 0 &&  //Thing has shared when to expect the location update
            millis() - device[index].lastLocationUpdate > (device[index].nextLocationUpdate + (device[index].nextLocationUpdate>>3))) //Allow margin of 1/8 the expected interval
        {
          device[index].lastLocationUpdate = millis();  //A failed update is an 'update'
          device[index].updateHistory = device[index].updateHistory >> 1; //Reduce update history quality
          //device[index].nextLocationUpdate = device[index].nextLocationUpdate >> 1; //Halve the timeout
          #ifdef ACT_AS_TRACKER
          if(index == currentBeacon)
          {
            localLog(F("Currently tracked beacon "));
          }
          else
          #endif
          {
            localLog(F("Device "));
          }
          localLog(index);
          if(device[index].updateHistory < 0x00ff)
          {
            localLogLn(F(" gone offline"));
            device[index].hasFix = false;
            #ifdef ACT_AS_TRACKER
              if(index == currentBeacon) //Need to stop tracking this beacon
              {
                currentBeacon = maximumNumberOfDevices;
                distanceToCurrentBeacon = BEACONUNREACHABLE;
                distanceToCurrentBeaconChanged = true;
                #ifdef SUPPORT_BEEPER
                  endRepeatingBeep();
                #endif
                #ifdef SUPPORT_DISPLAY
                  if(currentDisplayState == displayState::distance) //Clear distance if showing
                  {
                    displayDistanceToBeacon();
                  }
                #endif
              }
            #endif
          }
          else
          {
            localLog(F(" dropped packet, update history now:0x"));
            localLogLn(String(device[index].updateHistory,HEX));
          }
        }
      }
      if(updateLocation())  //Get the latest GPS location stored in device[0], if there's a fix
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
        #ifdef SUPPORT_BEEPER
          #ifdef ACT_AS_TRACKER
            if(currentBeacon != maximumNumberOfDevices && device[currentBeacon].hasFix == true && (distanceToCurrentBeaconChanged == true || millis() - lastDistanceChangeUpdate > 5000))  //Set beeper urgency based on current distance, if it has changed
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
        #ifdef SUPPORT_DISPLAY
          if(currentBeacon != maximumNumberOfDevices && device[currentBeacon].hasFix == true && (distanceToCurrentBeaconChanged == true || millis() - lastDistanceChangeUpdate > 5000)) //Show distance if it changes
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
      if(millis() - lastGpsTimeCheck > gpsTimeCheckInterval)  //Maintain system time using GPS which may be possible even without a full fix
      {
        lastGpsTimeCheck = millis();
        updateTimeFromGps();
      }
      xSemaphoreGive(gpsSemaphore);
    }
  }
  void updateTimeFromGps()
  {
    if(gps.time.isValid() == true)
    {
      if(timeIsValid() == false)  //Get an initial time at startup
      {
        localLog(F("Attempting to use GPS for time: "));
        setTimeFromGps();
        if(timeIsValid() == true)
        {
          localLogLn(F("OK"));
          gpsTimeCheckInterval = 1800000;  //Reduce the check interval to half an hour
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
        gpsTimeCheckInterval = 1800000;  //Reduce the check interval to half an hour
      }
    }
  }
  bool updateLocation() //True implies there's a GPS fix
  {
    if(gps.location.isValid() == true)
    {
      if(device[0].hasFix == false)
      {
        device[0].hasFix = true;
        localLogLn(F("GPS got fix"));
      }
      device[0].lastLocationUpdate = millis(); //Record when the last location update happened, so GPS updates are more resilient than pure isValid test
      device[0].latitude = gps.location.lat();
      device[0].longitude = gps.location.lng();
      device[0].course = gps.course.deg();
      device[0].speed = gps.speed.mps();
      device[0].hdop = gps.hdop.hdop();
      #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
        if(millis() - lastGPSstatus > 10000)
        {
          lastGPSstatus = millis();
          showGPSstatus();
        }
      #endif
      return true;
    }
    else if(device[0].hasFix == true)
    {
      device[0].hasFix = false;
      localLogLn(F("GPS lost fix"));
    }
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
        if((device[beaconIndex].typeOfDevice & 0x01) == 0x00 && device[beaconIndex].hasFix == true)
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
        if((device[beaconIndex].typeOfDevice & 0x01) == 0x00 && device[beaconIndex].hasFix == true)
        {
          device[beaconIndex].distanceTo = TinyGPSPlus::distanceBetween(device[0].latitude, device[0].longitude, device[beaconIndex].latitude, device[beaconIndex].longitude);
          device[beaconIndex].courseTo = TinyGPSPlus::courseTo(device[0].latitude, device[0].longitude, device[beaconIndex].latitude, device[beaconIndex].longitude);
          #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
            SERIAL_DEBUG_PORT.printf_P(PSTR("Beacon %u - distance:%01.1f(m) course:%03.1f(deg)"), beaconIndex, device[beaconIndex].distanceTo, device[beaconIndex].courseTo);
          #endif
          if(beaconIndex == currentBeacon)
          {
            #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
              SERIAL_DEBUG_PORT.println(F(" - tracking"));
            #endif
            if(distanceToCurrentBeacon != (uint32_t)device[beaconIndex].distanceTo)
            {
              distanceToCurrentBeacon = (uint32_t)device[beaconIndex].distanceTo;
              distanceToCurrentBeaconChanged = true;
              /*
              if(distanceToCurrentBeacon < loRaPerimiter1)
              {
                device[beaconIndex].timeout = locationSendInterval1 * 2.5;
              }
              else if(distanceToCurrentBeacon < loRaPerimiter2)
              {
                device[beaconIndex].timeout = locationSendInterval2 * 2.5;
              }
              else if(distanceToCurrentBeacon < loRaPerimiter3)
              {
                device[beaconIndex].timeout = locationSendInterval3 * 2.5;
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
            #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
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
            #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
              SERIAL_DEBUG_PORT.print(F("Tracking nearest beacon: "));
              SERIAL_DEBUG_PORT.println(currentBeacon);
            #endif
          }
        }
        else if(currentTrackingMode == trackingMode::furthest)
        {
          if(selectFurthestBeacon())
          {
            #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
              SERIAL_DEBUG_PORT.print(F("Tracking furthest beacon: "));
              SERIAL_DEBUG_PORT.println(currentBeacon);
            #endif
            currentTrackingMode = trackingMode::fixed;  //Switch to fixed as 'furthest' needs to fix once chose
          }
        }
        else if(currentTrackingMode == trackingMode::fixed)
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
            SERIAL_DEBUG_PORT.print(F("Tracking specific beacon: "));
            SERIAL_DEBUG_PORT.println(currentBeacon);
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
        #ifdef SUPPORT_DISPLAY
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
      else if(numberOfDevices == 2 && device[1].hasFix == true && device[1].distanceTo < maximumEffectiveRange)
      {
        if(currentBeacon != 1)  //Only assign this once
        {
          currentBeacon = 1;
          updateDistanceToBeacon(currentBeacon);
          return true;
        }
        updateDistanceToBeacon(currentBeacon);
        return false;
      }
      else
      {
        uint8_t nearestBeacon = maximumNumberOfDevices; //Determine this anew every time
        for(uint8_t index = 1; index < numberOfDevices; index++)
        {
          if((device[index].typeOfDevice & 0x01) == 0 && device[index].hasFix == true && device[index].distanceTo < maximumEffectiveRange && (nearestBeacon == maximumNumberOfDevices || device[index].distanceTo < device[nearestBeacon].distanceTo))
          {
            nearestBeacon = index;
          }
        }
        if(nearestBeacon != maximumNumberOfDevices && currentBeacon != nearestBeacon) //Choose a new nearest beacon
        {
          currentBeacon = nearestBeacon;
          updateDistanceToBeacon(currentBeacon);
          return true;
        }
        updateDistanceToBeacon(currentBeacon);
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
      else if(numberOfDevices == 2 && device[1].hasFix == true && device[1].distanceTo < maximumEffectiveRange)
      {
        if(currentBeacon != 1)
        {
          currentBeacon = 1;
          updateDistanceToBeacon(currentBeacon);
          return true;
        }
        updateDistanceToBeacon(currentBeacon);
        return false;
      }
      else
      {
        uint8_t furthestBeacon = maximumNumberOfDevices;
        for(uint8_t index = 1; index < numberOfDevices; index++)
        {
          if((device[index].typeOfDevice & 0x01) == 0 && device[index].hasFix == true && device[index].distanceTo < maximumEffectiveRange && (furthestBeacon == maximumNumberOfDevices || device[index].distanceTo > device[furthestBeacon].distanceTo))
          {
            furthestBeacon = index;
          }
        }
        if(furthestBeacon != maximumNumberOfDevices && currentBeacon != furthestBeacon)
        {
          currentBeacon = furthestBeacon;
          updateDistanceToBeacon(currentBeacon);
          return true;
        }
        updateDistanceToBeacon(currentBeacon);
        return false;
      }
      return false;
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
      distanceToClosestTracker = TRACKERUNREACHABLE;
      for(uint8_t index = 1; index < numberOfDevices; index++)
      {
        if((device[index].typeOfDevice & 0x01) == 0x01 && device[index].hasFix == true)
        {
          device[index].distanceTo = TinyGPSPlus::distanceBetween(device[0].latitude, device[0].longitude, device[0].latitude, device[0].longitude);
          device[index].courseTo = TinyGPSPlus::courseTo(device[0].latitude, device[0].longitude, device[0].latitude, device[0].longitude);
          if(device[index].distanceTo < distanceToClosestTracker)
          {
            distanceToClosestTracker = device[index].distanceTo;
            closestTracker = index;
          }
          #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
            SERIAL_DEBUG_PORT.printf_P(PSTR("Tracker %u: distance %f course %f\r\n"), index, device[index].distanceTo, device[index].courseTo);
          #endif
        }
      }
    }
  #endif
  char* hdopDescription(float hdop)
  {
    if(hdop < 1)
    {
       return PSTR("excellent");
    }
    else if(hdop < 1.5)
    {
      return PSTR("good");
    }
    else if(hdop < 2)
    {
      return PSTR("normal");
    }
    else if(hdop < 3)
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
        //SERIAL_DEBUG_PORT.print(F("GPS - Chars:"));
        //SERIAL_DEBUG_PORT.print(gps.charsProcessed());
        SERIAL_DEBUG_PORT.print(F("GPS"));
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

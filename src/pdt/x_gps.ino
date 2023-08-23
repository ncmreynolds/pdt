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
    #ifdef USE_RTOS
      //Use a semaphore to control access to global variables
      gpsSemaphore = xSemaphoreCreateBinary();
      xSemaphoreGive(gpsSemaphore);
      //Start processing GPS data as an RTOS task
      xTaskCreate(processGpsSentences, "processGpsSentences", 10000, NULL, 1, &gpsManagementTask );
    #endif
    localLogLn(RXPin);
  }
  void manageGps()
  {
    if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
        if(millis() - lastGPSstatus > 10000)
        {
          lastGPSstatus = millis();
          showGPSstatus();
        }
      #endif
      if(device[0].hasFix == false && gps.location.isValid() == true)
      {
        device[0].hasFix = true;
        if(timeIsValid())
        {
          localLogLn(F("GPS got fix"));
        }
        else
        {
          localLog(F("GPS got fix, using for time: "));
          setTimeFromGps();
          if(timeIsValid())
          {
            gpsTimeCheckInterval = 1800000; //Change to hourly updates, if succesful
            localLogLn(F("OK"));
          }
          else
          {
            localLogLn(F("failed"));
          }
        }
      }
      if(device[0].hasFix == true && gps.location.isValid() == false)
      {
        device[0].hasFix = false;
        localLogLn(F("GPS lost fix"));
      }
      #ifdef SUPPORT_DISPLAY
        if(device[0].hasFix == true && device[currentBeacon].hasFix == true && currentDisplayState == displayState::distance && distanceToCurrentBeaconChanged == true && millis() - lastDisplayUpdate > 1000) //Show distance if it changes
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
            /*
            if(selectNearestBeacon())
            {
              currentTrackingMode = trackingMode::fixed;
              displayTrackingMode();
            }
            else
            */
            {
              displayDistanceToBeacon();  //Drop back to the range display
            }
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
      if(millis() - lastGpsTimeCheck > gpsTimeCheckInterval)  //Maintain system time using GPS
      {
        lastGpsTimeCheck = millis();
        //Update time if possible
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
        #ifdef SUPPORT_LED
          //manageLed();
        #endif
      }
      xSemaphoreGive(gpsSemaphore);
    }
  }
  #ifdef USE_RTOS
    bool updateLocation()
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        if(gps.location.isValid() == true)
        {
          device[0].hasFix = true;
          device[0].latitude = gps.location.lat();
          device[0].longitude = gps.location.lng();
          device[0].course = gps.course.deg();
          device[0].speed = gps.speed.mps();
          device[0].hdop = gps.hdop.hdop();
          xSemaphoreGive(gpsSemaphore);
          return true;
        }
        else
        {
          xSemaphoreGive(gpsSemaphore);
          return false;
        }
      }
      return false;
    }
    void processGpsSentences(void * parameter)
    {
      char character;
      while(true)
      {
        if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
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
    }
  #else
    void smartDelay(uint32_t ms)
    {
      uint32_t start = millis();
      do 
      {
        while(GPS_PORT.available())
        {
          //SERIAL_DEBUG_PORT.print(char(GPS_PORT.peek()));
          gps.encode(GPS_PORT.read());
        }
      } while (millis() - start < ms);
    }
  #endif
  void setTimeFromGps()
  {
    //if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
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
      //xSemaphoreGive(gpsSemaphore);
    }
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
        if((device[beaconIndex].typeOfDevice & 0x01) == 0x00)
        {
          count++;
        }
      }
      return count;
    }
    void calculateDistanceToBeacons()
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        for(uint8_t beaconIndex = 1; beaconIndex < numberOfDevices; beaconIndex++)
        {
          if((device[beaconIndex].typeOfDevice & 0x01) == 0x00 && device[beaconIndex].hasFix == true)
          {
            device[beaconIndex].distanceTo = TinyGPSPlus::distanceBetween(device[0].latitude, device[0].longitude, device[beaconIndex].latitude, device[beaconIndex].longitude);
            device[beaconIndex].courseTo = TinyGPSPlus::courseTo(device[0].latitude, device[0].longitude, device[beaconIndex].latitude, device[beaconIndex].longitude);
            #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
              SERIAL_DEBUG_PORT.printf_P(PSTR("Beacon %u - distance(m):%01.1f course(deg):%03.1f"), beaconIndex, device[beaconIndex].distanceTo, device[beaconIndex].courseTo);
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
        xSemaphoreGive(gpsSemaphore);
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
        setBeeperUrgency();
      }
    }
    bool selectNearestBeacon()  //True only implies it has changed!
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        if(numberOfDevices == 0)
        {
          xSemaphoreGive(gpsSemaphore);
          return false;
        }
        else if(numberOfDevices == 2 && device[1].distanceTo < maximumEffectiveRange)
        {
          if(currentBeacon != 1)  //Only assign this once
          {
            currentBeacon = 1;
            updateDistanceToBeacon(currentBeacon);
            xSemaphoreGive(gpsSemaphore);
            return true;
          }
          updateDistanceToBeacon(currentBeacon);
          xSemaphoreGive(gpsSemaphore);
          return false;
        }
        else
        {
          uint8_t nearestBeacon = maximumNumberOfDevices; //Determine this anew every time
          for(uint8_t index = 0; index < numberOfDevices; index++)
          {
            if((device[index].typeOfDevice & 0x01) == 0 && device[index].distanceTo < maximumEffectiveRange && (nearestBeacon == maximumNumberOfDevices || device[index].distanceTo < device[nearestBeacon].distanceTo))
            {
              nearestBeacon = index;
            }
          }
          if(nearestBeacon != maximumNumberOfDevices && currentBeacon != nearestBeacon) //Choose a new nearest beacon
          {
            currentBeacon = nearestBeacon;
            updateDistanceToBeacon(currentBeacon);
            xSemaphoreGive(gpsSemaphore);
            return true;
          }
          updateDistanceToBeacon(currentBeacon);
          xSemaphoreGive(gpsSemaphore);
          return false;
        }
        xSemaphoreGive(gpsSemaphore);
      }
      return false;
    }
    bool selectFurthestBeacon() //True implies this has changed!
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        if(numberOfDevices == 1)
        {
          xSemaphoreGive(gpsSemaphore);
          return false;
        }
        else if(numberOfDevices == 2 && device[1].distanceTo < maximumEffectiveRange)
        {
          if(currentBeacon != 1)
          {
            currentBeacon = 1;
            updateDistanceToBeacon(currentBeacon);
            xSemaphoreGive(gpsSemaphore);
            return true;
          }
          updateDistanceToBeacon(currentBeacon);
          xSemaphoreGive(gpsSemaphore);
          return false;
        }
        else
        {
          uint8_t furthestBeacon = maximumNumberOfDevices;
          for(uint8_t index = 0; index < numberOfDevices; index++)
          {
            if((device[index].typeOfDevice & 0x01) == 0 && device[index].distanceTo < maximumEffectiveRange && (furthestBeacon == maximumNumberOfDevices || device[index].distanceTo > device[furthestBeacon].distanceTo))
            {
              furthestBeacon = index;
            }
          }
          if(furthestBeacon != maximumNumberOfDevices && currentBeacon != furthestBeacon)
          {
            currentBeacon = furthestBeacon;
            updateDistanceToBeacon(currentBeacon);
            xSemaphoreGive(gpsSemaphore);
            return true;
          }
          updateDistanceToBeacon(currentBeacon);
          xSemaphoreGive(gpsSemaphore);
          return false;
        }
        xSemaphoreGive(gpsSemaphore);
      }
      return false;
    }
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
  #elif defined(ACT_AS_BEACON)
    void calculateDistanceToTrackers()
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        for(uint8_t trackerIndex = 1; trackerIndex < numberOfDevices; trackerIndex++)
        {
          if((device[trackerIndex].typeOfDevice & 0x01) == 1 && device[trackerIndex].hasFix == true)
          {
            device[trackerIndex].distanceTo = TinyGPSPlus::distanceBetween(device[0].latitude, device[0].longitude, device[0].latitude, device[0].longitude);
            device[trackerIndex].courseTo = TinyGPSPlus::courseTo(device[0].latitude, device[0].longitude, device[0].latitude, device[0].longitude);
            #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
              SERIAL_DEBUG_PORT.printf_P(PSTR("Tracker %u: distance %f course %f\r\n"), trackerIndex, device[trackerIndex].distanceTo, device[trackerIndex].courseTo);
            #endif
          }
        }
        xSemaphoreGive(gpsSemaphore);
      }
    }
  #endif
  #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
    void showGPSstatus()
    {
      //if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
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
        //xSemaphoreGive(gpsSemaphore);
      }
    }
  #endif
#endif

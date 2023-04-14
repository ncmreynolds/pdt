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
    localLog(F("GPS hardware serial RX: pin "));
    localLogLn(RXPin);
  }
  #ifdef USE_RTOS
    void processGpsSentences(void * parameter)
    {
      while(true)
      {
        if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
        {
          while(GPS_PORT.available()) //Check for incoming GPS data
          {
            gps.encode(GPS_PORT.read());  //Process the data
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
    if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
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
      xSemaphoreGive(gpsSemaphore);
    }
  }
  #if defined(ACT_AS_TRACKER)
    bool updateLocation()
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        tracker[0].hasFix = true;
        tracker[0].latitude = gps.location.lat();
        tracker[0].longitude = gps.location.lng();
        tracker[0].course = gps.course.deg();
        tracker[0].speed = gps.speed.kmph();
        tracker[0].hdop = gps.hdop.hdop();
        xSemaphoreGive(gpsSemaphore);
        return true;
      }
      return false;
    }
    void calculateDistanceToBeacon(uint8_t beaconIndex)
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        beacon[beaconIndex].distanceTo = TinyGPSPlus::distanceBetween(tracker[0].latitude, tracker[0].longitude, beacon[beaconIndex].latitude, beacon[beaconIndex].longitude);
        beacon[beaconIndex].courseTo = TinyGPSPlus::courseTo(tracker[0].latitude, tracker[0].longitude, beacon[beaconIndex].latitude, beacon[beaconIndex].longitude);
        if(distanceToCurrentBeacon < loRaPerimiter1)
        {
          beacon[beaconIndex].timeout = beaconInterval1 * 2.5;
        }
        else if(distanceToCurrentBeacon < loRaPerimiter2)
        {
          beacon[beaconIndex].timeout = beaconInterval2 * 2.5;
        }
        else if(distanceToCurrentBeacon < loRaPerimiter3)
        {
          beacon[beaconIndex].timeout = beaconInterval3 * 2.5;
        }
        else
        {
          beacon[beaconIndex].timeout = 60000;
        }
        xSemaphoreGive(gpsSemaphore);
      }
    }
    bool selectNearestBeacon()
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        if(numberOfBeacons == 0)
        {
          xSemaphoreGive(gpsSemaphore);
          return false;
        }
        else if(numberOfBeacons == 1)
        {
          currentBeacon = 0;
          xSemaphoreGive(gpsSemaphore);
          if(uint32_t(beacon[currentBeacon].distanceTo) != distanceToCurrentBeacon)
          {
            distanceToCurrentBeacon = uint32_t(beacon[currentBeacon].distanceTo);
            distanceToCurrentBeaconChanged = true;
            setBeeperUrgency();
          }
          return true;
        }
        else
        {
          uint8_t currentNearestBeacon = 0;
          for(uint8_t index = 0; index < numberOfBeacons; index++)
          {
            if(beacon[index].distanceTo < beacon[currentNearestBeacon].distanceTo)
            {
              currentNearestBeacon = index;
            }
          }
          currentBeacon = currentNearestBeacon;
          if(uint32_t(beacon[currentBeacon].distanceTo) != distanceToCurrentBeacon)
          {
            distanceToCurrentBeacon = uint32_t(beacon[currentBeacon].distanceTo);
            distanceToCurrentBeaconChanged = true;
            setBeeperUrgency();
          }
          xSemaphoreGive(gpsSemaphore);
          return true;
        }
        xSemaphoreGive(gpsSemaphore);
      }
      return false;
    }
    bool selectFurthestBeacon()
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        if(numberOfBeacons == 0)
        {
          xSemaphoreGive(gpsSemaphore);
          return false;
        }
        else if(numberOfBeacons == 1)
        {
          currentBeacon = 0;
          xSemaphoreGive(gpsSemaphore);
          if(uint32_t(beacon[currentBeacon].distanceTo) != distanceToCurrentBeacon)
          {
            distanceToCurrentBeacon = uint32_t(beacon[currentBeacon].distanceTo);
            distanceToCurrentBeaconChanged = true;
            setBeeperUrgency();
          }
          return true;
        }
        else
        {
          uint8_t currentFurthestBeacon = 0;
          for(uint8_t index = 0; index < numberOfBeacons; index++)
          {
            if(beacon[index].distanceTo > beacon[currentFurthestBeacon].distanceTo)
            {
              currentFurthestBeacon = index;
            }
          }
          currentBeacon = currentFurthestBeacon;
          if(uint32_t(beacon[currentBeacon].distanceTo) != distanceToCurrentBeacon)
          {
            distanceToCurrentBeacon = uint32_t(beacon[currentBeacon].distanceTo);
            distanceToCurrentBeaconChanged = true;
            setBeeperUrgency();
          }
          xSemaphoreGive(gpsSemaphore);
          return true;
        }
        xSemaphoreGive(gpsSemaphore);
      }
      return false;
    }
  #elif defined(ACT_AS_BEACON)
    bool updateLocation()
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        beacon[0].latitude = gps.location.lat();
        beacon[0].longitude = gps.location.lng();
        beacon[0].course = gps.course.deg();
        beacon[0].speed = gps.speed.kmph();
        beacon[0].hdop = gps.hdop.hdop();
        xSemaphoreGive(gpsSemaphore);
        return true;
      }
      return false;
    }
    void calculateDistanceToTracker(uint8_t trackerIndex)
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        tracker[trackerIndex].distanceTo = TinyGPSPlus::distanceBetween(beacon[0].latitude, beacon[0].longitude, tracker[0].latitude, tracker[0].longitude);
        tracker[trackerIndex].courseTo = TinyGPSPlus::courseTo(beacon[0].latitude, beacon[0].longitude, tracker[0].latitude, tracker[0].longitude);
        xSemaphoreGive(gpsSemaphore);
      }
    }
  #endif
  #ifdef SERIAL_DEBUG
    void showGPSstatus()
    {
      if(xSemaphoreTake(gpsSemaphore, gpsSemaphoreTimeout))
      {
        SERIAL_DEBUG_PORT.print(F("GPS - Chars: "));
        SERIAL_DEBUG_PORT.print(gps.charsProcessed());
        SERIAL_DEBUG_PORT.print(F(" Sentences: "));
        SERIAL_DEBUG_PORT.print(gps.sentencesWithFix());
        SERIAL_DEBUG_PORT.print(F(" Errors: "));
        SERIAL_DEBUG_PORT.print(gps.failedChecksum());
        if(gps.location.isValid())
        {
          SERIAL_DEBUG_PORT.print(F(" Lat: "));
          SERIAL_DEBUG_PORT.print(gps.location.lat());
          SERIAL_DEBUG_PORT.print(F(" Lon: "));
          SERIAL_DEBUG_PORT.print(gps.location.lng());
          SERIAL_DEBUG_PORT.print(F(" Alt: "));
          SERIAL_DEBUG_PORT.print(gps.altitude.meters());
          SERIAL_DEBUG_PORT.print(F(" HDOP: "));
          SERIAL_DEBUG_PORT.print(gps.hdop.hdop());
          SERIAL_DEBUG_PORT.print(F(" Age: "));
          SERIAL_DEBUG_PORT.print(gps.location.age());
          SERIAL_DEBUG_PORT.print(F(" Course: "));
          SERIAL_DEBUG_PORT.print(gps.course.deg());
          SERIAL_DEBUG_PORT.print(F(" Speed: "));
          SERIAL_DEBUG_PORT.print(gps.speed.kmph());
        }
        SERIAL_DEBUG_PORT.print(F(" Sat: "));
        if(gps.location.isValid())
        {
          SERIAL_DEBUG_PORT.println(gps.satellites.value());
        }
        else
        {
          SERIAL_DEBUG_PORT.print(gps.satellites.value());
          SERIAL_DEBUG_PORT.println(F(" - no fix"));
        }
      }
      xSemaphoreGive(gpsSemaphore);
    }
  #endif
#endif

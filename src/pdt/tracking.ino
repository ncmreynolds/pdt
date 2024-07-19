#if defined(SUPPORT_GPS)
  void calculateDistanceToAllDevices()
  {
    for(uint8_t index = 1; index < numberOfDevices; index++)
    {
      if(device[index].hasGpsFix == true)
      {
        float newDistance = TinyGPSPlus::distanceBetween(device[0].latitude, device[0].longitude, device[index].latitude, device[index].longitude);
        if(abs(device[index].distanceTo - newDistance) > 1)
        {
          #if defined(SUPPORT_LVGL) && defined(LVGL_SUPPORT_SCAN_INFO_TAB)
            findableDevicesChanged = true;
          #endif
          device[index].distanceTo = newDistance;
          device[index].courseTo = TinyGPSPlus::courseTo(device[0].latitude, device[0].longitude, device[index].latitude, device[index].longitude);
          #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
            SERIAL_DEBUG_PORT.printf_P(PSTR("NodeId:%02x - distance:%01.1f(m) course:%03.1f(deg) %04x"), device[index].id, device[index].distanceTo, device[index].courseTo, treacle.rxReliability(device[index].id));
          #endif
          if(index == currentlyTrackedDevice)
          {
            #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
              SERIAL_DEBUG_PORT.println(F(" - tracked"));
            #endif
            distanceToCurrentlyTrackedDeviceChanged = true;
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
  }
  #if defined(ACT_AS_TRACKER)
    void selectDeviceToTrack()
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
        SERIAL_DEBUG_PORT.print(F("Tracking "));
      #endif
      if(numberOfDevices != 0 && currentlyTrackedDevice != maximumNumberOfDevices)
      {
        if(treacle.online(device[currentlyTrackedDevice].id) == false &&  //Offline
          (device[currentlyTrackedDevice].typeOfDevice & 0x80) == 0x00)   //Not 'usually stationary'.
        {
          currentlyTrackedDevice = maximumNumberOfDevices;
          #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
            SERIAL_DEBUG_PORT.print(F("beacon gone offline"));
            SERIAL_DEBUG_PORT.print(F(" after "));
            SERIAL_DEBUG_PORT.println(printableDuration((millis()-lastChangeOfTrackedDevice)/1000));
          #endif
          #if defined(SUPPORT_LVGL) && defined(LVGL_SUPPORT_SCAN_INFO_TAB)
            findableDevicesChanged = true;
          #endif
          lastChangeOfTrackedDevice = millis();
          return;
        }
      }
      if(currentTrackingMode == trackingMode::nearest)
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
          SERIAL_DEBUG_PORT.print(F("nearest beacon"));
        #endif
        if(selectNearestBeacon())
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
            SERIAL_DEBUG_PORT.printf_P(PSTR(":%02x after %s\r\n"), currentlyTrackedDevice, printableDuration((millis()-lastChangeOfTrackedDevice)/1000).c_str());
          #endif
          #if defined(SUPPORT_LVGL) && defined(LVGL_SUPPORT_SCAN_INFO_TAB)
            findableDevicesChanged = true;
          #endif
          lastChangeOfTrackedDevice = millis();
          return;
        }
        else
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
            SERIAL_DEBUG_PORT.println(F(" unchanged"));
          #endif
          return;
        }
      }
      else if(currentTrackingMode == trackingMode::furthest)
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
          SERIAL_DEBUG_PORT.print(F("furthest beacon"));
        #endif
        if(selectFurthestBeacon())
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
            SERIAL_DEBUG_PORT.printf_P(PSTR(":%02x after %s\r\n"), currentlyTrackedDevice, printableDuration((millis()-lastChangeOfTrackedDevice)/1000).c_str());
          #endif
          currentTrackingMode = trackingMode::fixed;  //Switch to fixed as 'furthest' needs to fix once chose
          #if defined(SUPPORT_LVGL) && defined(LVGL_SUPPORT_SCAN_INFO_TAB)
            findableDevicesChanged = true;
          #endif
          lastChangeOfTrackedDevice = millis();
          return;
        }
        else
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
            SERIAL_DEBUG_PORT.println(F(" unchanged"));
          #endif
          return;
        }
      }
      else if(currentTrackingMode == trackingMode::fixed)
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_BEACON_SELECTION)
          SERIAL_DEBUG_PORT.printf_P(PSTR("specific beacon:%02x for %s\r\n"), currentlyTrackedDevice, printableDuration((millis()-lastChangeOfTrackedDevice)/1000).c_str());
        #endif
        return;
      }
    }
    bool selectNearestBeacon()  //True only implies it has changed!
    {
      if(numberOfDevices == 0)
      {
        currentlyTrackedDevice = maximumNumberOfDevices;
        return false;
      }
      if(numberOfDevices == 2 && deviceIsSensibleToTrack(1))
      {
        if(currentlyTrackedDevice != 1)  //Only assign this once
        {
          currentlyTrackedDevice = 1;
          //updateDistanceToBeacon(currentlyTrackedDevice);
          return true;
        }
        //updateDistanceToBeacon(currentlyTrackedDevice);
        return false;
      }
      else
      {
        uint8_t nearestBeacon = maximumNumberOfDevices; //Determine this anew every time
        for(uint8_t index = 1; index < numberOfDevices; index++)
        {
          if(deviceIsSensibleToTrack(index) && (nearestBeacon == maximumNumberOfDevices || device[index].distanceTo < device[nearestBeacon].distanceTo))
          {
            nearestBeacon = index;
          }
        }
        if(nearestBeacon != maximumNumberOfDevices && currentlyTrackedDevice != nearestBeacon) //Choose a new nearest beacon
        {
          currentlyTrackedDevice = nearestBeacon;
          return true;
        }
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
      if(numberOfDevices == 2 && deviceIsSensibleToTrack(1))
      {
        if(currentlyTrackedDevice != 1)
        {
          currentlyTrackedDevice = 1;
          return true;
        }
        return false;
      }
      else
      {
        uint8_t furthestBeacon = maximumNumberOfDevices;
        for(uint8_t index = 1; index < numberOfDevices; index++)
        {
          if(deviceIsSensibleToTrack(index) && (furthestBeacon == maximumNumberOfDevices || device[index].distanceTo > device[furthestBeacon].distanceTo))
          {
            furthestBeacon = index;
          }
        }
        if(furthestBeacon != maximumNumberOfDevices && currentlyTrackedDevice != furthestBeacon)
        {
          currentlyTrackedDevice = furthestBeacon;
          return true;
        }
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
    bool deviceIsSensibleToTrack(uint8_t index)
    {
       return (device[index].typeOfDevice & 0x01) == 0 && device[index].hasGpsFix == true && treacle.online(device[index].id) && rangeToIndicate(index) < maximumEffectiveRange;
    }
  #endif
#endif

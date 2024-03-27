#if defined(SUPPORT_TREACLE)
  void setupTreacle()
  {
    #if defined(DEBUG_TREACLE)
      treacle.enableDebug(SERIAL_DEBUG_PORT);
    #endif
    if(espNowEnabled == true)
    {
      treacle.enableEspNow();
    }
    if(loRaEnabled == true)
    {
      treacle.setLoRaPins(loRaCsPin, loRaResetPin); //Set the LoRa reset and CS pins, assuming other default SPI pins
      treacle.setLoRaFrequency(loRaFrequency);      //Set the LoRa frequency. Valid value are 433/868/915Mhz depending on region
      treacle.enableLoRa();                         //Enable LoRa
    }
    #if defined(SUPPORT_WIFI)
      if(startWiFiApOnBoot)
      {
        treacle.setEspNowChannel(softApChannel);
      }
    #endif
    if(device[0].id != 0)
    {
      treacle.setNodeId(device[0].id);
    }
    if(device[0].name != nullptr)
    {
      treacle.setNodeName(device[0].name);
    }
    #if !defined(DEBUG_TREACLE)
      localLog(F("Initialising treacle: "));
    #endif
    treacleIntialised = treacle.begin();
    #if !defined(DEBUG_TREACLE)
      if(treacleIntialised)
      {
        localLogLn(F("OK"));
      }
      else
      {
        localLogLn(F("failed"));
      }
      if(device[0].id != 0)
      {
        localLog(F("Treacle ID: "));
        localLogLn(device[0].id);
      }
    #endif
  }
  void manageTreacle()
  {
    if(treacle.online() == true)
    {
      if(treacle.messageWaiting() > 0) //There's something in the buffer to process
      {
        processTreaclePacket();
      }
      if(millis() - lastTreacleDeviceInfoSendTime > treacleDeviceInfoInterval)
      {
        lastTreacleDeviceInfoSendTime = millis() - random(100,200);
        if(treacle.getNodeId() != device[0].id) //Save any autonegotiated nodeId
        {
          device[0].id = treacle.getNodeId();
          saveConfigurationSoon = millis();
        }
        if(typeOfLastTreacleUpdate == deviceIcInfoId)
        {
          if(shareDeviceInfo())
          {
            typeOfLastTreacleUpdate = deviceStatusUpdateId;
          }
        }
        else if(typeOfLastTreacleUpdate == deviceStatusUpdateId)
        {
          if(shareIcInfo())
          {
            typeOfLastTreacleUpdate = deviceIcInfoId;
          }
        }
      }
      if(millis() - lastTreacleLocationSendTime > device[0].nextTreacleLocationUpdate)
      {
        lastTreacleLocationSendTime = millis() - random(100,200);
        #if defined(ACT_AS_TRACKER)
          if(numberOfBeacons() > 0)  //Only share location if there are some beacons
          {
            if(shareLocation())
            {
              if(currentlyTrackedBeacon != maximumNumberOfDevices) //There is a current beacon
              {
                device[0].nextTreacleLocationUpdate = newLocationSharingInterval(distanceToCurrentBeacon, device[0].speed);  //Include speed in calculation
              }
            }
          }
          else
          {
            device[0].nextTreacleLocationUpdate = newLocationSharingInterval(effectivelyUnreachable, 0);
          }
        #elif defined(ACT_AS_BEACON)
          if(numberOfTrackers() > 0)
          {
            if(shareLocation())
            {
              if(closestTracker != maximumNumberOfDevices) //There's a reasonable nearby tracker
              {
                device[0].nextTreacleLocationUpdate = newLocationSharingInterval(distanceToClosestTracker, device[0].speed);  //Include speed in calculation
              }
            }
          }
          else
          {
            device[0].nextTreacleLocationUpdate = newLocationSharingInterval(effectivelyUnreachable, 0);
          }
        #endif
      }
    }
    else
    {
      if(treacle.messageWaiting() > 0) //process packets
      {
        treacle.clearWaitingMessage();
      }
    }
  }
  bool shareLocation()
  {
    MsgPack::Packer packer;
    packer.pack(locationUpdateId);
    packer.pack(device[0].latitude);
    packer.pack(device[0].longitude);
    packer.pack(device[0].course);
    packer.pack(device[0].speed);
    packer.pack(device[0].hdop);
    if(packer.size() < treacle.maxPayloadSize())
    {
      if(treacle.queueMessage(packer.data(), packer.size()))
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_TREACLE)
          if(waitForBufferSpace(80))
          {
            SERIAL_DEBUG_PORT.printf_P(PSTR("Treacle TX location Lat:%03.4f Lon:%03.4f Course:%03.1f Speed:%02.1f HDOP:%.1f next update:%us\r\n"),
              device[0].latitude,
              device[0].longitude,
              device[0].course,
              device[0].speed,
              device[0].hdop,
              device[0].nextTreacleLocationUpdate/1000
              );
          }
        #endif
        return true;
      }
    }
    return false;
  }
  bool shareDeviceInfo()
  {
    MsgPack::Packer packer;
    packer.pack(deviceStatusUpdateId);
    packer.pack(device[0].typeOfDevice);
    packer.pack(device[0].majorVersion);
    packer.pack(device[0].minorVersion);
    packer.pack(device[0].patchVersion);
    packer.pack(millis());
    packer.pack(device[0].supplyVoltage);
    packer.pack(device[0].name);
    #if defined(ACT_AS_SENSOR)
      packer.pack(device[0].numberOfStartingHits);
      packer.pack(device[0].numberOfStartingStunHits);
      packer.pack(device[0].currentNumberOfHits);
      packer.pack(device[0].currentNumberOfStunHits);
    #endif
    if(packer.size() < treacle.maxPayloadSize())
    {
      if(treacle.queueMessage(packer.data(), packer.size()))
      {  
        #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
          if(waitForBufferSpace(80))
          {
            #if defined(ACT_AS_SENSOR)
            SERIAL_DEBUG_PORT.printf("Treacle TX device info type:%02X, version: %u.%u.%u name: '%s', uptime:%s, supply:%.1fv Hits:%u/%u Stun:%u/%u\r\n",
              device[0].typeOfDevice,
              device[0].majorVersion,
              device[0].minorVersion,
              device[0].patchVersion,
              device[0].name,
              printableUptime(millis()/1000).c_str(),
              device[0].supplyVoltage,
              device[0].currentNumberOfHits,
              device[0].numberOfStartingHits,
              device[0].currentNumberOfStunHits,
              device[0].numberOfStartingStunHits
              );
            #else
              SERIAL_DEBUG_PORT.printf("Treacle TX device info type:%02X, version: %u.%u.%u name: '%s', uptime:%s, supply:%.1fv\r\n",
              device[0].typeOfDevice,
              device[0].majorVersion,
              device[0].minorVersion,
              device[0].patchVersion,
              device[0].name,
              printableUptime(millis()/1000).c_str(),
              device[0].supplyVoltage
              );
            #endif
          }
        #endif
        return true;
      }
    }
    return false;
  }
  bool shareIcInfo()
  {
    if(device[0].icName != nullptr && device[0].icDescription != nullptr)
    {
      MsgPack::Packer packer;
      packer.pack(deviceIcInfoId);
      packer.pack(device[0].diameter);
      packer.pack(device[0].height);
      packer.pack(device[0].icName);
      packer.pack(device[0].icDescription);
      if(packer.size() < treacle.maxPayloadSize())
      {
        if(treacle.queueMessage(packer.data(), packer.size()))
        {  
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
            if(waitForBufferSpace(80))
            {
              SERIAL_DEBUG_PORT.printf("Treacle TX IC info name: '%s', desc:'%s'\r\n",device[0].id[0],
                device[0].icName,
                device[0].icDescription
                );
            }
          #endif
          return true;
        }
      }
    }
    return false;
  }
  uint32_t newLocationSharingInterval(uint16_t distance, float speed)
  {
    uint32_t newInterval = 0;
    if(speed == 0)
    {
      if(distance < treaclePerimiter1)
      {
        newInterval = treacleLocationInterval1;
      }
      else if(distance < treaclePerimiter2)
      {
        newInterval = treacleLocationInterval2;
      }
      else if(distance < treaclePerimiter3)
      {
        newInterval = treacleLocationInterval3;
      }
      else
      {
        newInterval = defaultTreacleLocationInterval;
      }
      return newInterval;
    }
    newInterval = newLocationSharingInterval(distance, 0);  //Get the interval as if not moving
    int16_t worstCaseDistance = (int16_t)distance - (speed * (newInterval/1000)); //Estimate worst case new distance after this interval
    if(worstCaseDistance > 0)
    {
      newInterval = newLocationSharingInterval(worstCaseDistance, 0); //Assess new interval based on worst case distance
    }
    else
    {
      newInterval = treacleLocationInterval1;  //Return the shortest interval
    }
    return newInterval;
  }
  void processTreaclePacket()
  {
  }
#endif

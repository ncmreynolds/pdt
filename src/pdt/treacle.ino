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
    treacle.setLoRaPins(loRaCsPin, loRaResetPin);         //Set the LoRa reset and CS pins, assuming other default SPI pins
    treacle.setLoRaFrequency(loRaFrequency);              //Set the LoRa frequency. Valid value are 433/868/915MHz depending on region
    treacle.setLoRaTxPower(loRaTxPower);                  //Supported values are 2-20
    treacle.setLoRaRxGain(loRaRxGain);                    //Supported values are 0-6
    treacle.setLoRaSpreadingFactor(loRaSpreadingFactor);  //Supported values are 7-12
    treacle.setLoRaSignalBandwidth(loRaSignalBandwidth);  //Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3(default), 250E3, and 500E3.
    treacle.enableLoRa();                                 //Enable LoRa
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
      localLog(F("PDT  ID: "));
      localLogLn(device[0].id);
    }
  #endif
  if(treacleIntialised)
  {
    if(treacle.espNowInitialised())
    {
      espNowInitialised = true;
      treacleDeviceInfoInterval = 10E3;
      treacle.setEspNowTickInterval(espNowTickInterval);
      if(treacle.loRaInitialised())
      {
        loRaInitialised = true;
      }
    }
    else if(treacle.loRaInitialised())
    {
      loRaInitialised = true;
      treacleDeviceInfoInterval = 30E3;
      treacle.setLoRaTickInterval(loRaTickInterval);
    }
    if(espNowInitialised)
    {
      if(espNowPhyMode == 1)
      {
        localLog(F("Enabling ESP-Now 11B mode:"));
        if(treacle.enableEspNow11bMode())
        {
          localLogLn(F("OK"));
        }
        else
        {
          localLogLn(F("failed"));
        }
      }
      else if(espNowPhyMode == 2)
      {
        localLog(F("Enabling ESP-Now LR mode:"));
        if(treacle.enableEspNowLrMode())
        {
          localLogLn(F("OK"));
        }
        else
        {
          localLogLn(F("failed"));
        }
      }
    }
  }
}
void manageTreacle()
{
  uint32_t waitingMessageSize = treacle.messageWaiting(); //This also updates the online/offline status so always call it!
  if(waitingMessageSize > 0) //There's something in the buffer to process
  {
    processTreacleMessage(waitingMessageSize);
  }
  else if(millis() - lastTreacleDeviceInfoSendTime > treacleDeviceInfoInterval) //Send periodic updates
  {
    if(treacle.getNodeId() != device[0].id) //Save any autonegotiated nodeId
    {
      device[0].id = treacle.getNodeId();
      saveConfigurationSoon = millis();
    }
    if(treacleIntialised)
    {
      if(typeOfLastTreacleUpdate%6 == 0)       //Every 6th update
      {
        if(shareDeviceInfo())
        {
          typeOfLastTreacleUpdate++;
        }
      }
      else if(typeOfLastTreacleUpdate%3 == 0)  //Every 3rd update
      {
        if(shareIcInfo())
        {
          typeOfLastTreacleUpdate++;
        }
      }
      else                                     //All other updates
      {
        if(shareLocation())
        {
          typeOfLastTreacleUpdate++;
        }
      }
    }
    lastTreacleDeviceInfoSendTime = millis();
    treacleDeviceInfoInterval = treacle.suggestedQueueInterval(); //Rely on treacle to suggest appropriate send intervals
  }
}
bool shareLocation()
{
  MsgPack::Packer packer;
  packer.pack(locationUpdateId);
  packer.pack(device[0].typeOfDevice);
  packer.pack(device[0].hasGpsFix);
  packer.pack(device[0].latitude);
  packer.pack(device[0].longitude);
  packer.pack(device[0].course);
  packer.pack(device[0].speed);
  packer.pack(device[0].hdop);
  if(packer.size() < treacle.maxPayloadSize())
  {
    #if defined(TREACLE_SEND_FAST)
    if(treacle.sendMessage(packer.data(), packer.size()))
    #else
    if(treacle.queueMessage(packer.data(), packer.size()))
    #endif
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
        if(waitForBufferSpace(80))
        {
          SERIAL_DEBUG_PORT.printf_P(PSTR("Update TX nodeId:%02x locn Lat:%03.4f Lon:%03.4f Course:%03.1f(deg) Speed:%01.1f(m/s) HDOP:%.1f Distance:%.1f(m)\r\n"),
            device[0].id,
            device[0].latitude,
            device[0].longitude,
            device[0].course,
            device[0].speed,
            device[0].hdop
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
    #if defined(TREACLE_SEND_FAST)
    if(treacle.sendMessage(packer.data(), packer.size()))
    #else
    if(treacle.queueMessage(packer.data(), packer.size()))
    #endif
    {  
      #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
        if(waitForBufferSpace(80))
        {
          #if defined(ACT_AS_SENSOR)
          SERIAL_DEBUG_PORT.printf_P(PSTR("Update TX nodeId:%02x info type:%02X, version: %u.%u.%u name: '%s', uptime:%s, supply:%.1fv Hits:%u/%u Stun:%u/%u\r\n"),
            device[0].id,
            device[0].typeOfDevice,
            device[0].majorVersion,
            device[0].minorVersion,
            device[0].patchVersion,
            device[0].name,
            printableDuration(millis()/1000).c_str(),
            device[0].supplyVoltage,
            device[0].currentNumberOfHits,
            device[0].numberOfStartingHits,
            device[0].currentNumberOfStunHits,
            device[0].numberOfStartingStunHits
            );
          #else
            SERIAL_DEBUG_PORT.printf_P(PSTR("Update TX nodeId:%02x info type:%02X, version: %u.%u.%u name: '%s', uptime:%s, supply:%.1fv\r\n"),
            device[0].id,
            device[0].typeOfDevice,
            device[0].majorVersion,
            device[0].minorVersion,
            device[0].patchVersion,
            device[0].name,
            printableDuration(millis()/1000).c_str(),
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
    packer.pack(device[0].typeOfDevice);
    packer.pack(device[0].diameter);
    packer.pack(device[0].height);
    packer.pack(device[0].icName);
    packer.pack(device[0].icDescription);
    if(packer.size() < treacle.maxPayloadSize())
    {
      #if defined(TREACLE_SEND_FAST)
      if(treacle.sendMessage(packer.data(), packer.size()))
      #else
      if(treacle.queueMessage(packer.data(), packer.size()))
      #endif
      {  
        #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
          if(waitForBufferSpace(80))
          {
            SERIAL_DEBUG_PORT.printf_P(PSTR("Update TX nodeId:%02x desc name: '%s', desc:'%s', diameter:%u height:%u\r\n"),
              device[0].id,
              device[0].icName,
              device[0].icDescription,
              device[0].diameter,
              device[0].height
              );
          }
        #endif
        return true;
      }
    }
  }
  return false;
}
uint8_t identifyDevice(uint8_t id)
{
  uint8_t deviceIndex = 1;
  if(numberOfDevices == maximumNumberOfDevices)
  {
    #if defined(SERIAL_DEBUG)
      if(waitForBufferSpace(50))
      {
        SERIAL_DEBUG_PORT.printf_P(PSTR("Too many devices to add %02x\r\n"), id);
      }
    #endif
    return maximumNumberOfDevices;
  }
  for(deviceIndex = 1; deviceIndex < numberOfDevices; deviceIndex++)
  {
    if(device[deviceIndex].id == id)
    {
      return deviceIndex;
    }
  }
  #if defined(SERIAL_DEBUG)
    if(waitForBufferSpace(50))
    {
      SERIAL_DEBUG_PORT.printf_P(PSTR("New device nodeId:%02x, now %u device(s)\r\n"), id, numberOfDevices);
    }
  #endif
  device[numberOfDevices].id = id;
  numberOfDevices++;
  lastTreacleDeviceInfoSendTime = (millis() - treacleDeviceInfoInterval) + random(1000,5000); //A new device prompts a status share in 1-5s
  return numberOfDevices-1;
}
void processTreacleMessage(uint32_t messageSize)
{
  uint8_t sender = treacle.messageSender();
  uint8_t message[messageSize];
  treacle.retrieveWaitingMessage(message);
  /*
  #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
    if(waitForBufferSpace(30))
    {
      SERIAL_DEBUG_PORT.println(F("Processing received packet..."));
    }
  #endif
  */
  MsgPack::Unpacker unpacker;
  unpacker.feed(message, messageSize);
  uint8_t messagetype;
  unpacker.unpack(messagetype);
  uint8_t deviceIndex = identifyDevice(sender);
  if(deviceIndex < maximumNumberOfDevices)
  {
    #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
      if(waitForBufferSpace(80))
      {
        SERIAL_DEBUG_PORT.printf_P(PSTR("Update RX NodeId:%02x "), sender);
      }
    #endif
    if(messagetype == locationUpdateId)
    {
      bool hasFix = false;
      unpacker.unpack(device[deviceIndex].typeOfDevice);
      unpacker.unpack(hasFix);
      unpacker.unpack(device[deviceIndex].latitude);
      unpacker.unpack(device[deviceIndex].longitude);
      unpacker.unpack(device[deviceIndex].course);
      unpacker.unpack(device[deviceIndex].speed);
      unpacker.unpack(device[deviceIndex].hdop);
      if(hasFix)
      {
        if(device[deviceIndex].hasGpsFix == false)
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
            if(waitForBufferSpace(40))
            {
              SERIAL_DEBUG_PORT.printf_P(PSTR("nodeId:%02x got GPS fix\r\n"), device[deviceIndex].id);
            }
          #endif
          device[deviceIndex].hasGpsFix = true;
        }
      }
      else if(device[deviceIndex].hasGpsFix == true)
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
          if(waitForBufferSpace(40))
          {
            SERIAL_DEBUG_PORT.printf_P(PSTR("nodeId:%02x lost GPS fix\r\n"), device[deviceIndex].id);
          }
        #endif
        device[deviceIndex].hasGpsFix = false;
      }
      if(device[deviceIndex].hdop < normalHdopThreshold)
      {
        device[deviceIndex].smoothedSpeed = (device[deviceIndex].smoothedSpeed * smoothingFactor) + (device[deviceIndex].speed * (1-smoothingFactor));
        if(device[deviceIndex].smoothedSpeed < movementThreshold)
        {
          if(device[deviceIndex].moving == true)
          {
            device[deviceIndex].moving = false;
            #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
              SERIAL_DEBUG_PORT.printf_P(PSTR("Device %02x stationary\r\n"), device[deviceIndex].id);
            #endif
          }
        }
        else
        {
          if(device[deviceIndex].moving == false)
          {
            device[deviceIndex].moving = true;
            #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
              SERIAL_DEBUG_PORT.printf_P(PSTR("Device %02x moving\r\n"), device[deviceIndex].id);
            #endif
          }
        }
      }
      #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
        if(waitForBufferSpace(80))
        {
            SERIAL_DEBUG_PORT.printf_P(PSTR("locn Lat:%03.4f Lon:%03.4f Course:%03.1f(deg) Speed:%01.1f(m/s) HDOP:%.1f Distance:%.1f(m)\r\n"),
            device[deviceIndex].latitude,
            device[deviceIndex].longitude,
            device[deviceIndex].course,
            device[deviceIndex].speed,
            device[deviceIndex].hdop,
            device[deviceIndex].distanceTo
            );
        }
      #endif
    }
    else if(messagetype == deviceStatusUpdateId)
    {
      unpacker.unpack(device[deviceIndex].typeOfDevice);
      unpacker.unpack(device[deviceIndex].majorVersion);
      unpacker.unpack(device[deviceIndex].minorVersion);
      unpacker.unpack(device[deviceIndex].patchVersion);
      unpacker.unpack(device[deviceIndex].uptime);
      unpacker.unpack(device[deviceIndex].supplyVoltage);
      String receivedDeviceName = String(unpacker.unpackString());
      bool storeReceivedName = false;
      if(device[deviceIndex].name == nullptr) //First time the name is received
      {
        storeReceivedName = true;
      }
      else if(receivedDeviceName.equals(String(device[deviceIndex].name)) == false) //Check the name hasn't changed
      {
        delete[] device[deviceIndex].name;
        storeReceivedName = true;
      }
      if(storeReceivedName == true)
      {
        device[deviceIndex].name = new char[receivedDeviceName.length() + 1];
        receivedDeviceName.toCharArray(device[deviceIndex].name, receivedDeviceName.length() + 1);
      }
      #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
        if(waitForBufferSpace(60))
        {
          SERIAL_DEBUG_PORT.printf_P(PSTR("info type:%02x version:%u.%u.%u name:'%s' up:%s battery voltage:%.1fv"),
            device[deviceIndex].typeOfDevice,
            device[deviceIndex].majorVersion,
            device[deviceIndex].minorVersion,
            device[deviceIndex].patchVersion,
            device[deviceIndex].name,
            printableDuration(device[deviceIndex].uptime/1000).c_str(),
            device[deviceIndex].supplyVoltage
            );
        }
      #endif
      if((device[deviceIndex].typeOfDevice & 0x02) == 0x02)  //It's acting as a sensor
      {
        uint8_t unpackerTemp = 0;
        unpacker.unpack(unpackerTemp);
        if(device[deviceIndex].numberOfStartingHits != unpackerTemp)
        {
          device[deviceIndex].numberOfStartingHits = unpackerTemp;
          #if defined(ACT_AS_TRACKER)
            currentlyTrackedDeviceStateChanged = true;
          #endif
        }
        unpacker.unpack(unpackerTemp);
        if(device[deviceIndex].numberOfStartingStunHits != unpackerTemp)
        {
          device[deviceIndex].numberOfStartingStunHits = unpackerTemp;
          #if defined(ACT_AS_TRACKER)
            currentlyTrackedDeviceStateChanged = true;
          #endif
        }
        unpacker.unpack(unpackerTemp);
        if(device[deviceIndex].currentNumberOfHits != unpackerTemp)
        {
          device[deviceIndex].currentNumberOfHits = unpackerTemp;
          #if defined(ACT_AS_TRACKER)
            currentlyTrackedDeviceStateChanged = true;
          #endif
        }
        unpacker.unpack(unpackerTemp);
        if(device[deviceIndex].currentNumberOfStunHits != unpackerTemp)
        {
          device[deviceIndex].currentNumberOfStunHits = unpackerTemp;
          #if defined(ACT_AS_TRACKER)
            currentlyTrackedDeviceStateChanged = true;
          #endif
        }
        #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
          if((device[deviceIndex].typeOfDevice & 0x02) == 0x02)  //It's acting as a sensor
          {
            if(waitForBufferSpace(60))
            {
              SERIAL_DEBUG_PORT.printf_P(" hits %u/%u Stun %u/%u\r\n",
                device[deviceIndex].currentNumberOfHits,
                device[deviceIndex].numberOfStartingHits,
                device[deviceIndex].currentNumberOfStunHits,
                device[deviceIndex].numberOfStartingStunHits
                );
            }
          }
        #endif
      }
      else
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
          SERIAL_DEBUG_PORT.println();
        #endif
      }
    }
    else if(messagetype == deviceIcInfoId)
    {
      unpacker.unpack(device[deviceIndex].typeOfDevice);
      unpacker.unpack(device[deviceIndex].diameter);
      unpacker.unpack(device[deviceIndex].height);
      String receivedIcName = String(unpacker.unpackString());
      bool storeReceivedIcName = false;
      if(device[deviceIndex].icName == nullptr) //First time the name is received
      {
        storeReceivedIcName = true;
      }
      else if(receivedIcName.equals(String(device[deviceIndex].icName)) == false) //Check the name hasn't changed
      {
        delete[] device[deviceIndex].icName;
        device[deviceIndex].icName = nullptr;
        storeReceivedIcName = true;
      }
      if(storeReceivedIcName == true && receivedIcName.length() > 0)
      {
        device[deviceIndex].icName = new char[receivedIcName.length() + 1];
        receivedIcName.toCharArray(device[deviceIndex].icName, receivedIcName.length() + 1);
      }
      String receivedIcDescription = String(unpacker.unpackString());
      bool storeReceivedIcDescription = false;
      if(device[deviceIndex].icDescription == nullptr) //First time the name is received
      {
        storeReceivedIcDescription = true;
      }
      else if(receivedIcDescription.equals(String(device[deviceIndex].icDescription)) == false) //Check the name hasn't changed
      {
        delete[] device[deviceIndex].icDescription;
        device[deviceIndex].icDescription = nullptr;
        storeReceivedIcDescription = true;
      }
      if(storeReceivedIcDescription == true && receivedIcDescription.length() > 0)
      {
        device[deviceIndex].icDescription = new char[receivedIcDescription.length() + 1];
        receivedIcDescription.toCharArray(device[deviceIndex].icDescription, receivedIcDescription.length() + 1);
      }
      #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
        if(waitForBufferSpace(60))
        {
          SERIAL_DEBUG_PORT.printf_P(PSTR("desc diameter:%u, height: %u, name:'%s', name:'%s'\r\n"),
            device[deviceIndex].diameter,
            device[deviceIndex].height,
            device[deviceIndex].icName,
            device[deviceIndex].icDescription
            );
        }
      #endif
      if(storeReceivedIcName == true || storeReceivedIcDescription == true)
      {
        localLog(F("Device ID: "));
        localLogLn(device[deviceIndex].id);
        if(device[deviceIndex].icName != nullptr)
        {
          localLog(F("Device IC name: "));
          localLogLn(device[deviceIndex].icName);
        }
        if(device[deviceIndex].icDescription != nullptr)
        {
          localLog(F("Device IC description: "));
          localLogLn(device[deviceIndex].icDescription);
        }
        localLog(F("Device IC diameter: "));
        localLog(device[deviceIndex].diameter);
        localLog(F(" device IC height: "));
        localLogLn(device[deviceIndex].height);
      }
      #if defined(SUPPORT_LVGL) && defined(LVGL_SUPPORT_SCAN_INFO_TAB)
        findableDevicesChanged = true;
      #endif
    }
    else
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_UPDATES)
        if(waitForBufferSpace(40))
        {
          SERIAL_DEBUG_PORT.print(F("unexpected message type: "));
          SERIAL_DEBUG_PORT.println(messagetype);
        }
      #endif
    }
  }
  treacle.clearWaitingMessage();
}

#ifdef SUPPORT_ESPNOW
  void calculateEspNowDutyCycle()
  {
    calculatedEspNowDutyCycle = ((float)espNowTxTime/(float)millis())*100;
    localLog(F("EspNow TX(ms): "));
    localLog(espNowTxTime);
    localLog('/');
    localLog(millis());
    localLog(F(" duty cycle: "));
    localLog(calculatedEspNowDutyCycle);
    localLogLn('%');
  }
  void setupEspNow()
  {
    if(espNowEnabled == true)
    {
      localLog(F("Initialising ESP-Now: "));
      if(esp_now_init() == ESP_OK)
      {
        localLogLn(F("OK"));
        localLog(F("Adding ESP-Now receive callback: "));
        if(esp_now_register_recv_cb([](const uint8_t *macAddress, const uint8_t *receivedMessage, int receivedMessageLength) {  //Very basic copy of buffer for later processing
          if(espNowReceiveBufferSize == 0)
          {
            memcpy(&espNowReceiveBuffer,receivedMessage,receivedMessageLength);
            espNowReceiveBufferSize = receivedMessageLength;
            espNowRxPackets++;
          }
          else
          {
            espNowRxPacketsDropped++;
          }
        }) == ESP_OK)
        {
            localLogLn(F("OK"));
        }
        else
        {
            localLogLn(F("Failed"));
            espNowInitialised = false;
            return;
        }
        localLog(F("Adding ESP-Now send callback: "));
        if(esp_now_register_send_cb([](const uint8_t* macAddress, esp_now_send_status_t status) {
          if(status == ESP_OK)
          {
            if(espNowPacketSent != 0)
            {
              espNowTxTime += millis() - espNowPacketSent;
              espNowPacketSent = 0;
            }
            espNowTxPackets++;
          }
          else
          {
            espNowTxPacketsDropped++;
          }
        }) == ESP_OK)
        {
            localLogLn(F("OK"));
        }
        else
        {
            localLogLn(F("Failed"));
            espNowInitialised = false;
            return;
        }
        if(addBroadcastPeer())
        {
          espNowInitialised = true;
          return;
        }
        else
        {
          localLogLn(F("Failed"));
        }
      }
    }
    espNowInitialised = false;
  }
  bool addBroadcastPeer()
  {
    esp_now_peer_info_t newPeer;
    newPeer.peer_addr[0] = (uint8_t) broadcastMacAddress[0];
    newPeer.peer_addr[1] = (uint8_t) broadcastMacAddress[1];
    newPeer.peer_addr[2] = (uint8_t) broadcastMacAddress[2];
    newPeer.peer_addr[3] = (uint8_t) broadcastMacAddress[3];
    newPeer.peer_addr[4] = (uint8_t) broadcastMacAddress[4];
    newPeer.peer_addr[5] = (uint8_t) broadcastMacAddress[5];
    if(WiFi.getMode() == WIFI_STA)
    {
      newPeer.ifidx = WIFI_IF_STA;
    }
    else
    {
      newPeer.ifidx = WIFI_IF_AP;
    }
    newPeer.channel = WiFi.channel();
    espNowChannel = newPeer.channel;
    newPeer.encrypt = false;
    localLog(F("Adding ESP-Now broadcast peer on channel "));
    localLog(newPeer.channel);
    localLog(F(" : "));
    if(esp_now_add_peer(&newPeer) == ESP_OK)
    {
      localLogLn(F("OK"));
      return true;
    }
    localLogLn(F("failed"));
    return false;
  }
  bool deleteBroadcastPeer()
  {
    localLog(F("Deleting ESP-Now broadcast peer: "));
    if(esp_now_del_peer(broadcastMacAddress) == ESP_OK)
    {
      localLogLn(F("OK"));
      return true;
    }
    localLogLn(F("failed"));
    return false;
  }
  void manageEspNow()
  {
    if(espNowEnabled == true && espNowInitialised == true)
    {
      if(espNowReceiveBufferSize > 0) //There's something in the buffer to process
      {
        if(validateEspNowChecksum())
        {
          processEspNowPacket();
        }
        espNowReceiveBufferSize = 0;
      }
      if(millis() - lastEspNowDeviceInfoSendTime > espNowDeviceInfoInterval)
      {
        lastEspNowDeviceInfoSendTime = millis() - random(100,200);
        if(typeOfLastEspNowUpdate == deviceIcInfoId)
        {
          if(shareDeviceInfoByEspNow())
          {
            calculateEspNowDutyCycle();
          }
          typeOfLastEspNowUpdate = deviceStatusUpdateId;
        }
        else if(typeOfLastEspNowUpdate == deviceStatusUpdateId)
        {
          if(shareIcInfoByEspNow())
          {
            calculateEspNowDutyCycle();
          }
          typeOfLastEspNowUpdate = deviceIcInfoId;
        }
      }
      if(millis() - lastEspNowLocationSendTime > device[0].nextEspNowLocationUpdate)
      {
        lastEspNowLocationSendTime = millis() - random(100,200);
        #if defined(ACT_AS_TRACKER)
          if(numberOfBeacons() > 0)  //Only share location if there are some beacons
          {
            if(shareLocationByEspNow())
            {
              if(currentlyTrackedBeacon != maximumNumberOfDevices) //There is a current beacon
              {
                device[0].nextEspNowLocationUpdate = newEspNowLocationSharingInterval(distanceToCurrentBeacon, device[0].speed);  //Include speed in calculation
              }
              calculateEspNowDutyCycle();
            }
          }
          else
          {
            device[0].nextEspNowLocationUpdate = newEspNowLocationSharingInterval(effectivelyUnreachable, 0);
          }
          #elif defined(ACT_AS_BEACON)
          if(numberOfTrackers() > 0)
          {
            if(shareLocationByEspNow())
            {
              if(closestTracker != maximumNumberOfDevices) //There's a reasonable nearby tracker
              {
                device[0].nextEspNowLocationUpdate = newEspNowLocationSharingInterval(distanceToClosestTracker, device[0].speed);  //Include speed in calculation
              }
              calculateEspNowDutyCycle();
            }
          }
          else
          {
            device[0].nextEspNowLocationUpdate = newEspNowLocationSharingInterval(effectivelyUnreachable, 0);
          }
        #endif
      }
      for(uint8_t index = 1; index < numberOfDevices; index++)  //Degrade quality if location updates missed, this INCLUDES this device!
      {
        if(device[index].hasGpsFix == true && //Thing has fix!
            device[index].espNowUpdateHistory > 0x00 && //Not already offline
            device[index].nextEspNowLocationUpdate != 0 &&  //Thing has shared when to expect the location update
            millis() - device[index].lastEspNowLocationUpdate > (device[index].nextEspNowLocationUpdate + (device[index].nextEspNowLocationUpdate>>3))) //Allow margin of 1/8 the expected interval
        {
          device[index].lastEspNowLocationUpdate = millis();  //A failed update is an 'update'
          device[index].espNowUpdateHistory = device[index].espNowUpdateHistory >> 1; //Reduce update history quality
          //device[index].nextEspNowLocationUpdate = device[index].nextEspNowLocationUpdate >> 1; //Halve the timeout
          #ifdef ACT_AS_TRACKER
            if(index == currentlyTrackedBeacon)
            {
              localLog(F("Currently tracked beacon "));
            }
            else
          #else
            if(index == closestTracker)
            {
              localLog(F("Closest tracker "));
            }
            else
          #endif
          {
            localLog(F("Device "));
          }
          localLog(index);
          if(device[index].espNowOnline == true && device[index].espNowUpdateHistory < sensitivityValues[trackingSensitivity])
          {
            localLogLn(F(" ESPNow gone offline"));
            device[index].espNowOnline = false;
            #ifdef LVGL_ADD_SCAN_INFO_TAB
              findableDevicesChanged = true;
            #endif
            #ifdef ACT_AS_TRACKER
              if(index == currentlyTrackedBeacon) //Need to stop tracking this beacon
              {
                currentlyTrackedBeacon = maximumNumberOfDevices;
                distanceToCurrentBeacon = effectivelyUnreachable;
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
            #else
              if(index == closestTracker) //Need to stop tracking this beacon
              {
                closestTracker = maximumNumberOfDevices;
                distanceToClosestTracker = effectivelyUnreachable;
              }
            #endif
          }
          else
          {
            localLog(F(" dropped EspNow packet, signal quality now:0x"));
            localLogLn(String(device[index].espNowUpdateHistory,HEX));
          }
        }
      }
    }
  }
  bool appendEspNowChecksum()
  {
    if(espNowSendBufferSize < maxEspNowBufferSize - 2)
    {
      CRC16 crc(LORA_CRC_POLYNOME);
      crc.add((uint8_t *)espNowSendBuffer, espNowSendBufferSize);
      uint16_t packetCrc = crc.calc();
      espNowSendBuffer[espNowSendBufferSize++] = (packetCrc & 0xff00) >> 8;
      espNowSendBuffer[espNowSendBufferSize++] = packetCrc & 0xff;
      return true;
    }
    else
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
        if(waitForBufferSpace(31))
        {
          SERIAL_DEBUG_PORT.println(F("EspNow packet too large to send"));
        }
      #endif
    }
    return false;
  }
  bool validateEspNowChecksum()
  {
    CRC16 crc(LORA_CRC_POLYNOME);
    crc.add((uint8_t *)espNowReceiveBuffer, espNowReceiveBufferSize - 2);
    uint16_t expectedChecksum = crc.calc();
    uint16_t packetChecksum = (espNowReceiveBuffer[espNowReceiveBufferSize - 2] << 8) + espNowReceiveBuffer[espNowReceiveBufferSize - 1];
    if(expectedChecksum != packetChecksum)
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
        if(waitForBufferSpace(52))
        {
          SERIAL_DEBUG_PORT.printf_P(PSTR("EspNow packet checksum invalid, expected %04X got %04X\r\n"), expectedChecksum, packetChecksum);
        }
      #endif
      return false;
    }
    espNowReceiveBufferSize -= 2;
    return true;
  }
  void processEspNowPacket()
  {
    /*
    #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
      if(waitForBufferSpace(30))
      {
        SERIAL_DEBUG_PORT.println(F("Processing received packet..."));
      }
    #endif
    */
    MsgPack::Unpacker unpacker;
    unpacker.feed(espNowReceiveBuffer, espNowReceiveBufferSize);
    uint8_t _remoteMacAddress[6] = {0, 0, 0, 0, 0, 0};  //MAC address of the remote device
    if(unpacker.isUInt7() || unpacker.isUInt8())
    {
      unpacker.unpack(_remoteMacAddress[0]);
      if(unpacker.isUInt7() || unpacker.isUInt8())
      {
        unpacker.unpack(_remoteMacAddress[1]);
        if(unpacker.isUInt7() || unpacker.isUInt8())
        {
          unpacker.unpack(_remoteMacAddress[2]);
          if(unpacker.isUInt7() || unpacker.isUInt8())
          {
            unpacker.unpack(_remoteMacAddress[3]);
            if(unpacker.isUInt7() || unpacker.isUInt8())
            {
              unpacker.unpack(_remoteMacAddress[4]);
              if(unpacker.isUInt7() || unpacker.isUInt8())
              {
                unpacker.unpack(_remoteMacAddress[5]);
                if(unpacker.isUInt7() || unpacker.isUInt8())
                {
                  uint8_t messagetype;
                  unpacker.unpack(messagetype);
                  uint8_t deviceIndex = identifyDevice(_remoteMacAddress);
                  if(deviceIndex < maximumNumberOfDevices)
                  {
                    #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                      if(waitForBufferSpace(80))
                      {
                        SERIAL_DEBUG_PORT.printf_P(PSTR("EspNow RX %02x:%02x:%02x:%02x:%02x:%02x device %u "),
                          _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5],
                          deviceIndex);
                      }
                    #endif
                    if(messagetype == locationUpdateId)
                    {
                      device[deviceIndex].lastEspNowLocationUpdate = millis();  //Only location updates matter for deciding when something has disappeared
                      device[deviceIndex].espNowUpdateHistory = (device[deviceIndex].espNowUpdateHistory >> 1) | 0x8000;
                      unpacker.unpack(device[deviceIndex].latitude);
                      unpacker.unpack(device[deviceIndex].longitude);
                      unpacker.unpack(device[deviceIndex].course);
                      unpacker.unpack(device[deviceIndex].speed);
                      unpacker.unpack(device[deviceIndex].hdop);
                      unpacker.unpack(device[deviceIndex].nextEspNowLocationUpdate);
                      if(device[deviceIndex].hdop != 0 && device[deviceIndex].latitude != 0 && device[deviceIndex].longitude != 0 && device[deviceIndex].hdop < minimumViableHdop)
                      {
                        if(device[deviceIndex].hasGpsFix == false)
                        {
                          #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
                            if(waitForBufferSpace(40))
                            {
                              SERIAL_DEBUG_PORT.printf_P(PSTR("Device %u got GPS fix\r\n"), deviceIndex);
                            }
                          #endif
                          device[deviceIndex].hasGpsFix = true;
                          #ifdef LVGL_ADD_SCAN_INFO_TAB
                            findableDevicesChanged = true;
                          #endif
                        }
                      }
                      else if(device[deviceIndex].hasGpsFix == true)
                      {
                        #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
                          if(waitForBufferSpace(40))
                          {
                            SERIAL_DEBUG_PORT.printf_P(PSTR("Device %u lost GPS fix\r\n"), deviceIndex);
                          }
                        #endif
                        device[deviceIndex].hasGpsFix = false;
                        #ifdef LVGL_ADD_SCAN_INFO_TAB
                          findableDevicesChanged = true;
                        #endif
                      }
                      #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                        if(waitForBufferSpace(80))
                        {
                            SERIAL_DEBUG_PORT.printf("location Lat:%03.4f Lon:%03.4f Course:%03.1f(deg) Speed:%01.1f(m/s) HDOP:%.1f Distance:%.1f(m) next update:%us update history:%04x\r\n",
                            device[deviceIndex].latitude,
                            device[deviceIndex].longitude,
                            device[deviceIndex].course,
                            device[deviceIndex].speed,
                            device[deviceIndex].hdop,
                            device[deviceIndex].distanceTo,
                            device[deviceIndex].nextEspNowLocationUpdate/1000,
                            device[deviceIndex].espNowUpdateHistory);
                        }
                      #endif
                      if(device[deviceIndex].espNowOnline == false && countBits(device[deviceIndex].espNowUpdateHistory) > countBits(sensitivityValues[trackingSensitivity]))   //7 bits in in total to go online
                      {
                        localLog(F("Device "));
                        localLog(deviceIndex);
                        localLogLn(F(" EspNow gone online"));
                        device[deviceIndex].espNowOnline = true;
                        #ifdef LVGL_ADD_SCAN_INFO_TAB
                          findableDevicesChanged = true;
                        #endif
                      }
                      #ifdef LVGL_ADD_SCAN_INFO_TAB
                        findableDevicesChanged = true;
                      #endif
                    }
                    else if(messagetype == deviceStatusUpdateId)
                    {
                      if(unpacker.isUInt7() || unpacker.isUInt8())  //Type of device/features
                      {
                        unpacker.unpack(device[deviceIndex].typeOfDevice);
                        if(unpacker.isUInt7() || unpacker.isUInt8())  //Major version
                        {
                          unpacker.unpack(device[deviceIndex].majorVersion);
                          if(unpacker.isUInt7() || unpacker.isUInt8())  //Minor version
                          {
                            unpacker.unpack(device[deviceIndex].minorVersion);
                            if(unpacker.isUInt7() || unpacker.isUInt8())  //Patch version
                            {
                              unpacker.unpack(device[deviceIndex].patchVersion);
                              if(unpacker.isUInt16() || unpacker.isUInt32()) //Uptime
                              {
                                unpacker.unpack(device[deviceIndex].uptime);
                                if(unpacker.isFloat32() || unpacker.isFloat64())  //Supply voltage
                                {
                                  unpacker.unpack(device[deviceIndex].supplyVoltage);
                                  if(unpacker.isStr())  //Name
                                  {
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
                                    #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                      if(waitForBufferSpace(60))
                                      {
                                        SERIAL_DEBUG_PORT.printf_P(PSTR("info type:%02x version:%u.%u.%u name:'%s' up:%s battery voltage:%.1fv"),
                                          device[deviceIndex].typeOfDevice,
                                          device[deviceIndex].majorVersion,
                                          device[deviceIndex].minorVersion,
                                          device[deviceIndex].patchVersion,
                                          device[deviceIndex].name,
                                          printableUptime(device[deviceIndex].uptime/1000).c_str(),
                                          device[deviceIndex].supplyVoltage
                                          );
                                      }
                                    #endif
                                    if((device[deviceIndex].typeOfDevice & 0x02) == 0x02)  //It's acting as a sensor
                                    {
                                      uint8_t unpackerTemp = 0;
                                      if(unpacker.isUInt7() || unpacker.isUInt8())  //Starting hits
                                      {
                                        unpacker.unpack(unpackerTemp);
                                        if(device[deviceIndex].numberOfStartingHits != unpackerTemp)
                                        {
                                          device[deviceIndex].numberOfStartingHits = unpackerTemp;
                                          #if defined(ACT_AS_TRACKER)
                                            currentlyTrackedBeaconStateChanged = true;
                                          #endif
                                        }
                                        if(unpacker.isUInt7() || unpacker.isUInt8())  //Starting stun
                                        {
                                          unpacker.unpack(unpackerTemp);
                                          if(device[deviceIndex].numberOfStartingStunHits != unpackerTemp)
                                          {
                                            device[deviceIndex].numberOfStartingStunHits = unpackerTemp;
                                            #if defined(ACT_AS_TRACKER)
                                              currentlyTrackedBeaconStateChanged = true;
                                            #endif
                                          }
                                          if(unpacker.isUInt7() || unpacker.isUInt8())  //Current hits
                                          {
                                              unpacker.unpack(unpackerTemp);
                                              if(device[deviceIndex].currentNumberOfHits != unpackerTemp)
                                              {
                                                device[deviceIndex].currentNumberOfHits = unpackerTemp;
                                                #if defined(ACT_AS_TRACKER)
                                                  currentlyTrackedBeaconStateChanged = true;
                                                #endif
                                              }
                                              if(unpacker.isUInt7() || unpacker.isUInt8())  //Current hits
                                              {
                                                unpacker.unpack(unpackerTemp);
                                                if(device[deviceIndex].currentNumberOfStunHits != unpackerTemp)
                                                {
                                                  device[deviceIndex].currentNumberOfStunHits = unpackerTemp;
                                                  #if defined(ACT_AS_TRACKER)
                                                    currentlyTrackedBeaconStateChanged = true;
                                                  #endif
                                                }
                                                #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
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
                                                #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                                  if(waitForBufferSpace(40))
                                                  {
                                                    SERIAL_DEBUG_PORT.print(F("Unexpected type for current stun: "));
                                                    SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                                                  }
                                                #endif
                                              }
                                            }
                                            else
                                            {
                                              #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                                if(waitForBufferSpace(40))
                                                {
                                                  SERIAL_DEBUG_PORT.print(F("Unexpected type for current hits: "));
                                                  SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                                                }
                                              #endif
                                            }
                                          }
                                          else
                                          {
                                            #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                              if(waitForBufferSpace(40))
                                              {
                                                SERIAL_DEBUG_PORT.print(F("Unexpected type for starting stun: "));
                                                SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                                              }
                                            #endif
                                          }
                                        }
                                        else
                                        {
                                          #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                            if(waitForBufferSpace(40))
                                            {
                                              SERIAL_DEBUG_PORT.print(F("Unexpected type for starting hits: "));
                                              SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                                            }
                                          #endif
                                        }
                                    }
                                    else
                                    {
                                      uint32_t receivedDefaultEspNowLocationInterval;
                                      uint16_t receivedEspNowPerimiter1;
                                      uint32_t receivedEspNowLocationInterval1;
                                      uint16_t receivedEspNowPerimiter2;
                                      uint32_t receivedEspNowLocationInterval2;
                                      uint16_t receivedEspNowPerimiter3;
                                      uint32_t receivedEspNowLocationInterval3;
                                      unpacker.unpack(receivedDefaultEspNowLocationInterval);
                                      unpacker.unpack(receivedEspNowPerimiter1);
                                      unpacker.unpack(receivedEspNowLocationInterval1);
                                      unpacker.unpack(receivedEspNowPerimiter2);
                                      unpacker.unpack(receivedEspNowLocationInterval2);
                                      unpacker.unpack(receivedEspNowPerimiter3);
                                      unpacker.unpack(receivedEspNowLocationInterval3);
                                      #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                        if(waitForBufferSpace(75))
                                        {
                                          SERIAL_DEBUG_PORT.printf_P(PSTR(" intervals: default %us, %um %us, %um %us, %um %us"),
                                            receivedDefaultEspNowLocationInterval/1000,
                                            receivedEspNowPerimiter1,
                                            receivedEspNowLocationInterval1/1000,
                                            receivedEspNowPerimiter2,
                                            receivedEspNowLocationInterval2/1000,
                                            receivedEspNowPerimiter3,
                                            receivedEspNowLocationInterval3/1000
                                          );
                                        }
                                      #endif
                                      if(defaultEspNowLocationInterval != receivedDefaultEspNowLocationInterval ||
                                        espNowPerimiter1 != receivedEspNowPerimiter1 ||
                                        espNowLocationInterval1 != receivedEspNowLocationInterval1 ||
                                        espNowPerimiter2 != receivedEspNowPerimiter2 ||
                                        espNowLocationInterval2 != receivedEspNowLocationInterval2 ||
                                        espNowPerimiter3 != receivedEspNowPerimiter3 ||
                                        espNowLocationInterval3 != receivedEspNowLocationInterval3)
                                      {
                                        defaultEspNowLocationInterval = receivedDefaultEspNowLocationInterval;
                                        espNowPerimiter1 = receivedEspNowPerimiter1;
                                        espNowLocationInterval1 = receivedEspNowLocationInterval1;
                                        espNowPerimiter2 = receivedEspNowPerimiter2;
                                        espNowLocationInterval2 = receivedEspNowLocationInterval2;
                                        espNowPerimiter3 = receivedEspNowPerimiter3;
                                        espNowLocationInterval3 = receivedEspNowLocationInterval3;
                                        saveConfigurationSoon = millis();
                                        #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                          if(waitForBufferSpace(75))
                                          {
                                            SERIAL_DEBUG_PORT.print(F(" syncing\r\n"));
                                          }
                                        #endif
                                      }
                                      else
                                      {
                                        #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                          if(waitForBufferSpace(75))
                                          {
                                            SERIAL_DEBUG_PORT.print(F(" matches\r\n"));
                                          }
                                        #endif
                                      }
                                    }
                                  }
                                  else
                                  {
                                    #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                      if(waitForBufferSpace(40))
                                      {
                                        SERIAL_DEBUG_PORT.print(F("Unexpected type for name: "));
                                        SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                                      }
                                    #endif
                                  }
                                }
                                else
                                {
                                  #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                    if(waitForBufferSpace(40))
                                    {
                                      SERIAL_DEBUG_PORT.print(F("Unexpected type for supply voltage: "));
                                      SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                                    }
                                  #endif
                                }
                              }
                              else
                              {
                                #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                  if(waitForBufferSpace(40))
                                  {
                                    SERIAL_DEBUG_PORT.print(F("Unexpected type for uptime: "));
                                    SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                                  }
                                #endif
                              }
                            }
                            else
                            {
                              #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                                if(waitForBufferSpace(40))
                                {
                                  SERIAL_DEBUG_PORT.print(F("Unexpected type for patch version: "));
                                  SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                                }
                              #endif
                            }
                          }
                          else
                          {
                            #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                              if(waitForBufferSpace(40))
                              {
                                SERIAL_DEBUG_PORT.print(F("Unexpected type for minor version: "));
                                SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                              }
                            #endif
                          }
                        }
                        else
                        {
                          #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                            if(waitForBufferSpace(40))
                            {
                              SERIAL_DEBUG_PORT.print(F("Unexpected type for major version: "));
                              SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                            }
                          #endif
                        }
                      }
                      else
                      {
                        #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                          if(waitForBufferSpace(40))
                          {
                            SERIAL_DEBUG_PORT.print(F("Unexpected deviceType type: "));
                            SERIAL_DEBUG_PORT.println(messagetype);
                          }
                        #endif
                      }
                    }
                    else if(messagetype == deviceIcInfoId)
                    {
                      unpacker.unpack(device[deviceIndex].diameter);
                      unpacker.unpack(device[deviceIndex].height);
                      if(unpacker.isStr())  //Name
                      {
                        String receivedIcName = String(unpacker.unpackString());
                        bool storeReceivedIcName = false;
                        if(device[deviceIndex].icName == nullptr) //First time the name is received
                        {
                          storeReceivedIcName = true;
                        }
                        else if(receivedIcName.equals(String(device[deviceIndex].icName)) == false) //Check the name hasn't changed
                        {
                          delete[] device[deviceIndex].icName;
                          storeReceivedIcName = true;
                        }
                        if(storeReceivedIcName == true)
                        {
                          device[deviceIndex].icName = new char[receivedIcName.length() + 1];
                          receivedIcName.toCharArray(device[deviceIndex].icName, receivedIcName.length() + 1);
                          #ifdef LVGL_ADD_SCAN_INFO_TAB
                            findableDevicesChanged = true;
                          #endif
                        }
                        if(unpacker.isStr())  //Desc
                        {
                          String receivedIcDescription = String(unpacker.unpackString());
                          bool storeReceivedIcDescription = false;
                          if(device[deviceIndex].icDescription == nullptr) //First time the name is received
                          {
                            storeReceivedIcDescription = true;
                          }
                          else if(receivedIcDescription.equals(String(device[deviceIndex].icDescription)) == false) //Check the name hasn't changed
                          {
                            delete[] device[deviceIndex].icDescription;
                            storeReceivedIcDescription = true;
                          }
                          if(storeReceivedIcDescription == true)
                          {
                            device[deviceIndex].icDescription = new char[receivedIcDescription.length() + 1];
                            receivedIcDescription.toCharArray(device[deviceIndex].icDescription, receivedIcDescription.length() + 1);
                            #ifdef LVGL_ADD_SCAN_INFO_TAB
                              findableDevicesChanged = true;
                            #endif
                          }
                          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                            if(waitForBufferSpace(60))
                            {
                              SERIAL_DEBUG_PORT.printf_P(PSTR("IC info diameter:%u, height: %u, name:'%s', name:'%s'\r\n"),
                                device[deviceIndex].diameter,
                                device[deviceIndex].height,
                                device[deviceIndex].icName,
                                device[deviceIndex].icDescription
                                );
                            }
                          #endif
                        }
                        else
                        {
                          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                            if(waitForBufferSpace(30))
                            {
                              SERIAL_DEBUG_PORT.print(F("Unexpected type for IC description"));
                            }
                          #endif
                        }
                      }
                      else
                      {
                        #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                          if(waitForBufferSpace(30))
                          {
                            SERIAL_DEBUG_PORT.print(F("Unexpected type for IC name"));
                          }
                        #endif
                      }
                    }
                    else
                    {
                      #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                        if(waitForBufferSpace(40))
                        {
                          SERIAL_DEBUG_PORT.print(F("Unexpected message type: "));
                          SERIAL_DEBUG_PORT.println(messagetype);
                        }
                      #endif
                    }
                  }
                }
                else
                {
                  #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                    if(waitForBufferSpace(40))
                    {
                      SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
                      SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                    }
                  #endif
                }
              }
              else
              {
                #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                  if(waitForBufferSpace(40))
                  {
                    SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
                    SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                  }
                #endif
              }
            }
            else
            {
              #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
                if(waitForBufferSpace(40))
                {
                  SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
                  SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                }
              #endif
            }
          }
          else
          {
            #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
              if(waitForBufferSpace(40))
              {
                SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
                SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
              }
            #endif
          }
        }
        else
        {
          #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
            if(waitForBufferSpace(40))
            {
              SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
              SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
            }
          #endif
        }
      }
      else
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
          if(waitForBufferSpace(40))
          {
            SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
            SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
          }
        #endif
      }
    }
    else
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
        if(waitForBufferSpace(40))
        {
          SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
          SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
        }
      #endif
    }
    #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
      //if(waitForBufferSpace(40))
      //{
        //SERIAL_DEBUG_PORT.println(F("Processing received packet done"));
      //}
    #endif
  }
  bool sendEspNowBuffer()
  {
    if(appendEspNowChecksum()) //Add the checksum, if there is space
    {
      espNowPacketSent = millis();
      esp_err_t espNowSendResult = esp_now_send(broadcastMacAddress, espNowSendBuffer, espNowSendBufferSize);
      if(espNowSendResult == ESP_OK)
      {
        return true;
      }
      else
      {
        espNowTxPacketsDropped++;
        if(WiFi.channel() != espNowChannel) //Channel has changed, move peer address
        {
          if(deleteBroadcastPeer())
          {
            addBroadcastPeer();
          }
        }
      }
      espNowPacketSent = 0;
    }
    return false;
  }
  uint32_t newEspNowLocationSharingInterval(uint16_t distance, float speed)
  {
    uint32_t newInterval = 0;
    if(speed == 0)
    {
      if(distance < espNowPerimiter1)
      {
        newInterval = espNowLocationInterval1;
      }
      else if(distance < espNowPerimiter2)
      {
        newInterval = espNowLocationInterval2;
      }
      else if(distance < espNowPerimiter3)
      {
        newInterval = espNowLocationInterval3;
      }
      else
      {
        newInterval = defaultEspNowLocationInterval;
      }
      return newInterval;
    }
    newInterval = newEspNowLocationSharingInterval(distance, 0);  //Get the interval as if not moving
    int16_t worstCaseDistance = (int16_t)distance - (speed * (newInterval/1000)); //Estimate worst case new distance after this interval
    if(worstCaseDistance > 0)
    {
      newInterval = newEspNowLocationSharingInterval(worstCaseDistance, 0); //Assess new interval based on worst case distance
    }
    else
    {
      newInterval = espNowLocationInterval1;  //Return the shortest interval
    }
    return newInterval;
  }
  bool shareLocationByEspNow()
  {
    /*
    if(espNowTxBusy)
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
        if(waitForBufferSpace(80))
        {
          SERIAL_DEBUG_PORT.println(F("Cannot share location, EspNow busy"));
        }
      #endif
      return;
    }
    espNowTxBusy = true;
    */
    MsgPack::Packer packer;
    packer.pack(device[0].id[0]);
    packer.pack(device[0].id[1]);
    packer.pack(device[0].id[2]);
    packer.pack(device[0].id[3]);
    packer.pack(device[0].id[4]);
    packer.pack(device[0].id[5]);
    packer.pack(locationUpdateId);
    packer.pack(device[0].latitude);
    packer.pack(device[0].longitude);
    packer.pack(device[0].course);
    packer.pack(device[0].speed);
    packer.pack(device[0].hdop);
    packer.pack(device[0].nextEspNowLocationUpdate);
    if(packer.size() < maxEspNowBufferSize)
    {
      memcpy(espNowSendBuffer, packer.data(),packer.size());
      espNowSendBufferSize = packer.size();
      if(sendEspNowBuffer())
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
          if(waitForBufferSpace(80))
          {
            SERIAL_DEBUG_PORT.printf_P(PSTR("ESPNow TX %02x:%02x:%02x:%02x:%02x:%02x location Lat:%03.4f Lon:%03.4f Course:%03.1f Speed:%02.1f HDOP:%.1f next update:%us channel:%u\r\n"),
              device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5],
              device[0].latitude,
              device[0].longitude,
              device[0].course,
              device[0].speed,
              device[0].hdop,
              device[0].nextEspNowLocationUpdate/1000,
              espNowChannel
              );
          }
        #endif
        return true;
      }
    }
    else
    {
      espNowTxPacketsDropped++;
    }
    return false;
  }
  bool shareDeviceInfoByEspNow()
  {
    MsgPack::Packer packer;
    packer.pack(device[0].id[0]);
    packer.pack(device[0].id[1]);
    packer.pack(device[0].id[2]);
    packer.pack(device[0].id[3]);
    packer.pack(device[0].id[4]);
    packer.pack(device[0].id[5]);
    packer.pack(deviceStatusUpdateId);
    packer.pack(device[0].typeOfDevice);
    packer.pack(device[0].majorVersion);
    packer.pack(device[0].minorVersion);
    packer.pack(device[0].patchVersion);
    packer.pack(millis());
    packer.pack(device[0].supplyVoltage);
    packer.pack(device[0].name);
    #ifdef ACT_AS_SENSOR
      packer.pack(device[0].numberOfStartingHits);
      packer.pack(device[0].numberOfStartingStunHits);
      packer.pack(device[0].currentNumberOfHits);
      packer.pack(device[0].currentNumberOfStunHits);
    #else
      packer.pack(defaultEspNowLocationInterval);
      packer.pack(espNowPerimiter1);
      packer.pack(espNowLocationInterval1);
      packer.pack(espNowPerimiter2);
      packer.pack(espNowLocationInterval2);
      packer.pack(espNowPerimiter3);
      packer.pack(espNowLocationInterval3);
    #endif
    if(packer.size() < maxEspNowBufferSize)
    {
      memcpy(espNowSendBuffer, packer.data(),packer.size());
      espNowSendBufferSize = packer.size();
      if(sendEspNowBuffer())
      {  
        #if defined(SERIAL_DEBUG) && defined(DEBUG_ESPNOW)
          if(waitForBufferSpace(80))
          {
            #ifdef ACT_AS_SENSOR
            SERIAL_DEBUG_PORT.printf("ESPNow TX %02x:%02x:%02x:%02x:%02x:%02x device info type:%02X, version: %u.%u.%u name: '%s', uptime:%s, supply:%.1fv Hits:%u/%u Stun:%u/%u\r\n",device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5],
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
              SERIAL_DEBUG_PORT.printf("ESPNow TX %02x:%02x:%02x:%02x:%02x:%02x device info type:%02X, version: %u.%u.%u name: '%s', uptime:%s, supply:%.1fv intervals: default %us, %um %us, %um %us, %um %us  channel:%u\r\n",device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5],
              device[0].typeOfDevice,
              device[0].majorVersion,
              device[0].minorVersion,
              device[0].patchVersion,
              device[0].name,
              printableUptime(millis()/1000).c_str(),
              device[0].supplyVoltage,
              defaultEspNowLocationInterval/1000,
              espNowPerimiter1,
              espNowLocationInterval1/1000,
              espNowPerimiter2,
              espNowLocationInterval2/1000,
              espNowPerimiter3,
              espNowLocationInterval3/1000,
              espNowChannel
              );
            #endif
          }
        #endif
        return true;
      }
    }
    espNowTxPacketsDropped++;
    return false;
  }
  bool shareIcInfoByEspNow()
  {
    if(device[0].icName != nullptr && device[0].icDescription != nullptr)
    {
      MsgPack::Packer packer;
      packer.pack(device[0].id[0]);
      packer.pack(device[0].id[1]);
      packer.pack(device[0].id[2]);
      packer.pack(device[0].id[3]);
      packer.pack(device[0].id[4]);
      packer.pack(device[0].id[5]);
      packer.pack(deviceIcInfoId);
      packer.pack(device[0].diameter);
      packer.pack(device[0].height);
      packer.pack(device[0].icName);
      packer.pack(device[0].icDescription);
      if(packer.size() < maxEspNowBufferSize)
      {
        memcpy(espNowSendBuffer, packer.data(),packer.size());
        espNowSendBufferSize = packer.size();
        if(sendEspNowBuffer())
        {  
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
            if(waitForBufferSpace(80))
            {
              SERIAL_DEBUG_PORT.printf("ESPNow TX %02x:%02x:%02x:%02x:%02x:%02x IC info name: '%s', desc:'%s'\r\n",device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5],
                device[0].icName,
                device[0].icDescription
                );
            }
          #endif
          return true;
        }
      }
      espNowTxPacketsDropped++;
    }
    return false;
  }
#endif

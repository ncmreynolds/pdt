/*
 * 
 * This file contains functions related to the LoRa radio
 * 
 */
#ifdef SUPPORT_LORA
  #ifdef LORA_NON_BLOCKING
    /*
     * Interrupt service routines for LoRa, which need some care when there are other async things in use
     */
    void ICACHE_RAM_ATTR onSend()
    {
      //This block of code is to handle multiprocessor ESP32s
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          portENTER_CRITICAL(&loRaTxSynch);
        #elif CONFIG_IDF_TARGET_ESP32S3
          portENTER_CRITICAL(&loRaTxSynch);
        #endif
      #endif
      loRaTxPackets++;
      LoRa.receive();
      loRaTxBusy = false;
      //This block of code is to handle multiprocessor ESP32s
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          portEXIT_CRITICAL(&loRaTxSynch);
        #elif CONFIG_IDF_TARGET_ESP32S3
          portEXIT_CRITICAL(&loRaTxSynch);
        #endif
      #endif
    }
    #if defined(ESP32)
      void IRAM_ATTR onReceive(int packetSize)
    #elif defined(ESP8266)
      void ICACHE_RAM_ATTR onReceive(int packetSize)
    #endif
    {
      //This block of code is to handle multiprocessor ESP32s
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          portENTER_CRITICAL(&loRaRxSynch); //
        #elif CONFIG_IDF_TARGET_ESP32S3
          portENTER_CRITICAL(&loRaRxSynch);
        #endif
      #endif
      if(loRaRxBusy == true || loRaReceiveBufferSize > 0)  //Already dealing with a LoRa packet
      {
        //This block of code is to handle multiprocessor ESP32s
        #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
          #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
            portEXIT_CRITICAL(&loRaRxSynch);
          #elif CONFIG_IDF_TARGET_ESP32S3
            portEXIT_CRITICAL(&loRaRxSynch);
          #endif
        #endif
        if(loRaRxBusy == true)  //Already dealing with a LoRa packet
        {
          #if defined(SERIAL_DEBUG)
          if(waitForBufferSpace(40))
          {
            SERIAL_DEBUG_PORT.println(F("Packet received but busy, discarding"));
          }
          #endif
        }
        if(loRaReceiveBufferSize > 0)  //Already dealing with a LoRa packet
        {
          #if defined(SERIAL_DEBUG)
          if(waitForBufferSpace(50))
          {
            SERIAL_DEBUG_PORT.println(F("Packet received but buffer full, discarding"));
          }
          #endif
        }
        return;
      }
      loRaRxBusy = true;
      loRaRxPackets++;
      if(packetSize == 0)
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
          if(waitForBufferSpace(30))
          {
            SERIAL_DEBUG_PORT.println(F("Empty packet received"));
          }
        #endif
        return;
      }
      else if(packetSize <= MAX_LORA_BUFFER_SIZE)
      {
        /*
        #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(48))
        {
          SERIAL_DEBUG_PORT.print(F("Packet received, length: "));
          SERIAL_DEBUG_PORT.println(packetSize);
          SERIAL_DEBUG_PORT.print(F("Packet: "));
        }
        #endif
        */
        for(uint8_t index = 0; index < packetSize; index++)
        {
          loRaReceiveBuffer[index] = LoRa.read();
          /*
          #if defined(SERIAL_DEBUG)
          if(waitForBufferSpace(3))
          {
            SERIAL_DEBUG_PORT.printf_P("%02x ",loRaReceiveBuffer[index]);
          }
          #endif
          */
        }
        /*
        #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(1))
        {
          SERIAL_DEBUG_PORT.println();
        }
        #endif
        */
        lastRssi = LoRa.packetRssi();
        loRaReceiveBufferSize = packetSize;
      }
      else
      {
        #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(45))
        {
          SERIAL_DEBUG_PORT.print(F("Oversize packet received, length: "));
          SERIAL_DEBUG_PORT.println(packetSize);
        }
        #endif
      }
      loRaRxBusy = false;
      //This block of code is to handle multiprocessor ESP32s
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          portEXIT_CRITICAL(&loRaRxSynch);
        #elif CONFIG_IDF_TARGET_ESP32S3
          portEXIT_CRITICAL(&loRaRxSynch);
        #endif
      #endif
    }
  #endif
  void setupLoRa()
  {
    LoRa.setPins(loRaCSpin, loRaResetPin, loRaIrqPin);// set CS, reset, IRQ pin
    localLog(F("Configuring LoRa radio: "));
    if (LoRa.begin(868E6) == true)
    {
      localLogLn(F("OK"));
      loRaConnected = true;
      LoRa.enableCrc();
      LoRa.onReceive(onReceive);
      #if defined(LORA_NON_BLOCKING)
        LoRa.onTxDone(onSend);
      #endif
      LoRa.receive();
    }
    else
    {
      localLogLn(F("failed"));
      loRaConnected = false;
    }
  }
  void manageLoRa()
  {
    if(loRaReceiveBufferSize > 0) //There's something in the buffer to process
    {
      if(validateLoRaChecksum())
      {
        processLoRaPacket();
      }
      loRaReceiveBufferSize = 0;
    }
    #if defined(ACT_AS_TRACKER)
      if(device[currentBeacon].hasFix == true && millis() - device[currentBeacon].lastReceive > device[currentBeacon].timeout)
      {
        device[currentBeacon].hasFix = false;
        currentBeacon = maximumNumberOfDevices;
        localLogLn(F("Currently tracked beacon gone offline"));
        #ifdef SUPPORT_DISPLAY
          if(currentDisplayState == displayState::distance && millis() - lastDisplayUpdate > 1000) //Clear distance if showing
          {
            displayDistanceToBeacon();
          }
        #endif
      }
    #endif
    #ifdef SUPPORT_GPS
    if(millis() - lastLocationSendTime > currentLocationSendInterval)
    {
      if(updateLocation())
      {
        #if defined(ACT_AS_TRACKER)
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
          if(loRaConnected && numberOfBeacons() > 0)
          {
      #elif defined(ACT_AS_BEACON)
          calculateDistanceToTrackers();
          if(loRaConnected)
          {
      #endif
            shareLocation();
          }
        } //matches if(updateLocation())
        lastLocationSendTime = millis();
    }
    #endif
  }
  void shareLocation()
  {
    if(loRaTxBusy)
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
        if(waitForBufferSpace(80))
        {
          SERIAL_DEBUG_PORT.println(F("Cannot share location, LoRa busy"));
        }
      #endif
      return;
    }
    loRaTxBusy = true;
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
    if(packer.size() < MAX_LORA_BUFFER_SIZE)
    {
      memcpy(loRaSendBuffer, packer.data(),packer.size());
      loRaSendBufferSize = packer.size();
      if(transmitLoRaBuffer())
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
          if(waitForBufferSpace(80))
          {
            SERIAL_DEBUG_PORT.printf("TX %02x:%02x:%02x:%02x:%02x:%02x location Lat:%03.4f Lon:%03.4f Course:%03.1f Speed:%02.1f HDOP:%.1f\r\n",device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5], device[0].latitude, device[0].longitude, device[0].course, device[0].speed, device[0].hdop);
          }
        #endif
      }
    }
  }
  #if defined(SUPPORT_BATTERY_METER)
    void shareDeviceInfo()
    {
      if(loRaTxBusy)
      {
        return;
      }
      loRaTxBusy = true;
      MsgPack::Packer packer;
      packer.pack(device[0].id[0]);
      packer.pack(device[0].id[1]);
      packer.pack(device[0].id[2]);
      packer.pack(device[0].id[3]);
      packer.pack(device[0].id[4]);
      packer.pack(device[0].id[5]);
      packer.pack(deviceStatusUpdateId);
      packer.pack(device[0].typeOfDevice);
      packer.pack(millis());
      packer.pack(batteryVoltage);
      packer.pack(device[0].name);
      #ifdef ACT_AS_SENSOR
        packer.pack(numberOfStartingHits);
        packer.pack(numberOfStartingStunHits);
        packer.pack(currentNumberOfHits);
        packer.pack(currentNumberOfStunHits);
      #endif
      if(packer.size() < MAX_LORA_BUFFER_SIZE)
      {
        memcpy(loRaSendBuffer, packer.data(),packer.size());
        loRaSendBufferSize = packer.size();
        if(transmitLoRaBuffer())
        {  
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
            if(waitForBufferSpace(80))
            {
              #ifdef ACT_AS_SENSOR
              SERIAL_DEBUG_PORT.printf("TX %02x:%02x:%02x:%02x:%02x:%02x device info type:%02X, name: '%s', uptime:%s, supply:%.1fv Hits:%u/%u Stun:%u/%u\r\n",device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5],
                device[0].typeOfDevice,
                device[0].name,
                printableUptime(millis()/1000).c_str(),
                batteryVoltage,
                currentNumberOfHits,
                numberOfStartingHits,
                currentNumberOfStunHits,
                numberOfStartingStunHits
                );
              #else
                SERIAL_DEBUG_PORT.printf("TX %02x:%02x:%02x:%02x:%02x:%02x device info type:%02X, name: '%s', uptime:%s, supply:%.1fv\r\n",device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5],
                device[0].typeOfDevice,
                device[0].name,
                printableUptime(millis()/1000).c_str(),
                batteryVoltage
                );
              #endif
            }
          #endif
        }
      }
    }
  #endif
  bool transmitLoRaBuffer()
  {
    if(appendLoRaChecksum()) //Add the checksum, if there is space
    {  
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          portENTER_CRITICAL(&loRaTxSynch);
        #elif CONFIG_IDF_TARGET_ESP32S3
          portENTER_CRITICAL(&loRaTxSynch);
        #endif
      #endif
      LoRa.beginPacket(); //Start a new packet
      LoRa.write(loRaSendBuffer, loRaSendBufferSize);
      #if defined(LORA_NON_BLOCKING)
        txTimer = millis() ; //Time the send
        LoRa.endPacket(true); //Send the packet
      #else
        LoRa.endPacket(); //Send the packet
        LoRa.receive(); //Start receiving again
      #endif
      #if not defined(LORA_NON_BLOCKING)
        loRaTxBusy = false;
      #endif
      //This block of code is to handle multiprocessor ESP32s
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          portEXIT_CRITICAL(&loRaTxSynch);
        #elif CONFIG_IDF_TARGET_ESP32S3
          portEXIT_CRITICAL(&loRaTxSynch);
        #endif
      #endif
      return true;
    }
    return false;
  }
  bool appendLoRaChecksum()
  {
    if(loRaSendBufferSize < MAX_LORA_BUFFER_SIZE - 2)
    {
      CRC16 crc(LORA_CRC_POLYNOME);
      crc.add((uint8_t *)loRaSendBuffer, loRaSendBufferSize);
      uint16_t packetCrc = crc.calc();
      loRaSendBuffer[loRaSendBufferSize++] = (packetCrc & 0xff00) >> 8;
      loRaSendBuffer[loRaSendBufferSize++] = packetCrc & 0xff;
      return true;
    }
    else
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
        if(waitForBufferSpace(31))
        {
          SERIAL_DEBUG_PORT.println(F("LoRa packet too large to send"));
        }
      #endif
    }
    return false;
  }
  bool validateLoRaChecksum()
  {
    CRC16 crc(LORA_CRC_POLYNOME);
    crc.add((uint8_t *)loRaReceiveBuffer, loRaReceiveBufferSize - 2);
    uint16_t expectedChecksum = crc.calc();
    uint16_t packetChecksum = (loRaReceiveBuffer[loRaReceiveBufferSize - 2] << 8) + loRaReceiveBuffer[loRaReceiveBufferSize - 1];
    if(expectedChecksum != packetChecksum)
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
        if(waitForBufferSpace(52))
        {
          SERIAL_DEBUG_PORT.printf_P(PSTR("LoRa packet checksum invalid, expected %04X got %04X\r\n"), expectedChecksum, packetChecksum);
        }
      #endif
      return false;
    }
    loRaReceiveBufferSize -= 2;
    return true;
  }
  void processLoRaPacket()
  {
    /*
    #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
      if(waitForBufferSpace(30))
      {
        SERIAL_DEBUG_PORT.println(F("Processing received packet..."));
      }
    #endif
    */
    MsgPack::Unpacker unpacker;
    unpacker.feed(loRaReceiveBuffer, loRaReceiveBufferSize);
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
                    device[deviceIndex].lastRssi = lastRssi;
                    device[deviceIndex].lastReceive = millis();
                    if(messagetype == locationUpdateId)
                    {
                      unpacker.unpack(device[deviceIndex].latitude);
                      unpacker.unpack(device[deviceIndex].longitude);
                      unpacker.unpack(device[deviceIndex].course);
                      unpacker.unpack(device[deviceIndex].speed);
                      unpacker.unpack(device[deviceIndex].hdop);
                      if(device[deviceIndex].hdop < MINIMUM_VIABLE_HDOP)
                      {
                        if(device[deviceIndex].hasFix == false)
                        {
                          #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
                            if(waitForBufferSpace(40))
                            {
                              SERIAL_DEBUG_PORT.printf("Device %u got GPS fix\r\n", deviceIndex);
                            }
                          #endif
                          device[deviceIndex].hasFix = true;
                        }
                      }
                      else if(device[deviceIndex].hasFix == true)
                      {
                        #if defined(SERIAL_DEBUG) && defined(DEBUG_GPS)
                          if(waitForBufferSpace(40))
                          {
                            SERIAL_DEBUG_PORT.printf("Device %u lost GPS fix\r\n", deviceIndex);
                          }
                        #endif
                        device[deviceIndex].hasFix = false;
                      }
                      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                        if(waitForBufferSpace(80))
                        {
                            SERIAL_DEBUG_PORT.printf("RX %02x:%02x:%02x:%02x:%02x:%02x device %u location Lat:%03.4f Lon:%03.4f Course(deg):%03.1f Speed(m/s):%01.1f HDOP:%.1f Distance(m):%.1f RSSI:%.1f\r\n",
                            _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5],
                            deviceIndex, device[deviceIndex].latitude, device[deviceIndex].longitude, device[deviceIndex].course, device[deviceIndex].speed, device[deviceIndex].hdop, device[deviceIndex].distanceTo, lastRssi);
                        }
                      #endif
                    }
                    else if(messagetype == deviceStatusUpdateId)
                    {
                      if(unpacker.isUInt7() || unpacker.isUInt8())  //Type of device/features
                      {
                        unpacker.unpack(device[deviceIndex].typeOfDevice);
                        if(unpacker.isUInt16() || unpacker.isUInt32()) //Uptime
                        {
                          unpacker.unpack(device[deviceIndex].uptime);
                          if(unpacker.isFloat32() || unpacker.isFloat64())  //Supply voltage
                          {
                            unpacker.unpack(device[deviceIndex].supplyVoltage);
                            if(unpacker.isStr())  //Name
                            {
                              if(device[deviceIndex].name == nullptr) //First time the name is received
                              {
                                String temp = String(unpacker.unpackString());
                                device[deviceIndex].name = new char[temp.length() + 1];
                                temp.toCharArray(device[deviceIndex].name, temp.length() + 1);
                              }
                              if(device[deviceIndex].typeOfDevice & 2)  //It's acting as a sensor
                              {
                                unpacker.unpack(device[deviceIndex].numberOfStartingHits);
                                unpacker.unpack(device[deviceIndex].numberOfStartingStunHits);
                                unpacker.unpack(device[deviceIndex].currentNumberOfHits);
                                unpacker.unpack(device[deviceIndex].currentNumberOfStunHits);
                              }
                              #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                                if(waitForBufferSpace(60))
                                {
                                  SERIAL_DEBUG_PORT.printf_P(PSTR("RX %02x:%02x:%02x:%02x:%02x:%02x device %u info type:%02x name:'%s' up:%s battery voltage:%.1fv RSSI:%.1f"),
                                    _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5],
                                    deviceIndex,
                                    device[deviceIndex].typeOfDevice,
                                    device[deviceIndex].name,
                                    printableUptime(device[deviceIndex].uptime/1000).c_str(),
                                    device[deviceIndex].supplyVoltage,
                                    lastRssi);
                                }
                              #endif
                              if(device[deviceIndex].typeOfDevice & 2)  //It's acting as a sensor
                              {
                                #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                                  if(waitForBufferSpace(60))
                                  {
                                    SERIAL_DEBUG_PORT.printf_P(" hits %u/%u Stun %u/%u\r\n",
                                      device[deviceIndex].currentNumberOfHits,
                                      device[deviceIndex].numberOfStartingHits,
                                      device[deviceIndex].currentNumberOfStunHits,
                                      device[deviceIndex].numberOfStartingStunHits
                                      );
                                  }
                                #endif
                              }
                              else
                              {
                                #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                                  if(waitForBufferSpace(2))
                                  {
                                    SERIAL_DEBUG_PORT.println();
                                  }
                                #endif
                              }
                            }
                            else
                            {
                              #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                            #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                        #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                          if(waitForBufferSpace(40))
                          {
                            SERIAL_DEBUG_PORT.print(F("Unexpected deviceType type: "));
                            SERIAL_DEBUG_PORT.println(messagetype);
                          }
                        #endif
                      }
                    }
                    else
                    {
                      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                  #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
              #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
            #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
        if(waitForBufferSpace(40))
        {
          SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
          SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
        }
      #endif
    }
    #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
      //if(waitForBufferSpace(40))
      //{
        //SERIAL_DEBUG_PORT.println(F("Processing received packet done"));
      //}
    #endif
  }
  #ifdef SUPPORT_GPS
  uint8_t identifyDevice(uint8_t *macAddress)
  {
    uint8_t deviceIndex = 1;
    if(numberOfDevices == maximumNumberOfDevices)
    {
      #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(50))
        {
          SERIAL_DEBUG_PORT.printf("Too many devices to add %02x:%02x:%02x:%02x:%02x:%02x\r\n", _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5]);
        }
      #endif
      return maximumNumberOfDevices;
    }
    for(deviceIndex = 1; deviceIndex < numberOfDevices; deviceIndex++)
    {
      if(device[deviceIndex].id[0] == macAddress[0] &&
          device[deviceIndex].id[1] == macAddress[1] &&
          device[deviceIndex].id[2] == macAddress[2] &&
          device[deviceIndex].id[3] == macAddress[3] &&
          device[deviceIndex].id[4] == macAddress[4] &&
          device[deviceIndex].id[5] == macAddress[5])
      {
        return deviceIndex;
      }
    }
    #if defined(SERIAL_DEBUG)
      if(waitForBufferSpace(50))
      {
        SERIAL_DEBUG_PORT.printf("New device %u found %02x:%02x:%02x:%02x:%02x:%02x\r\n", numberOfDevices, _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5]);
      }
    #endif
    device[numberOfDevices].id[0] = _remoteMacAddress[0];
    device[numberOfDevices].id[1] = _remoteMacAddress[1];
    device[numberOfDevices].id[2] = _remoteMacAddress[2];
    device[numberOfDevices].id[3] = _remoteMacAddress[3];
    device[numberOfDevices].id[4] = _remoteMacAddress[4];
    device[numberOfDevices].id[5] = _remoteMacAddress[5];
    numberOfDevices++;
    lastDeviceStatus = (millis() - deviceStatusInterval) + random(5000,10000); //A new device prompts a status share in 5-10s
    lastLocationSendTime = (millis() -  currentLocationSendInterval) + random(10000,20000); //A new device prompts a location share in 10-20s
    return numberOfDevices-1;
  }
  #endif
#endif

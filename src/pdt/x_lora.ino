/*
 * 
 * This file contains functions related to the LoRa radio
 * 
 */
#if defined(SUPPORT_LORA)
  void calculateLoRaDutyCycle()
  {
    calculatedLoRaDutyCycle = ((float)loRaTxTime/(float)millis())*100;
    localLog(F("LoRa   TX(ms): "));
    localLog(loRaTxTime);
    localLog('/');
    localLog(millis());
    localLog(F(" TX duty cycle: "));
    localLog(calculatedLoRaDutyCycle);
    localLogLn('%');
  }
  #if defined(LORA_ASYNC_METHODS)
    #if defined(ESP32)
      void IRAM_ATTR copyLoRaPacketIntoBuffer(int packetSize)
    #elif defined(ESP8266)
      void ICACHE_RAM_ATTR copyLoRaPacketIntoBuffer(int packetSize)
    #endif
  #else
    void copyLoRaPacketIntoBuffer(int packetSize)
  #endif
  {
    //This block of code is to handle multiprocessor ESP32s
    #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
      #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
        #if defined(LORA_ASYNC_METHODS)
          portENTER_CRITICAL(&loRaRxSynch); //
        #endif
      #elif CONFIG_IDF_TARGET_ESP32S3
        #if defined(LORA_ASYNC_METHODS)
          portENTER_CRITICAL(&loRaRxSynch);
        #endif
      #endif
    #endif
    if(loRaRxBusy == true || loRaReceiveBufferSize > 0)  //Already dealing with a LoRa packet
    {
      //This block of code is to handle multiprocessor ESP32s
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          #if defined(LORA_ASYNC_METHODS)
            portEXIT_CRITICAL(&loRaRxSynch);
          #endif
        #elif CONFIG_IDF_TARGET_ESP32S3
          #if defined(LORA_ASYNC_METHODS)
            portEXIT_CRITICAL(&loRaRxSynch);
          #endif
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
      loRaRxPacketsDropped++;
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
    else if(packetSize <= maxLoRaBufferSize)
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
      lastLoRaRssi = LoRa.packetRssi();
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
        #if defined(LORA_ASYNC_METHODS)
          portEXIT_CRITICAL(&loRaRxSynch);
        #endif
      #elif CONFIG_IDF_TARGET_ESP32S3
        #if defined(LORA_ASYNC_METHODS)
          portEXIT_CRITICAL(&loRaRxSynch);
        #endif
      #endif
    #endif
  }
  #if defined(LORA_ASYNC_METHODS)
    /*
     * Interrupt service routines for LoRa, which need some care when there are other async things in use
     */
    #if defined(ESP32)
      void IRAM_ATTR onSend()
    #elif defined(ESP8266)
      void ICACHE_RAM_ATTR onSend()
    #endif
    {
      //This block of code is to handle multiprocessor ESP32s
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          #if defined(LORA_ASYNC_METHODS)
            portENTER_CRITICAL(&loRaTxSynch);
          #endif
        #elif CONFIG_IDF_TARGET_ESP32S3
          #if defined(LORA_ASYNC_METHODS)
            portENTER_CRITICAL(&loRaTxSynch);
          #endif
        #endif
      #endif
      loRaTxTime += millis() - loRaTxStartTime; //Calculate the time spent sending for duty cycle
      loRaTxPackets++;
      loRaTxComplete = true;
      //This block of code is to handle multiprocessor ESP32s
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          #if defined(LORA_ASYNC_METHODS)
            portEXIT_CRITICAL(&loRaTxSynch);
          #endif
        #elif CONFIG_IDF_TARGET_ESP32S3
          #if defined(LORA_ASYNC_METHODS)
            portEXIT_CRITICAL(&loRaTxSynch);
          #endif
        #endif
      #endif
    }
    #if defined(ESP32)
      void IRAM_ATTR onReceive(int packetSize)
    #elif defined(ESP8266)
      void ICACHE_RAM_ATTR onReceive(int packetSize)
    #endif
    {
      copyLoRaPacketIntoBuffer(packetSize); //This keeps the code the same across sync/async methods
    }
  #endif
  void setupLoRa()
  {
    localLog(F("Configuring LoRa radio: "));
    if(loRaEnabled == true)
    {
      #if defined(LVGL) && defined(SUPPORT_TOUCHSCREEN)
        LoRa.setSPI(touchscreenSPI);
      #endif
      if(loRaIrqPin != -1)
      {
        LoRa.setPins(loRaCsPin, loRaResetPin, loRaIrqPin);  //Set CS, Reset & IRQ pin
      }
      else
      {
        LoRa.setPins(loRaCsPin, loRaResetPin);  //Set CS & Reset only
      }
      if (LoRa.begin(868E6) == true)  //For EU, US is 915E6, Asia 433E6
      {
        LoRa.setTxPower(defaultLoRaTxPower);
        LoRa.setSpreadingFactor(defaultLoRaSpreadingFactor);
        LoRa.setSignalBandwidth(defaultLoRaSignalBandwidth);
        LoRa.setSyncWord(loRaSyncWord);
        localLogLn(F("OK"));
        loRaInitialised = true;
        LoRa.enableCrc();
        #if defined(LORA_ASYNC_METHODS)
          LoRa.onReceive(onReceive);
          LoRa.onTxDone(onSend);
        #endif
        LoRa.receive();
      }
      else
      {
        localLogLn(F("failed"));
      }
    }
    else
    {
      localLogLn(F("disabled"));
    }
  }
  void manageLoRa()
  {
    if(loRaEnabled == true && loRaInitialised == true)
    {
      #if defined(LORA_ASYNC_METHODS)
        if(loRaTxComplete == true && loRaTxBusy == true)
        {
          loRaTxComplete = false;
          loRaTxBusy = false;
          LoRa.receive();
        }
      #else
        int packetSize = LoRa.parsePacket();
        if(packetSize > 0)
        {
          copyLoRaPacketIntoBuffer(packetSize);
        }
      #endif
      if(loRaReceiveBufferSize > 0) //There's something in the buffer to process
      {
        if(validateLoRaChecksum())
        {
          processLoRaPacket();
        }
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
        else
        {
          SERIAL_DEBUG_PORT.println(F("Dropped LoRa packet: bad checksum"));
        }
        #endif
        loRaReceiveBufferSize = 0;
      }
      if(millis() - lastLoRaDeviceInfoSendTime > loRaDeviceInfoInterval)
      {
        lastLoRaDeviceInfoSendTime = millis() - random(100,200);
        if(typeOfLastLoRaUpdate == deviceIcInfoId)
        {
          shareDeviceInfoByLoRa();
          typeOfLastLoRaUpdate = deviceStatusUpdateId;
        }
        else if(typeOfLastLoRaUpdate == deviceStatusUpdateId)
        {
          shareIcInfoByLoRa();
          typeOfLastLoRaUpdate = deviceIcInfoId;
        }
        calculateLoRaDutyCycle();
      }
      if(millis() - lastLoRaLocationSendTime > device[0].nextLoRaLocationUpdate)
      {
        lastLoRaLocationSendTime = millis() - random(100,200);
        #if defined(ACT_AS_TRACKER)
          if(numberOfBeacons() > 0)  //Only share ocation if there are some beacons
          {
            shareLocationByLoRa();
            calculateLoRaDutyCycle();
            if(currentlyTrackedBeacon != maximumNumberOfDevices) //There is a current beacon
            {
              device[0].nextLoRaLocationUpdate = newLoRaLocationSharingInterval(distanceToCurrentBeacon, device[0].speed);  //Include speed in calculation
            }
          }
          else
          {
            device[0].nextLoRaLocationUpdate = newLoRaLocationSharingInterval(effectivelyUnreachable, 0);
          }
        #elif defined(ACT_AS_BEACON)
            if(numberOfTrackers() > 0)
            {
              shareLocationByLoRa();
              calculateLoRaDutyCycle();
              if(closestTracker != maximumNumberOfDevices) //There's a reasonable nearby tracker
              {
                device[0].nextLoRaLocationUpdate = newLoRaLocationSharingInterval(distanceToClosestTracker, device[0].speed);  //Include speed in calculation
              }
            }
            else
            {
              device[0].nextLoRaLocationUpdate = newLoRaLocationSharingInterval(effectivelyUnreachable, 0);
            }
        #endif
      }
      for(uint8_t index = 1; index < numberOfDevices; index++)  //Degrade quality if location updates missed, this INCLUDES this device!
      {
        if(device[index].hasGpsFix == true && //Thing has fix!
            device[index].loRaUpdateHistory > 0x00 && //Not already offline
            device[index].nextLoRaLocationUpdate != 0 &&  //Thing has shared when to expect the location update
            millis() - device[index].lastLoRaLocationUpdate > (device[index].nextLoRaLocationUpdate + (device[index].nextLoRaLocationUpdate>>3))) //Allow margin of 1/8 the expected interval
        {
          device[index].lastLoRaLocationUpdate = millis();  //A failed update is an 'update'
          device[index].loRaUpdateHistory = device[index].loRaUpdateHistory >> 1; //Reduce update history quality
          //device[index].nextLoRaLocationUpdate = device[index].nextLoRaLocationUpdate >> 1; //Halve the timeout
          #if defined(ACT_AS_TRACKER)
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
          if(device[index].loRaOnline == true && device[index].loRaUpdateHistory < sensitivityValues[trackingSensitivity])  //7 bits in the least significant section
          {
            localLogLn(F(" LoRa gone offline"));
            device[index].loRaOnline = false;
            #if defined(SUPPORT_LVGL) && defined(LVGL_ADD_SCAN_INFO_TAB)
              findableDevicesChanged = true;
            #endif
            #if defined(ACT_AS_TRACKER)
              if(index == currentlyTrackedBeacon) //Need to stop tracking this beacon
              {
                currentlyTrackedBeacon = maximumNumberOfDevices;
                distanceToCurrentBeacon = effectivelyUnreachable;
                distanceToCurrentBeaconChanged = true;
                #if defined(SUPPORT_BEEPER)
                  endRepeatingBeep();
                #endif
                #if defined(SUPPORT_DISPLAY)
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
            localLog(F(" dropped LoRa packet, signal quality now:0x"));
            localLogLn(String(device[index].loRaUpdateHistory,HEX));
          }
        }
      }
    }
  }
  void scheduleDeviceInfoShareSoon()
  {
    lastLoRaDeviceInfoSendTime = millis() - (loRaDeviceInfoInterval + 5000);  //Force the sensor to update any trackers soon, 5s is a good time to allow for more hits before sending
  }
  uint32_t newLoRaLocationSharingInterval(uint16_t distance, float speed)
  {
    uint32_t newInterval = 0;
    if(speed == 0)
    {
      if(distance < loRaPerimiter1)
      {
        newInterval = loRaLocationInterval1;
      }
      else if(distance < loRaPerimiter2)
      {
        newInterval = loRaLocationInterval2;
      }
      else if(distance < loRaPerimiter3)
      {
        newInterval = loRaLocationInterval3;
      }
      else
      {
        newInterval = defaultLoRaLocationInterval;
      }
      return newInterval;
    }
    newInterval = newLoRaLocationSharingInterval(distance, 0);  //Get the interval as if not moving
    int16_t worstCaseDistance = (int16_t)distance - (speed * (newInterval/1000)); //Estimate worst case new distance after this interval
    if(worstCaseDistance > 0)
    {
      newInterval = newLoRaLocationSharingInterval(worstCaseDistance, 0); //Assess new interval based on worst case distance
    }
    else
    {
      newInterval = loRaLocationInterval1;  //Return the shortest interval
    }
    return newInterval;
  }
  void shareLocationByLoRa()
  {
    if(loRaTxBusy)
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
        if(waitForBufferSpace(80))
        {
          SERIAL_DEBUG_PORT.println(F("Cannot share location, LoRa busy"));
        }
      #endif
      loRaTxPacketsDropped++;
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
    packer.pack(device[0].nextLoRaLocationUpdate);
    if(packer.size() < maxLoRaBufferSize)
    {
      memcpy(loRaSendBuffer, packer.data(),packer.size());
      loRaSendBufferSize = packer.size();
      if(transmitLoRaBuffer())
      {
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
          if(waitForBufferSpace(80))
          {
            SERIAL_DEBUG_PORT.printf_P(PSTR("LoRa   TX %02x:%02x:%02x:%02x:%02x:%02x location Lat:%03.4f Lon:%03.4f Course:%03.1f Speed:%02.1f HDOP:%.1f next update:%us\r\n"),
              device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5],
              device[0].latitude,
              device[0].longitude,
              device[0].course,
              device[0].speed,
              device[0].hdop,
              device[0].nextLoRaLocationUpdate/1000);
          }
        #endif
      }
    }
  }
  void shareDeviceInfoByLoRa()
  {
    if(loRaTxBusy)
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
        if(waitForBufferSpace(80))
        {
          SERIAL_DEBUG_PORT.println(F("Cannot share device info, LoRa busy"));
        }
      #endif
      loRaTxPacketsDropped++;
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
    #else
      packer.pack(defaultLoRaLocationInterval);
      packer.pack(loRaPerimiter1);
      packer.pack(loRaLocationInterval1);
      packer.pack(loRaPerimiter2);
      packer.pack(loRaLocationInterval2);
      packer.pack(loRaPerimiter3);
      packer.pack(loRaLocationInterval3);
    #endif
    if(packer.size() < maxLoRaBufferSize)
    {
      memcpy(loRaSendBuffer, packer.data(),packer.size());
      loRaSendBufferSize = packer.size();
      if(transmitLoRaBuffer())
      {  
        #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
          if(waitForBufferSpace(80))
          {
            #if defined(ACT_AS_SENSOR)
            SERIAL_DEBUG_PORT.printf("LoRa   TX %02x:%02x:%02x:%02x:%02x:%02x device info type:%02X, version: %u.%u.%u name: '%s', uptime:%s, supply:%.1fv Hits:%u/%u Stun:%u/%u\r\n",device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5],
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
              SERIAL_DEBUG_PORT.printf("LoRa   TX %02x:%02x:%02x:%02x:%02x:%02x device info type:%02X, version: %u.%u.%u name: '%s', uptime:%s, supply:%.1fv intervals: default %us, %um %us, %um %us, %um %us\r\n",device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5],
              device[0].typeOfDevice,
              device[0].majorVersion,
              device[0].minorVersion,
              device[0].patchVersion,
              device[0].name,
              printableUptime(millis()/1000).c_str(),
              device[0].supplyVoltage,
              defaultLoRaLocationInterval/1000,
              loRaPerimiter1,
              loRaLocationInterval1/1000,
              loRaPerimiter2,
              loRaLocationInterval2/1000,
              loRaPerimiter3,
              loRaLocationInterval3/1000
              );
            #endif
          }
        #endif
      }
    }
  }
  void shareIcInfoByLoRa()
  {
    if(loRaTxBusy)
    {
      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
        if(waitForBufferSpace(80))
        {
          SERIAL_DEBUG_PORT.println(F("Cannot share device IC info, LoRa busy"));
        }
      #endif
      loRaTxPacketsDropped++;
      return;
    }
    if(device[0].icName != nullptr && device[0].icDescription != nullptr)
    {
      loRaTxBusy = true;
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
      if(packer.size() < maxLoRaBufferSize)
      {
        memcpy(loRaSendBuffer, packer.data(),packer.size());
        loRaSendBufferSize = packer.size();
        if(transmitLoRaBuffer())
        {  
          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
            if(waitForBufferSpace(80))
            {
              SERIAL_DEBUG_PORT.printf("LoRa   TX %02x:%02x:%02x:%02x:%02x:%02x IC info name: '%s', desc:'%s'\r\n",device[0].id[0],device[0].id[1],device[0].id[2],device[0].id[3],device[0].id[4],device[0].id[5],
                device[0].icName,
                device[0].icDescription
                );
            }
          #endif
        }
      }
    }
  }
  bool transmitLoRaBuffer()
  {
    if(appendLoRaChecksum()) //Add the checksum, if there is space
    {
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          #if defined(LORA_ASYNC_METHODS)
            portENTER_CRITICAL(&loRaTxSynch);
          #endif
        #elif CONFIG_IDF_TARGET_ESP32S3
          #if defined(LORA_ASYNC_METHODS)
            portENTER_CRITICAL(&loRaTxSynch);
          #endif
        #endif
      #endif
      LoRa.beginPacket(); //Start a new packet
      LoRa.write(loRaSendBuffer, loRaSendBufferSize);
      loRaTxStartTime = millis() ; //Time the send
      #if defined(LORA_ASYNC_METHODS)
        LoRa.endPacket(true); //Send the packet asynchronously
      #else
        LoRa.endPacket(); //Send the packet and wait
        loRaTxTime += millis() - loRaTxStartTime; //Calculate the time spent sending for duty cycle
        loRaTxPackets++;
        loRaTxBusy = false;
        LoRa.receive(); //Start receiving again
      #endif
      //This block of code is to handle multiprocessor ESP32s
      #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
        #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
          #if defined(LORA_ASYNC_METHODS)
            portEXIT_CRITICAL(&loRaTxSynch);
          #endif
        #elif CONFIG_IDF_TARGET_ESP32S3
          #if defined(LORA_ASYNC_METHODS)
            portEXIT_CRITICAL(&loRaTxSynch);
          #endif
        #endif
      #endif
      return true;
    }
    return false;
  }
  bool appendLoRaChecksum()
  {
    if(loRaSendBufferSize < maxLoRaBufferSize - 2)
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
    uint8_t _remoteMacAddress[6] = {0, 0, 0, 0, 0, 0};  //MAC address of the remote device
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
                    #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                      if(waitForBufferSpace(80))
                      {
                        SERIAL_DEBUG_PORT.printf_P(PSTR("LoRa   RX %02x:%02x:%02x:%02x:%02x:%02x device %u "),
                          _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5],
                          deviceIndex);
                      }
                    #endif
                    #if defined(SUPPORT_LORA)
                      device[deviceIndex].lastLoRaRssi = lastLoRaRssi;
                    #endif
                    if(messagetype == locationUpdateId)
                    {
                      device[deviceIndex].lastLoRaLocationUpdate = millis();  //Only location updates matter for deciding when something has disappeared
                      device[deviceIndex].loRaUpdateHistory = (device[deviceIndex].loRaUpdateHistory >> 1) | 0x8000;
                      unpacker.unpack(device[deviceIndex].latitude);
                      unpacker.unpack(device[deviceIndex].longitude);
                      unpacker.unpack(device[deviceIndex].course);
                      unpacker.unpack(device[deviceIndex].speed);
                      unpacker.unpack(device[deviceIndex].hdop);
                      unpacker.unpack(device[deviceIndex].nextLoRaLocationUpdate);
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
                          #if defined(SUPPORT_LVGL) && defined(LVGL_ADD_SCAN_INFO_TAB)
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
                        #if defined(SUPPORT_LVGL) && defined(LVGL_ADD_SCAN_INFO_TAB)
                          findableDevicesChanged = true;
                        #endif
                      }
                      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                        if(waitForBufferSpace(80))
                        {
                            SERIAL_DEBUG_PORT.printf("location Lat:%03.4f Lon:%03.4f Course:%03.1f(deg) Speed:%01.1f(m/s) HDOP:%.1f Distance:%.1f(m) RSSI:%.1f next update:%us update history:%04x\r\n",
                            device[deviceIndex].latitude,
                            device[deviceIndex].longitude,
                            device[deviceIndex].course,
                            device[deviceIndex].speed,
                            device[deviceIndex].hdop,
                            device[deviceIndex].distanceTo,
                            device[deviceIndex].lastLoRaRssi,
                            device[deviceIndex].nextLoRaLocationUpdate/1000,
                            device[deviceIndex].loRaUpdateHistory);
                        }
                      #endif
                      if(device[deviceIndex].loRaOnline == false && countBits(device[deviceIndex].loRaUpdateHistory) > countBits(sensitivityValues[trackingSensitivity]))   //7 bits in in total to go online
                      {
                        localLog(F("Device "));
                        localLog(deviceIndex);
                        localLogLn(F(" LoRa gone online"));
                        device[deviceIndex].loRaOnline = true;
                        #if defined(SUPPORT_LVGL) && defined(LVGL_ADD_SCAN_INFO_TAB)
                          findableDevicesChanged = true;
                        #endif
                      }
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
                                    #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                                      if(waitForBufferSpace(60))
                                      {
                                        SERIAL_DEBUG_PORT.printf_P(PSTR("info type:%02x version:%u.%u.%u name:'%s' up:%s battery voltage:%.1fv RSSI:%.1f"),
                                          device[deviceIndex].typeOfDevice,
                                          device[deviceIndex].majorVersion,
                                          device[deviceIndex].minorVersion,
                                          device[deviceIndex].patchVersion,
                                          device[deviceIndex].name,
                                          printableUptime(device[deviceIndex].uptime/1000).c_str(),
                                          device[deviceIndex].supplyVoltage,
                                          device[deviceIndex].lastLoRaRssi);
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
                                                #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                                                #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                                              #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                                            #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                                          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                                      uint32_t receivedDefaultLoRaLocationInterval;
                                      uint16_t receivedLoRaPerimiter1;
                                      uint32_t receivedLoRaLocationInterval1;
                                      uint16_t receivedLoRaPerimiter2;
                                      uint32_t receivedLoRaLocationInterval2;
                                      uint16_t receivedLoRaPerimiter3;
                                      uint32_t receivedLoRaLocationInterval3;
                                      unpacker.unpack(receivedDefaultLoRaLocationInterval);
                                      unpacker.unpack(receivedLoRaPerimiter1);
                                      unpacker.unpack(receivedLoRaLocationInterval1);
                                      unpacker.unpack(receivedLoRaPerimiter2);
                                      unpacker.unpack(receivedLoRaLocationInterval2);
                                      unpacker.unpack(receivedLoRaPerimiter3);
                                      unpacker.unpack(receivedLoRaLocationInterval3);
                                      #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                                        if(waitForBufferSpace(75))
                                        {
                                          SERIAL_DEBUG_PORT.printf_P(PSTR(" intervals: default %us, %um %us, %um %us, %um %us"),
                                            receivedDefaultLoRaLocationInterval/1000,
                                            receivedLoRaPerimiter1,
                                            receivedLoRaLocationInterval1/1000,
                                            receivedLoRaPerimiter2,
                                            receivedLoRaLocationInterval2/1000,
                                            receivedLoRaPerimiter3,
                                            receivedLoRaLocationInterval3/1000
                                          );
                                        }
                                      #endif
                                      if(defaultLoRaLocationInterval != receivedDefaultLoRaLocationInterval ||
                                        loRaPerimiter1 != receivedLoRaPerimiter1 ||
                                        loRaLocationInterval1 != receivedLoRaLocationInterval1 ||
                                        loRaPerimiter2 != receivedLoRaPerimiter2 ||
                                        loRaLocationInterval2 != receivedLoRaLocationInterval2 ||
                                        loRaPerimiter3 != receivedLoRaPerimiter3 ||
                                        loRaLocationInterval3 != receivedLoRaLocationInterval3)
                                      {
                                        defaultLoRaLocationInterval = receivedDefaultLoRaLocationInterval;
                                        loRaPerimiter1 = receivedLoRaPerimiter1;
                                        loRaLocationInterval1 = receivedLoRaLocationInterval1;
                                        loRaPerimiter2 = receivedLoRaPerimiter2;
                                        loRaLocationInterval2 = receivedLoRaLocationInterval2;
                                        loRaPerimiter3 = receivedLoRaPerimiter3;
                                        loRaLocationInterval3 = receivedLoRaLocationInterval3;
                                        saveConfigurationSoon = millis();
                                        #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                                          if(waitForBufferSpace(75))
                                          {
                                            SERIAL_DEBUG_PORT.print(F(" syncing\r\n"));
                                          }
                                        #endif
                                      }
                                      else
                                      {
                                        #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
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
                                  SERIAL_DEBUG_PORT.print(F("Unexpected type for patch version: "));
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
                                SERIAL_DEBUG_PORT.print(F("Unexpected type for minor version: "));
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
                              SERIAL_DEBUG_PORT.print(F("Unexpected type for major version: "));
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
                          #if defined(SUPPORT_LVGL) && defined(LVGL_ADD_SCAN_INFO_TAB)
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
                            #if defined(SUPPORT_LVGL) && defined(LVGL_ADD_SCAN_INFO_TAB)
                              findableDevicesChanged = true;
                            #endif
                          }
                          #if defined(SERIAL_DEBUG) && defined(DEBUG_LORA)
                            if(waitForBufferSpace(60))
                            {
                              SERIAL_DEBUG_PORT.printf_P(PSTR("IC info name:'%s', name:'%s'\r\n"),
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
#endif

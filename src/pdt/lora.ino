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
      #ifdef SERIAL_DEBUG
        //SERIAL_DEBUG_PORT.print(F("TxDone: "));
        //SERIAL_DEBUG_PORT.print(millis() - txTimer);
        //SERIAL_DEBUG_PORT.println(F("ms"));
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
      if(loRaRxBusy == true || loRaBufferSize > 0)  //Already dealing with a LoRa packet
      {
        //This block of code is to handle multiprocessor ESP32s
        #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
          #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
            portEXIT_CRITICAL(&loRaRxSynch);
          #elif CONFIG_IDF_TARGET_ESP32S3
            portEXIT_CRITICAL(&loRaRxSynch);
          #endif
        #endif
        #ifdef SERIAL_DEBUG
          if(loRaRxBusy == true)  //Already dealing with a LoRa packet
          {
            SERIAL_DEBUG_PORT.println(F("Packet received but busy, discarding"));
          }
          if(loRaBufferSize > 0)  //Already dealing with a LoRa packet
          {
            SERIAL_DEBUG_PORT.println(F("Packet received but buffer full, discarding"));
          }
        #endif
        return;
      }
      loRaRxBusy = true;
      loRaRxPackets++;
      if(packetSize == 0)
      {
        #ifdef SERIAL_DEBUG
          SERIAL_DEBUG_PORT.println(F("Empty packet received"));
        #endif
        return;
      }
      else if(packetSize <= MAX_LORA_BUFFER_SIZE)
      {
        //SERIAL_DEBUG_PORT.print(F("Packet received, length: "));
        //SERIAL_DEBUG_PORT.println(packetSize);
        for(uint8_t index = 0; index < packetSize; index++)
        {
          loRaBuffer[index] = LoRa.read();
        }
        lastRssi = LoRa.packetRssi();
        loRaBufferSize = packetSize;
      }
      else
      {
        SERIAL_DEBUG_PORT.print(F("Oversize packet received, length: "));
        SERIAL_DEBUG_PORT.println(packetSize);
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
    localLog(F("Initialising LoRa radio: "));
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
  void shareLocation()
  {
    if(loRaTxBusy)
    {
      return;
    }
    loRaTxBusy = true;
    MsgPack::Packer packer;
    packer.pack(_localMacAddress[0]);
    packer.pack(_localMacAddress[1]);
    packer.pack(_localMacAddress[2]);
    packer.pack(_localMacAddress[3]);
    packer.pack(_localMacAddress[4]);
    packer.pack(_localMacAddress[5]);
    #if defined(ACT_AS_TRACKER)
      packer.pack(trackerLocationUpdateId);
      packer.pack(tracker[0].latitude);
      packer.pack(tracker[0].longitude);
      packer.pack(tracker[0].course);
      packer.pack(tracker[0].speed);
      packer.pack(tracker[0].hdop);
    #elif defined(ACT_AS_BEACON)
      packer.pack(beaconLocationUpdateId);
      packer.pack(beacon[0].latitude);
      packer.pack(beacon[0].longitude);
      packer.pack(beacon[0].course);
      packer.pack(beacon[0].speed);
      packer.pack(beacon[0].hdop);
    #endif
    LoRa.beginPacket(); //Start a new packet
    LoRa.write(packer.data(), packer.size());
    #if defined(LORA_NON_BLOCKING)
      txTimer = millis() ; //Time the send
      LoRa.endPacket(true); //Send the packet
    #else
      LoRa.endPacket(); //Send the packet
      LoRa.receive(); //Start receiving again
    #endif
    #ifdef SERIAL_DEBUG
      #if defined(ACT_AS_TRACKER)
        SERIAL_DEBUG_PORT.printf("TX %02x:%02x:%02x:%02x:%02x:%02x tracker info Lat:%03.4f Lon:%03.4f Course:%03.4f Speed:%03.4f HDOP:%.1f\r\n",_localMacAddress[0],_localMacAddress[1],_localMacAddress[2],_localMacAddress[3],_localMacAddress[4],_localMacAddress[5], tracker[0].latitude, tracker[0].longitude, tracker[0].course, tracker[0].speed, tracker[0].hdop);
      #elif defined(ACT_AS_BEACON)
        SERIAL_DEBUG_PORT.printf("TX %02x:%02x:%02x:%02x:%02x:%02x beacon info  Lat:%03.4f Lon:%03.4f Course:%03.4f Speed:%03.4f HDOP:%.1f\r\n",_localMacAddress[0],_localMacAddress[1],_localMacAddress[2],_localMacAddress[3],_localMacAddress[4],_localMacAddress[5], beacon[0].latitude, beacon[0].longitude, beacon[0].course, beacon[0].speed, beacon[0].hdop);
      #endif
    #endif
    #if not defined(LORA_NON_BLOCKING)
      loRaTxBusy = false;
    #endif
  }
  #if defined(SUPPORT_BATTERY_METER)
    void shareBatteryVoltage()
    {
      if(loRaTxBusy)
      {
        return;
      }
      loRaTxBusy = true;
      MsgPack::Packer packer;
      packer.pack(_localMacAddress[0]);
      packer.pack(_localMacAddress[1]);
      packer.pack(_localMacAddress[2]);
      packer.pack(_localMacAddress[3]);
      packer.pack(_localMacAddress[4]);
      packer.pack(_localMacAddress[5]);
      packer.pack(powerUpdateId);
      packer.pack(batteryVoltage);
      LoRa.beginPacket(); //Start a new packet
      LoRa.write(packer.data(), packer.size());
      #if defined(LORA_NON_BLOCKING)
        txTimer = millis() ; //Time the send
        LoRa.endPacket(true); //Send the packet
      #else
        LoRa.endPacket(); //Send the packet
        LoRa.receive(); //Start receiving again
      #endif
      #ifdef SERIAL_DEBUG
        SERIAL_DEBUG_PORT.printf("TX %02x:%02x:%02x:%02x:%02x:%02x battery info %.1fv\r\n",_localMacAddress[0],_localMacAddress[1],_localMacAddress[2],_localMacAddress[3],_localMacAddress[4],_localMacAddress[5], batteryVoltage);
      #endif
      #if not defined(LORA_NON_BLOCKING)
        loRaTxBusy = false;
      #endif
    }
  #endif
  void processLoRaPacket()
  {
    #ifdef SERIAL_DEBUG
      //SERIAL_DEBUG_PORT.println(F("Processing received packet..."));
    #endif
    MsgPack::Unpacker unpacker;
    unpacker.feed(loRaBuffer, loRaBufferSize);
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
                  if(messagetype == beaconLocationUpdateId)
                  {
                      uint8_t beaconIndex = identifyBeacon(_remoteMacAddress);
                      if(beaconIndex == numberOfBeacons)  //This is a not previously seen beacon
                      {
                        if(beaconIndex < maximumNumberOfBeacons)
                        {
                          #ifdef SERIAL_DEBUG
                            SERIAL_DEBUG_PORT.printf("New beacon %u found %02x:%02x:%02x:%02x:%02x:%02x\r\n", beaconIndex, _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5]);
                          #endif
                          beacon[beaconIndex].id[0] = _remoteMacAddress[0];
                          beacon[beaconIndex].id[1] = _remoteMacAddress[1];
                          beacon[beaconIndex].id[2] = _remoteMacAddress[2];
                          beacon[beaconIndex].id[3] = _remoteMacAddress[3];
                          beacon[beaconIndex].id[4] = _remoteMacAddress[4];
                          beacon[beaconIndex].id[5] = _remoteMacAddress[5];
                          numberOfBeacons++;
                        }
                        else
                        {
                          #ifdef SERIAL_DEBUG
                            SERIAL_DEBUG_PORT.printf("Too many beacons to add %02x:%02x:%02x:%02x:%02x:%02x\r\n", _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5]);
                          #endif
                        }
                      }
                      if(beaconIndex < maximumNumberOfBeacons)
                      {
                        unpacker.unpack(beacon[beaconIndex].latitude);
                        unpacker.unpack(beacon[beaconIndex].longitude);
                        unpacker.unpack(beacon[beaconIndex].course);
                        unpacker.unpack(beacon[beaconIndex].speed);
                        unpacker.unpack(beacon[beaconIndex].hdop);
                        beacon[beaconIndex].lastRssi = lastRssi;
                        beacon[beaconIndex].lastReceive = millis();
                        if(beacon[beaconIndex].hdop < 5)
                        {
                          if(beacon[beaconIndex].hasFix == false)
                          {
                            #ifdef SERIAL_DEBUG
                              SERIAL_DEBUG_PORT.printf("Beacon %u got GPS fix\r\n", beaconIndex);
                            #endif
                            beacon[beaconIndex].hasFix = true;
                          }
                        }
                        else if(beacon[beaconIndex].hasFix == true)
                        {
                          #ifdef SERIAL_DEBUG
                            SERIAL_DEBUG_PORT.printf("Beacon %u lost GPS fix\r\n", beaconIndex);
                          #endif
                          beacon[beaconIndex].hasFix = false;
                        }
                        beacon[beaconIndex].lastReceive = millis();
                        #if defined(ACT_AS_TRACKER)
                          if(beacon[beaconIndex].hasFix == true)
                          {
                            calculateDistanceToBeacon(beaconIndex);
                          }
                        #endif
                        #ifdef SERIAL_DEBUG
                          SERIAL_DEBUG_PORT.printf("RX %02x:%02x:%02x:%02x:%02x:%02x beacon %u info  Lat:%03.4f Lon:%03.4f Course:%03.4f Speed:%03.4f HDOP:%.1f Distance:%.1f RSSI:%.1f\r\n",
                            _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5],
                            beaconIndex, beacon[beaconIndex].latitude, beacon[beaconIndex].longitude, beacon[beaconIndex].course, beacon[beaconIndex].speed, beacon[beaconIndex].hdop, beacon[beaconIndex].distanceTo, lastRssi);
                        #endif
                      }
                  }
                  else if(messagetype == trackerLocationUpdateId)
                  {
                      uint8_t trackerIndex = identifyTracker(_remoteMacAddress);
                      if(trackerIndex == trackerIndex)  //This is a not previously seen beacon
                      {
                        if(trackerIndex < maximumNumberOfTrackers)
                        {
                          SERIAL_DEBUG_PORT.printf("New tracker %u %02x:%02x:%02x:%02x:%02x:%02x\r\n", trackerIndex, _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5]);
                          tracker[trackerIndex].id[0] = _remoteMacAddress[0];
                          tracker[trackerIndex].id[1] = _remoteMacAddress[1];
                          tracker[trackerIndex].id[2] = _remoteMacAddress[2];
                          tracker[trackerIndex].id[3] = _remoteMacAddress[3];
                          tracker[trackerIndex].id[4] = _remoteMacAddress[4];
                          tracker[trackerIndex].id[5] = _remoteMacAddress[5];
                          numberOfTrackers++;
                        }
                        else
                        {
                          SERIAL_DEBUG_PORT.printf("Too many trackers to add %02x:%02x:%02x:%02x:%02x:%02x\r\n", _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5]);
                        }
                      }
                      if(trackerIndex < maximumNumberOfTrackers)
                      {
                        unpacker.unpack(tracker[trackerIndex].latitude);
                        unpacker.unpack(tracker[trackerIndex].longitude);
                        unpacker.unpack(tracker[trackerIndex].course);
                        unpacker.unpack(tracker[trackerIndex].speed);
                        unpacker.unpack(tracker[trackerIndex].hdop);
                        tracker[trackerIndex].lastRssi = lastRssi;
                        tracker[trackerIndex].lastReceive = millis();
                        #ifdef SERIAL_DEBUG
                          SERIAL_DEBUG_PORT.printf("RX %02x:%02x:%02x:%02x:%02x:%02x tracker info Lat:%03.4f Lon:%03.4f Course:%03.4f Speed:%03.4f HDOP:%.1f Distance:%.1f RSSI:%.1f\r\n",
                            _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5],
                            tracker[trackerIndex].latitude, tracker[trackerIndex].longitude, tracker[trackerIndex].course, tracker[trackerIndex].speed, tracker[trackerIndex].hdop, tracker[trackerIndex].distanceTo, lastRssi);
                        #endif
                      }
                  }
                  else if(messagetype == powerUpdateId)
                  {
                    float remoteBatteryVoltage;
                    unpacker.unpack(remoteBatteryVoltage);
                    #ifdef SERIAL_DEBUG
                      SERIAL_DEBUG_PORT.printf("RX %02x:%02x:%02x:%02x:%02x:%02x battery info %.1fv RSSI:%.1f\r\n",
                        _remoteMacAddress[0], _remoteMacAddress[1], _remoteMacAddress[2], _remoteMacAddress[3], _remoteMacAddress[4], _remoteMacAddress[5],
                        remoteBatteryVoltage, lastRssi);
                    #endif
                  }
                  else
                  {
                    #ifdef SERIAL_DEBUG
                      SERIAL_DEBUG_PORT.print(F("Unexpected message type: "));
                      SERIAL_DEBUG_PORT.println(messagetype);
                    #endif
                  }
                }
                else
                {
                  #ifdef SERIAL_DEBUG
                    SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
                    SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                  #endif
                }
              }
              else
              {
                #ifdef SERIAL_DEBUG
                  SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
                  SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
                #endif
              }
            }
            else
            {
              #ifdef SERIAL_DEBUG
                SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
                SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
              #endif
            }
          }
          else
          {
            #ifdef SERIAL_DEBUG
              SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
              SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
            #endif
          }
        }
        else
        {
          #ifdef SERIAL_DEBUG
            SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
            SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
          #endif
        }
      }
      else
      {
        #ifdef SERIAL_DEBUG
          SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
          SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
        #endif
      }
    }
    else
    {
      #ifdef SERIAL_DEBUG
        SERIAL_DEBUG_PORT.print(F("Unexpected type: "));
        SERIAL_DEBUG_PORT.println(uint8_t(unpacker.getType()), HEX);
      #endif
    }
    #ifdef SERIAL_DEBUG
      //SERIAL_DEBUG_PORT.println(F("Processing received packet done"));
    #endif
  }
  uint8_t identifyBeacon(uint8_t *macAddress)
  {
    for(uint8_t index = 0; index < maximumNumberOfBeacons; index++)
    {
      if(beacon[index].id[0] == macAddress[0] &&
          beacon[index].id[1] == macAddress[1] &&
          beacon[index].id[2] == macAddress[2] &&
          beacon[index].id[3] == macAddress[3] &&
          beacon[index].id[4] == macAddress[4] &&
          beacon[index].id[5] == macAddress[5])
      {
        return index;
      }
      else if(beacon[index].id[0] == 0x00 &&
              beacon[index].id[1] == 0x00 &&
              beacon[index].id[2] == 0x00 &&
              beacon[index].id[3] == 0x00 &&
              beacon[index].id[4] == 0x00 &&
              beacon[index].id[5] == 0x00)
      {
        return index;
      }
    }
    return maximumNumberOfBeacons;
  }
  uint8_t identifyTracker(uint8_t *macAddress)
  {
    for(uint8_t index = 0; index < maximumNumberOfTrackers; index++)
    {
      if(tracker[index].id[0] == macAddress[0] &&
          tracker[index].id[1] == macAddress[1] &&
          tracker[index].id[2] == macAddress[2] &&
          tracker[index].id[3] == macAddress[3] &&
          tracker[index].id[4] == macAddress[4] &&
          tracker[index].id[5] == macAddress[5])
      {
        return index;
      }
      else if(tracker[index].id[0] == 0x00 &&
              tracker[index].id[1] == 0x00 &&
              tracker[index].id[2] == 0x00 &&
              tracker[index].id[3] == 0x00 &&
              tracker[index].id[4] == 0x00 &&
              tracker[index].id[5] == 0x00)
      {
        return index;
      }
    }
    return maximumNumberOfTrackers;
  }
#endif

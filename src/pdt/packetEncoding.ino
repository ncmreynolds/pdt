#if defined(SUPPORT_ESPNOW) || defined(SUPPORT_LORA)
  uint8_t identifyDevice(uint8_t *macAddress)
  {
    uint8_t deviceIndex = 1;
    if(numberOfDevices == maximumNumberOfDevices)
    {
      #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(50))
        {
          SERIAL_DEBUG_PORT.printf("Too many devices to add %02x:%02x:%02x:%02x:%02x:%02x\r\n", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
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
        SERIAL_DEBUG_PORT.printf("New device %u found %02x:%02x:%02x:%02x:%02x:%02x\r\n", numberOfDevices, macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
      }
    #endif
    device[numberOfDevices].id[0] = macAddress[0];
    device[numberOfDevices].id[1] = macAddress[1];
    device[numberOfDevices].id[2] = macAddress[2];
    device[numberOfDevices].id[3] = macAddress[3];
    device[numberOfDevices].id[4] = macAddress[4];
    device[numberOfDevices].id[5] = macAddress[5];
    numberOfDevices++;
    #if defined(SUPPORT_LORA)
      lastLoRaDeviceInfoSendTime = (millis() - loRaDeviceInfoInterval) + random(5000,10000); //A new device prompts a status share in 5-10s
      lastLoRaLocationSendTime = (millis() -  device[0].nextLoRaLocationUpdate) + random(10000,20000); //A new device prompts a location share in 10-20s
    #endif
    #if defined(SUPPORT_ESPNOW)
      lastEspNowDeviceInfoSendTime = (millis() - espNowDeviceInfoInterval) + random(1000,5000); //A new device prompts a status share in 1-5s
      lastEspNowLocationSendTime = (millis() -  device[0].nextEspNowLocationUpdate) + random(10000,20000); //A new device prompts a location share in 10-20s
    #endif
    return numberOfDevices-1;
  }
#endif

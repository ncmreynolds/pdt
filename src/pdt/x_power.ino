#ifdef SUPPORT_SOFT_POWER_OFF
  void setupSoftPowerOff()
  {
    pinMode(softPowerOffPin, OUTPUT);
    if(softPowerOffPinInverted == true)
    {
      digitalWrite(softPowerOffPin,LOW);
    }
    else
    {
      digitalWrite(softPowerOffPin,HIGH);
    }
  }
  void powerOff()
  {
    if(softPowerOffPinInverted == true)
    {
      digitalWrite(softPowerOffPin,HIGH);
    }
    else
    {
      digitalWrite(softPowerOffPin,LOW);
    }
  }
#endif
#ifdef SUPPORT_SOFT_PERIPHERAL_POWER_OFF
  void setupPeripheralPowerOff()
  {
    pinMode(peripheralPowerOffPin, OUTPUT);
  }
  void managePeripheralPower()
  {
    #ifdef SUPPORT_GPS
      if(peripheralsEnabled == true && moving == false && gpsStationaryTimeout != 0 && millis() - lastGPSstateChange > gpsStationaryTimeout)
      {
        peripheralPowerOff();
        lastGPSstateChange = millis();
      }
      if(peripheralsEnabled == false && gpsCheckInterval != 0 && millis() - lastGPSstateChange > gpsCheckInterval)
      {
        peripheralPowerOn();
        lastGPSstateChange = millis();
      }
    #endif
  }
  void peripheralPowerOn()
  {
    if(peripheralsEnabled == false)
    {
      if(peripheralPowerOffPinInverted == true)
      {
        digitalWrite(peripheralPowerOffPin,LOW);
      }
      else
      {
        digitalWrite(peripheralPowerOffPin,HIGH);
      }
      peripheralsEnabled = true;
      localLogLn(F("Peripheral power on"));
    }
  }
  void peripheralPowerOff()
  {
    if(peripheralsEnabled == true)
    {
      if(peripheralPowerOffPinInverted == true)
      {
        digitalWrite(peripheralPowerOffPin,HIGH);
      }
      else
      {
        digitalWrite(peripheralPowerOffPin,LOW);
      }
      peripheralsEnabled = false;
      localLogLn(F("Peripheral power off"));
    }
  }
#endif

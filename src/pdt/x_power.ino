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
    if(peripheralPowerOffPinInverted == true)
    {
      digitalWrite(peripheralPowerOffPin,LOW);
    }
    else
    {
      digitalWrite(peripheralPowerOffPin,HIGH);
    }
  }
#endif

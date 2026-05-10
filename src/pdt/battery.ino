#if defined(SUPPORT_BATTERY_METER)
  void setupBattery()
  {
    if(enableBatteryMonitor == true)
    {
      localLog(F("Configuring battery monitor: "));
      #if HARDWARE_VARIANT == C3PDT || HARDWARE_VARIANT == C3PDTasBeacon
        analogReadResolution(12);
        analogSetAttenuation(ADC_11db); //Set ADC to read up to 2500mV
      #elif HARDWARE_VARIANT == C3TrackedSensor || HARDWARE_VARIANT == C3TrackedSensorAsBeacon
        analogReadResolution(12);
        analogSetAttenuation(ADC_11db); //Set ADC to read up to 2500mV
      #elif HARDWARE_VARIANT == C3LoRaBeacon
        analogReadResolution(12);
        analogSetAttenuation(ADC_6db); //Set ADC to read up to 1300mV
      #endif
      localLogLn(F("OK"));
      //checkBatteryVoltage();  //Set initial voltage reading
    }
    else
    {
      device[0].supplyVoltage = 0;
    }
  }
  void manageBattery()
  {
    if(autoAdjustBatterySoon != 0 && millis() - autoAdjustBatterySoon > 1000)
    {
      setAutoBatteryConstant();
    }
    else if(millis() - lastBatteryStatus > batteryStatusInterval && enableBatteryMonitor == true)
    {
      lastBatteryStatus = millis();
      checkBatteryVoltage();
    }
  }
  void setAutoBatteryConstant()
  {
    localLogLn(F("Auto setting battery constant now"));
    autoAdjustBatterySoon = 0;  //Mark the battery constant set
    saveConfigurationSoon = millis(); //Save the new constant soon
    batteryConstant = chargingVoltage/(ADCpeakVoltage*float(analogRead(0))/4095.0); //Assume the battery is fully charged right now
    lastBatteryStatus = millis();
    checkBatteryVoltage(); 
  }
  void checkBatteryVoltage()
  {
    #if HARDWARE_VARIANT == C3PDT || HARDWARE_VARIANT == C3PDTasBeacon
      if(autoAdjustBatteryConstant == true)
      {
        device[0].supplyVoltage = (ADCpeakVoltage*float(analogRead(0))/4095.0)*batteryConstant/ADCpeakVoltage;
      }
      else
      {
        device[0].supplyVoltage = (ADCpeakVoltage*float(analogRead(0))/4095.0)*((topLadderResistor+bottomLadderResistor)/bottomLadderResistor)/ADCpeakVoltage;
      }
    #elif HARDWARE_VARIANT == C3TrackedSensor || HARDWARE_VARIANT == C3TrackedSensorAsBeacon
      if(autoAdjustBatteryConstant == true)
      {
        device[0].supplyVoltage = (ADCpeakVoltage*float(analogRead(0))/4095.0)*batteryConstant/ADCpeakVoltage;
      }
      else
      {
        device[0].supplyVoltage = (ADCpeakVoltage*float(analogRead(0))/4095.0)*((topLadderResistor+bottomLadderResistor)/bottomLadderResistor)/ADCpeakVoltage;
      }
    #elif HARDWARE_VARIANT == C3LoRaBeacon
      if(autoAdjustBatteryConstant == true)
      {
        device[0].supplyVoltage = (ADCpeakVoltage*float(analogRead(0))/4095.0)*batteryConstant;
      }
      else
      {
        device[0].supplyVoltage = (ADCpeakVoltage*float(analogRead(0))/4095.0)*((topLadderResistor+bottomLadderResistor)/bottomLadderResistor);
      }
    #endif
    if(autoAdjustBatteryConstant == true && device[0].supplyVoltage > chargingVoltage)
    {
      device[0].supplyVoltage = chargingVoltage;
      autoAdjustBatterySoon = millis();
    }
    else
    {
      batteryPercentage = estimateBatteryPercentage(device[0].supplyVoltage);
      localLog(F("Battery voltage: "));
      localLog(device[0].supplyVoltage);
      localLog('v');
      localLog(F(" capacity: "));
      localLog(batteryPercentage);
      localLogLn('%');
    }
  }
  /*
   * 
   * Paraphrasing code from https://github.com/G6EJD/LiPo_Battery_Capacity_Estimator
   * 
   */
  uint8_t estimateBatteryPercentage(float voltage)
  {
    uint8_t percentage = 2808.3808 * pow(voltage, 4) - 43560.9157 * pow(voltage, 3) + 252848.5888 * pow(voltage, 2) - 650767.4615 * voltage + 626532.5703;
    if(voltage > chargingVoltage)
    {
      percentage = 100;
    }
    else if (voltage <= 3.50)
    {
      percentage = 0;
    }
    return percentage;
  }
#endif

#ifdef SUPPORT_BATTERY_METER
  void setupBattery()
  {
    localLog(F("Configuring battery monitor: "));
    analogSetAttenuation(ADC_11db); //Set ADC to read up to 2600mV
    #if defined(ARDUINO_ESP32C3_DEV)
      //device[0].supplyVoltage = analogRead(voltageMonitorPin)*5.8/4095.0;
      device[0].supplyVoltage = (ADCpeakVoltage*float(analogRead(0))/4095.0)*((topLadderResistor+bottomLadderResistor)/bottomLadderResistor)/2.8;
    #else
      device[0].supplyVoltage = analogRead(voltageMonitorPin)*3.3/512.0;
    #endif
    batteryPercentage = estimateBatteryPercentage(device[0].supplyVoltage);
    //checkBatteryVoltage();  //Set initial voltage reading
    localLogLn(F("OK"));
  }
  void manageBattery()
  {
    if(millis() - lastDeviceStatus > deviceStatusInterval)
    {
      lastDeviceStatus = millis();
      checkBatteryVoltage();
    }
  }
  void checkBatteryVoltage()
  {
    #if defined(ARDUINO_ESP32C3_DEV)
      //device[0].supplyVoltage = analogRead(voltageMonitorPin)*5.8/4095.0;
      device[0].supplyVoltage = (ADCpeakVoltage*float(analogRead(0))/4095.0)*((topLadderResistor+bottomLadderResistor)/bottomLadderResistor)/2.8;
    #else
      device[0].supplyVoltage = analogRead(voltageMonitorPin)*3.3/512.0;
    #endif
    batteryPercentage = estimateBatteryPercentage(device[0].supplyVoltage);
    localLog(F("Battery voltage: "));
    localLog(device[0].supplyVoltage);
    localLog('v');
    localLog(F(" capacity: "));
    localLog(batteryPercentage);
    localLogLn('%');
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

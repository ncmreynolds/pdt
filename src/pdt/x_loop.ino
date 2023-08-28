/*
 *
 * This file contains the main loop, which runs endlessly 
 * 
 */
void loop()
{
  #if defined(SUPPORT_WIFI)
    manageNetwork();
  #endif
  #if defined(SUPPORT_LORA) && defined(SUPPORT_GPS)
    manageLoRa();
  #endif
  #ifdef SUPPORT_GPS
    manageGps();
  #endif
  #ifdef SUPPORT_BATTERY_METER
    manageBattery();
  #endif
  #ifdef SUPPORT_BUTTON
    checkButton();
  #endif
  if(bootTime == 0)
  {
    if(timeIsValid())
    {
      recordBootTime();
      localLogLn(F("Time synced"));
    }
  }
  if(saveConfigurationSoon != 0 && millis() - saveConfigurationSoon > 5000) //Save configuration after a delay to avoid AsyncWebserver doing it in a callback
  {
    saveConfigurationSoon = 0;
    saveConfiguration(configurationFile);
  }
  #if defined(ENABLE_REMOTE_RESTART)
    if(restartTimer !=0 && millis() - restartTimer > 7000)  //Restart the ESP based off a request in the Web UI
    {
      localLogLn(F("Restarting now"));
      flushLog();
      delay(3000);
      #ifdef ESP32
        ESP.restart();
      #else
        ESP.reset();
      #endif
    }
  #endif
  #if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
    if(debugPortAvailable == false && millis() - serialBufferCheckTime > 10000) //Check if the serial port is catching up/online
    {
      serialBufferCheckTime = millis();
      if(debugPortStartingBufferSize > 0 && SERIAL_DEBUG_PORT.availableForWrite() == debugPortStartingBufferSize)
      {
        debugPortAvailable = true;
      }
    }
  #endif
  #if defined(ACT_AS_SENSOR)
    manageLasertag();
  #endif
  #ifdef SUPPORT_LED
    //manageLed();
  #endif
}

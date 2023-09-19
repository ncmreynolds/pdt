/*
 *
 * This file contains the main loop, which runs endlessly 
 * 
 */
void loop()
{
  #ifdef SUPPORT_OTA
    if(otaEnabled == true)
    {
      ArduinoOTA.handle();  //Handle software updates
      if(otaInProgress == true)
      {
        return; //Pause the usual loop behaviour
      }
    }
  #endif
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
  #ifdef ACT_AS_TRACKER
    if(currentBeaconStateChanged == true)  //Ideally show the status to the tracker, but not if asleep
    {
      currentBeaconStateChanged = false;
      //if(currentDisplayState == displayState::distance|| currentDisplayState == displayState::status)
      {
        displayStatus();
      }
    }
  #endif
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
      #if defined(ACT_AS_SENSOR)
        sensorPersitentData.end(); 
      #endif
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
    if(saveSensorConfigurationSoon != 0 && millis() - saveSensorConfigurationSoon > 5000) //Save configuration after a delay to avoid AsyncWebserver doing it in a callback 
    {
      saveSensorConfigurationSoon = 0;
      saveSensorConfiguration();
    }
  #endif
  vTaskDelay(10 / portTICK_PERIOD_MS);
}

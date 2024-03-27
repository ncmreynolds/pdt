/*
 *
 * This file contains the main loop, which runs endlessly 
 * 
 */
void loop()
{
  #if defined(SUPPORT_OTA)
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
  #if defined(SUPPORT_HACKING)
    manageGame();
  #endif
  #if defined(SUPPORT_ESPNOW)
    manageEspNow();
  #endif
  #if defined(SUPPORT_LORA) && defined(SUPPORT_GPS)
    manageLoRa();
  #endif
  #if defined(SUPPORT_TREACLE)
    manageTreacle();
  #endif
  #if defined(SUPPORT_GPS)
    manageGps();
  #endif
  #if defined(SUPPORT_BATTERY_METER)
    manageBattery();
  #endif
  #if defined(SUPPORT_BUTTON)
    checkButton();
  #endif
  #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
    managePeripheralPower();
  #endif
  if(bootTime == 0)
  {
    if(timeIsValid())
    {
      recordBootTime();
      localLogLn(F("Time synced"));
    }
  }
  #if defined(SUPPORT_DISPLAY)
    manageDisplay();
  #endif
  #if defined(SUPPORT_LVGL)
    manageLVGL();
    #if defined(LVGL_MANAGE_BACKLIGHT)
      manageBacklight();
    #endif
  #endif
  if(saveConfigurationSoon != 0 && millis() - saveConfigurationSoon > 5000) //Save configuration after a delay to avoid AsyncWebserver doing it in a callback
  {
    saveConfigurationSoon = 0;
    saveConfiguration(configurationFile);
    #ifndef SERVE_CONFIG_FILE
      printConfiguration();
    #endif
  }
  if(wipeTimer !=0 && millis() - wipeTimer > 1000)
  {
    wipeTimer = 0;
    localLogLn(F("Wiping now"));
    flushLog();
    pdtDeleteFile(configurationFile);
    restartTimer = millis();
  }
  #if defined(ENABLE_REMOTE_RESTART)
    if(restartTimer !=0 && millis() - restartTimer > 9000)  //Restart the ESP based off a request in the Web UI
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
  #if defined(SUPPORT_SOFT_POWER_OFF)
    if(powerOffTimer !=0 && millis() - powerOffTimer > 3E3)  //Power off
    {
      localLogLn(F("Powering off now"));
      flushLog();
      #if defined(ACT_AS_SENSOR)
        sensorPersitentData.end(); 
      #endif
      powerOff();
      powerOffTimer = 0;
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
  vTaskDelay(10 / portTICK_PERIOD_MS);
}

#if defined(SUPPORT_BEEPER)
  void setupBeeper()
  {
    #if HARDWARE_VARIANT == CYDTracker
      pinMode(beeperPin, OUTPUT);
      digitalWrite(beeperPin, LOW);
    #else
      pinMode(beeperPin, INPUT);
      digitalWrite(beeperPin, HIGH);
    #endif
    localLog(F("Configuring beeper: "));
    //Use a semaphore to control access to global variables
    beeperSemaphore = xSemaphoreCreateBinary();
    stopBeep();
    xSemaphoreGive(beeperSemaphore);
    xTaskCreate(manageBeeper, "manageBeeper", 1024, NULL, configMAX_PRIORITIES - 2 , &beeperManagementTask); //configMAX_PRIORITIES - 2
    localLogLn(F("OK"));
  }
  void makeAsingleBeep(uint16_t frequency, uint16_t duration) //A single beep can override a repeating one
  {
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout) == pdTRUE)
    {
      if(beeperState == false)
      {
        #if ESP_IDF_VERSION_MAJOR > 3
          ledcAttach(beeperPin, frequency, 8);
          ledcWriteTone(beeperPin, frequency);
        #else
          ledcAttachPin(beeperPin, beeperChannel);
          ledcWriteTone(beeperChannel, frequency);
        #endif
        singleBeepOnTime = duration; //0 means continuous
        singleBeepLastStateChange = millis();
        beeperState = true;
      }
      xSemaphoreGive(beeperSemaphore);
    }
  }
  void makeArepeatingBeep(uint16_t frequency, uint32_t ontime, uint32_t offtime = 0) //0 ontime means continuous, 0 offtime means don't repeat
  {
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout) == pdTRUE)
    {
      if(beeperState == false)
      {
        #if ESP_IDF_VERSION_MAJOR > 3
          ledcAttach(beeperPin, frequency, 8);
          ledcWriteTone(beeperPin, frequency);
        #else
          ledcAttachPin(beeperPin, beeperChannel);
          ledcWriteTone(beeperChannel, frequency);
        #endif
        beeperTone = frequency;
        repeatingBeepOnTime = ontime;
        repeatingBeepOffTime = offtime;
        repeatingBeepLastStateChange = millis();
      }
      beeperState = true;
      xSemaphoreGive(beeperSemaphore);
    }
  }
  void stopSingleBeep()
  {
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout) == pdTRUE)
    {
      if(beeperState == true)
      {
        stopBeep();
        singleBeepLastStateChange = millis(); //Update the state change time
        singleBeepOnTime = 0; //Mark the single beep as done
      }
      beeperState = false;
      xSemaphoreGive(beeperSemaphore);
    }
  }
  void pauseRepeatingBeep()
  {
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout) == pdTRUE)
    {
      if(beeperState == true)
      {
        stopBeep();
        repeatingBeepLastStateChange = millis();
        beeperState = false;
      }
      xSemaphoreGive(beeperSemaphore);
    }
  }
  void endRepeatingBeep()
  {
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout) == pdTRUE)
    {
      repeatingBeepOffTime = 0;
      if(beeperState == true)
      {
        stopBeep();
        repeatingBeepLastStateChange = millis();
        beeperState = false;
      }
      xSemaphoreGive(beeperSemaphore);
    }
  }
  void enableBeeper()
  {
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout) == pdTRUE)
    {
      beeperEnabled = true;
      xSemaphoreGive(beeperSemaphore);
    }
  }
  void disableBeeper()
  {
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout) == pdTRUE)
    {
      beeperEnabled = false;
      xSemaphoreGive(beeperSemaphore);
    }
  }
  void manageBeeper(void * parameter)
  {
    #if defined(ENABLE_OTA_UPDATE)
    while(otaInProgress == false)
    #else
    while(true)
    #endif
    {
      if(beeperState == true)
      {
        if(singleBeepOnTime > 0)  //Single beep is playing out
        {
          if(millis() - singleBeepLastStateChange > singleBeepOnTime) //There's an expiring single beep
          {
            stopSingleBeep(); //Turn off the beeper
          }
        }
        else if(repeatingBeepOnTime > 0)  //Repeating beep is playing out
        {
          if(millis() - repeatingBeepLastStateChange > repeatingBeepOnTime)  //There's an expiring repeating beep
          {
            pauseRepeatingBeep(); //Turn off the beeper until next time
          }
        }
      }
      else
      {
        if(repeatingBeepOffTime != 0 && millis() - repeatingBeepLastStateChange > repeatingBeepOffTime)  //A repeating beep needs to happen again
        {
          makeArepeatingBeep(beeperTone, repeatingBeepOnTime, repeatingBeepOffTime); //Do the next repeating beep
        }
      }
      vTaskDelay(beeperYieldTime / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);  //Kill this task
  }
  void stopBeep()
  {
    #if ESP_IDF_VERSION_MAJOR > 3
      ledcWriteTone(beeperPin, 0);  //Write no tone
      ledcDetach(beeperPin);
      #if HARDWARE_VARIANT == CYDTracker
        digitalWrite(beeperPin, LOW);
      #else
        digitalWrite(beeperPin, HIGH);
      #endif
    #else
      ledcSetup(beeperChannel, beeperButtonTone, 8);  //Set up eight bit resolution
      ledcAttachPin(beeperPin, beeperChannel);
      ledcWriteTone(beeperChannel, 0);  //Set an initial tone of nothing
      ledcDetachPin(beeperPin);
    #endif
  }
  #if defined(ACT_AS_TRACKER)
    void setBeeperUrgency()
    {
      if(currentlyTrackedDevice != maximumNumberOfDevices && device[currentlyTrackedDevice].distanceTo < maximumEffectiveRange)
      {
        if(device[currentlyTrackedDevice].distanceTo < 5) 
        {
          #if defined(SUPPORT_LED)
            ledOffTime = 100;
          #endif
          repeatingBeepOffTime = repeatingBeepOnTime + 50;  //Fastest beep offtime
        }
        else
        {
          #if defined(SUPPORT_LED)
            ledOffTime = device[currentlyTrackedDevice].distanceTo * 100;
          #endif
          repeatingBeepOffTime = repeatingBeepOnTime + 50 + pow(device[currentlyTrackedDevice].distanceTo,2); //Change urgency, ie. off time between beeps
        }
      }
      else
      {
        #if defined(SUPPORT_LED)
          ledOffTime = 100;
        #endif
        repeatingBeepOffTime = 0;  //Stop beeping after this one finishes
      }
      #if defined(SERIAL_DEBUG) && defined(DEBUG_BEEPER)
        if(repeatingBeepOffTime > 0)
        {
          SERIAL_DEBUG_PORT.printf_P(PSTR("Beeper off time: %ums\r\n"),repeatingBeepOffTime);
        }
        else
        {
          //SERIAL_DEBUG_PORT.println(F("Beeper off"));
        }
      #endif
    }
  #endif
#endif

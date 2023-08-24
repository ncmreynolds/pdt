#ifdef SUPPORT_BEEPER
  void setupBeeper()
  {
    localLog(F("Configuring beeper: "));
    pinMode(beeperPin, OUTPUT);
    //Use a semaphore to control access to global variables
    #ifdef USE_RTOS
      beeperSemaphore = xSemaphoreCreateBinary();
    #endif
    //pinMode(beeperPin, OUTPUT);
    pinMode(beeperPin, INPUT);
    digitalWrite(beeperPin, HIGH);
    //digitalWrite(beeperPin, HIGH);
    ledcSetup(beeperChannel, beeperButtonTone, 8);  //Set up eight bit resolution
    ledcWriteTone(beeperChannel, 0);  //Set an initial tone of nothing
    ledcAttachPin(beeperPin, beeperChannel);
    #ifdef USE_RTOS
      xSemaphoreGive(beeperSemaphore);
      xTaskCreate(manageBeeper, "manageBeeper", 1000, NULL, configMAX_PRIORITIES - 2 , &beeperManagementTask); //configMAX_PRIORITIES - 2
    #endif
    localLogLn(F("OK"));
  }
  void makeAsingleBeep(uint16_t frequency, uint16_t duration) //A single beep can override a repeating one
  {
    #ifdef USE_RTOS
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout))
    {
    #endif
      if(beeperState == false)
      {
        //ledcAttachPin(beeperPin, beeperChannel);
      }
      ledcWriteTone(beeperChannel, frequency);
      singleBeepOnTime = duration; //0 means continuous
      singleBeepLastStateChange = millis();
      beeperState = true;
    #ifdef USE_RTOS
      xSemaphoreGive(beeperSemaphore);
    }
    #endif
  }
  void makeArepeatingBeep(uint16_t frequency, uint32_t ontime, uint32_t offtime = 0) //0 ontime means continuous, 0 offtime means don't repeat
  {
    if(beeperState == false)
    {
      #ifdef USE_RTOS
      if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout))
      {
      #endif
        ledcAttachPin(beeperPin, beeperChannel);
        ledcWriteTone(beeperChannel, frequency);
        beeperTone = frequency;
        repeatingBeepOnTime = ontime;
        repeatingBeepOffTime = offtime;
        repeatingBeepLastStateChange = millis();
        beeperState = true;
      #ifdef USE_RTOS
        xSemaphoreGive(beeperSemaphore);
      }
      #endif
    }
  }
  void stopSingleBeep()
  {
    #ifdef USE_RTOS
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout))
    {
    #endif
      ledcWriteTone(beeperChannel, 0);  //Write no tone
      //ledcDetachPin(beeperPin); //Detach because beeper is active low
      //digitalWrite(beeperPin, HIGH);  //Write high because beeper is active low
      beeperState = false;
      singleBeepLastStateChange = millis(); //Update the state change time
      singleBeepOnTime = 0; //Mark the single beep as done
    #ifdef USE_RTOS
      xSemaphoreGive(beeperSemaphore);
    }
    #endif
  }
  void stopRepeatingBeep()
  {
    #ifdef USE_RTOS
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout))
    {
    #endif
      ledcWriteTone(beeperChannel, 0);  //Write no tone
      //ledcDetachPin(beeperPin); //Detach because beeper is active low
      //digitalWrite(beeperPin, HIGH);  //Write high because beeper is active low
      beeperState = false;
      repeatingBeepLastStateChange = millis();
    #ifdef USE_RTOS
      xSemaphoreGive(beeperSemaphore);
    }
    #endif
  }
  void manageBeeper(void * parameter)
  {
    while(true)
    {
      if(beeperEnabled == true)
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
              stopRepeatingBeep(); //Turn off the beeper until next time
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
      }
      vTaskDelay(beeperYieldTime / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);  //Kill this task
  }
  #if defined(ACT_AS_TRACKER)
    void setBeeperUrgency()
    {
      if(currentBeacon != maximumNumberOfDevices && distanceToCurrentBeacon < maximumEffectiveRange)
      {
        if(distanceToCurrentBeacon < 5) 
        {
          #ifdef SUPPORT_LED
            ledOffTime = 100;
          #endif
          repeatingBeepOffTime = repeatingBeepOnTime + 50;  //Fastest beep offtime
        }
        else
        {
          #ifdef SUPPORT_LED
            ledOffTime = distanceToCurrentBeacon * 100;
          #endif
          repeatingBeepOffTime = repeatingBeepOnTime + 50 + pow(distanceToCurrentBeacon,2); //Change urgency, ie. off time between beeps
        }
      }
      else
      {
        #ifdef SUPPORT_LED
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
          SERIAL_DEBUG_PORT.println(F("Beeper off"));
        }
      #endif
    }
  #endif
#endif

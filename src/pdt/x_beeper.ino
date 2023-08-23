#ifdef SUPPORT_BEEPER
  void setupBeeper()
  {
    localLog(F("Configuring beeper: "));
    pinMode(beeperPin, OUTPUT);
    //Use a semaphore to control access to global variables
    #ifdef USE_RTOS
      beeperSemaphore = xSemaphoreCreateBinary();
    #endif
    pinMode(beeperPin, OUTPUT);
    //digitalWrite(beeperPin, HIGH);
    ledcSetup(beeperChannel, beeperButtonTone, 16);
    ledcAttachPin(beeperPin, beeperChannel);
    #ifdef USE_RTOS
      xSemaphoreGive(beeperSemaphore);
      xTaskCreate(manageBeeper, "manageBeeper", 1000, NULL, configMAX_PRIORITIES - 2 , &beeperManagementTask); //configMAX_PRIORITIES - 2
    #endif
    localLogLn(F("OK"));
  }
  void makeAbeep(uint16_t frequency)
  {
    if(beeperState == false)
    {
      if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout))
      {
        //ledcAttachPin(beeperPin, beeperChannel);
        ledcWriteTone(beeperChannel, frequency);
        beeperLastStateChange = millis();
        beeperState = true;
        beeperTone = frequency;
        xSemaphoreGive(beeperSemaphore);
      }
    }
  }
  void makeAbeep(uint16_t frequency, uint32_t ontime, uint32_t offtime = 0)
  {
    if(beeperState == false)
    {
      #ifdef USE_RTOS
      if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout))
      {
      #endif
        //ledcAttachPin(beeperPin, beeperChannel);
        ledcWriteTone(beeperChannel, frequency);
        beeperTone = frequency;
        beeperOnTime = ontime;
        beeperOffTime = offtime;
        beeperLastStateChange = millis();
        beeperState = true;
      #ifdef USE_RTOS
        xSemaphoreGive(beeperSemaphore);
      }
      #endif
    }
  }
  void stopBeep()
  {
    #ifdef USE_RTOS
    if(xSemaphoreTake(beeperSemaphore, beeperSemaphoreTimeout))
    {
    #endif
      ledcWriteTone(beeperChannel, 0);
      //ledcDetachPin(beeperPin);
      //pinMode(beeperPin, OUTPUT);
      //digitalWrite(beeperPin, HIGH);
      beeperState = false;
      beeperLastStateChange = millis();
      #ifdef USE_RTOS
      xSemaphoreGive(beeperSemaphore);
      #endif
    #ifdef USE_RTOS
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
          if(millis() - beeperLastStateChange > beeperOnTime)
          {
            stopBeep();
          }
        }
        if(beeperState == false && beeperOffTime != 0)
        {
          if(millis() - beeperLastStateChange > beeperOffTime)
          {
            #ifdef SUPPORT_BEEPER
              makeAbeep(beeperTone, beeperOnTime, beeperOffTime);
            #endif
          }
        }
        #if defined(ACT_AS_TRACKER)
          else if(device[currentBeacon].hasFix == false && beeperState == true)
          {
            stopBeep();
          }
          else if(device[currentBeacon].hasFix == true)
          {
            if(beeperState == true && millis() - beeperLastStateChange > beeperOnTime)
            {
              stopBeep();
            }
            else if(beeperState == false && beeperOffTime !=0 && millis() - beeperLastStateChange > beeperOffTime)
            {
              makeAbeep(beeperTone);
            }
          }
        #endif
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
          beeperOffTime = beeperOnTime + 50;
        }
        else
        {
          #ifdef SUPPORT_LED
            ledOffTime = distanceToCurrentBeacon * 100;
          #endif
          beeperOffTime = beeperOnTime + 50 + pow(distanceToCurrentBeacon,2);
        }
      }
      else
      {
        #ifdef SUPPORT_LED
          ledOffTime = 100;
        #endif
        beeperOffTime = 0;
      }
      #if defined(SERIAL_DEBUG) && defined(DEBUG_BEEPER)
        if(beeperOffTime > 0)
        {
          SERIAL_DEBUG_PORT.printf_P(PSTR("Beeper off time: %ums\r\n"),beeperOffTime);
        }
        else
        {
          SERIAL_DEBUG_PORT.println(F("Beeper off"));
        }
      #endif
    }
  #endif
#endif

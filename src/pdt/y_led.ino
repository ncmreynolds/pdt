#ifdef SUPPORT_LED
  void setupLed()
  {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin,LOW);
    #ifdef USE_RTOS
      ledSemaphore = xSemaphoreCreateBinary();
    #endif
    #ifdef USE_RTOS
      xSemaphoreGive(ledSemaphore);
      xTaskCreate(manageLed, "manageLed", 1000, NULL, configMAX_PRIORITIES - 3, &ledManagementTask);
    #endif
  }
  void ledOn(uint32_t ontime, uint32_t offtime)
  {
    #ifdef USE_RTOS
    if(xSemaphoreTake(ledSemaphore, ledSemaphoreTimeout))
    {
    #endif
      if(ledState == false)
      {
        digitalWrite(ledPin, HIGH);
        ledOnTime = ontime;
        ledLastStateChange = millis();
        ledState = true;
      }
    #ifdef USE_RTOS
      xSemaphoreGive(ledSemaphore);
    }
    #endif
  }
  void ledOff()
  {
    #ifdef USE_RTOS
    if(xSemaphoreTake(ledSemaphore, ledSemaphoreTimeout))
    {
    #endif
      digitalWrite(ledPin, LOW);
      ledState = false;
      ledLastStateChange = millis();
      #ifdef USE_RTOS
      xSemaphoreGive(ledSemaphore);
      #endif
    #ifdef USE_RTOS
    }
    #endif
  }
  void manageLed(void * parameter)
  {
    while(true)
    {
      if(ledEnabled == true)
      {
        if(ledState == true)
        {
          if(millis() - ledLastStateChange > ledOnTime)
          {
            ledOff();
          }
        }
        if(ledState == false && ledOffTime != 0)
        {
          if(millis() - ledLastStateChange > ledOffTime)
          {
            ledOn(ledOnTime, ledOffTime);
          }
        }
      }
      vTaskDelay(ledYieldTime / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);  //Kill this task
  }
#endif

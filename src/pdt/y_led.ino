#ifdef SUPPORT_LED
  void setupLed()
  {
    pinMode(ledPin, OUTPUT);
    if(ledPinInverted == true)
    {
      digitalWrite(ledPin,HIGH);
    }
    else
    {
      digitalWrite(ledPin,LOW);
    }
    ledSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(ledSemaphore);
    xTaskCreate(manageLed, "manageLed", 512, NULL, configMAX_PRIORITIES - 3, &ledManagementTask);
  }
  void ledOn(uint32_t ontime, uint32_t offtime)
  {
    if(xSemaphoreTake(ledSemaphore, ledSemaphoreTimeout) == pdTRUE)
    {
      if(ledState == false)
      {
        if(ledPinInverted == true)
        {
          digitalWrite(ledPin, LOW);
        }
        else
        {
          digitalWrite(ledPin, HIGH);
        }
        ledOnTime = ontime;
        ledLastStateChange = millis();
        ledState = true;
      }
      xSemaphoreGive(ledSemaphore);
    }
  }
  void ledOff()
  {
    if(xSemaphoreTake(ledSemaphore, ledSemaphoreTimeout) == pdTRUE)
    {
      if(ledPinInverted == true)
      {
        digitalWrite(ledPin, HIGH);
      }
      else
      {
        digitalWrite(ledPin, LOW);
      }
      ledState = false;
      ledLastStateChange = millis();
      xSemaphoreGive(ledSemaphore);
    }
  }
  void manageLed(void * parameter)
  {
    #if defined(ENABLE_OTA_UPDATE)
    while(otaInProgress == false)
    #else
    while(true)
    #endif
    {
      if(ledEnabled == true)
      {
        if(ledState == true)
        {
          if(ledOnTime > 0 && millis() - ledLastStateChange > ledOnTime)
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

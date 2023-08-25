#ifdef SUPPORT_LED
  void setupLed()
  {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin,LOW);
    ledSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(ledSemaphore);
    xTaskCreate(manageLed, "manageLed", 1000, NULL, configMAX_PRIORITIES - 3, &ledManagementTask);
  }
  void ledOn(uint32_t ontime, uint32_t offtime)
  {
    if(xSemaphoreTake(ledSemaphore, ledSemaphoreTimeout))
    {
      if(ledState == false)
      {
        digitalWrite(ledPin, HIGH);
        ledOnTime = ontime;
        ledLastStateChange = millis();
        ledState = true;
      }
      xSemaphoreGive(ledSemaphore);
    }
  }
  void ledOff()
  {
    if(xSemaphoreTake(ledSemaphore, ledSemaphoreTimeout))
    {
      digitalWrite(ledPin, LOW);
      ledState = false;
      ledLastStateChange = millis();
      xSemaphoreGive(ledSemaphore);
    }
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

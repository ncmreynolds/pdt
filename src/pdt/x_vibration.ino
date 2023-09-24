#ifdef SUPPORT_VIBRATION
  void setupVibration()
  {
    pinMode(vibrationPin, OUTPUT);
    if(vibrationPinInverted == true)
    {
      digitalWrite(vibrationPin,HIGH);
    }
    else
    {
      digitalWrite(vibrationPin,LOW);
    }
    vibrationSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(vibrationSemaphore);
    xTaskCreate(manageVibration, "manageVibration", 512, NULL, configMAX_PRIORITIES - 3, &vibrationManagementTask);
  }
  void vibrationOn(uint32_t ontime, uint32_t offtime)
  {
    if(xSemaphoreTake(vibrationSemaphore, vibrationSemaphoreTimeout) == pdTRUE)
    {
      if(vibrationState == false)
      {
        if(vibrationPinInverted == true)
        {
          digitalWrite(vibrationPin, LOW);
        }
        else
        {
          digitalWrite(vibrationPin, HIGH);
        }
        vibrationOnTime = ontime;
        vibrationLastStateChange = millis();
        vibrationState = true;
      }
      xSemaphoreGive(vibrationSemaphore);
    }
  }
  void vibrationOff()
  {
    if(xSemaphoreTake(vibrationSemaphore, vibrationSemaphoreTimeout) == pdTRUE)
    {
      if(vibrationPinInverted == true)
      {
        digitalWrite(vibrationPin, HIGH);
      }
      else
      {
        digitalWrite(vibrationPin, LOW);
      }
      vibrationState = false;
      vibrationLastStateChange = millis();
      xSemaphoreGive(vibrationSemaphore);
    }
  }
  void manageVibration(void * parameter)
  {
    while(otaInProgress == false)
    {
      if(vibrationEnabled == true)
      {
        if(vibrationState == true)
        {
          if(vibrationOnTime > 0 && millis() - vibrationLastStateChange > vibrationOnTime)
          {
            vibrationOff();
          }
        }
        if(vibrationState == false && vibrationOffTime != 0)
        {
          if(millis() - vibrationLastStateChange > vibrationOffTime)
          {
            vibrationOn(vibrationOnTime, vibrationOffTime);
          }
        }
      }
      vTaskDelay(vibrationYieldTime / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);  //Kill this task
  }
#endif

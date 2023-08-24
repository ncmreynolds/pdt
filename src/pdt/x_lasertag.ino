#if defined(ACT_AS_SENSOR)
  void setupLasertag()
  {
    localLog(F("Configuring Lasertag receiver: "));
    localLogLn(F("OK"));
  }
  void manageLasertag()
  {
    if(currentSensorState == sensorState::starting)
    {
      if(millis() - lastSensorStateChange > 500)
      {
        lastSensorStateChange = millis();
        currentSensorState = sensorState::playStartupAnimation;
        localLogLn(F("Sensor: playStartupAnimation"));
        #ifdef SUPPORT_BEEPER
          xTaskCreate(playStartupAnimation, "playStartupAnimation", 1000, NULL, configMAX_PRIORITIES - 1, NULL);
        #endif

      }
    }
    else if(currentSensorState == sensorState::playStartupAnimation)
    {
      if(millis() - lastSensorStateChange > 2000)
      {
        lastSensorStateChange = millis();
        if(currentNumberOfHits == 0)
        {
          currentSensorState = sensorState::dead;
          localLogLn(F("Sensor: dead"));
          #ifdef SUPPORT_BEEPER
            xTaskCreate(playDeadAnimation, "playDeadAnimation", 1000, NULL, configMAX_PRIORITIES - 1, NULL);
          #endif
        }
        else if(currentNumberOfStunHits == 0)
        {
          currentSensorState = sensorState::stunned;
          localLogLn(F("Sensor: stunned"));
        }
        else
        {
          currentSensorState = sensorState::active;
          DoTstart();
          localLogLn(F("Sensor: active"));
        }
      }
    }
    else if(currentSensorState == sensorState::active)
    {
      if(sensor.received())
      {
        // Note that the library disables incoming hits once one is received for you to act on it.
        // You need to resume afterwards
        if(sensor.validDoT() || sensor.validWoW())
        {
          String message;
          timeOfLastHit = millis();
          message = "Hit: " + sensor.data_description() + " (confidence " + sensor.confidence() + "%)";
          uint8_t effectiveHits = sensor.hitsReceived();
          if(sensor.stunReceived())
          {
            if(effectiveHits >= currentNumberOfStunHits)
            {
              currentNumberOfStunHits = 0;
              currentSensorState = sensorState::stunned;
              sensorPersitentData.putUChar(currentNumberOfStunHitsKey, currentNumberOfStunHits);
              //sensorPersitentData.putUChar(currentSensorStateKey, (uint8_t)currentSensorState);
              lastSensorStateChange = millis();
              localLogLn(F("Sensor: stunned"));
              sensor.pause();
            }
            else
            {
              currentNumberOfStunHits -= effectiveHits;
              sensorPersitentData.putUChar(currentNumberOfStunHitsKey, currentNumberOfStunHits);
              currentSensorState = sensorState::takenHit;
              localLogLn(F("Sensor: takenHit"));
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true)
                {
                    xTaskCreate(playHitAnimation, "playHitAnimation", 1000, (void*)&effectiveHits, configMAX_PRIORITIES - 1, NULL);
                }
              #endif
            }
            message += " " + String(effectiveHits) + " stun damage received, now " + String(currentNumberOfStunHits) + "/" + String(numberOfStartingStunHits) + " stun hits";
            localLogLn(message);
          }
          else if(sensor.healingReceived())
          {
            currentNumberOfHits += effectiveHits;
            currentNumberOfStunHits += effectiveHits;
            if(currentNumberOfHits > numberOfStartingHits)
            {
              currentNumberOfHits = numberOfStartingHits;
            }
            if(currentNumberOfStunHits > numberOfStartingStunHits)
            {
              currentNumberOfStunHits = numberOfStartingStunHits;
            }
            sensorPersitentData.putUChar(currentNumberOfHitsKey, currentNumberOfHits);
            sensorPersitentData.putUChar(currentNumberOfStunHitsKey, currentNumberOfStunHits);
            message += " " + String(effectiveHits) + " healing received, now " + String(currentNumberOfHits) + "/" + String(numberOfStartingHits) + " hits";
            localLogLn(message);
          }
          else  //Normal damage hit
          {
            if(armourValue >= effectiveHits)
            {
              effectiveHits = 0;
              message += " no effect, blocked by armour";
              #ifdef SUPPORT_BEEPER
                //xTaskCreate(playNearMissAnimation, "playNearMissAnimation", 1000, NULL, 2, NULL);
              #endif
            }
            else
            {
              effectiveHits -= armourValue;
            }
            if(effectiveHits >= currentNumberOfHits)
            {
              currentNumberOfHits = 0;
              bleedOutCounter = 0;
              currentSensorState = sensorState::dead;
              sensorPersitentData.putUChar(currentNumberOfHitsKey, currentNumberOfHits);
              sensorPersitentData.putUChar(bleedOutCounterKey, bleedOutCounter);
              //sensorPersitentData.putUChar(currentSensorStateKey, (uint8_t)currentSensorState);
              lastSensorStateChange = millis();
              localLogLn(F("Sensor: dead"));
              sensor.pause();
              #ifdef SUPPORT_BEEPER
                xTaskCreate(playDeadAnimation, "playDeadAnimation", 1000, NULL, 2, NULL);
              #endif
            }
            else
            {
              currentNumberOfHits -= effectiveHits;
              sensorPersitentData.putUChar(currentNumberOfHitsKey, currentNumberOfHits);
              currentSensorState = sensorState::takenHit;
              localLogLn(F("Sensor: takenHit"));
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true)
                {
                    xTaskCreate(playHitAnimation, "playHitAnimation", 1000, (void*)&effectiveHits, 2, NULL);
                }
              #endif
            }
            message += " " + String(effectiveHits) + " damage received, now " + String(currentNumberOfHits) + "/" + String(numberOfStartingHits) + " hits";
            localLogLn(message);
          }
        }
        else if(sensor.nearMiss())
        {
          currentSensorState = sensorState::nearMiss;
          localLogLn(F("Sensor: nearMiss"));
          #ifdef SUPPORT_BEEPER
            //xTaskCreate(playNearMissAnimation, "playNearMissAnimation", 1000, NULL, 2, NULL);
          #endif
        }
        //else if(sensor.invalidSignal())
        else
        {
          currentSensorState = sensorState::invalidSignal;
          localLogLn(F("Sensor: invalidSignal"));
        }
      }
    }
    else if(currentSensorState == sensorState::nearMiss)
    {
      if(millis() - lastSensorStateChange > 250)
      {
        lastSensorStateChange = millis();
        // Resume taking hits again
        sensor.resume();
        currentSensorState = sensorState::active;
        localLogLn(F("Sensor: active"));
      }
    }
    else if(currentSensorState == sensorState::invalidSignal)
    {
      if(millis() - lastSensorStateChange > 250)
      {
        lastSensorStateChange = millis();
        // Resume taking hits again
        sensor.resume();
        currentSensorState = sensorState::active;
        localLogLn(F("Sensor: active"));
      }
    }
    else if(currentSensorState == sensorState::playCurrentHits)
    {
      if(millis() - lastSensorStateChange > 5000)
      {
        lastSensorStateChange = millis();
        currentSensorState = sensorState::active;
        localLogLn(F("Sensor: active"));
      }
    }
    else if(currentSensorState == sensorState::takenHit)
    {
      if(millis() - lastSensorStateChange > 1000)
      {
        // Resume taking hits again
        sensor.resume();
        lastSensorStateChange = millis();
        currentSensorState = sensorState::active;
        localLogLn(F("Sensor: active"));
      }
    }
    else if(currentSensorState == sensorState::dead)
    {
      /*
      if(millis() - lastSensorStateChange > 10000)
      {
        lastSensorStateChange = millis();
        currentSensorState = sensorState::starting;
        localLogLn(F("Sensor: starting"));
        sensor.resume();
      }
      */
    }
    else if(currentSensorState == sensorState::bleedOut)
    {
      if(millis() - lastSensorStateChange > 60000)
      {
        lastSensorStateChange = millis();
        bleedOutCounter++;
        sensorPersitentData.putUChar(bleedOutCounterKey, bleedOutCounter);
        if(bleedOutCounter >= bleedOutTime)
        {
          currentSensorState = sensorState::dead;
          lastSensorStateChange = millis();
          localLogLn(F("Sensor: dead"));
          #ifdef SUPPORT_BEEPER
            xTaskCreate(playDeadAnimation, "playDeadAnimation", 1000, NULL, 2, NULL);
          #endif
        }
        else
        {
          localLog(F("Sensor: bleedOut "));
          localLog(bleedOutCounter);
          if(bleedOutCounter > 1)
          {
            localLogLn(F(" minutes"));
          }
          else
          {
            localLogLn(F(" minute"));
          }
          #ifdef SUPPORT_BEEPER
            xTaskCreate(playCountOutMinutesAnimation, "playCountOutMinutesAnimation", 1000, (void*)&bleedOutCounter, 2, NULL);
          #endif
        }
      }
    }
    else if(currentSensorState == sensorState::stunned)
    {
      if(millis() - lastSensorStateChange > 30000)
      {
        lastSensorStateChange = millis();
        currentSensorState = sensorState::starting;
        localLogLn(F("Sensor: starting"));
        sensor.resume();
      }
    }
  }
  void playCountOutMinutesAnimation(void * parameter)
  {
    uint8_t numberOfMinutes = *((uint8_t*)parameter);
    for(uint8_t i = 0; i < numberOfMinutes; i++)
    {
      #ifdef SUPPORT_BEEPER
        makeAsingleBeep(300, 250);
      #endif
      #ifdef SUPPORT_LED
        ledOn(125, 0);
      #endif
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);  //Kill this task
  }
  void playCurrentHitsAnimation(void * parameter)
  {
    for(uint8_t i = 0; i < currentNumberOfHits; i++)
    {
      #ifdef SUPPORT_BEEPER
        makeAsingleBeep(sensorTones[2], 50);
      #endif
      #ifdef SUPPORT_LED
        ledOn(25, 0);
      #endif
      if(i%3 == 2)
      {
        vTaskDelay(300 / portTICK_PERIOD_MS);
      }
      else
      {
        vTaskDelay(150 / portTICK_PERIOD_MS);
      }
    }
    vTaskDelete(NULL);  //Kill this task
  }
  void playHitAnimation(void * parameter)
  {
    uint8_t numberOfHits = *((uint8_t*)parameter);
    for(uint8_t i = 0; i < numberOfHits; i++)
    {
      #ifdef SUPPORT_BEEPER
        makeAsingleBeep(sensorTones[2], 75);
      #endif
      #ifdef SUPPORT_LED
        ledOn(100, 0);
      #endif
      vTaskDelay(150 / portTICK_PERIOD_MS);
    }
    #ifdef SUPPORT_BEEPER
      makeAsingleBeep(900, 250);
    #endif
    #ifdef SUPPORT_LED
      ledOn(125, 0);
    #endif
    vTaskDelay(260 / portTICK_PERIOD_MS);
    #ifdef SUPPORT_BEEPER
      makeAsingleBeep(500, 400);
    #endif
    #ifdef SUPPORT_LED
      ledOn(200, 0);
    #endif
    vTaskDelay(410 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);  //Kill this task
  }
  void playNearMissAnimation(void * parameter)
  {
    #ifdef SUPPORT_BEEPER
      makeAsingleBeep(sensorTones[2], 100);
    #endif
    #ifdef SUPPORT_LED
      ledOn(50, 0);
    #endif
    vTaskDelay(150 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);  //Kill this task
  }
  void playDeadAnimation(void * parameter)
  {
    while(currentSensorState == sensorState::dead)
    {
      #ifdef SUPPORT_BEEPER
        makeAsingleBeep(1000, 199);
      #endif
      #ifdef SUPPORT_LED
        ledOn(100, 0);
      #endif
      vTaskDelay(205 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);  //Kill this task
  }
  void playStartupAnimation(void * parameter)
  {
    for(uint8_t i = 0; i < 3; i++)
    {
      #ifdef SUPPORT_BEEPER
        makeAsingleBeep(900, 200);
      #endif
      #ifdef SUPPORT_LED
        ledOn(100, 0);
      #endif
      vTaskDelay(210 / portTICK_PERIOD_MS);
      #ifdef SUPPORT_BEEPER
        makeAsingleBeep(500, 200);
      #endif
      #ifdef SUPPORT_LED
        ledOn(100, 0);
      #endif
      vTaskDelay(210 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);  //Kill this task
  }
  //Non-class functions for IR receiving
  
  //You CANNOT work with interrupt service routines in a class, so these two functions are here to act as wrappers. No I can't see a nice fix for this.
  
  void DoTstart()
  {
    pinMode(IR_RECEIVER_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(IR_RECEIVER_PIN), DoTisrWrapper, CHANGE);
    //Need to do some internal setup in the lasertag library
    sensor.resume();
  }
  
  //This does nothing but wrap the ISR in the library into a static function. There is no way to fix this, leave it alone!
  
  void DoTisrWrapper()
  {
    sensor.isr();
  }
#endif

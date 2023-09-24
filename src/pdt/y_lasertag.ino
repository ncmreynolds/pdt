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
        #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
          xTaskCreate(playStartupAnimation, "playStartupAnimation", 512, NULL, configMAX_PRIORITIES - 1, NULL);
        #endif

      }
    }
    else if(currentSensorState == sensorState::playStartupAnimation)
    {
      if(millis() - lastSensorStateChange > 5000)
      {
        lastSensorStateChange = millis();
        if(currentNumberOfHits == 0)
        {
          currentSensorState = sensorState::dead;
          localLogLn(F("Sensor: dead"));
          #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
            xTaskCreate(playDeadAnimation, "playDeadAnimation", 512, NULL, configMAX_PRIORITIES - 1, NULL);
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
              sensorPersitentData.putUChar(currentStunKey, currentNumberOfStunHits);
              //sensorPersitentData.putUChar(currentSensorStateKey, (uint8_t)currentSensorState);
              lastSensorStateChange = millis();
              #ifdef SUPPORT_LORA
                scheduleDeviceInfoShareSoon(); //Force the sensor to update any trackers soon
              #endif
              localLogLn(F("Sensor: stunned"));
              sensor.pause();
            }
            else
            {
              currentNumberOfStunHits -= effectiveHits;
              #ifdef SUPPORT_LORA
                scheduleDeviceInfoShareSoon(); //Force the sensor to update any trackers soon
              #endif
              sensorPersitentData.putUChar(currentStunKey, currentNumberOfStunHits);
              currentSensorState = sensorState::takenHit;
              localLogLn(F("Sensor: takenHit"));
              #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
                xTaskCreate(playHitAnimation, "playHitAnimation", 1000, (void*)&effectiveHits, configMAX_PRIORITIES - 1, NULL);
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
            #ifdef SUPPORT_LORA
              scheduleDeviceInfoShareSoon(); //Force the sensor to update any trackers soon
            #endif
            sensorPersitentData.putUChar(currentHitsKey, currentNumberOfHits);
            sensorPersitentData.putUChar(currentStunKey, currentNumberOfStunHits);
            message += " " + String(effectiveHits) + " healing received, now " + String(currentNumberOfHits) + "/" + String(numberOfStartingHits) + " hits";
            localLogLn(message);
          }
          else  //Normal damage hit
          {
            if(armourValue >= effectiveHits)
            {
              effectiveHits = 0;
              message += " no effect, blocked by armour";
              #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
                //xTaskCreate(playNearMissAnimation, "playNearMissAnimation", 512, NULL, 2, NULL);
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
              sensorPersitentData.putUChar(currentHitsKey, currentNumberOfHits);
              sensorPersitentData.putUChar(bleedOutCounterKey, bleedOutCounter);
              //sensorPersitentData.putUChar(currentSensorStateKey, (uint8_t)currentSensorState);
              lastSensorStateChange = millis();
              localLogLn(F("Sensor: dead"));
              sensor.pause();
              #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
                xTaskCreate(playDeadAnimation, "playDeadAnimation", 512, NULL, 2, NULL);
              #endif
              #ifdef SUPPORT_LORA
                scheduleDeviceInfoShareSoon(); //Force the sensor to update any trackers soon
              #endif
            }
            else
            {
              currentNumberOfHits -= effectiveHits;
              sensorPersitentData.putUChar(currentHitsKey, currentNumberOfHits);
              currentSensorState = sensorState::takenHit;
              localLogLn(F("Sensor: takenHit"));
              #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
                xTaskCreate(playHitAnimation, "playHitAnimation", 1000, (void*)&effectiveHits, 2, NULL);
              #endif
            }
            #ifdef SUPPORT_LORA
              scheduleDeviceInfoShareSoon(); //Force the sensor to update any trackers soon
            #endif
            message += " " + String(effectiveHits) + " damage received, now " + String(currentNumberOfHits) + "/" + String(numberOfStartingHits) + " hits";
            localLogLn(message);
          }
        }
        else if(sensor.nearMiss())
        {
          currentSensorState = sensorState::nearMiss;
          localLogLn(F("Sensor: nearMiss"));
          #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
            //xTaskCreate(playNearMissAnimation, "playNearMissAnimation", 512, NULL, 2, NULL);
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
      /*
      if(millis() - lastSensorStateChange > 30000)
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
          #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
            xTaskCreate(playDeadAnimation, "playDeadAnimation", 512, NULL, 2, NULL);
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
          #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
            xTaskCreate(playCountOutMinutesAnimation, "playCountOutMinutesAnimation", 1000, (void*)&bleedOutCounter, 2, NULL);
          #endif
        }
      }
    }
    else if(currentSensorState == sensorState::resetting) //Sensor has been asked to reset
    {
      lastSensorStateChange = millis();
      currentSensorState = sensorState::starting;
      resetSensor(); //Reset the sensor and inform and trackers
    }
  }
  void loadSensorConfiguration()
  {
    numberOfStartingHits = sensorPersitentData.getUChar(startingHitsKey, numberOfStartingHits);
    currentNumberOfHits = sensorPersitentData.getUChar(currentHitsKey, numberOfStartingHits);
    numberOfStartingStunHits = sensorPersitentData.getUChar(startingStunKey, numberOfStartingHits);
    currentNumberOfStunHits = sensorPersitentData.getUChar(currentStunKey, numberOfStartingStunHits);
    EP_flag = sensorPersitentData.getBool(EP_flag_key, false); //Sensor requires EP set in signal to be hit
    ig_healing_flag = sensorPersitentData.getBool(ig_healing_flag_key, false); //Sensor ignores healing
    ig_stun_flag = sensorPersitentData.getBool(ig_stun_flag_key, false); //Sensor ignores stun hits
    ig_ongoing_flag = sensorPersitentData.getBool(ig_ongoing_flag_key, false); //Sensor ignores ongoing hits
    regen_while_zero = sensorPersitentData.getBool(regen_from_zero_key, false); //Sensor can regenerate from zero
    treat_as_one = sensorPersitentData.getBool(hit_as_one_key, false); //Sensor treats all hits as one damage
    treat_stun_as_one = sensorPersitentData.getBool(stun_as_one_key, false); //Sensor treats all stun as one
    ongoing_is_cumulative = sensorPersitentData.getBool(ongoing_adds_key, false); //Sensor adds ongoing damage to current ongoing value
    ig_non_dot = sensorPersitentData.getBool(ig_non_dot_key, false); //Sensor ignores non-DOT signals
    bleedOutCounter = sensorPersitentData.getUChar(bleedOutCounterKey, 0);
  }
  void saveSensorConfiguration()
  {
    uint8_t items_saved = 0;
    localLog(F("Saving sensor configuration: "));
    if(sensorPersitentData.putUChar(startingHitsKey, numberOfStartingHits) == 0)
    {
      localLog(startingHitsKey);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putUChar(currentHitsKey, currentNumberOfHits) == 0)
    {
      localLog(currentHitsKey);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putUChar(startingStunKey, numberOfStartingStunHits) == 0)
    {
      localLog(startingStunKey);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putUChar(currentStunKey, currentNumberOfStunHits) == 0)
    {
      localLog(currentStunKey);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putBool(EP_flag_key, EP_flag) == 0) //Sensor requires EP set in signal to be hit
    {
      localLog(EP_flag_key);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putBool(ig_healing_flag_key, ig_healing_flag) == 0) //Sensor ignores healing
    {
      localLog(ig_healing_flag_key);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putBool(ig_stun_flag_key, ig_stun_flag) == 0) //Sensor ignores stun hits
    {
      localLog(ig_stun_flag_key);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putBool(ig_ongoing_flag_key, ig_ongoing_flag) == 0) //Sensor ignores ongoing hits
    {
      localLog(ig_ongoing_flag_key);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putBool(regen_from_zero_key, regen_while_zero) == 0) //Sensor can regenerate from zero
    {
      localLog(regen_from_zero_key);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putBool(hit_as_one_key, treat_as_one) == 0) //Sensor treats all hits as one damage
    {
      localLog(hit_as_one_key);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putBool(stun_as_one_key, treat_stun_as_one) == 0) //Sensor treats all stun as one
    {
      localLog(stun_as_one_key);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putBool(ongoing_adds_key, ongoing_is_cumulative) == 0) //Sensor adds ongoing damage to current ongoing value
    {
      localLog(ongoing_adds_key);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(sensorPersitentData.putBool(ig_non_dot_key, ig_non_dot) == 0) //Sensor ignores non-DOT signals
    {
      localLog(ig_non_dot_key);
      localLog(F("-failed "));
    }
    else
    {
      items_saved++;
    }
    if(items_saved == 13)
    {
      localLogLn(F("OK"));
    }
    else
    {
      localLogLn(F("not OK"));
    }
  }
  void showSensorConfiguration()
  {
    localLog(F("Starting hits: "));localLogLn(numberOfStartingHits);
    localLog(F("Current hits: "));localLogLn(currentNumberOfHits);
    localLog(F("Starting stun hits: "));localLogLn(numberOfStartingStunHits);
    localLog(F("Current stun hits: "));localLogLn(currentNumberOfStunHits);
    localLog(F("Starting bleed out counter: "));localLogLn(bleedOutCounter);
    localLog(F("Require EP to hit: "));localLogLn(EP_flag == true ? "Yes":"No");
    localLog(F("Ignore healing: "));localLogLn(ig_healing_flag == true ? "Yes":"No");
    localLog(F("Ignore stun: "));localLogLn(ig_stun_flag == true ? "Yes":"No");
    localLog(F("Ignore ongoing: "));localLogLn(ig_ongoing_flag == true ? "Yes":"No");
    localLog(F("Regenerate while zero: "));localLogLn(regen_while_zero == true ? "Yes":"No");
    localLog(F("Treat hits as one: "));localLogLn(treat_as_one == true ? "Yes":"No");
    localLog(F("Treat stun as one: "));localLogLn(treat_stun_as_one == true ? "Yes":"No");
    localLog(F("Ongoing effects are cumulative: "));localLogLn(ongoing_is_cumulative == true ? "Yes":"No");
    localLog(F("Ignore non-DOT signals: "));localLogLn(ig_non_dot == true ? "Yes":"No");
    localLog(F("Previous sensor state: "));localLogLn((uint8_t)currentSensorState);
  }
  void resetSensor()
  {
    //ledOff();
    currentNumberOfHits = numberOfStartingHits;
    currentNumberOfStunHits = numberOfStartingStunHits;
    sensorPersitentData.putUChar(currentHitsKey, currentNumberOfHits);
    sensorPersitentData.putUChar(currentStunKey, currentNumberOfStunHits);
    sensorPersitentData.putUChar(bleedOutCounterKey, bleedOutCounter);
    #ifdef SUPPORT_LORA
      scheduleDeviceInfoShareSoon(); //Force the sensor to update any trackers soon
    #endif
  }
  void playCountOutMinutesAnimation(void * parameter)
  {
    uint8_t numberOfMinutes = *((uint8_t*)parameter);
    for(uint8_t i = 0; i < numberOfMinutes; i++)
    {
      #ifdef SUPPORT_BEEPER
        if(beeperEnabled == true)
        {
          makeAsingleBeep(300, 250);
        }
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
        if(beeperEnabled == true)
        {
          makeAsingleBeep(sensorTones[2], 50);
        }
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
        if(beeperEnabled == true)
        {
          makeAsingleBeep(sensorTones[2], 75);
        }
      #endif
      #ifdef SUPPORT_LED
        ledOn(100, 0);
      #endif
      vTaskDelay(150 / portTICK_PERIOD_MS);
    }
    #ifdef SUPPORT_BEEPER
      if(beeperEnabled == true)
      {
        makeAsingleBeep(900, 250);
      }
    #endif
    #ifdef SUPPORT_LED
      ledOn(125, 0);
    #endif
    vTaskDelay(260 / portTICK_PERIOD_MS);
    #ifdef SUPPORT_BEEPER
      if(beeperEnabled == true)
      {
        makeAsingleBeep(500, 400);
      }
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
      if(beeperEnabled == true)
      {
        makeAsingleBeep(sensorTones[2], 100);
      }
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
        if(beeperEnabled == true)
        {
          makeAsingleBeep(1000, 199);
        }
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
        if(beeperEnabled == true)
        {
          makeAsingleBeep(900, 200);
        }
      #endif
      #ifdef SUPPORT_LED
        ledOn(100, 0);
      #endif
      vTaskDelay(210 / portTICK_PERIOD_MS);
      #ifdef SUPPORT_BEEPER
        if(beeperEnabled == true)
        {
          makeAsingleBeep(500, 200);
        }
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

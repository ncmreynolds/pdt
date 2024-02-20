#ifdef SUPPORT_BUTTON
  void setupButton()
  {
    pinMode(buttonPin, INPUT_PULLUP);
  }
  #if defined(ACT_AS_TRACKER) && HARDWARE_VARIANT == C3PDT
    void checkButton()
    {
      if(digitalRead(buttonPin) == false && buttonHeld == false)// && millis() - buttonPushTime > buttonDebounceTime) //Handle single button pushes, which don't do anything
      {
        buttonPushTime = millis();
        buttonHeld = true;
      }
      else if(digitalRead(buttonPin) == false && buttonHeld == true && millis() - buttonPushTime > buttonLongPressTime) //Long press
      {
        buttonPushTime = millis();
        buttonLongPress = true;
        localLogLn(F("Button: long press"));
        #ifdef SUPPORT_BEEPER
        if(currentDisplayState == displayState::beeper)
        {
          beeperEnabled = not beeperEnabled;
          displayBeeperStatus();
          saveConfigurationSoon = millis();
        }
        else if(currentDisplayState == displayState::trackingMode)
        {
          if(currentTrackingMode == trackingMode::nearest)
          {
            currentTrackingMode = trackingMode::furthest;
            displayTrackingMode();
          }
          else if(currentTrackingMode == trackingMode::furthest)
          {
            if(numberOfDevices > 1)
            {
              currentTrackingMode = trackingMode::fixed;
              displayTrackingMode();
            }
            else
            {
              currentTrackingMode = trackingMode::nearest;
              displayTrackingMode();
            }
          }
          else if(currentTrackingMode == trackingMode::fixed)
          {
            currentlyTrackedBeacon++;
            if(currentlyTrackedBeacon == numberOfDevices)
            {
              currentlyTrackedBeacon = maximumNumberOfDevices;
              currentTrackingMode = trackingMode::nearest;
            }
            displayTrackingMode();
          }
        }
        if(currentDisplayState == displayState::version)
        {
          printConfiguration();
        }
        #endif
      }
      else if(digitalRead(buttonPin) == true && buttonHeld == true && millis() - buttonPushTime > buttonDebounceTime) //Handle button releases
      {
        buttonHeld = false;
        localLogLn(F("Button: short press"));
        if(buttonLongPress == false) //Ignore release after a long press
        {
          #ifdef SUPPORT_DISPLAY
            if(currentDisplayState == displayState::welcome) //Show distance when button pushed
            {
              displayDistanceToBeacon();
            }
            else if(currentDisplayState == displayState::blank) //Show distance when button pushed
            {
              displayDistanceToBeacon();
            }
            else if(currentDisplayState == displayState::distance)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayCourse();
            }
            else if(currentDisplayState == displayState::course)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayStatus();
            }
            else if(currentDisplayState == displayState::status)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayAccuracy();
            }
            else if(currentDisplayState == displayState::accuracy)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayTrackingMode();
            }
            else if(currentDisplayState == displayState::trackingMode)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displaySignalStrengthFromBeacon();
            }
            else if(currentDisplayState == displayState::signal)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayBatteryPercentage();
            }
            else if(currentDisplayState == displayState::battery)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
                displayBeeperStatus();
              #else
                displayDistanceToBeacon();
              #endif
            }
            #ifdef SUPPORT_BEEPER
              else if(currentDisplayState == displayState::beeper)
              {
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
                displayVersion();
              }
            #endif
            else if(currentDisplayState == displayState::version)
            {
              if(beeperEnabled == true && beepOnPress == true)
              {
                makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
              }
              displayDistanceToBeacon();
            }
          #endif
        }
        else
        {
          buttonPushTime = millis();
          buttonLongPress = false;
        }
      }
    }
  #elif defined(ACT_AS_BEACON) && HARDWARE_VARIANT == C3TrackedSensor
    void checkButton()
    {
      if(digitalRead(buttonPin) == false && buttonHeld == false)// && millis() - buttonPushTime > buttonDebounceTime) //Handle single button pushes, which don't do anything
      {
        buttonPushTime = millis();
        buttonHeld = true;
      }
      else if(digitalRead(buttonPin) == false && buttonHeld == true && millis() - buttonPushTime > buttonLongPressTime) //Handle long press
      {
        buttonPushTime = millis();
        buttonLongPress = true;
        localLogLn(F("Button: long press"));
        printConfiguration();
        
      }
      else if(digitalRead(buttonPin) == true && buttonHeld == true && millis() - buttonPushTime > buttonDebounceTime) //Handle button releases
      {
        buttonHeld = false;
        if(buttonLongPress == false) //This is a short press
        {
          localLogLn(F("Button: short press"));
          #if defined(ACT_AS_SENSOR)
            if(currentSensorState == sensorState::active)
            {
              localLogLn(F("Sensor: playCurrentHits"));
              currentSensorState = sensorState::playCurrentHits;
              xTaskCreate(playCurrentHitsAnimation, "playCurrentHitsAnimation", 512, NULL, 2, NULL);
            }
            else if(currentSensorState == sensorState::dead)
            {
              localLogLn(F("Sensor: bleedOut"));
              #ifdef SUPPORT_LED
                ledOn(0, 0);  //Hard put the LED on
              #endif
              currentSensorState = sensorState::bleedOut;
            }
          #endif
        }
        else
        {
          buttonPushTime = millis();
          buttonLongPress = false;
        }
      }
    }
  #elif defined(ACT_AS_BEACON) && HARDWARE_VARIANT == C3LoRaBeacon
    void checkButton()
    {
      if(digitalRead(buttonPin) == false && buttonHeld == false)// && millis() - buttonPushTime > buttonDebounceTime) //Handle single button pushes, which don't do anything
      {
        buttonPushTime = millis();
        buttonHeld = true;
      }
      else if(digitalRead(buttonPin) == false && buttonHeld == true && millis() - buttonPushTime > buttonLongPressTime) //Handle long press
      {
        buttonPushTime = millis();
        buttonLongPress = true;
        localLogLn(F("Button: long press - powering off"));
        ledFastBlink();
        powerOffTimer = millis();
      }
      else if(digitalRead(buttonPin) == true && buttonHeld == true && millis() - buttonPushTime > buttonDebounceTime) //Handle button releases
      {
        buttonHeld = false;
        if(buttonLongPress == false) //This is a short press
        {
          localLogLn(F("Button: short press"));
          if(peripheralsEnabled == false)
          {
            peripheralPowerOn();
          }
          ledPulse();
        }
        else
        {
          buttonPushTime = millis();
          buttonLongPress = false;
        }
      }
    }
  #endif
#endif

#if defined(SUPPORT_BUTTON)
  void setupButton()
  {
    pinMode(buttonPin, INPUT_PULLUP);
  }
  #if defined(ACT_AS_TRACKER) && (HARDWARE_VARIANT == C3PDT || HARDWARE_VARIANT == C3PDTasBeacon)
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
        #if defined(SUPPORT_BEEPER)
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
            currentlyTrackedDevice++;
            if(currentlyTrackedDevice == numberOfDevices)
            {
              currentlyTrackedDevice = maximumNumberOfDevices;
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
          #if defined(SUPPORT_DISPLAY)
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
              #if defined(SUPPORT_BEEPER)
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayCourse();
            }
            else if(currentDisplayState == displayState::course)
            {
              #if defined(SUPPORT_BEEPER)
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayStatus();
            }
            else if(currentDisplayState == displayState::status)
            {
              #if defined(SUPPORT_BEEPER)
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayAccuracy();
            }
            else if(currentDisplayState == displayState::accuracy)
            {
              #if defined(SUPPORT_BEEPER)
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayTrackingMode();
            }
            else if(currentDisplayState == displayState::trackingMode)
            {
              #if defined(SUPPORT_BEEPER)
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              #if defined SUPPORT_SIGNAL_STRENGTH
                displaySignalStrengthFromBeacon();
              #else
                displayBatteryPercentage();
              #endif
            }
            #if defined SUPPORT_SIGNAL_STRENGTH
              else if(currentDisplayState == displayState::signal)
              {
                #if defined(SUPPORT_BEEPER)
                  if(beeperEnabled == true && beepOnPress == true)
                  {
                    makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                  }
                #endif
                displayBatteryPercentage();
              }
            #endif
            else if(currentDisplayState == displayState::battery)
            {
              #if defined(SUPPORT_BEEPER)
                if(beeperEnabled == true && beepOnPress == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
                displayBeeperStatus();
              #else
                displayDistanceToBeacon();
              #endif
            }
            #if defined(SUPPORT_BEEPER)
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
  #elif defined(ACT_AS_BEACON) && HARDWARE_VARIANT == C3TrackedSensor || HARDWARE_VARIANT == C3TrackedSensorAsBeacon
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
              #if defined(SUPPORT_LED)
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

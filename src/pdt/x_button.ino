#ifdef SUPPORT_BUTTON
  void setupButton()
  {
    pinMode(buttonPin, INPUT_PULLUP);
  }
  #ifdef ACT_AS_TRACKER
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
            currentBeacon++;
            if(currentBeacon == numberOfDevices)
            {
              currentBeacon = 0;
              currentTrackingMode = trackingMode::nearest;
            }
            displayTrackingMode();
          }
        }
        #endif
      }
      else if(digitalRead(buttonPin) == true && buttonHeld == true && millis() - buttonPushTime > buttonDebounceTime) //Handle button releases
      {
        buttonHeld = false;
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
                if(beeperEnabled == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayCourse();
            }
            else if(currentDisplayState == displayState::course)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayStatus();
            }
            else if(currentDisplayState == displayState::status)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayAccuracy();
            }
            else if(currentDisplayState == displayState::accuracy)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayTrackingMode();
            }
            else if(currentDisplayState == displayState::trackingMode)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displaySignalStrengthFromBeacon();
            }
            else if(currentDisplayState == displayState::signal)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
              #endif
              displayBatteryPercentage();
            }
            else if(currentDisplayState == displayState::battery)
            {
              #ifdef SUPPORT_BEEPER
                if(beeperEnabled == true)
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
                if(beeperEnabled == true)
                {
                  makeAsingleBeep(beeperButtonTone,beeperButtonOnTime);
                }
                displayVersion();
              }
            #endif
            else if(currentDisplayState == displayState::version)
            {
              if(beeperEnabled == true)
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
  #endif
  #ifdef ACT_AS_BEACON
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
        localLogLn(F("Long press button"));
      }
      else if(digitalRead(buttonPin) == true && buttonHeld == true && millis() - buttonPushTime > buttonDebounceTime) //Handle button releases
      {
        buttonHeld = false;
        if(buttonLongPress == false) //This is a short press
        {
          localLogLn(F("Press button"));
          #if defined(ACT_AS_SENSOR)
            if(currentSensorState == sensorState::active)
            {
                    xTaskCreate(playCurrentHitsAnimation, "playCurrentHitsAnimation", 1000, NULL, 2, NULL);
            }
            else if(currentSensorState == sensorState::dead)
            {
              localLogLn(F("Sensor: bleedOut"));
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
  #endif
#endif

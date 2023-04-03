#ifdef ACT_AS_TRACKER
  void checkButton()
  {
    if(digitalRead(buttonPin) == false && buttonHeld == false)// && millis() - buttonPushTime > buttonDebounceTime) //Handle single button pushes, which don't do anything
    {
      buttonPushTime = millis();
      buttonHeld = true;
    }
    else if(digitalRead(buttonPin) == false && buttonHeld == true && millis() - buttonPushTime > buttonLongPressTime)
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
          displayBeaconMode();
        }
        else if(currentTrackingMode == trackingMode::furthest)
        {
          if(numberOfBeacons > 0)
          {
            currentTrackingMode = trackingMode::fixed;
            displayBeaconMode();
          }
          else
          {
            currentTrackingMode = trackingMode::nearest;
            displayBeaconMode();
          }
        }
        else if(currentTrackingMode == trackingMode::fixed)
        {
          currentBeacon++;
          if(currentBeacon == numberOfBeacons)
          {
            currentBeacon = 0;
            currentTrackingMode = trackingMode::nearest;
          }
          displayBeaconMode();
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
                makeAbeep(beeperButtonTone);
              }
            #endif
            displayBeaconMode();
          }
          else if(currentDisplayState == displayState::trackingMode)
          {
            #ifdef SUPPORT_BEEPER
              if(beeperEnabled == true)
              {
                makeAbeep(beeperButtonTone);
              }
            #endif
            displaySignalStrengthFromBeacon();
          }
          else if(currentDisplayState == displayState::signal)
          {
            #ifdef SUPPORT_BEEPER
              if(beeperEnabled == true)
              {
                makeAbeep(beeperButtonTone);
              }
            #endif
            displayBatteryPercentage();
          }
          else if(currentDisplayState == displayState::battery)
          {
            #ifdef SUPPORT_BEEPER
              if(beeperEnabled == true)
              {
                makeAbeep(beeperButtonTone);
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
                makeAbeep(beeperButtonTone);
              }
              displayDistanceToBeacon();
            }
          #endif
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

#ifdef SUPPORT_BEEPER
  void setupBeeper()
  {
    pinMode(beeperPin, OUTPUT);
    ledcSetup(beeperChannel, beeperButtonTone, 8);
    ledcAttachPin(beeperPin, 0);
  }
  void makeAbeep(uint16_t frequency)
  {
    if(beeperState == false)
    {
      ledcWriteTone(beeperChannel, frequency);
      beeperLastStateChange = millis();
      beeperState = true;
    }
  }
  void stopBeep()
  {
    ledcWriteTone(beeperChannel, 0);
    beeperState = false;
  }
  void setBeeperUrgency()
  {
    if(distanceToCurrentBeacon > 0)
    {
      #ifdef SUPPORT_LED
        ledOffTime = distanceToCurrentBeacon * 100;
      #endif
      #ifdef SUPPORT_BEEPER
        beeperOffTime = beeperOnTime + 50 + pow(distanceToCurrentBeacon,2);
      #endif
    }
    else
    {
      #ifdef SUPPORT_LED
        ledOffTime = 100;
      #endif
      #ifdef SUPPORT_BEEPER
        beeperOffTime = beeperOnTime + 50;
      #endif
    }
  }
  void manageBeeper()
  {
    if(beeperState == true)
    {
      if(millis() - beeperLastStateChange > beeperOnTime)
      {
        stopBeep();
      }
    }
    else if(beacon[currentBeacon].hasFix == false && beeperState == true)
    {
      stopBeep();
    }
    else if(beacon[currentBeacon].hasFix == true)
    {
      if(beeperState == true && millis() - beeperLastStateChange > beeperOnTime)
      {
        stopBeep();
      }
      else if(beeperState == false && millis() - beeperLastStateChange > beeperOffTime)
      {
        makeAbeep(beeperTone);
      }
    }
  }
#endif

#ifdef SUPPORT_BEEPER
  void setupBeeper()
  {
    pinMode(beeperPin, OUTPUT);
  }
  void manageBeeper()
  {
    if(beacon[0].hasFix == false && beeperState == true)
    {
      noTone(beeperPin);
      pinMode(beeperPin, INPUT);
      beeperState = false;
    }
    else if(beacon[0].hasFix == true)
    {
      if(beeperState == true && millis() - beeperLastStateChange > beeperOnTime)
      {
        noTone(beeperPin);
        pinMode(beeperPin, INPUT);
        beeperLastStateChange = millis();
        beeperState = false;
      }
      else if(beeperState == false && millis() - beeperLastStateChange > beeperOffTime)
      {
        pinMode(beeperPin, OUTPUT);
        tone(beeperPin, beeperTone, beeperOnTime);
        beeperLastStateChange = millis();
        beeperState = true;
      }
    }
  }
#endif

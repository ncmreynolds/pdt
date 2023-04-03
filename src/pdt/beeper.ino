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
  void manageBeeper()
  {
    if(beeperState == true)
    {
      if(millis() - beeperLastStateChange > beeperOnTime)
      {
        ledcWriteTone(beeperChannel, 0);
        beeperState = false;
      }
    }
    else if(beacon[0].hasFix == false && beeperState == true)
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

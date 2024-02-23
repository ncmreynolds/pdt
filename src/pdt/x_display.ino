/*
 * 
 * This file contains functions related to showing content on the display
 * 
 */
#ifdef SUPPORT_DISPLAY
  void setupDisplay()
  {
    //ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1331_96x64_spi_init(displayResetPin, displayCSpin, displayDCpin);
    ssd1306_setMode( LCD_MODE_NORMAL );
    displayWelcomeScreen();
  }
  void printTopLine(const char *text)
  {
    ssd1306_setColor(RGB_COLOR8(255,255,255));
    if(strlen(text) < screenWidth/8)
    {
      ssd1306_setFixedFont(ssd1306xled_font8x16);
      ssd1306_printFixed8((screenWidth - (strlen(text)*8))/2,  0, text, STYLE_NORMAL);
    }
    else if(strlen(text) < screenWidth/6)
    {
      ssd1306_setFixedFont(ssd1306xled_font6x8);
      ssd1306_printFixed8((screenWidth - (strlen(text)*6))/2,  0, text, STYLE_NORMAL);
    }
    else if(strlen(text) < screenWidth/5)
    {
      ssd1306_setFixedFont(ssd1306xled_font5x7);
      ssd1306_printFixed8((screenWidth - (strlen(text)*5))/2,  0, text, STYLE_NORMAL);
    }
  }
  void manageDisplay()
  {
    #ifdef ACT_AS_TRACKER
      if(currentlyTrackedBeaconStateChanged == true)  //Ideally show the status to the tracker, but not if asleep
      {
        currentlyTrackedBeaconStateChanged = false;
        //if(currentDisplayState == displayState::distance|| currentDisplayState == displayState::status)
        {
          displayStatus();
        }
      }
    #endif
  }
  void printRange(const char *text)
  {
    ssd1306_setColor(RGB_COLOR8(255,0,0));
    //ssd1306_setColor(RGB_COLOR8(255,255,255));
    //ssd1306_setFixedFont(comic_sans_font24x32_123);
    ssd1306_setFixedFont(courier_new_font11x16_digits);
    ssd1306_printFixed8((screenWidth - (strlen(text)*11))/2,  (screenHeight - 16)/2, text, STYLE_BOLD);
  }
  void printMiddleLine(const char *text)
  {
    ssd1306_setColor(RGB_COLOR8(255,0,0));
    //ssd1306_setColor(RGB_COLOR8(255,255,255));
    if(strlen(text) < screenWidth/8)
    {
      ssd1306_setFixedFont(ssd1306xled_font8x16);
      ssd1306_printFixed8((screenWidth - (strlen(text)*8))/2,  (screenHeight - 16)/2, text, STYLE_BOLD);
    }
    else if(strlen(text) < screenWidth/6)
    {
      ssd1306_setFixedFont(ssd1306xled_font6x8);
      ssd1306_printFixed8((screenWidth - (strlen(text)*6))/2,  (screenHeight - 8)/2, text, STYLE_BOLD);
    }
    else if(strlen(text) < screenWidth/5)
    {
      ssd1306_setFixedFont(ssd1306xled_font5x7);
      ssd1306_printFixed8((screenWidth - (strlen(text)*5))/2,  (screenHeight - 7)/2, text, STYLE_BOLD);
    }
  }
  void printBottomLine(const char *text)
  {
    ssd1306_setColor(RGB_COLOR8(255,255,255));
    if(strlen(text) < screenWidth/8)
    {
      ssd1306_setFixedFont(ssd1306xled_font8x16);
      ssd1306_printFixed8((screenWidth - (strlen(text)*8))/2,  screenHeight - 16, text, STYLE_NORMAL);
    }
    else if(strlen(text) < screenWidth/6)
    {
      ssd1306_setFixedFont(ssd1306xled_font6x8);
      ssd1306_printFixed8((screenWidth - (strlen(text)*6))/2,  screenHeight - 8, text, STYLE_NORMAL);
    }
    else if(strlen(text) < screenWidth/5)
    {
      ssd1306_setFixedFont(ssd1306xled_font5x7);
      ssd1306_printFixed8((screenWidth - (strlen(text)*5))/2,  screenHeight - 7, text, STYLE_NORMAL);
    }
  }
  void displayWelcomeScreen()
  {
    ssd1306_clearScreen8();
    printTopLine((const char*)F("HALE"));
    printMiddleLine((const char*)F("PDT TRACKER"));
    printBottomLine((const char*)F("LIMITED"));
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::welcome;
  }
  void displayDistanceToBeacon()
  {
    ssd1306_clearScreen8();
    printTopLine((const char*)F("RANGE"));
    printBottomLine((const char*)F("METERS"));
    if(device[currentlyTrackedBeacon].hasGpsFix == false || rangeToIndicate(currentlyTrackedBeacon) > maximumEffectiveRange || currentlyTrackedBeacon == maximumNumberOfDevices)
    {
      if(maximumEffectiveRange > 999)
      {
        #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(24))
        {
          SERIAL_DEBUG_PORT.println(F("Displaying range: ----"));
        }
        #endif
        printRange("----");
      }
      else if(maximumEffectiveRange > 99)
      {
        #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(23))
        {
          SERIAL_DEBUG_PORT.println(F("Displaying range: ---"));
        }
        #endif
        printRange("---");
      }
      else
      {
        #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(22))
        {
          SERIAL_DEBUG_PORT.println(F("Displaying range: --"));
        }
        #endif
        printRange("--");
      }
    }
    else
    {
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(28))
        {
          SERIAL_DEBUG_PORT.print(F("Displaying range:"));
          SERIAL_DEBUG_PORT.println(distanceToCurrentBeacon);
          //SERIAL_DEBUG_PORT.print(F(" cardinal:"));
          //SERIAL_DEBUG_PORT.println(TinyGPSPlus::cardinal(device[currentlyTrackedBeacon].course));
        }
      #endif
      if(maximumEffectiveRange > 999)
      {
        char displayText[5];
        sprintf_P(displayText,PSTR("%04u"),distanceToCurrentBeacon);
        printRange(displayText);
      }
      else if(maximumEffectiveRange > 99)
      {
        char displayText[4];
        sprintf_P(displayText,PSTR("%03u"),distanceToCurrentBeacon);
        printRange(displayText);
      }
      else
      {
        char displayText[3];
        sprintf_P(displayText,PSTR("%02u"),distanceToCurrentBeacon);
        printRange(displayText);
      }
    }
    lastDisplayUpdate = millis();
    displayTimeout = longDisplayTimeout; //Long timeout on this display
    currentDisplayState = displayState::distance;
  }
  void displayCourse()
  {
    
    #ifdef SERIAL_DEBUG
      if(waitForBufferSpace(20))
      {
        SERIAL_DEBUG_PORT.print(F("Displaying course: "));
      }
    #endif
    ssd1306_clearScreen8();
    printTopLine((const char*)F("DIRECTION"));
    if(device[currentlyTrackedBeacon].hasGpsFix == false || rangeToIndicate(currentlyTrackedBeacon) > maximumEffectiveRange || currentlyTrackedBeacon == maximumNumberOfDevices)
    {
      printMiddleLine((const char*)F("UNKNOWN"));
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(20))
        {
          SERIAL_DEBUG_PORT.println(F("UNKNOWN"));
        }
      #endif
    }
    else
    {
      if(distanceToCurrentBeacon > 10) //Below 10m direction gets a bit meaningless
      {
        printMiddleLine(TinyGPSPlus::cardinal(device[currentlyTrackedBeacon].course));
        #ifdef SERIAL_DEBUG
          if(waitForBufferSpace(20))
          {
            SERIAL_DEBUG_PORT.println(TinyGPSPlus::cardinal(device[currentlyTrackedBeacon].course));
          }
        #endif
      }
      else
      {
        printMiddleLine((const char*)F("CLOSE"));
        #ifdef SERIAL_DEBUG
          if(waitForBufferSpace(20))
          {
            SERIAL_DEBUG_PORT.println(F("CLOSE"));
          }
        #endif
      }
    }
    printBottomLine((const char*)F("CARDINAL"));
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::course;
  }
  void displayStatus()
  {
    
    #ifdef SERIAL_DEBUG
      if(waitForBufferSpace(20))
      {
        SERIAL_DEBUG_PORT.print(F("Displaying drone status: "));
      }
    #endif
    ssd1306_clearScreen8();
    printTopLine((const char*)F("DRONE"));
    printBottomLine((const char*)F("STATUS"));
    if(device[currentlyTrackedBeacon].numberOfStartingStunHits == 0 || device[currentlyTrackedBeacon].numberOfStartingHits == 0 || device[currentlyTrackedBeacon].hasGpsFix == false || drangeToIndicate(currentlyTrackedBeacon) > maximumEffectiveRange || currentlyTrackedBeacon == maximumNumberOfDevices)
    {
      printMiddleLine((const char*)F("UNKNOWN"));
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(20))
        {
          SERIAL_DEBUG_PORT.println(F("UNKNOWN"));
        }
      #endif
    }
    else
    {
      if(device[currentlyTrackedBeacon].currentNumberOfHits == 0)
      {
        printMiddleLine((const char*)F("DESTROYED"));
        #ifdef SERIAL_DEBUG
          if(waitForBufferSpace(20))
          {
            SERIAL_DEBUG_PORT.println(F("DESTROYED"));
          }
        #endif
      }
      else if(device[currentlyTrackedBeacon].currentNumberOfStunHits == 0)
      {
        printMiddleLine((const char*)F("SHUTDOWN"));
        #ifdef SERIAL_DEBUG
          if(waitForBufferSpace(20))
          {
            SERIAL_DEBUG_PORT.println(F("SHUTDOWN"));
          }
        #endif
      }
      else if(device[currentlyTrackedBeacon].currentNumberOfHits > 0 && device[currentlyTrackedBeacon].currentNumberOfHits == device[currentlyTrackedBeacon].numberOfStartingHits && device[currentlyTrackedBeacon].currentNumberOfStunHits > 0) //Below 10m direction gets a bit meaningless
      {
        printMiddleLine((const char*)F("UNDAMAGED"));
        #ifdef SERIAL_DEBUG
          if(waitForBufferSpace(20))
          {
            SERIAL_DEBUG_PORT.println(F("UNDAMAGED"));
          }
        #endif
      }
      else
      {
        //if(device[currentlyTrackedBeacon].currentNumberOfHits <= device[currentlyTrackedBeacon].currentNumberOfStunHits) //Show damage status
        {
          char displayText[14];
          //sprintf_P(displayText, PSTR("%02u%% DAMAGE"), (100*(uint16_t)(device[currentlyTrackedBeacon].numberOfStartingHits - device[currentlyTrackedBeacon].currentNumberOfHits))/(uint16_t)device[currentlyTrackedBeacon].numberOfStartingHits);
          sprintf_P(displayText, PSTR("%02u%% HEALTH"), (100*(uint16_t)device[currentlyTrackedBeacon].currentNumberOfHits)/(uint16_t)device[currentlyTrackedBeacon].numberOfStartingHits);
          printMiddleLine(displayText);
          #ifdef SERIAL_DEBUG
            if(waitForBufferSpace(20))
            {
              SERIAL_DEBUG_PORT.println(displayText);
            }
          #endif
        }
        /*
        else  //Show stun status
        {
          char displayText[16];
          sprintf_P(displayText, PSTR("%02u%% DISRUPTED"), (100*(uint16_t)(device[currentlyTrackedBeacon].numberOfStartingStunHits - device[currentlyTrackedBeacon].currentNumberOfStunHits))/(uint16_t)device[currentlyTrackedBeacon].numberOfStartingStunHits);
          printMiddleLine(displayText);
          #ifdef SERIAL_DEBUG
            if(waitForBufferSpace(20))
            {
              SERIAL_DEBUG_PORT.println(displayText);
            }
          #endif
        }
        */
      }
    }
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::status;
  }
  void displayAccuracy()
  {
    ssd1306_clearScreen8();
    printTopLine((const char*)F("LOCATION"));
    printBottomLine((const char*)F("ACCURACY"));
    #ifdef SERIAL_DEBUG
      if(waitForBufferSpace(20))
      {
        SERIAL_DEBUG_PORT.print(F("Displaying accuracy: "));
      }
    #endif
    String temp;
    if(currentlyTrackedBeacon == maximumNumberOfDevices || device[0].hdop > device[currentlyTrackedBeacon].hdop) //Show the worst HDOP
    {
      temp = String(hdopDescription(device[0].hdop));
      temp.toUpperCase();
      printMiddleLine(temp.c_str());
    }
    else
    {
      temp = String(hdopDescription(device[currentlyTrackedBeacon].hdop));
      temp.toUpperCase();
      printMiddleLine(temp.c_str());
    }
    #ifdef SERIAL_DEBUG
      if(waitForBufferSpace(temp.length() + 2))
      {
        SERIAL_DEBUG_PORT.println(temp);
      }
    #endif
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::accuracy;
  }
  void displayTrackingMode()
  {
    ssd1306_clearScreen8();
    printTopLine((const char*)F("TRACKING"));
    printBottomLine((const char*)F("HOLD TO CHANGE"));
    #ifdef SERIAL_DEBUG
      if(waitForBufferSpace(28))
      {
        SERIAL_DEBUG_PORT.print(F("Displaying beacon choice: "));
      }
    #endif
    if(currentTrackingMode == trackingMode::nearest)
    {
      printMiddleLine((const char*)F("NEAREST"));
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(9))
        {
          SERIAL_DEBUG_PORT.println(F("Nearest"));
        }
      #endif
    }
    else if(currentTrackingMode == trackingMode::furthest)
    {
      printMiddleLine((const char*)F("FURTHEST"));
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(10))
        {
          SERIAL_DEBUG_PORT.println(F("Furthest"));
        }
      #endif
    }
    else if(currentTrackingMode == trackingMode::fixed)
    {
      //ssd1306_setFixedFont(digital_font5x7_AB);
      if(device[currentlyTrackedBeacon].name != nullptr)
      {
        printMiddleLine(device[currentlyTrackedBeacon].name);
        #ifdef SERIAL_DEBUG
          if(waitForBufferSpace(80))
          {
            SERIAL_DEBUG_PORT.println(device[currentlyTrackedBeacon].name);
          }
        #endif
      }
      else
      {
        char beaconDetail[17];
        sprintf_P(beaconDetail, PSTR("%02u %02X%02X%02X%02X%02X%02X"), currentlyTrackedBeacon, device[currentlyTrackedBeacon].id[0], device[currentlyTrackedBeacon].id[1], device[currentlyTrackedBeacon].id[2], device[currentlyTrackedBeacon].id[3], device[currentlyTrackedBeacon].id[4], device[currentlyTrackedBeacon].id[5]);
        printMiddleLine(beaconDetail);
        #ifdef SERIAL_DEBUG
          if(waitForBufferSpace(80))
          {
            SERIAL_DEBUG_PORT.println(beaconDetail);
          }
        #endif
      }
    }
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::trackingMode;
  }
  void displayBatteryPercentage()
  {
    ssd1306_clearScreen8();
    printTopLine((const char*)F("BATTERY"));
    printBottomLine((const char*)F("CAPACITY"));
    if(device[0].supplyVoltage > chargingVoltage)
    {
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(32))
        {
          SERIAL_DEBUG_PORT.println(F("Displaying battery: charging"));
        }
      #endif
      printMiddleLine((const char*)F("CHARGING"));
    }
    else
    {
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(36))
        {
          SERIAL_DEBUG_PORT.print(F("Displaying battery capacity: "));
          SERIAL_DEBUG_PORT.println(batteryPercentage);
        }
      #endif
      if(batteryPercentage > 99)
      {
        printMiddleLine((const char*)F("FULL"));
      }
      else
      {
        char displayText[4];
        sprintf_P(displayText,PSTR("%02u%%"),batteryPercentage);
        printMiddleLine(displayText);
      }
    }
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::battery;
  }
  void displayVersion()
  {
    ssd1306_clearScreen8();
    printTopLine((const char*)F("SOFTWARE"));
    printBottomLine((const char*)F("VERSION"));
    char displayText[10];
    sprintf_P(displayText,PSTR("v%02u.%02u.%02u"),device[0].majorVersion,device[0].minorVersion,device[0].patchVersion);
    printMiddleLine(displayText);
    #ifdef SERIAL_DEBUG
      if(waitForBufferSpace(32))
      {
        SERIAL_DEBUG_PORT.print(F("Displaying version: "));
        SERIAL_DEBUG_PORT.println(displayText);
      }
    #endif
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::version;
  }
  void displaySignalStrengthFromBeacon()
  {
    ssd1306_clearScreen8();
    printTopLine((const char*)F("SIGNAL"));
    printBottomLine((const char*)F("STRENGTH"));
    if(currentlyTrackedBeacon == maximumNumberOfDevices || device[currentlyTrackedBeacon].hasGpsFix == false || rangeToIndicate(currentlyTrackedBeacon) > maximumEffectiveRange)
    {
      #if defined(SERIAL_DEBUG)
      if(waitForBufferSpace(35))
      {
        SERIAL_DEBUG_PORT.println(F("Displaying signal strength: --"));
      }
      #endif
      printMiddleLine((const char*)F("N/A"));
    }
    else
    {
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(35))
        {
          SERIAL_DEBUG_PORT.print(F("Displaying signal strength: "));
          SERIAL_DEBUG_PORT.println(device[currentlyTrackedBeacon].lastLoRaRssi);
        }
      #endif
      char displayText[11];
      sprintf_P(displayText,PSTR("%03.1fdbM"),device[currentlyTrackedBeacon].lastLoRaRssi);
      printMiddleLine(displayText);
    }
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::signal;
  }
  #ifdef SUPPORT_BEEPER
    void displayBeeperStatus()
    {
      ssd1306_clearScreen8();
      printTopLine((const char*)F("BEEPER"));
      printBottomLine((const char*)F("HOLD TO CHANGE"));
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(30))
        {
          SERIAL_DEBUG_PORT.print(F("Displaying beeper state: "));
          SERIAL_DEBUG_PORT.println(beeperEnabled);
        }
      #endif
      if(beeperEnabled == true)
      {
        printMiddleLine((const char*)F("ENABLED"));
      }
      else
      {
        printMiddleLine((const char*)F("DISABLED"));
      }
      lastDisplayUpdate = millis();
      displayTimeout = shortDisplayTimeout; //Short timeout on this display
      currentDisplayState = displayState::beeper;
    }
  #endif
  void blankDisplay()
  {
    #ifdef SERIAL_DEBUG
      if(waitForBufferSpace(18))
      {
        SERIAL_DEBUG_PORT.println(F("Clearing display"));
      }
    #endif
    ssd1306_clearScreen8();
    lastDisplayUpdate = millis();
    currentDisplayState = displayState::blank;
  }
#endif

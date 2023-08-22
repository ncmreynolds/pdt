/*
 * 
 * This file contains functions related to showing content on the display
 * 
 */
#ifdef SUPPORT_DISPLAY
  void setupScreen()
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
  void printRange(const char *text)
  {
    ssd1306_setColor(RGB_COLOR8(255,0,0));
    //ssd1306_setFixedFont(comic_sans_font24x32_123);
    ssd1306_setFixedFont(courier_new_font11x16_digits);
    ssd1306_printFixed8((screenWidth - (strlen(text)*11))/2,  (screenHeight - 16)/2, text, STYLE_BOLD);
  }
  void printMiddleLine(const char *text)
  {
    ssd1306_setColor(RGB_COLOR8(255,0,0));
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
    printTopLine("HALE");
    printMiddleLine("PDT TRACKER");
    printBottomLine("LIMITED");
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::welcome;
  }
  void displayDistanceToBeacon()
  {
    ssd1306_clearScreen8();
    printTopLine("RANGE");
    printBottomLine("METERS");
    if(device[currentBeacon].hasFix == false || device[currentBeacon].distanceTo > maximumEffectiveRange || currentBeacon == maximumNumberOfDevices)
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
          SERIAL_DEBUG_PORT.print (distanceToCurrentBeacon);
          SERIAL_DEBUG_PORT.print(F(" cardinal:"));
          SERIAL_DEBUG_PORT.println(TinyGPSPlus::cardinal(device[currentBeacon].course));
        }
      #endif
      if(maximumEffectiveRange > 999)
      {
        char displayText[5];
        sprintf(displayText,"%04u",distanceToCurrentBeacon);
        printRange(displayText);
      }
      else if(maximumEffectiveRange > 99)
      {
        char displayText[4];
        sprintf(displayText,"%03u",distanceToCurrentBeacon);
        printRange(displayText);
      }
      else
      {
        char displayText[3];
        sprintf(displayText,"%02u",distanceToCurrentBeacon);
        printRange(displayText);
      }
    }
    lastDisplayUpdate = millis();
    displayTimeout = longDisplayTimeout; //Long timeout on this display
    currentDisplayState = displayState::distance;
  }
  void displayCourse()
  {
    
    ssd1306_clearScreen8();
    printTopLine("DIRECTION");
    if(distanceToCurrentBeacon > 10) //Below 10m direction gets a bit meaningless
    {
      printMiddleLine(TinyGPSPlus::cardinal(device[currentBeacon].course));
    }
    else
    {
      printMiddleLine("CLOSE");
    }
    printBottomLine("CARDINAL");
    #ifdef SERIAL_DEBUG
      if(waitForBufferSpace(20))
      {
        SERIAL_DEBUG_PORT.print(F("Displaying course: "));
        SERIAL_DEBUG_PORT.println(TinyGPSPlus::cardinal(device[currentBeacon].course));
      }
    #endif
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::course;
  }
  void displayAccuracy()
  {
    ssd1306_clearScreen8();
    printTopLine("LOCATION");
    printBottomLine("ACCURACY");
    #ifdef SERIAL_DEBUG
      if(waitForBufferSpace(20))
      {
        SERIAL_DEBUG_PORT.print(F("Displaying accuracy: "));
      }
    #endif
    String temp;
    if(currentBeacon == maximumNumberOfDevices || device[0].hdop > device[currentBeacon].hdop) //Show the worst HDOP
    {
      temp = String(hdopDescription(device[0].hdop));
      temp.toUpperCase();
      printMiddleLine(temp.c_str());
    }
    else
    {
      temp = String(hdopDescription(device[currentBeacon].hdop));
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
    printTopLine("BEACON");
    printBottomLine("HOLD TO CHANGE");
    #ifdef SERIAL_DEBUG
      if(waitForBufferSpace(28))
      {
        SERIAL_DEBUG_PORT.print(F("Displaying beacon choice: "));
      }
    #endif
    if(currentTrackingMode == trackingMode::nearest)
    {
      printMiddleLine("NEAREST");
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(9))
        {
          SERIAL_DEBUG_PORT.println(F("Nearest"));
        }
      #endif
    }
    else if(currentTrackingMode == trackingMode::furthest)
    {
      printMiddleLine("FURTHEST");
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
      if(device[currentBeacon].name != nullptr)
      {
        printMiddleLine(device[currentBeacon].name);
        #ifdef SERIAL_DEBUG
          if(waitForBufferSpace(80))
          {
            SERIAL_DEBUG_PORT.println(device[currentBeacon].name);
          }
        #endif
      }
      else
      {
        char beaconDetail[17];
        sprintf_P(beaconDetail, PSTR("%02u %02X%02X%02X%02X%02X%02X"), currentBeacon, device[currentBeacon].id[0], device[currentBeacon].id[1], device[currentBeacon].id[2], device[currentBeacon].id[3], device[currentBeacon].id[4], device[currentBeacon].id[5]);
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
    printTopLine("BATTERY");
    printBottomLine("CAPACITY");
    if(batteryVoltage > chargingVoltage)
    {
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(32))
        {
          SERIAL_DEBUG_PORT.println(F("Displaying battery: charging"));
        }
      #endif
      printMiddleLine("CHARGING");
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
        printMiddleLine("FULL");
      }
      else
      {
        char displayText[4];
        sprintf(displayText,"%02u%%",batteryPercentage);
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
    printTopLine("SOFTWARE");
    printBottomLine("VERSION");
    char displayText[10];
    sprintf_P(displayText,PSTR("v%02u.%02u.%02u"),majorVersion,minorVersion,patchVersion);
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
    printTopLine("SIGNAL");
    printBottomLine("STRENGTH");
    if(device[currentBeacon].hasFix == false || device[currentBeacon].distanceTo > maximumEffectiveRange)
    {
      if(maximumEffectiveRange > 999)
      {
        #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(35))
        {
          SERIAL_DEBUG_PORT.println(F("Displaying signal strength: ----"));
        }
        #endif
        printMiddleLine("----");
      }
      else if(maximumEffectiveRange > 99)
      {
        #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(35))
        {
          SERIAL_DEBUG_PORT.println(F("Displaying signal strength: ---"));
        }
        #endif
        printMiddleLine("---");
      }
      else
      {
        #if defined(SERIAL_DEBUG)
        if(waitForBufferSpace(35))
        {
          SERIAL_DEBUG_PORT.println(F("Displaying signal strength: --"));
        }
        #endif
        printMiddleLine("--");
      }
    }
    else
    {
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(35))
        {
          SERIAL_DEBUG_PORT.print(F("Displaying signal strength: "));
          SERIAL_DEBUG_PORT.println(lastRssi);
        }
      #endif
      char displayText[11];
      sprintf(displayText,"%03.1fdbM",lastRssi);
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
      printTopLine("BEEPER");
      printBottomLine("HOLD TO CHANGE");
      #ifdef SERIAL_DEBUG
        if(waitForBufferSpace(30))
        {
          SERIAL_DEBUG_PORT.print(F("Displaying beeper state: "));
          SERIAL_DEBUG_PORT.println(beeperEnabled);
        }
      #endif
      if(beeperEnabled == true)
      {
        printMiddleLine("ENABLED");
      }
      else
      {
        printMiddleLine("DISABLED");
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

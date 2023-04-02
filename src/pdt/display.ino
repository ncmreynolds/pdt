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
    if(beacon[0].hasFix == false || beacon[0].distanceTo > maximumEffectiveRange)
    {
      if(maximumEffectiveRange > 999)
      {
        SERIAL_DEBUG_PORT.println(F("Displaying range: ----"));
        printRange("----");
      }
      else if(maximumEffectiveRange > 99)
      {
        SERIAL_DEBUG_PORT.println(F("Displaying range: ---"));
        printRange("---");
      }
      else
      {
        SERIAL_DEBUG_PORT.println(F("Displaying range: --"));
        printRange("--");
      }
    }
    else
    {
      #ifdef SERIAL_DEBUG
        SERIAL_DEBUG_PORT.print(F("Displaying range: "));
        SERIAL_DEBUG_PORT.println(distanceToCurrentBeacon);
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
  void displayBeaconMode()
  {
    ssd1306_clearScreen8();
    printTopLine("BEACON");
    printBottomLine("HOLD TO CHANGE");
    #ifdef SERIAL_DEBUG
      SERIAL_DEBUG_PORT.print(F("Displaying beacon choice: "));
    #endif
    if(currentTrackingMode == trackingMode::nearest)
    {
      printMiddleLine("NEAREST");
      #ifdef SERIAL_DEBUG
        SERIAL_DEBUG_PORT.println(F("Nearest"));
      #endif
    }
    else if(currentTrackingMode == trackingMode::furthest)
    {
      printMiddleLine("FURTHEST");
      #ifdef SERIAL_DEBUG
        SERIAL_DEBUG_PORT.println(F("Furthest"));
      #endif
    }
    else if(currentTrackingMode == trackingMode::fixed)
    {
      ssd1306_setFixedFont(digital_font5x7_AB);
      char beaconDetail[17];
      sprintf(beaconDetail, "%02u %02X%02X%02X%02X%02X%02X", currentBeacon, beacon[currentBeacon].id[0], beacon[currentBeacon].id[1], beacon[currentBeacon].id[2], beacon[currentBeacon].id[3], beacon[currentBeacon].id[4], beacon[currentBeacon].id[5]);
      printMiddleLine(beaconDetail);
      #ifdef SERIAL_DEBUG
        SERIAL_DEBUG_PORT.println(beaconDetail);
      #endif
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
    #ifdef SERIAL_DEBUG
      SERIAL_DEBUG_PORT.print(F("Displaying battery capacity: "));
      SERIAL_DEBUG_PORT.println(batteryPercentage);
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
    lastDisplayUpdate = millis();
    displayTimeout = shortDisplayTimeout; //Short timeout on this display
    currentDisplayState = displayState::battery;
  }
  void displaySignalStrengthFromBeacon()
  {
    ssd1306_clearScreen8();
    printTopLine("SIGNAL");
    printBottomLine("STRENGTH");
    if(beacon[0].hasFix == false || beacon[0].distanceTo > maximumEffectiveRange)
    {
      if(maximumEffectiveRange > 999)
      {
        SERIAL_DEBUG_PORT.println(F("Displaying signal strength: ----"));
        printMiddleLine("----");
      }
      else if(maximumEffectiveRange > 99)
      {
        SERIAL_DEBUG_PORT.println(F("Displaying signal strength: ---"));
        printMiddleLine("---");
      }
      else
      {
        SERIAL_DEBUG_PORT.println(F("Displaying signal strength: --"));
        printMiddleLine("--");
      }
    }
    else
    {
      #ifdef SERIAL_DEBUG
        SERIAL_DEBUG_PORT.print(F("Displaying signal strength: "));
        SERIAL_DEBUG_PORT.println(lastRssi);
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
        SERIAL_DEBUG_PORT.print(F("Displaying beeper state: "));
        SERIAL_DEBUG_PORT.println(beeperEnabled);
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
      SERIAL_DEBUG_PORT.println(F("Clearing display"));
    #endif
    ssd1306_clearScreen8();
    lastDisplayUpdate = millis();
    currentDisplayState = displayState::blank;
  }
#endif

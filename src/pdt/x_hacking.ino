#if defined(SUPPORT_HACKING)
  void setupHacking()
  {
    localLogLn(F("Starting hacking game"));
    game.debug(Serial);
    game.type(ESPUIgames::gameType::whackamole);
    game.setLength(gameLength);
    game.setMaximumAttempts(gameRetries);
    game.setGameSpeedup(gameSpeedup);
    //Game tab
    game.setTitle(device[0].name);
    game.setTabTitle("Onboard firewall");
    game.enableStartSwitch("Hack!");
    game.setWinContent("Security breached!","Check the files and controls in the new tabs."); //Setting this makes a widget pop up if you win, not setting it relies on colours alone to show it
    game.setLoseContent("Hack failed!","Flick the switch to try again","No further hack attempts possible"); //Setting this makes a widget pop up if you lose, not setting it relies on colours alone to show it
    game.addPlayButton("Core Operating system", "Hack", ControlColor::Peterriver);
    game.addPlayButton("Secure storage", "Hack", ControlColor::Sunflower);
    game.addPlayButton("System bus", "Hack", ControlColor::Turquoise);
    game.addPlayButton("Watchdog Daemon", "Hack", ControlColor::Carrot);
    game.addGameTab(); //Builds all the game controls
    //Our own control tab
    SERIAL_DEBUG_PORT.println(F("Adding files tab"));
    controlsTabID = ESPUI.addControl(ControlType::Tab, "Drone control", "Drone control");
    statusWidgetID = ESPUI.addControl( ControlType::Label, "System status", "Running", ControlColor::Emerald, controlsTabID);
    //Shutdown
    //shutdownButtonID = ESPUI.addControl(ControlType::Button, "Shutdown", "Shutdown", ControlColor::Emerald, controlsTabID, &shutdownButtonCallback);
    //ESPUI.setPanelWide(shutdownButtonID, true);
    //Self destruct
    selfDestructButtonID = ESPUI.addControl(ControlType::Button, "Self Destruct", "Destruct", ControlColor::Carrot, controlsTabID, &selfDestructButtonCallback);
    ESPUI.setPanelWide(selfDestructButtonID, true);
    ESPUI.setPanelWide(statusWidgetID, true);
    ESPUI.updateVisibility(controlsTabID, false);
    #if defined(ALLOW_VIEW_FILES) || defined(ALLOW_DOWNLOAD_FILES)
      localLog(F("Starting filesystem: "));
      if (!LittleFS.begin())
      {
        localLogLn(F("LittleFS mount failed"));
        return;
      }
      else
      {
        localLogLn(F("Little FS Mounted Successfully"));
      }
      //Files tab
      addFilesTab();
    #endif
    //Help tab
    game.setHelpTabTitle("OC: How to hack");
    game.setHelpContent("How to play this game","Toggle the 'Hack!' switch to start the game.<br /><br />Every time a button lights, press it. If you react fast enough for long enough you will win.<br /><br />When you win all the controls turn green.<br /><br />If you don't hit all the buttons fast enough all the controls turn red and it will tell you.<br /><br />You can restart the hack by flicking the 'Hack!' switch and it will start from scratch but you only get <b>three attempts</b> and the game will get harder.<br /><br />After you have done what you want if you flick the 'Hack' button to the off position, it clears up so it looks like you were never there and disconnects you.<br /><br />Please remember to disconnect after the hack otherwise you won't be able to receive incoming video calls.");
    game.addHelpTab(); //Builds all the help controls
    ESPUI.begin(game.title());  //ESPUI is started from the sketch in case you want to add your own controls and pages as well before starting ESPUI
    #if defined(ALLOW_VIEW_FILES) || defined(ALLOW_DOWNLOAD_FILES)
      //Add things served directly by ESPAsyncWebserver
      ESPUI.server->serveStatic(filesRoot, LittleFS, filesRoot);
    #endif
  }
  void manageGame()
  {
    game.runFsm();  //Run the game finite state machine
    if(game.won())
    {
      #if defined(ALLOW_VIEW_FILES) || defined(ALLOW_DOWNLOAD_FILES)
      if(filesVisible == false)
      {
        hackComplete = true;
        filesVisible = true;
        ESPUI.updateVisibility(filesTabID, true);
        ESPUI.updateVisibility(controlsTabID, true);
      }
      #endif
    }
    else if(hackComplete == true)
    {
      hackComplete = false;
      turnOffWifiSoon = millis();
      #if defined(ALLOW_VIEW_FILES) || defined(ALLOW_DOWNLOAD_FILES)
      Serial.println(F("Hack complete, hiding files again"));
      ESPUI.updateVisibility(filesTabID, false);
      ESPUI.updateVisibility(controlsTabID, false);
      #endif
    }
    if(turnOffWifiSoon !=0 && millis() - turnOffWifiSoon > 3e3)
    {
      turnOffWifiSoon = 0;
      Serial.println(F("Wifi Off"));
      WiFi.mode(WIFI_OFF);
    }
    /*
    if(shutdownNow !=0)
    {
      if(millis() - shutdownNow > 10000)
      {
        shutdownNow = 0;
        device[0].currentNumberOfStunHits = 0;
        currentSensorState = sensorState::stunned;
        sensorPersitentData.putUChar(currentStunKey, device[0].currentNumberOfStunHits);
        lastSensorStateChange = millis();
        localLogLn(F("Drone shutdown by hacker"));
        sensor.pause();
        #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
          xTaskCreate(playDeadAnimation, "playDeadAnimation", 512, NULL, 2, NULL);
        #endif
        localLogLn(F("Disabling WiFi"));
        //WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
      }
      else if((millis() - shutdownNow)/1000 != shutdownCountdown)
      {
        shutdownCountdown = (millis() - shutdownNow)/1000;
        ESPUI.updateControlValue(statusWidgetID, "Shutdown in " + String(10-shutdownCountdown) + "s");
        ESPUI.updateControl( statusWidgetID );
      }
    }
    */
    if(selfDestructNow !=0)
    {
      if(millis() - selfDestructNow > 10000)
      {
        selfDestructNow = 0;
        device[0].currentNumberOfHits = 0;
        bleedOutCounter = 0;
        currentSensorState = sensorState::dead;
        sensorPersitentData.putUChar(currentHitsKey, device[0].currentNumberOfHits);
        sensorPersitentData.putUChar(bleedOutCounterKey, bleedOutCounter);
        lastSensorStateChange = millis();
        localLogLn(F("Drone destroyed by hacker"));
        sensor.pause();
        #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
          xTaskCreate(playDeadAnimation, "playDeadAnimation", 512, NULL, 2, NULL);
        #endif
        WiFi.mode(WIFI_OFF);
      }
      else if((millis() - selfDestructNow)/1000 != selfDestructCountdown)
      {
        selfDestructCountdown = (millis() - selfDestructNow)/1000;
        ESPUI.updateControlValue(statusWidgetID, "Self Destruct in " + String(10-selfDestructCountdown) + "s");
        ESPUI.updateControl( statusWidgetID );
      }
    }
  }
  uint16_t countFiles(const char *location)
  {
    uint16_t count = 0;
    File root = LittleFS.open(location);
    File file = root.openNextFile();
    while(file)
    {
      if(file.isDirectory())
      {
        count+=countFiles(file.path());
      }
      else
      {
        count++;
      }
      file = root.openNextFile();
    }
    return count;
  }
  void addFilesTab()
  {
    localLog(F("Adding files tab with "));
    numberOfFiles = countFiles(filesRoot);
    localLog(numberOfFiles);
    localLogLn(F(" files"));
    fileLabel = new char*[numberOfFiles];
    #if defined (ALLOW_VIEW_FILES)
      viewControlID = new uint16_t[numberOfFiles];
      fileViewLink = new char*[numberOfFiles];
    #endif
    #if defined (ALLOW_DOWNLOAD_FILES)
      downloadControlID = new uint16_t[numberOfFiles];
      fileDownloadLink = new char*[numberOfFiles];
    #endif
    filesTabID = ESPUI.addControl(ControlType::Tab, filesTabTitle, filesTabTitle);
    //Iterate all the files
    uint8_t index = 0;
    iterateFiles(filesRoot, index);
    #if !defined(ENABLE_HACKING_TESTING)
      ESPUI.updateVisibility(filesTabID, false);
      ESPUI.updateVisibility(controlsTabID, false);
    #endif
  }
  void iterateFiles(const char *location, uint8_t &index)
  {
    File root = LittleFS.open(location);
    File file = root.openNextFile();
    while(file)
    {
      if(file.isDirectory())
      {
        localLog(F("Directory: "));
        localLogLn(file.path());
        iterateFiles(file.path(), index);
      }
      else
      {
        fileLabel[index] = new char[strlen(file.name())+strlen(location)+strlen(fileLabelTemplate)+1];
        sprintf_P(fileLabel[index], fileLabelTemplate, location, file.name());
        #if defined(ALLOW_VIEW_FILES)
          fileViewLink[index] = new char[strlen(file.name())+strlen(location)+strlen(fileViewLinkTemplate)+1];
          sprintf_P(fileViewLink[index], fileViewLinkTemplate, location, file.name());
          //Serial.printf_P(PSTR("Adding file view %u:\"%s\" Label:\"%s\" Link:\"%s\"\r\n"), index, file.name().c_str(), fileLabel[index], fileViewLink[index]);
        #endif
        #if defined(ALLOW_DOWNLOAD_FILES)
          fileDownloadLink[index] = new char[strlen(file.name())+strlen(location)+strlen(fileDownloadLinkTemplate)+1];
          sprintf_P(fileDownloadLink[index], fileDownloadLinkTemplate, location, file.name());
          //Serial.printf_P(PSTR("Adding file download %u:\"%s\" Label:\"%s\" Link:\"%s\"\r\n"), index, file.name().c_str(), fileLabel[index], fileDownloadLink[index]);
        #endif
        #if defined(ALLOW_VIEW_FILES) && defined(ALLOW_DOWNLOAD_FILES)
          viewControlID[index] = ESPUI.addControl(ControlType::Label, fileLabel[index], fileViewLink[index], ControlColor::Wetasphalt, filesTabID);
          downloadControlID[index] = ESPUI.addControl(ControlType::Label, fileLabel[index], fileDownloadLink[index], ControlColor::Wetasphalt, viewControlID[index]);
        #elif defined(ALLOW_VIEW_FILES)
          viewControlID[index] = ESPUI.addControl(ControlType::Label, fileLabel[index], fileViewLink[index], ControlColor::Wetasphalt, filesTabID);
          ESPUI.setPanelWide(viewControlID[index], true);
        #elif defined(ALLOW_DOWNLOAD_FILES)
          downloadControlID[index] = ESPUI.addControl(ControlType::Label, fileLabel[index], fileDownloadLink[index], ControlColor::Wetasphalt, filesTabID);
          ESPUI.setPanelWide(downloadControlID[index], true);
        #endif
        index++;
      }
      file = root.openNextFile();
    }
  }
#endif

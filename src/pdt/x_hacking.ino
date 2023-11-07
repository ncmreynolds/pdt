#ifdef SUPPORT_HACKING
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
    Serial.println(F("Adding files tab"));
    controlsTabID = ESPUI.addControl(ControlType::Tab, "Drone control", "Drone control");
    filesTabID = ESPUI.addControl(ControlType::Tab, "Local files", "Local files");
    statusWidgetID = ESPUI.addControl( ControlType::Label, "System status", "Running", ControlColor::Emerald, controlsTabID);
    //shutdownButtonID = ESPUI.addControl(ControlType::Button, "Shutdown", "Shutdown", ControlColor::Emerald, controlsTabID, &shutdownButtonCallback);
    selfDestructButtonID = ESPUI.addControl(ControlType::Button, "Shutdown", "Shutdown", ControlColor::Carrot, controlsTabID, &selfDestructButtonCallback);
    ESPUI.setPanelWide(statusWidgetID, true);
    //ESPUI.setPanelWide(shutdownButtonID, true);
    ESPUI.setPanelWide(selfDestructButtonID, true);
    ESPUI.addControl(ControlType::Label, "Project Waterfall Dossier", "This section contains a large chunk of data on Project Waterfall, which you have downloaded to your Cyberdeck.", ControlColor::Wetasphalt, filesTabID);
    ESPUI.updateVisibility(controlsTabID, false);
    ESPUI.updateVisibility(filesTabID, false);

    //Help tab
    game.setHelpTabTitle("OC: How to hack");
    game.setHelpContent("How to play this game","Toggle the 'Hack!' switch to start the game.<br /><br />Every time a button lights, press it. If you react fast enough for long enough you will win.<br /><br />When you win all the controls turn green.<br /><br />If you don't hit all the buttons fast enough all the controls turn red and it will tell you.<br /><br />You can restart the hack by flicking the 'Hack!' switch and it will start from scratch but you only get <b>three attempts</b> and the game will get harder.");
    game.addHelpTab(); //Builds all the help controls
    ESPUI.begin(game.title());  //ESPUI is started from the sketch in case you want to add your own controls and pages as well before starting ESPUI
  }
  void manageGame()
  {
    game.runFsm();  //Run the game finite state machine
    if(game.won())
    {
      if(filesTabVisible == false)
      {
        filesTabVisible = true;
        ESPUI.updateVisibility(controlsTabID, true);
        ESPUI.updateVisibility(filesTabID, true);
      }
    }
    /*
    if(shutdownNow !=0)
    {
      if(millis() - shutdownNow > 10000)
      {
        shutdownNow = 0;
        device[0].currentNumberOfStunHits = 0;
        #ifdef SUPPORT_LORA
          scheduleDeviceInfoShareSoon(); //Force the sensor to update any trackers soon
        #endif
        currentSensorState = sensorState::stunned;
        sensorPersitentData.putUChar(currentStunKey, device[0].currentNumberOfStunHits);
        lastSensorStateChange = millis();
        localLogLn(F("Drone shutdown by hacker"));
        sensor.pause();
        #if defined(SUPPORT_BEEPER) || defined(SUPPORT_LED)
          xTaskCreate(playDeadAnimation, "playDeadAnimation", 512, NULL, 2, NULL);
        #endif
        #ifdef SUPPORT_LORA
          scheduleDeviceInfoShareSoon(); //Force the sensor to update any trackers soon
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
        #ifdef SUPPORT_LORA
          scheduleDeviceInfoShareSoon(); //Force the sensor to update any trackers soon
        #endif
        WiFi.mode(WIFI_OFF);
      }
      else if((millis() - selfDestructNow)/1000 != selfDestructCountdown)
      {
        selfDestructCountdown = (millis() - selfDestructNow)/1000;
        ESPUI.updateControlValue(statusWidgetID, "Shutdown in " + String(10-selfDestructCountdown) + "s");
        ESPUI.updateControl( statusWidgetID );
      }
    }
  }
#endif

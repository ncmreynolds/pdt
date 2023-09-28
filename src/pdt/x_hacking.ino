#ifdef SUPPORT_HACKING
  void setupHacking()
  {
    localLogLn(F("Starting hacking game"));
    game.debug(Serial);
    game.type(ESPUIgames::gameType::simon);
    //Game tab
    game.setTitle(device[0].name);
    game.setTabTitle("Onboard firewall");
    game.enableStartSwitch("Hack!");
    game.setWinContent("Security breached","See the power controls in the new tab."); //Setting this makes a widget pop up if you win, not setting it relies on colours alone to show it
    game.setLoseContent("Hack failed!","Flick the switch to try again","No further hack attempts possible"); //Setting this makes a widget pop up if you lose, not setting it relies on colours alone to show it
    game.addPlayButton("Core Operating system", "Hack", ControlColor::Peterriver);
    game.addPlayButton("Secure storage", "Hack", ControlColor::Sunflower);
    game.addPlayButton("System bus", "Hack", ControlColor::Turquoise);
    game.addPlayButton("Watchdog Daemon", "Hack", ControlColor::Carrot);
    game.addGameTab(); //Builds all the game controls
    //Help tab
    game.setHelpTabTitle("This is the help");
    game.setHelpContent("How to play this game","Toggle the 'start the game' switch to start the game.<br /><br />Repeat the sequence of buttons back on the screen as it happens, eventually you will win.<br /><br />When you win all the controls turn green.<br /><br />If you get something wrong, all the controls turn red.<br /><br />You can restart the game by flicking the 'Start the game' switch.");
    game.addHelpTab(); //Builds all the help controls
    ESPUI.begin(game.title());  //ESPUI is started from the sketch in case you want to add your own controls and pages as well before starting ESPUI
  }
#endif

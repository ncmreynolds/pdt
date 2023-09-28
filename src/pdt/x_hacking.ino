#ifdef SUPPORT_HACKING
  void setupHacking()
  {
    game.debug(Serial);
    game.type(ESPUIgames::gameType::simon);
    //Game tab
    game.setTabTitle("This is the game");
    game.setTitle("Simon says!");
    game.enableStartSwitch("Start the game");
    game.setWinContent("You have won!","Congratulations, you have won."); //Setting this makes a widget pop up if you win, not setting it relies on colours alone to show it
    game.setLoseContent("Sorry, you lose!","Flick the switch to try again"); //Setting this makes a widget pop up if you lose, not setting it relies on colours alone to show it
    game.addGameTab(); //Builds all the game controls
    //Help tab
    game.setHelpTabTitle("This is the help");
    game.setHelpContent("How to play this game","Toggle the 'start the game' switch to start the game.<br /><br />Repeat the sequence of buttons back on the screen as it happens, eventually you will win.<br /><br />When you win all the controls turn green.<br /><br />If you get something wrong, all the controls turn red.<br /><br />You can restart the game by flicking the 'Start the game' switch.");
    game.addHelpTab(); //Builds all the help controls
    ESPUI.begin(game.title());  //ESPUI is started from the sketch in case you want to add your own controls and pages as well before starting ESPUI
  }
#endif

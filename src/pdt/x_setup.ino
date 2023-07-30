/*
 *
 * This file contains functions related to initial setup
 * 
 */
void setup() {
  WiFi.macAddress(device[0].id); //Copy in local MAC address as 'device 0'
  #ifdef SUPPORT_GPS
    numberOfDevices++;
    #ifdef ACT_AS_TRACKER
      device[0].typeOfDevice = 1;
      pinMode(buttonPin, INPUT_PULLUP);
    #endif
  #endif
  #if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
    setupLogging();
    showStartupInfo();
  #endif
  //Mount file system so configuration can be read
  setupFilesystem();
  //Set the name in the device record
  device[0].name = new char[strlen(nodeName) + 1];
  strcpy(device[0].name, nodeName);
  //Use preferences storage for temporary 'persistent' values when running as a sensor
  #if defined(ACT_AS_SENSOR)
    sensorPersitentData.begin("sensor", false); 
  #endif
  #ifdef SUPPORT_WIFI
    setupNetwork();
  #endif
  if(networkConnected == true)
  {
    showNetworkInformation();
    networkStateChanged = false; //Suppress normal handling of network state changes
  }
  #if defined(ENABLE_LOCAL_WEBSERVER)
    setupWebServer(); //Set up the web server callbacks which are used to get hold of logs
  #endif
  #if defined(ENABLE_OTA_UPDATE)
    configureOTA(); //Set up OTA callbacks
  #endif
  #ifdef SUPPORT_LORA
    setupLoRa();  //Needs to be before SPI display
  #endif
  #ifdef SUPPORT_DISPLAY
    setupScreen();  //Needs to be after LoRa
  #endif
  #ifdef SUPPORT_GPS
    setupGps();
  #endif
  #ifdef SUPPORT_BEEPER
    setupBeeper();
  #endif
  #ifdef SUPPORT_LED
    setupLed();
  #endif
  #ifdef SUPPORT_BATTERY_METER
    setupBattery();
  #endif
  #if defined(ACT_AS_SENSOR)
    setupLasertag();
  #endif
  #if defined(ACT_AS_SENSOR)
    #ifdef SUPPORT_BUTTON
    if(digitalRead(buttonPin) == false)
    {
      localLogLn(F("Resetting sensor due to button press"));
      currentNumberOfHits = numberOfStartingHits;
      currentNumberOfStunHits = numberOfStartingStunHits;
      previousSensorState = sensorState::starting;
      bleedOutCounter = 0;
      sensorPersitentData.putUChar(currentNumberOfHitsKey, currentNumberOfHits);
      sensorPersitentData.putUChar(currentNumberOfStunHitsKey, currentNumberOfStunHits);
      sensorPersitentData.putUChar(bleedOutCounterKey, bleedOutCounter);
      //sensorPersitentData.putUChar(currentSensorStateKey, (uint8_t)currentSensorState);
    }
    else
    #endif
    {
      currentNumberOfHits = sensorPersitentData.getUChar(currentNumberOfHitsKey, numberOfStartingHits);
      currentNumberOfStunHits = sensorPersitentData.getUChar(currentNumberOfStunHitsKey, numberOfStartingStunHits);
      //previousSensorState = (sensorState)sensorPersitentData.getUChar(currentSensorStateKey, (uint8_t)sensorState::playStartupAnimation);
      bleedOutCounter = sensorPersitentData.getUChar(bleedOutCounterKey, 0);
    }
    localLog(F("Starting hits: "));
    localLogLn(currentNumberOfHits);
    localLog(F("Starting stun hits: "));
    localLogLn(currentNumberOfStunHits);
    localLog(F("Previous sensor state: "));
    localLogLn((uint8_t)currentSensorState);
    localLog(F("Starting bleed out counter: "));
    localLogLn(bleedOutCounter);
  #endif
}

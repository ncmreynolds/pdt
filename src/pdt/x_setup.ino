/*
 *
 * This file contains functions related to initial setup
 * 
 */
void setup() {
  //The very first thing to do is set up GPIO, as some things need pulling up/down ASAP
  #ifdef SUPPORT_SOFT_POWER_OFF
    setupSoftPowerOff();
  #endif
  #ifdef SUPPORT_SOFT_PERIPHERAL_POWER_OFF
    setupPeripheralPowerOff();
    peripheralPowerOn();
  #endif
  #ifdef SUPPORT_VIBRATION
    setupVibration();
  #endif
  #ifdef SUPPORT_LED
    setupLed();
    #if HARDWARE_VARIANT == C3LoRaBeacon
      ledSlowBlink();
    #endif
  #endif
  #ifdef SUPPORT_BEEPER
    setupBeeper();
  #endif
  #ifdef SUPPORT_BUTTON
    setupButton();
    #if defined(ACT_AS_SENSOR)
      if(digitalRead(buttonPin) == false)
      {
        sensorReset = true;
      }
    #endif
  #endif
  WiFi.macAddress(device[0].id); //Copy in local MAC address as 'device 0'
  #ifdef ACT_AS_TRACKER
    device[0].typeOfDevice = device[0].typeOfDevice | 1;  //Mark it as a tracker
  #endif
  #ifdef ACT_AS_SENSOR
    device[0].typeOfDevice = device[0].typeOfDevice | 2; //Mark it as a sensor
  #endif
  #ifdef SUPPORT_FTM
    device[0].typeOfDevice = device[0].typeOfDevice | 8;  //Mark it as a tracker
  #endif
  device[0].majorVersion = PDT_MAJOR_VERSION;
  device[0].minorVersion = PDT_MINOR_VERSION;
  device[0].patchVersion = PDT_PATCH_VERSION;
  numberOfDevices++;
  #if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
    setupLogging();
    showStartupInfo();
  #endif
  //Mount file system so configuration can be read
  setupFilesystem();
  //Use preferences storage for temporary 'persistent' values when running as a sensor
  #if defined(ACT_AS_SENSOR)
    sensorPersitentData.begin("sensor", false); 
    loadSensorConfiguration();
    #ifdef SUPPORT_BUTTON
      if(sensorReset == true)
      {
        localLogLn(F("Resetting sensor due to button press"));
        resetSensor();
      }
    #endif
    showSensorConfiguration();
  #endif
  #ifdef SUPPORT_WIFI
    setupNetwork();
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
  #ifdef SUPPORT_BATTERY_METER
    setupBattery();
  #endif
  #if defined(ACT_AS_SENSOR)
    setupLasertag();
  #endif
}

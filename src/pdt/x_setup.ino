/*
 *
 * This file contains functions related to initial setup
 * 
 */
void setup() {
  //The very first thing to do is set up GPIO, as some things need pulling up/down ASAP
  #if defined(SUPPORT_SOFT_POWER_OFF)
    setupSoftPowerOff();
  #endif
  #if defined(SUPPORT_SOFT_PERIPHERAL_POWER_OFF)
    setupPeripheralPowerOff();
    peripheralPowerOn();
  #endif
  #if defined(SUPPORT_VIBRATION)
    setupVibration();
  #endif
  #if defined(SUPPORT_LED)
    setupLed();
    #if HARDWARE_VARIANT == C3LoRaBeacon
      ledSlowBlink();
    #endif
  #endif
  #if defined(SUPPORT_BEEPER)
    setupBeeper();
  #endif
  #if defined(SUPPORT_BUTTON)
    setupButton();
    #if defined(ACT_AS_SENSOR)
      if(digitalRead(buttonPin) == false)
      {
        sensorReset = true;
      }
    #endif
  #endif
  WiFi.macAddress(localMacAddress); //Copy in local MAC address
  #if defined(ACT_AS_TRACKER)
    device[0].typeOfDevice = device[0].typeOfDevice | 0x01;  //Mark it as a tracker
  #endif
  #if defined(ACT_AS_SENSOR)
    device[0].typeOfDevice = device[0].typeOfDevice | 0x02; //Mark it as a sensor
  #endif
  #if defined(SUPPORT_FTM)
    if(ftmEnabled == true)
    {
      device[0].typeOfDevice = device[0].typeOfDevice | 0x08;  //Mark it as an FTM device
    }
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
    #if defined(SUPPORT_BUTTON)
      if(sensorReset == true)
      {
        localLogLn(F("Resetting sensor due to button press"));
        resetSensor();
      }
    #endif
    showSensorConfiguration();
  #endif
  #if defined(ACT_AS_BEACON)
    if(deviceUsuallyStatic)
    {
      device[0].typeOfDevice = device[0].typeOfDevice | 0x80; //Mark it as usually static
    }
  #endif
  #if defined(SUPPORT_WIFI)
    setupNetwork();
  #endif
  #if defined(SUPPORT_DISPLAY)
    setupTreacle();
    setupDisplay();  //Needs to be after Treacle!
  #elif defined(SUPPORT_LVGL)
    setupLvgl();    //Need to be before Treacle!
    setupTreacle();
  #else
    setupTreacle();
  #endif
  #if defined(SUPPORT_GPS)
    setupGps();
  #endif
  #if defined(SUPPORT_BATTERY_METER)
    setupBattery();
    if(enableBatteryMonitor == true)
    {
      device[0].typeOfDevice = device[0].typeOfDevice | 0x40;  //Mark it as monitoring the battery
    }
  #endif
  #if defined(ACT_AS_SENSOR)
    setupLasertag();
  #endif
}

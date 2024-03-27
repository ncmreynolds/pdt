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
  #if defined(SUPPORT_ESPNOW) || defined(SUPPORT_LORA)
    WiFi.macAddress(device[0].id); //Copy in local MAC address as 'device 0'
  #elif defined(SUPPORT_TREACLE)
    WiFi.macAddress(localMacAddress); //Copy in local MAC address
  #endif
  #if defined(ACT_AS_TRACKER)
    device[0].typeOfDevice = device[0].typeOfDevice | 1;  //Mark it as a tracker
  #endif
  #if defined(ACT_AS_SENSOR)
    device[0].typeOfDevice = device[0].typeOfDevice | 2; //Mark it as a sensor
  #endif
  #if defined(SUPPORT_FTM)
    if(ftmEnabled == true)
    {
      device[0].typeOfDevice = device[0].typeOfDevice | 8;  //Mark it as an FTM device
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
  #if defined(SUPPORT_WIFI)
    setupNetwork();
  #endif
  #if defined(SUPPORT_ESPNOW)
    setupEspNow();
  #endif
  #if defined(SUPPORT_DISPLAY) && defined(SUPPORT_LORA)
    setupLoRa();  //Needs to be before SPI display!
  #endif
  #if defined(SUPPORT_DISPLAY) && defined(SUPPORT_TREACLE)
    setupTreacle();
  #endif
  #if defined(SUPPORT_DISPLAY)
    setupDisplay();  //Needs to be after LoRa
  #endif
  #if defined(SUPPORT_LVGL)
    setupLvgl();
  #endif
  #if defined(SUPPORT_LVGL) && defined(SUPPORT_LORA)
    setupLoRa();  //Needs to be after LVGL and touchscreen!
  #elif defined(SUPPORT_LORA)
    setupLoRa();  //Here's as good a place as any to start LoRa with no other worries about which library starts SPI
  #endif
  #if defined(SUPPORT_LVGL) && defined(SUPPORT_TREACLE)
    setupTreacle();
  #elif defined(SUPPORT_TREACLE)
    setupTreacle();
  #endif
  #if defined(SUPPORT_GPS)
    setupGps();
  #endif
  #if defined(SUPPORT_BATTERY_METER)
    setupBattery();
  #endif
  #if defined(ACT_AS_SENSOR)
    setupLasertag();
  #endif
}

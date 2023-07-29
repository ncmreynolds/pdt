/*
 * 
 * This file contains related to Over-The-Air software updates
 * 
 */
#ifdef ENABLE_LOCAL_WEBSERVER
  #if defined(ENABLE_OTA_UPDATE)
    void configureOTA()
    {
      localLog(F("Configuring OTA updates: "));
      ArduinoOTA.setHostname(nodeName);  //This hostname appears in the list of devices in the IDE, so is used to differentiate between devices
      if(otaAuthenticationEnabled == true)
      {
        ArduinoOTA.setPassword(http_password);  //Sets the OTA password
      }
      ArduinoOTA.setRebootOnSuccess(false); //Leaves it to the sketch to reboot the device, instead of it happening by default
      ArduinoOTA.onStart([]() { //This Lambda function is called at the start of an OTA update
        otaInProgress = true;
        localLogLn(F("Over-the-air software update started"));
        flushLog();
        #if defined(USE_LITTLEFS)
          LittleFS.end(); //Must stop littleFS to start OTA update
          filesystemMounted = false;
        #endif
      });
      ArduinoOTA.onEnd([]() { //This lambda function is called when OTA is complete, immediately before the reboot
        otaInProgress = false;
        if(filesystemMounted == false)
        {
          mountFilesystem(false); //Don't show filesystem info on mount
        }
        localLogLn(F("Over-the-air software update complete, restarting shortly"));
        restartTimer = millis();
      });
      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { //This lambda function is called during the OTA process to provide feedback
        uint8_t currentProgress = (progress / (total / 100));
        if(currentProgress > otaProgress + 9)
        {
          otaProgress = currentProgress;
          localLog(F("OTA Progress: "));
          localLog(currentProgress);
          localLogLn('%');
        }
      });
      ArduinoOTA.onError([](ota_error_t error) {  //This lambda function is called when an OTA error occurs
        otaInProgress = false;
        if(filesystemMounted == false)
        {
          mountFilesystem(false); //Don't show filesystem info on mount
        }
        localLog(F("Error["));
        localLog(error);
        localLog(F("]: "));
        if (error == OTA_AUTH_ERROR) {
          localLogLn(F("OTA Auth Failed"));
        } else if
        (error == OTA_BEGIN_ERROR) {
          otaInProgress = false;
          localLogLn(F("OTA Begin failed"));
        }
        else if
        (error == OTA_CONNECT_ERROR) {
          otaInProgress = false;
          localLogLn(F("OTA connect failed"));
        }
        else if (error == OTA_RECEIVE_ERROR) {
          otaInProgress = false;
          localLogLn(F("OTA receive failed"));
        }
        else if (error == OTA_END_ERROR) {
          otaInProgress = false;
          localLogLn(F("OTA end failed"));
        }
        flushLog();
      });
      ArduinoOTA.begin();
      localLogLn(F("OK"));
    }
  #endif
#endif

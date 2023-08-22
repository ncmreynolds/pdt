/*
 * 
 * This file contains code related to local logging, which is written to the console and local storage
 * 
 * The functions are templated so you can print most types to the log with no fuss. Adding a printf equivalent is on my to-do list
 * 
 */
#if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
void setupLogging()
{
  #ifdef USE_RTOS
    loggingSemaphore = xSemaphoreCreateBinary();
  #endif
  #if defined(SERIAL_DEBUG) || defined(SERIAL_LOG)
    SERIAL_DEBUG_PORT.begin();
    debugPortStartingBufferSize = SERIAL_DEBUG_PORT.availableForWrite();
    SERIAL_DEBUG_PORT.println(F("Online?"));  //Try to send something
    delay(100); //Wait for it to send
    debugPortAvailable = SERIAL_DEBUG_PORT.availableForWrite() == debugPortStartingBufferSize; //Check if USB port is connected by seeing if it has been able to send stuff
    if(debugPortAvailable)
    {
        delay(5000);  //Allow time for the serial console to be opened by a person
    }
  #endif
  loggingBuffer.reserve(loggingBufferSize); //Reserve heap for the logging backlog
  #ifdef USE_RTOS
    xSemaphoreGive(loggingSemaphore);
    xTaskCreate(manageLogging, "manageLogging", 10000, NULL, configMAX_PRIORITIES - 1, &loggingManagementTask);
  #endif
}
#endif

#ifdef USE_RTOS
void manageLogging(void * parameter)
#else
void manageLogging()
#endif
{
  #ifdef USE_RTOS
  while(true)
  {
    if(xSemaphoreTake(loggingSemaphore, loggingSemaphoreTimeout))
    {
  #endif
  if(flushLogNow == true || ((millis() - logLastFlushed) >= (logFlushInterval * 1000)))
  {
    logLastFlushed = millis();
    flushLogNow = false;
    #ifdef SERIAL_LOG
      if(waitForBufferSpace(18))
      {
        SERIAL_DEBUG_PORT.println(F("PERIODIC LOG FLUSH"));
      }
    #endif
    if(loggingBuffer.length() > 0)
    {
      flushLog();
    }
    else
    {
      #ifdef SERIAL_LOG
        if(waitForBufferSpace(30))
        {
          SERIAL_DEBUG_PORT.println(F("LOG BUFFER SUSPICIOUSLY EMPTY"));
        }
      #endif
    }
  }
  #ifdef USE_RTOS
      xSemaphoreGive(loggingSemaphore);
    }
    vTaskDelay(loggingYieldTime / portTICK_PERIOD_MS); //Hand back for 10ms
  }
  vTaskDelete(NULL);  //Kill this task
  #endif
}

template<typename typeToLog>
void localLog(typeToLog message)  //Add a partial line to the local log, starting with a timestampt
{
  #ifdef USE_RTOS
  if(xSemaphoreTake(loggingSemaphore, loggingSemaphoreTimeout))
  {
  #endif
  if(logfileYear == 0)  //If a log file is not chose, try and choose one
  {
    setLogFilename();
  }
  else if(logRolloverOccured() == true && startOfLogLine == true)  //Check for log rollover, which only occurs after a new line. It is done here so it is rolled at the time of logging, not the time of the log flush
  {
    if(loggingBuffer.length() > 0)
    {
      flushLog();
    }
    setLogFilename(); //Try to choose a next log file
  }
  if(startOfLogLine == true)
  {
    startOfLogLine = false;
    if(loggingBuffer.length() > logFlushThreshold)  //Check for the need to flush a buffer that's about to get too large (which is not fatal)
    {
      if(autoFlush == true)
      {
        #ifdef SERIAL_LOG
          if(waitForBufferSpace(27))
          {
            SERIAL_DEBUG_PORT.println(F("THRESHOLD HIT FOR LOG FLUSH"));
          }
        #endif
        logLastFlushed = millis();
        flushLog();
      }
      else
      {
        flushLogNow = true;
      }
    }
    updateTimestamp();
    #ifdef SERIAL_LOG
      if(waitForBufferSpace(20))
      {
        SERIAL_DEBUG_PORT.print(timestamp);
        SERIAL_DEBUG_PORT.print(' ');
      }
    #endif
    logToFile(timestamp);
    logToFile(' ');
  }
  #ifdef SERIAL_LOG
    if(waitForBufferSpace(String(message).length()))
    {
      SERIAL_DEBUG_PORT.print(message);
    }
  #endif
  logToFile(message);
  #ifdef USE_RTOS
  xSemaphoreGive(loggingSemaphore);
  }
  #endif
}
template<typename typeToLog>
void localLog(typeToLog message, uint8_t base)  //Add a partial line to the local log, starting with a timestampt
{
  #ifdef USE_RTOS
  if(xSemaphoreTake(loggingSemaphore, loggingSemaphoreTimeout))
  {
  #endif
  if(logfileYear == 0)  //If a log file is not chose, try and choose one
  {
    setLogFilename();
  }
  else if(autoFlush == true && logRolloverOccured() == true && startOfLogLine == true)  //Check for log rollover, which only occurs after a new line. It is done here so it is rolled at the time of logging, not the time of the log flush
  {
    if(loggingBuffer.length() > 0)
    {
      flushLog();
    }
    setLogFilename(); //Try to choose a next log file
  }
  if(startOfLogLine == true)
  {
    startOfLogLine = false;
    if(loggingBuffer.length() > logFlushThreshold)  //Check for the need to flush a buffer that's about to get too large (which is not fatal)
    {
      if(autoFlush == true)
      {
        #ifdef SERIAL_LOG
          if(waitForBufferSpace(29))
          {
            SERIAL_DEBUG_PORT.println(F("THRESHOLD HIT FOR LOG FLUSH"));
          }
        #endif
        logLastFlushed = millis();
        flushLog();
      }
      else
      {
        flushLogNow = true;
      }
    }
    updateTimestamp();
    #ifdef SERIAL_LOG
      if(waitForBufferSpace(21))
      {
        SERIAL_DEBUG_PORT.print(timestamp);
        SERIAL_DEBUG_PORT.print(' ');
      }
    #endif
    logToFile(timestamp);
    logToFile(' ');
  }
  #ifdef SERIAL_LOG
    if(waitForBufferSpace(32))
    {
      SERIAL_DEBUG_PORT.print(message, base);
    }
  #endif
  logToFile(message);
  #ifdef USE_RTOS
  xSemaphoreGive(loggingSemaphore);
  }
  #endif
}
template<typename typeToLog>
void localLogLn(typeToLog message) //Add to the local log, starting with a timestampt
{
  #ifdef USE_RTOS
  if(xSemaphoreTake(loggingSemaphore, loggingSemaphoreTimeout))
  {
  #endif
  if(logfileYear == 0)  //If a log file is not chose, try and choose one
  {
    setLogFilename();
  }
  else if(logRolloverOccured() == true && startOfLogLine == true)  //Check for log rollover, which only occurs after a new line. It is done here so it is rolled at the time of logging, not the time of the log flush
  {
    if(loggingBuffer.length() > 0)
    {
      flushLog();
    }
    setLogFilename(); //Try to choose a next log file
  }
  if(startOfLogLine == true)
  {
    startOfLogLine = false;
    if(loggingBuffer.length() > logFlushThreshold)  //Check for the need to flush a buffer that's about to get too large (which is not fatal)
    {
      if(autoFlush == true)
      {
        #ifdef SERIAL_LOG
          if(waitForBufferSpace(25))
          {
            SERIAL_DEBUG_PORT.println(F("THRESHOLD HIT FOR LOG FLUSH"));
          }
        #endif
        logLastFlushed = millis();
        flushLog();
      }
      else
      {
        flushLogNow = true;
      }
    }
    updateTimestamp();
    #ifdef SERIAL_LOG
      if(waitForBufferSpace(21))
      {
        SERIAL_DEBUG_PORT.print(timestamp);
        SERIAL_DEBUG_PORT.print(' ');
      }
    #endif
    logToFile(timestamp);
    logToFile(' ');
  }
  #ifdef SERIAL_LOG
    if(waitForBufferSpace(String(message).length() + 1))
    {
      SERIAL_DEBUG_PORT.println(message);
    }
  #endif
  logToFileLn(message);
  startOfLogLine = true;
  #ifdef USE_RTOS
  xSemaphoreGive(loggingSemaphore);
  }
  #endif
}
template<typename typeToLog>
void logToFile(typeToLog message)
{
    loggingBuffer+=message;
}
template<typename typeToLog>
void logToFileLn(typeToLog message)
{
    loggingBuffer+=message;
    loggingBuffer+="\r\n";
}

bool waitForBufferSpace(uint16_t spaceNeeded)   //The TX buffer DOES get full so wait for it to empty out
{
  if(debugPortAvailable == true)
  {
    serialBufferCheckTime = millis();
    while(SERIAL_DEBUG_PORT.availableForWrite() < spaceNeeded && millis() - serialBufferCheckTime < (spaceNeeded * 2))
    {
      delay(1);
    }
    if(millis() - serialBufferCheckTime >= spaceNeeded * 2)
    {
      debugPortAvailable = false; //Stop using the serial output for a bit to let it clear, or come online
      return false;
    }
    debugPortAvailable = true;  //Succesfully sent, keep using the serial output
    return true;
  }
  return false;
}

void flushLog() //Flush the log to filesystem if it seems safe to do so
{
  if(timeIsValid() == true && logfileYear!= 0 && filesystemMounted == true) //Valid time & a log file is selected & filesystem is mounted
  {
    #ifdef SERIAL_LOG
      if(waitForBufferSpace(50))
      {
        SERIAL_DEBUG_PORT.print(F("FLUSHING LOG TO FILE: "));
        SERIAL_DEBUG_PORT.println(logFilename);
      }
    #endif
    File logFile = openFileForAppend(logFilename); //Open file for appending
    if(logFile) //Log file is open
    {
      logFile.print(loggingBuffer);
      logFile.close();
      #ifdef SERIAL_LOG
        if(waitForBufferSpace(30))
        {
          SERIAL_DEBUG_PORT.print(F("FLUSHED, FILE SYSTEM USED: "));
          SERIAL_DEBUG_PORT.print(percentageOfFilesystemUsed());
          SERIAL_DEBUG_PORT.println('%');
        }
      #endif
      loggingBuffer = "";
      if(percentageOfFilesystemUsed() > 89) //Delete the older logs
      {
        deleteOldestLogs();
      }
    }
    else
    {
      #ifdef SERIAL_LOG
        if(waitForBufferSpace(50))
        {
          SERIAL_DEBUG_PORT.print(F("UNABLE TO FLUSH LOG TO FILE: "));
          SERIAL_DEBUG_PORT.print(logFilename);
        }
      #endif
    }
  }
  else
  { 
    if(filesystemMounted == false)
    {
      #ifdef SERIAL_LOG
        if(waitForBufferSpace(70))
        {
          SERIAL_DEBUG_PORT.println(F("UNABLE TO FLUSH LOG: FILESYSTEM NOT MOUNTED"));
        }
      #endif
    }
    else
    {
      #ifdef SERIAL_LOG
        if(waitForBufferSpace(80))
        {
          SERIAL_DEBUG_PORT.println(F("UNABLE TO FLUSH LOG: NOT YET OPENED/TIME NOT SET"));
        }
      #endif
    }
    if(loggingBuffer.length() > logFlushThreshold)
    {
      loggingBuffer = "";
    }
  }
}
void deleteOldestLogs() //Iterate through the logs and delete the oldest file
{
  uint16_t oldestYear = 0xFFFF;
  uint8_t  oldestMonth = 0xFF;
  uint8_t  oldestDay = 0xFF;
  localLogLn(F("Deleting oldest log to free space"));
  #if defined(USE_SPIFFS)
    Dir dir = SPIFFS.openDir(logDirectory);
    while (dir.next ())
    {
      if(String(dir.fileName()).startsWith("log"))
      {
        uint16_t fileYear = String(dir.fileName()).substring(4,8).toInt();
        uint8_t fileMonth = String(dir.fileName()).substring(9,11).toInt();
        uint8_t fileDay   = String(dir.fileName()).substring(12,14).toInt();
        if(fileYear < oldestYear)
        {
          oldestYear = fileYear;
          oldestMonth = fileMonth;
          oldestDay = fileDay;
        }
        else if(fileYear <= oldestYear && fileMonth < oldestMonth)
        {
          oldestYear = fileYear;
          oldestMonth = fileMonth;
          oldestDay = fileDay;
        }
        else if(fileYear <= oldestYear && fileMonth <= oldestMonth && fileDay < oldestDay)
        {
          oldestYear = fileYear;
          oldestMonth = fileMonth;
          oldestDay = fileDay;
        }
      }
    }
  #elif defined(USE_LITTLEFS)
    #if defined(ESP8266)
      File dir = LittleFS.open(logDirectory,"r");
    #elif defined(ESP32)
      File dir = LittleFS.open(logDirectory);
    #endif
    File file = dir.openNextFile();
    while(file)
    {
      if(!file.isDirectory())
      {
        if(String(file.name()).startsWith("log"))
        {
          uint16_t fileYear = String(file.name()).substring(4,8).toInt();
          uint8_t fileMonth = String(file.name()).substring(9,11).toInt();
          uint8_t fileDay   = String(file.name()).substring(12,14).toInt();
          if(fileYear < oldestYear)
          {
            oldestYear = fileYear;
            oldestMonth = fileMonth;
            oldestDay = fileDay;
          }
          else if(fileYear <= oldestYear && fileMonth < oldestMonth)
          {
            oldestYear = fileYear;
            oldestMonth = fileMonth;
            oldestDay = fileDay;
          }
          else if(fileYear <= oldestYear && fileMonth <= oldestMonth && fileDay < oldestDay)
          {
            oldestYear = fileYear;
            oldestMonth = fileMonth;
            oldestDay = fileDay;
          }
        }
      }
      file = dir.openNextFile();
    }
  #endif
  char logFileToDelete[logFilenameLength]; //Big enough for with or without leading /
  sprintf_P(logFileToDelete,logfilenameTemplate,logDirectory,oldestYear,oldestMonth,oldestDay); //Make the current filename
  deleteFile(logFileToDelete);  //Remove the oldest log  
}
void showStartupInfo()
{
  #if defined(ACT_AS_TRACKER)
    localLogLn(F("============================ Booting tracker ============================"));
  #elif defined(ACT_AS_BEACON)
    localLogLn(F("============================ Booting beacon ============================="));
  #endif
  localLog(F("Firmware: ")); localLog(majorVersion); localLog('.'); localLog(minorVersion); localLog('.'); localLogLn(patchVersion);
  localLog(F("Built: ")); localLog(__TIME__); localLog(' '); localLogLn(__DATE__);
  #ifdef ESP_IDF_VERSION_MAJOR
    localLog(F("IDF version: "));      
    #ifdef ESP_IDF_VERSION_MINOR
      localLog(ESP_IDF_VERSION_MAJOR);
      localLog('.');
      localLogLn(ESP_IDF_VERSION_MINOR);
    #else
      localLogLn(ESP_IDF_VERSION_MAJOR);
    #endif
  #endif
  localLog(F("Core: "));
  #if defined(ESP32)
    #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
      #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
        localLogLn(F("ESP32"));
      #elif CONFIG_IDF_TARGET_ESP32S2
        localLogLn(F("ESP32S2"));
      #elif CONFIG_IDF_TARGET_ESP32C3
        localLogLn(F("ESP32C3"));
      #else 
        #error Target CONFIG_IDF_TARGET is not supported
      #endif
      localLog(F("Processor clock: "));
      localLog(getCpuFrequencyMhz());
      localLogLn(F("Mhz"));
    #else // ESP32 Before IDF 4.0
      localLogLn(F("ESP32"));
    #endif
  #else
    localLogLn(F("Uknown"));
  #endif
  localLog(F("Board: ")); localLogLn(ARDUINO_BOARD);
  #if defined(ESP32)
    #ifdef ESP_IDF_VERSION_MAJOR // IDF 4+
      #if CONFIG_IDF_TARGET_ESP32 // ESP32/PICO-D4
        localLog(F("Restart reason core 0: "));
        localLogLn(es32ResetReason(0));
        localLog(F("Restart reason core 1: "));
        localLogLn(es32ResetReason(1));
      #elif CONFIG_IDF_TARGET_ESP32S2
        localLog(F("Restart reason: "));
        localLogLn(es32ResetReason(0));
      #elif CONFIG_IDF_TARGET_ESP32C3
        localLog(F("Restart reason: "));
        localLogLn(es32ResetReason(0));
      #else 
        #error Target CONFIG_IDF_TARGET is not supported
      #endif
    #else // ESP32 Before IDF 4.0
      localLog(F("Restart reason core 0: "));
      localLogLn(es32ResetReason(0));
      localLog(F("Restart reason core 1: "));
      localLogLn(es32ResetReason(1));
    #endif
  #endif
}
#if defined(ESP32)
String es32ResetReason(uint8_t core)
{
  switch (rtc_get_reset_reason(core))
  {
    case 1 : return(String("Power on reset"));break;          /**<1,  Vbat power on reset*/
    case 3 : return(String("Software Reset"));break;               /**<3,  Software reset digital core*/
    case 4 : return(String("Legacy WDT_SYS_RESET"));break;             /**<4,  Legacy watch dog reset digital core*/
    case 5 : return(String("Deep Sleep RESET"));break;        /**<5,  Deep Sleep reset digital core*/
    case 6 : return(String("SDIO_RESET"));break;             /**<6,  Reset by SLC module, reset digital core*/
    case 7 : return(String("Timer Group 0 WDT_SYS_RESET"));break;       /**<7,  Timer Group0 Watch dog reset digital core*/
    case 8 : return(String("Timer Group 1 WDT_SYS_RESET"));break;       /**<8,  Timer Group1 Watch dog reset digital core*/
    case 9 : return(String("RTC WDT_SYS_RESET"));break;       /**<9,  RTC Watch dog Reset digital core*/
    case 10 : return(String("INTRUSION_RESET"));break;       /**<10, Instrusion tested to reset CPU*/
    case 11 : return(String("Time Group WDT_CPU_RESET"));break;       /**<11, Time Group reset CPU*/
    case 12 : return(String("Software CPU_RESET"));break;          /**<12, Software reset CPU*/
    case 13 : return(String("RTC Watchdog CPU_RESET"));break;      /**<13, RTC Watch dog Reset CPU*/
    case 14 : return(String("EXT_CPU_RESET"));break;         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : return(String("RTCWDT_BROWN_OUT_RESET"));break;/**<15, Reset when the vdd voltage is not stable*/
    case 16 : return(String("RTCWDT_RTC_RESET"));break;      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : return(String("Unknown"));
  }
}
#endif

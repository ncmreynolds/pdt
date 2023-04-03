/*
 * 
 * This file contains code related to local logging, which is written to the console and local storage
 * 
 * The functions are templated so you can print most types to the log with no fuss. Adding a printf equivalent is on my to-do list
 * 
 */
template<typename typeToLog>
void localLog(typeToLog message)  //Add a partial line to the local log, starting with a timestampt
{
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
          SERIAL_DEBUG_PORT.println(F("THRESHOLD HIT FOR LOG FLUSH"));
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
      #ifdef USE_RTOS
        while(SERIAL_DEBUG_PORT.availableForWrite() < 21){vTaskDelay(10 / portTICK_PERIOD_MS);}  //The TX buffer DOES get full so wait for it to empty out
      #else
        while(SERIAL_DEBUG_PORT.availableForWrite() < 21){delay(10);}  //The TX buffer DOES get full so wait for it to empty out
      #endif
      SERIAL_DEBUG_PORT.print(timestamp);
      SERIAL_DEBUG_PORT.print(' ');
    #endif
    logToFile(timestamp);
    logToFile(' ');
  }
  #ifdef SERIAL_LOG
      #ifdef USE_RTOS
        while(SERIAL_DEBUG_PORT.availableForWrite() < String(message).length()){vTaskDelay(10 / portTICK_PERIOD_MS);}  //The TX buffer DOES get full so wait for it to empty out
      #else
        while(SERIAL_DEBUG_PORT.availableForWrite() < String(message).length()){delay(10);}  //The TX buffer DOES get full so wait for it to empty out
      #endif
    SERIAL_DEBUG_PORT.print(message);
  #endif
  logToFile(message);
}
template<typename typeToLog>
void localLog(typeToLog message, uint8_t base)  //Add a partial line to the local log, starting with a timestampt
{
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
          SERIAL_DEBUG_PORT.println(F("THRESHOLD HIT FOR LOG FLUSH"));
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
      #ifdef USE_RTOS
        while(SERIAL_DEBUG_PORT.availableForWrite() < 21){vTaskDelay(10 / portTICK_PERIOD_MS);}  //The TX buffer DOES get full so wait for it to empty out
      #else
        while(SERIAL_DEBUG_PORT.availableForWrite() < 21){delay(10);}  //The TX buffer DOES get full so wait for it to empty out
      #endif
      SERIAL_DEBUG_PORT.print(timestamp);
      SERIAL_DEBUG_PORT.print(' ');
    #endif
    logToFile(timestamp);
    logToFile(' ');
  }
  #ifdef SERIAL_LOG
    while(SERIAL_DEBUG_PORT.availableForWrite() < 32){delay(10);}  //The TX buffer DOES get full so wait for it to empty out
    SERIAL_DEBUG_PORT.print(message, base);
  #endif
  logToFile(message);
}
template<typename typeToLog>
void localLogLn(typeToLog message) //Add to the local log, starting with a timestampt
{
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
          SERIAL_DEBUG_PORT.println(F("THRESHOLD HIT FOR LOG FLUSH"));
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
      #ifdef USE_RTOS
        while(SERIAL_DEBUG_PORT.availableForWrite() < 21){vTaskDelay(10 / portTICK_PERIOD_MS);}  //The TX buffer DOES get full so wait for it to empty out
      #else
        while(SERIAL_DEBUG_PORT.availableForWrite() < 21){delay(10);}  //The TX buffer DOES get full so wait for it to empty out
      #endif
      SERIAL_DEBUG_PORT.print(timestamp);
      SERIAL_DEBUG_PORT.print(' ');
    #endif
    logToFile(timestamp);
    logToFile(' ');
  }
  #ifdef SERIAL_LOG
    #ifdef USE_RTOS
      while(SERIAL_DEBUG_PORT.availableForWrite() < String(message).length() + 1){vTaskDelay(10 / portTICK_PERIOD_MS);}  //The TX buffer DOES get full so wait for it to empty out
    #else
      while(SERIAL_DEBUG_PORT.availableForWrite() < String(message).length() + 1){delay(10);}  //The TX buffer DOES get full so wait for it to empty out
    #endif
    SERIAL_DEBUG_PORT.println(message);
  #endif
  logToFileLn(message);
  startOfLogLine = true;
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
void flushLog() //Flush the log to filesystem if it seems safe to do so
{
  if(timeIsValid() == true && logfileYear!= 0 && filesystemMounted == true) //Valid time & a log file is selected & filesystem is mounted
  {
    #ifdef SERIAL_LOG
      SERIAL_DEBUG_PORT.print(F("FLUSHING LOG TO FILE: "));
      SERIAL_DEBUG_PORT.println(logFilename);
    #endif
    File logFile = openFileForAppend(logFilename); //Open file for appending
    if(logFile) //Log file is open
    {
      logFile.print(loggingBuffer);
      logFile.close();
      #ifdef SERIAL_LOG
        SERIAL_DEBUG_PORT.print(F("FLUSHED, FILE SYSTEM USED: "));
        SERIAL_DEBUG_PORT.print(percentageOfFilesystemUsed());
        SERIAL_DEBUG_PORT.println('%');
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
        SERIAL_DEBUG_PORT.print(F("UNABLE TO FLUSH LOG TO FILE: "));
        SERIAL_DEBUG_PORT.print(logFilename);
      #endif
    }
  }
  else
  { 
    if(filesystemMounted == false)
    {
      #ifdef SERIAL_LOG
        SERIAL_DEBUG_PORT.println(F("UNABLE TO FLUSH LOG: FILESYSTEM NOT MOUNTED"));
      #endif
    }
    else
    {
      #ifdef SERIAL_LOG
        SERIAL_DEBUG_PORT.println(F("UNABLE TO FLUSH LOG: NOT YET OPENED/TIME NOT SET"));
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
  sprintf(logFileToDelete,logfilenameTemplate,logDirectory,oldestYear,oldestMonth,oldestDay); //Make the current filename
  deleteFile(logFileToDelete);  //Remove the oldest log  
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

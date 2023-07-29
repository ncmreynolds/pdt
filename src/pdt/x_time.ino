/*
 * 
 * This file contains functions related to time/NTP
 * 
 */

void configureTimeServer()
{
  localLog(F("Configuring time server: "));
  localLogLn(timeServer);
  localLog(F("Configuring time zone: "));
  localLogLn(timeZone);
  configTime(0, 0, timeServer);
  setenv("TZ",timeZone,1);
  tzset();
}

bool timeIsValid()
{
  time_t now;
  time(&now);
  struct tm * timeinfo;
  timeinfo = localtime(&now);
  if(timeinfo->tm_year > 100)  //It's a pretty fair bet we're beyond 2000. There doesn't seem to be a function in time.h that indicates valid NTP sync only that it starts at the epoch
  {
    return true;
  }
  return false;
}
void updateTimestamp()
{
  time_t now;
  time(&now);
  struct tm * timeinfo;
  timeinfo = localtime(&now);
  if(timeinfo->tm_year > 100)  //It's a pretty fair bet we're beyond 2000. There doesn't seem to be a function in time.h that indicates valid NTP sync only that it starts at the epoch
  {
    sprintf_P(timestamp,PSTR("%02u:%02u:%02u %02u-%02u-%04u"),timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec,timeinfo->tm_mday,timeinfo->tm_mon + 1,timeinfo->tm_year+1900);
  }
}
bool logRolloverOccured()
{
  if(timeIsValid() == true)
  {
    time_t now;
    time(&now);
    struct tm * timeinfo;
    timeinfo = localtime(&now);
    if(logfileYear != timeinfo->tm_year+1900 || logfileMonth != timeinfo->tm_mon + 1 || logfileDay != timeinfo->tm_mday)
    {
      #ifdef SERIAL_LOG
        if(waitForBufferSpace(14))
        {
          SERIAL_DEBUG_PORT.println(F("LOG ROLLOVER"));
        }
      #endif
      return true;
    }
  }
  return false;
}
void setLogFilename()
{
  if(timeIsValid() == true)
  {
    time_t now;
    time(&now);
    struct tm * timeinfo;
    timeinfo = localtime(&now);
    logfileDay = timeinfo->tm_mday;
    logfileMonth = timeinfo->tm_mon + 1;
    logfileYear = timeinfo->tm_year+1900;
    sprintf(logFilename,logfilenameTemplate,logDirectory,logfileYear,logfileMonth,logfileDay); //Make the current filename
    #ifdef SERIAL_LOG
      if(waitForBufferSpace(40))
      {
        SERIAL_DEBUG_PORT.print(F("USING LOG FILE: "));
        SERIAL_DEBUG_PORT.println(logFilename);
      }
    #endif
  }
}
void recordBootTime() //Gets epoch time into an uint64_t then subtracts millis() to get an approximate time of boot
{
  time_t now;
  time(&now);
  bootTime = uint64_t(now); //This should be epoch time
  bootTime -= millis()/1000; //Subtract the time it took to get a first NTP sync
}
uint64_t upTime()
{
  time_t now;
  time(&now);
  return uint64_t(now) - bootTime;
}
String printableUptime(uint64_t seconds)
{
  String formattedUptime = "";
  if(seconds > 172805)
  {
    formattedUptime += String(uint32_t(seconds/86400));
    formattedUptime += " days ";
    formattedUptime += String(uint32_t((seconds/3600)%24));
    formattedUptime += " hours ";
    formattedUptime += String(uint32_t((seconds/60)%60));
    formattedUptime += " minutes ";
    formattedUptime += String(uint32_t(seconds%60));
    formattedUptime += " seconds";
  }
  else if(seconds > 7205)
  {
    formattedUptime += String(uint32_t(seconds/3600));
    formattedUptime += " hours ";
    formattedUptime += String(uint32_t((seconds/60)%60));
    formattedUptime += " minutes ";
    formattedUptime += String(uint32_t(seconds%60));
    formattedUptime += " seconds";
  }
  else if(seconds > 125)
  {
    formattedUptime += String(uint32_t(seconds/60));
    formattedUptime += " minutes ";
    formattedUptime += String(uint32_t(seconds%60));
    formattedUptime += " seconds";
  }
  else
  {
    formattedUptime += String(uint32_t(seconds));
    formattedUptime += " seconds";
  }
  return(formattedUptime);
}

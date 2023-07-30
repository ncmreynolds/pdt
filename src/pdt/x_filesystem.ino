/*
 * 
 * This file contains functions related to accessing files and directories on the local flash filesystem
 * 
 * Much of it is used internally for local logging
 * 
 */
#if defined(USE_LITTLEFS)
  void mountFilesystem(bool showInfo = true)  //Attempt to mount the filesystem, showinfo false shows no info
  {
    #if defined(USE_SPIFFS)
      if(showInfo == true)
      {
        localLog(F("Mounting SPIFFS: "));
      }
      if (!SPIFFS.begin())
      {
        if(showInfo == true)
        {
          localLogLn(F("failed"));
        }
      }
      else
      {
        filesystemMounted = true;
        if(showInfo == true)
        {
          localLogLn(F("OK"));
          FSInfo fsInfo;
          SPIFFS.info(fsInfo);
          localLog(F("Filesystem KB: "));
          localLog(fsInfo.totalBytes/1024);
          localLog(F(" Used KB: "));
          localLogLn(fsInfo.usedBytes/1024);
          listDir("");  //The SPIFFS syntax doesn't need a leading /
        }
      }
    #elif defined(USE_LITTLEFS)
      if(showInfo == true)
      {
        localLog(F("Mounting LittleFS: "));
      }
      #if defined(ESP32)
        if (!LittleFS.begin(true))  //Autoformat on failure to mount
        {
      #elif defined(ESP8266)    //Autoformat on failure to mount
        if (!LittleFS.begin())
        {
      #endif
          if(showInfo == true)
          {
            localLogLn(F("failed"));
          }
        }
        else
        {
          filesystemMounted = true;
          if(showInfo == true)
          {
            localLogLn(F("OK"));
            localLog(F("Filesystem KB: "));
            localLog(LittleFS.totalBytes()/1024);
            localLog(F(" Used KB: "));
            localLogLn(LittleFS.usedBytes()/1024);
          }
        }
    #endif
    if(filesystemMounted == true)
    {
      if(fileExists(logDirectory) == false)
      {
        mkdir(logDirectory);
      }
    }
  }
  void setupFilesystem()
  {  
    mountFilesystem(true);
    if(loadConfiguration(configurationFile) == false)
    {
      loadDefaultConfiguration();
    }
    printConfiguration();
  }
  void listAllFiles() //Just lists ALL files on the filesytem to the log
  {
    #if defined(USE_SPIFFS)
      localLogLn("SPIFFS file list");
      listDir(""); //The SPIFFS syntax doesn't need a leading /
    #elif defined(USE_LITTLEFS)
      localLogLn("LittleFS file list");
      listDir("/"); //The LittleFS syntax needs a leading /
    #endif  
  }
  void listDir(const char* path)  //Shows the list of files in a directory to the local log
  {
    #if defined(USE_SPIFFS)
      Dir dir = SPIFFS.openDir(path);
      while (dir.next ()) {
        localLog(dir.fileName ());
        localLog("  ");
        localLogLn(dir.fileSize ());
      }
    #elif defined(USE_LITTLEFS)
      #if defined(ESP8266)
        File dir = LittleFS.open(path,"r");
      #elif defined(ESP32)
        File dir = LittleFS.open(path);
      #endif
      File file = dir.openNextFile();
      while(file)
      {
        if(!file.isDirectory())
        {
          localLog(F(" FILE: "));
          #if defined(ESP32)
            #if (ESP_IDF_VERSION_MAJOR == 4 && ESP_IDF_VERSION_MINOR >= 4) || ESP_IDF_VERSION_MAJOR > 4
              localLog(file.path());
            #else
              localLog(file.name());
            #endif
          #elif defined(ESP8266)
            localLog(file.fullName());
          #endif
          localLog(F("  SIZE: "));
          if(file.size() < 4096)
          {
            localLog(file.size());
            localLogLn(" Bytes");
          }
          else
          {
            localLog(file.size()/1024);
            localLogLn(" KB");
          }
        }
        else
        {
          localLog(F("  DIR: "));
          #if defined(ESP32)
            #if (ESP_IDF_VERSION_MAJOR == 4 && ESP_IDF_VERSION_MINOR >= 4) || ESP_IDF_VERSION_MAJOR > 4
              localLogLn(file.path());
              listDir(file.path());
            #else
              localLogLn(file.name());
              listDir(file.name());
            #endif
          #elif defined(ESP8266)
            localLogLn(file.fullName());
            listDir(file.fullName());
          #endif
        }
        file = dir.openNextFile();
      }
    #endif
  }
  bool renameFile(const char* oldName,const char* newName)  //A simple rename, only exists to clear up the SPIFFS vs LittleFS defines
  {
    localLog(F("Renaming file \""));
    localLog(oldName);
    localLog(F("\" -> \""));
    localLog(newName);
    localLog(F("\": "));
    #if defined(USE_SPIFFS)
      if(SPIFFS.rename(oldName, newName))
      {
        localLogLn(F("OK"));
        return true;
      }
    #elif defined(USE_LITTLEFS)
      if(LittleFS.rename(oldName, newName))
      {
        localLogLn(F("OK"));
        return true;
      }
    #endif
    localLogLn(F("failed"));
    return false;
  }
  bool deleteFile(const char* filename) //This only exists to improve code readibility by removing the SPIFFS/LittleFS conditional compilation
  {
    localLog(F("Deleting file \""));
    localLog(filename);
    localLog(F("\": "));
    #if defined(USE_SPIFFS)
      if (SPIFFS.exists(filename))
      {
        if(SPIFFS.remove(filename))
        {
          localLogLn("OK");
          return true;
        }
      }
    #elif defined(USE_LITTLEFS)
      if (LittleFS.exists(filename))
      {
        if(LittleFS.remove(filename))
        {
          localLogLn("OK");
          return true;
        }
      }
    #endif
      else
      {
        localLogLn(F("failed, does not exist"));
      }
    return false;
  }
  bool fileExists(const char* filename) //This only exists to improve code readibility by removing the SPIFFS/LittleFS conditional compilation
  {
    #if defined(USE_SPIFFS)
      return SPIFFS.exists(filename);
    #elif defined(USE_LITTLEFS)
      return LittleFS.exists(filename);
    #else
      return false;
    #endif
  }
  File openFileForReading(const char* filename) //This only exists to improve code readibility by removing the SPIFFS/LittleFS conditional compilation
  {
    #if defined(USE_SPIFFS)
      return SPIFFS.open(filename, "r");
    #elif defined(USE_LITTLEFS)
      return LittleFS.open(filename, "r");
    #endif
  }
  File openFileForWriting(const char* filename) //This only exists to improve code readibility by removing the SPIFFS/LittleFS conditional compilation
  {
    #if defined(USE_SPIFFS)
      return SPIFFS.open(filename, "w");
    #elif defined(USE_LITTLEFS)
      return LittleFS.open(filename, "w");
    #endif
  }
  File openFileForAppend(const char* filename) //This only exists to improve code readibility by removing the SPIFFS/LittleFS conditional compilation
  {
    #if defined(USE_SPIFFS)
      return SPIFFS.open(filename, "a");
    #elif defined(USE_LITTLEFS)
      return LittleFS.open(filename, "a");
    #endif
  }
  bool mkdir(const char* filename)
  {
    localLog(F("Creating directory \""));
    localLog(filename);
    localLog(F("\" : "));
    #if defined(USE_SPIFFS)
      if(SPIFFS.mkdir(filename))
      {
        localLogLn(F("OK"));
        return true;
      }
      else
      {
        localLogLn(F("OK"));
      }
    #elif defined(USE_LITTLEFS)
      if(LittleFS.mkdir(filename))
      {
        localLogLn(F("OK"));
        return true;
      }
      else
      {
        localLogLn(F("OK"));
      }
    #endif
    return false;
  }
  /*
  String md5hashOfFile(const char* filename)  //Returns a string of the MD5 hash of a file
  {
    localLog(F("Calculating MD5 hash of \""));
    localLog(filename);
    localLog(F("\": "));
    if (filesystemMounted)
    {
      if(fileExists(filename) == false) {
        localLogLn(F("failed, file not found"));
        if (networkConnected) {
        }
        return "";
      }
      else
      {
        #if defined(USE_SPIFFS)
          File fileToHash = SPIFFS.open(filename, "r");
        #elif defined(USE_LITTLEFS)
          File fileToHash = LittleFS.open(filename, "r");
        #endif
        if (!fileToHash)
        {
          localLogLn(F("failed to open file"));
        }
        String stuffToHash = fileToHash.readString();
        fileToHash.close();
        MD5Builder fileMd5;  //An MD5 hash of the file
        fileMd5.begin(); //Initialise the MD5
        fileMd5.add(stuffToHash);  //Add the payload
        fileMd5.calculate(); //Calculate the MD5
        localLogLn(fileMd5.toString());
        return(fileMd5.toString());
      }
    }
    else
    {
      localLogLn(F("failed, filesystem not mounted"));
    }
    return "";
  }
  */
  uint8_t percentageOfFilesystemUsed()  //What it says on the tin, how much space available on the file system is in use
  {
    if(filesystemMounted == true)
    {
      #if defined(USE_SPIFFS)
        FSInfo fsInfo;
        SPIFFS.info(fsInfo);
        if(fsInfo.usedBytes > 0 && fsInfo.totalBytes > 0)
        {
          return uint8_t(float(fsInfo.usedBytes) * 100/float(fsInfo.totalBytes));
        }
        else
        {
          return 0;
        }
      #elif defined(USE_LITTLEFS)
        #if defined(ESP8266)
          FSInfo fsInfo;
          LittleFS.info(fsInfo);
          if(fsInfo.usedBytes > 0 && fsInfo.totalBytes > 0)
          {
            return uint8_t(float(fsInfo.usedBytes) * 100/float(fsInfo.totalBytes));
          }
          else
          {
            return 0;
          }
        #elif defined(ESP32)
          if(LittleFS.usedBytes() > 0 && LittleFS.totalBytes() > 0)
          {
            return uint8_t(float(LittleFS.usedBytes()) * 100/float(LittleFS.totalBytes()));
          }
          else
          {
            return 0;
          }
        #endif
      #endif
    }
    return 0;
  }
#endif

/**************************************************************************/
// Init command table
/**************************************************************************/
void cmdTableInit()
{
    cmd.add("play", cmdPlay);
    cmd.add("settime", cmdSetDateTime);
    cmd.add("gettime", cmdGetDateTime);     
    cmd.add("setname", cmdSetName);
    cmd.add("setmode", cmdSetMode);
    cmd.add("setshuffle", cmdSetShuffle);
    cmd.add("setdelay", cmdSetDelay);
    cmd.add("setoffdelay", cmdSetOffDelay);
    cmd.add("setplaylisttime", cmdSetPlaylistTime);
    cmd.add("setnumsounds", cmdSetNumSounds);
    cmd.add("initplaylist", cmdInitPlaylist);
    cmd.add("testactive", cmdTestActive);
    cmd.add("config", cmdDumpConfig);
    cmd.add("normal", cmdSetNormal);
    cmd.add("help", cmdHelp);  
    cmd.add("test", cmdTest);   
}

/************************************************************/

/************************************************************/
void cmdHelp(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;
        
    Serial.println(F("play          - Play sound. Usage: 'play <sound number>'"));        
    Serial.println(F("stop          - Stop a sound from playing. Usage: 'stop'")); 
    Serial.println(F("vol           - Set volume. Usage: 'vol <num from 1-30>'"));
    Serial.println(F("pause         - Pause sound playing. Usage: 'pause'"));
    Serial.println(F("resume        - Resume sound playing. Usage: 'resume'"));
    Serial.println(F("sleep         - Go into sleep mode. Need to reset to exit sleep mode. Usage: 'sleep'"));
    Serial.println(F("setname       - Set name. Usage: 'setname <name>'"));
    Serial.println(F("setmode       - Set test mode. Usage: 'setmode <0=normal, 1=test>'"));    
    Serial.println(F("setshuffle    - Set shuffle mode. Usage: 'setshuffle <0=standalone, 1=camera trap>'"));
    Serial.println(F("setdelay      - Set delay. This is delay from trigger to playback. Usage: 'setdelay <delay in seconds>'"));
    Serial.println(F("setoffdelay   - Set offdelay. This is blackout period after playback & before next trigger is allowed. Usage: 'setoffdelay <delay in seconds>'"));    
    Serial.println(F("setplaylisttime - Set time to start each playlist based on daily schedule. Usage: setplaylisttime <playlist num> <hour> <min>"));
    Serial.println(F("setnumsounds  - Set max number of sounds for each playlist. Usage: 'setnumsounds <playlist num> <maxsounds>'")); 
    Serial.println(F("config        - Display metadata configuration data. Usage: 'config'"));
    Serial.println(F("normal        - Go into normal (deployment) mode and exit command line mode. Usage: 'normal'"));
}

/********************************************************************/
// 
/********************************************************************/
void cmdTest(int argCnt, char **args)
{
    uint8_t listNum = cmd.conv(args[1]);
    getNextSound(listNum);
}

/********************************************************************/
// 
/********************************************************************/
void cmdTestActive(int argCnt, char **args)
{
    (void) argCnt;
    
    uint8_t hr = cmd.conv(args[1], 10);
    uint8_t min = cmd.conv(args[2],10);   
    printf_P(PSTR("Time entered: %02d:%02d. %s active.\n"), hr, min, playlistChooser(hr, min)?"Playlist 1" : "Playlist 0");
}

/********************************************************************/
// 
/********************************************************************/
void cmdInitPlaylist(int argCnt, char **args)
{
    (void) argCnt;
    
    uint8_t *list, numSounds;
    uint8_t listNum = cmd.conv(args[1]);

    if (listNum > NUM_PLAYLISTS-1)
    {
        Serial.println(F("Value exceeds number of playlists"));
        return;
    }

    if (meta.shuffleEnable)
    {
      shufflePlaylist(listNum);  
    }
    
    numSounds = meta.numSounds[listNum];
    list = playlist[listNum];

    printf("Playlist %d members: ", listNum);
    for (int i=0; i<numSounds; i++)
    {
        if ((i%10) == 0)
        {
            Serial.println();
        }
        printf("%02d ", list[i]);
    }
}

/********************************************************************/
// 
/********************************************************************/
void cmdSetPlaylistTime(int argCnt, char **args)
{
    (void) argCnt;
    
    uint8_t listNum, hr, min;

    EEPROM.get(EEPROM_META_LOC, meta);
    listNum = cmd.conv(args[1]);
    hr = cmd.conv(args[2]);
    min = cmd.conv(args[3]); 

    if (listNum > NUM_PLAYLISTS-1)
    {
        Serial.println(F("Value exceeds number of playlists"));
        return;
    }

    meta.playListTime[listNum].hour = hr;
    meta.playListTime[listNum].min = min;
       
    EEPROM.put(EEPROM_META_LOC, meta);    
    printf_P(PSTR("Playlist %d = %02d:%02d.\n"), listNum, hr, min);
}

/********************************************************************/
// 
/********************************************************************/
void cmdSetNumSounds(int argCnt, char **args)
{
    if (argCnt != 3)
    {
        Serial.println(F("Incorrect number of arguments."));
        return;
    }

    uint8_t listNum = cmd.conv(args[1]);
    uint8_t numSounds = cmd.conv(args[2]);
    EEPROM.get(EEPROM_META_LOC, meta);
    meta.numSounds[listNum] = numSounds;
    EEPROM.put(EEPROM_META_LOC, meta);
    printf_P(PSTR("Playlist %d number of sounds set to %d.\n"), listNum, numSounds);
}

/********************************************************************/
// cmdSetNormal
/********************************************************************/
void cmdSetNormal(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;
    
    normalMode = true;   
    Serial.println(F("Going into normal operation mode."));
    Serial.flush();     
}

/**************************************************************************/
// setDateTime
/**************************************************************************/
void cmdSetDateTime(int argCnt, char **args)
{
#if (BOOMBOX == 1)    
    (void) argCnt;
    (void) args;
        
    uint8_t day, mon, year, hr, min, sec;

    year = strtol(args[1], NULL, 10);
    mon = strtol(args[2], NULL, 10);
    day = strtol(args[3], NULL, 10);
    hr = strtol(args[4], NULL, 10);
    min = strtol(args[5], NULL, 10);
    sec = strtol(args[6], NULL, 10);

    rtc.setDateTime(day, 0, mon, 0, year, hr, min, sec);      
    printf_P(PSTR("Now = %s.\n"), boombox.rtcPrintTimeAndDate());
#endif    
}

/**************************************************************************/
// getDateTime
/**************************************************************************/
void cmdGetDateTime(int argCnt, char **args)
{
#if (BOOMBOX == 1)    
    (void) argCnt;
    (void) args;
        
    printf_P(PSTR("Now = %s.\n"), boombox.rtcPrintTimeAndDate());
#endif    
}

/************************************************************/
//
/************************************************************/
void cmdSetName(int argCnt, char **args)
{
  char buf[MAX_FIELD_SIZE];

  EEPROM.get(EEPROM_META_LOC, meta);
  memset(meta.devName, 0, sizeof(meta.devName));
  strCat(buf, 1, argCnt, args);
  strncpy(meta.devName, buf, MAX_FIELD_SIZE - 1);
  EEPROM.put(EEPROM_META_LOC, meta);
}

/************************************************************/
//
/************************************************************/
void cmdSetDelay(int argCnt, char **args)
{
  if (argCnt != 2)
  {
    Serial.println(F("Incorrect number of arguments."));
    return;
  }

  uint16_t delayTime = cmd.conv(args[1]);
  EEPROM.get(EEPROM_META_LOC, meta);
  meta.delayTime = delayTime;
  EEPROM.put(EEPROM_META_LOC, meta);
}

/************************************************************/
//
/************************************************************/
void cmdSetOffDelay(int argCnt, char **args)
{
  if (argCnt != 2)
  {
    Serial.println(F("Incorrect number of arguments."));
    return;
  }

  uint16_t offDelay = cmd.conv(args[1]);
  EEPROM.get(EEPROM_META_LOC, meta);
  meta.offDelayTime = offDelay;
  EEPROM.put(EEPROM_META_LOC, meta);
}

/************************************************************/
//
/************************************************************/
void cmdSetMode(int argCnt, char **args)
{
  if (argCnt != 2)
  {
    Serial.println("Incorrect number of arguments.");
    return;
  }

  uint8_t mode = cmd.conv(args[1]);

  if (mode > 1)
  {
    printf("ERROR: Invalid value. Setting to 0.\n");
    printf("Usage: 0 = STANDALONE mode, 1 = TRAILCAM mode\n");
  }
  
  EEPROM.get(EEPROM_META_LOC, meta);
  meta.devMode = mode;
  EEPROM.put(EEPROM_META_LOC, meta);
}

/************************************************************/
//
/************************************************************/
void cmdSetShuffle(int argCnt, char **args)
{
  if (argCnt != 2)
  {
    Serial.println("Incorrect number of arguments.");
    return;
  }

  bool shuffleEnb = cmd.conv(args[1]);
  EEPROM.get(EEPROM_META_LOC, meta);
  meta.shuffleEnable = shuffleEnb;
  EEPROM.put(EEPROM_META_LOC, meta);
}

/**************************************************************************/
// cmdSetSite
/**************************************************************************/
void cmdDumpConfig(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;

    printf_P(PSTR("Device Name: \t%s\n"), meta.devName);
    printf_P(PSTR("Shuffle:     \t%s\n"), meta.shuffleEnable ? "TRUE" : "FALSE");
    printf_P(PSTR("Device Mode: \t%s\n"), meta.devMode ? "TRAILCAM" : "STANDALONE");
    printf_P(PSTR("Delay Time:  \t%d\n"), meta.delayTime);
    printf_P(PSTR("Off Delay:   \t%d\n"), meta.offDelayTime);

    for (int i=0; i<NUM_PLAYLISTS; i++)
    {
        printf_P(PSTR("Playlist %d Time: \t%02d:%02d\n"),i, meta.playListTime[i].hour, meta.playListTime[i].min);
        printf_P(PSTR("Playlist %d Sounds:   \t%d\n"), i, meta.numSounds[i]);
    }
}

/************************************************************/
// Play track
// Usage: play <track number>
/************************************************************/
void cmdPlay(int argCnt, char **args)
{
    (void) argCnt;   
    
    uint8_t track = cmd.conv(args[1]);
    boombox.playBusy(track);
}

/************************************************************/
// Set volume
// Usage: vol <volume level>
// volume level is between 0 and 30
/************************************************************/
void cmdSetVolume(int argCnt, char **args)
{
    (void) argCnt;
    
    uint8_t vol = cmd.conv(args[1]);
    if (vol > 30)
    {
        vol = 30;
    }
    boombox.setVol(vol);
}

/************************************************************/
// Stop playing
/************************************************************/
void cmdStop(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;
        
    boombox.stop();
}

/************************************************************/
// Go into hibernation mode
/************************************************************/
void cmdSleep(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;
    
    boombox.ampDisable();
    boombox.sleep();

    // need to wake up if you sleep.
    // can only wake up from external interrupt,
    // ie: button push or motion event
    boombox.wake();
    boombox.ampEnable();
    delay(500);
}

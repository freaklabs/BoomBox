/**************************************************************************/
// Init command table
/**************************************************************************/
void cmdTableInit()
{
    cmd.add("play", cmdPlay);
//    cmd.add("stop", cmdStop);
//    cmd.add("sleep", cmdSleep);
    cmd.add("settime", cmdSetDateTime);
    cmd.add("gettime", cmdGetDateTime);     
    cmd.add("setname", cmdSetName);
    cmd.add("setid", cmdSetId);
    cmd.add("setmode", cmdSetMode);
    cmd.add("setmaxsounds", cmdSetMaxSounds);
    cmd.add("setshuffle", cmdSetShuffle);
    cmd.add("setdelay", cmdSetDelay);
    cmd.add("setoffdelay", cmdSetOffDelay);
    cmd.add("config", cmdDumpConfig);
    cmd.add("normal", cmdSetNormal);
    cmd.add("help", cmdHelp);
    cmd.add("debounce", cmdDebounce);
    cmd.add("setdebouncetime", cmdDebounceTime);
    cmd.add("timelock", cmdTimelock);
    cmd.add("settimeon", cmdTimelockOn);
    cmd.add("settimeoff", cmdTimelockOff);
    cmd.add("testtimelock", cmdTestTimelock);
    cmd.add("testdebounce", cmdTestDebounce);
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
    Serial.println(F("setid         - Set boombox ID. Usage: 'setid <num from 0-255>'"));
    Serial.println(F("setmode       - Set test mode. Usage: 'setmode <0=normal, 1=test>'"));
    Serial.println(F("setmaxsounds  - Set max number of sounds. Usage: 'setmaxsounds <num>'"));
    Serial.println(F("setshuffle    - Set shuffle mode. Usage: 'setshuffle <0=standalone, 1=camera trap>'"));
    Serial.println(F("setinterval   - Set interval. Currently unused."));
    Serial.println(F("setdelay      - Set delay. This is delay from trigger to playback. Usage: 'setdelay <delay in seconds>'"));
    Serial.println(F("setoffdelay   - Set offdelay. This is blackout period after playback & before next trigger is allowed. Usage: 'setoffdelay <delay in seconds>'"));    
    Serial.println(F("config        - Display metadata configuration data. Usage: 'config'"));
    Serial.println(F("normal        - Go into normal (deployment) mode and exit command line mode. Usage: 'normal'"));
    Serial.println(F("debounce      - Enable debounce mode to filter noise from system"));
    Serial.println(F("setdebouncetime  - Debounce time to delay in milliseconds"));
    Serial.println(F("timelock      - Enable timelock mode"));
    Serial.println(F("settimeon     - Time (in 24-hour format) to start allowing playback"));
    Serial.println(F("settimeoff    - Time (in 24-hour format) to stop allowing playback"));
    Serial.println(F("testtimelock  - Takes a time in 24-hour format. Will tell you if timelock will return true or false. Please make sure timelock is enabled"));
    Serial.println(F("testdebounce  - This function just demonstrates the debounce time. Please make sure debounce is enabled."));
}

/********************************************************************/
// cmdTestDebounce
/********************************************************************/
void cmdTestDebounce(int argCnt, char **args)
{
    uint16_t debounceTime = cmd.conv(args[1]);
    checkValidTrigger(debounceTime);
}

/********************************************************************/
// cmdTestTimelock
/********************************************************************/
void cmdTestTimelock(int argCnt, char **args)
{
    (void) argCnt;
    uint8_t currentHour = cmd.conv(args[1]);

    if (currentHour > 24)
    {
        Serial.println(F("Please input a valid hour."));
    }

    bool timelock = checkTimelock(currentHour);
    printf_P(PSTR("Testing timelock code. At %d:00, playback will be %s.\n"), currentHour, timelock?"TRUE":"FALSE");
}

/********************************************************************/
// cmdDebounce
/********************************************************************/
void cmdDebounce(int argCnt, char **args)
{       
    bool debounceEnb = false;
    
    if (argCnt != 2)
    {
    Serial.println(F("Incorrect number of arguments."));
    return;
    }

    uint8_t enb = cmd.conv(args[1]);
    if (enb)
    {
       debounceEnb = true;   
    }
    else
    {
        debounceEnb = false;;
    }
    
    EEPROM.get(EEPROM_META_LOC, meta);
    meta.debounceEnb = debounceEnb;
    EEPROM.put(EEPROM_META_LOC, meta);
}

/********************************************************************/
// cmdDebounceTime
/********************************************************************/
void cmdDebounceTime(int argCnt, char **args)
{       
    if (argCnt != 2)
    {
    Serial.println(F("Incorrect number of arguments."));
    return;
    }

    uint16_t debounceTime = cmd.conv(args[1]);   
    EEPROM.get(EEPROM_META_LOC, meta);
    meta.debounceTime = debounceTime;
    EEPROM.put(EEPROM_META_LOC, meta);
}

/********************************************************************/
// cmdTimelock
/********************************************************************/
void cmdTimelock(int argCnt, char **args)
{       
    bool timelockEnb = false;
    
    if (argCnt != 2)
    {
    Serial.println(F("Incorrect number of arguments."));
    return;
    }

    uint8_t enb = cmd.conv(args[1]);
    if (enb)
    {
       timelockEnb = true;   
    }
    else
    {
        timelockEnb = false;;
    }
    
    EEPROM.get(EEPROM_META_LOC, meta);
    meta.timelockEnb = timelockEnb;
    EEPROM.put(EEPROM_META_LOC, meta);
}

/********************************************************************/
// cmdTimelockOn
/********************************************************************/
void cmdTimelockOn(int argCnt, char **args)
{       
    if (argCnt != 2)
    {
    Serial.println(F("Incorrect number of arguments."));
    return;
    }

    uint8_t timelockon = cmd.conv(args[1]);   
    EEPROM.get(EEPROM_META_LOC, meta);
    meta.timelockon = timelockon;
    EEPROM.put(EEPROM_META_LOC, meta);
}

/********************************************************************/
// cmdTimelockOff
/********************************************************************/
void cmdTimelockOff(int argCnt, char **args)
{       
    if (argCnt != 2)
    {
    Serial.println(F("Incorrect number of arguments."));
    return;
    }

    uint8_t timelockoff = cmd.conv(args[1]);   
    EEPROM.get(EEPROM_META_LOC, meta);
    meta.timelockoff = timelockoff;
    EEPROM.put(EEPROM_META_LOC, meta);
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

/************************************************************/
//
/************************************************************/
void cmdDumpPlaylist(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;

    boombox.dumpPlaylist(playlist, meta.maxSounds);
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
    printf_P(PSTR("Now = %s.\n"), bb.rtcPrintTimeAndDate());
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
        
    printf_P(PSTR("Now = %s.\n"), bb.rtcPrintTimeAndDate());
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
void cmdSetId(int argCnt, char **args)
{
  if (argCnt != 2)
  {
    Serial.println(F("Incorrect number of arguments."));
    return;
  }

  uint8_t id = cmd.conv(args[1]);
  EEPROM.get(EEPROM_META_LOC, meta);
  meta.devID = id;
  EEPROM.put(EEPROM_META_LOC, meta);
}

/************************************************************/
//
/************************************************************/
void cmdSetMode(int argCnt, char **args)
{
  if (argCnt != 2)
  {
    Serial.println(F("Incorrect number of arguments."));
    return;
  }

  uint8_t mode = cmd.conv(args[1]);

  if (mode > 1)
  {
    printf_P(PSTR("ERROR: Invalid value. Setting to 0.\n"));
    printf_P(PSTR("Usage: 0 = STANDALONE mode, 1 = TRAILCAM mode\n"));
  }
  
  EEPROM.get(EEPROM_META_LOC, meta);
  meta.devMode = mode;
  EEPROM.put(EEPROM_META_LOC, meta);
}

/************************************************************/
//
/************************************************************/
void cmdSetMaxSounds(int argCnt, char **args)
{
  if (argCnt != 2)
  {
    Serial.println(F("Incorrect number of arguments."));
    return;
  }

  uint16_t maxSounds = cmd.conv(args[1]);
  EEPROM.get(EEPROM_META_LOC, meta);
  meta.maxSounds = maxSounds;
  EEPROM.put(EEPROM_META_LOC, meta);
  boombox.setMaxSounds(meta.maxSounds);
}

/************************************************************/
//
/************************************************************/
void cmdSetShuffle(int argCnt, char **args)
{
  if (argCnt != 2)
  {
    Serial.println(F("Incorrect number of arguments."));
    return;
  }

  bool shuffleEnb = cmd.conv(args[1]);
  EEPROM.get(EEPROM_META_LOC, meta);
  meta.shuffleEnable = shuffleEnb;
  EEPROM.put(EEPROM_META_LOC, meta);

  if (meta.shuffleEnable == 0)
  {
    boombox.setShuffle(0);
  }
  else
  {
    boombox.setShuffle(1);
  }
  boombox.initPlaylist(playlist, meta.maxSounds, meta.shuffleEnable);
}

/**************************************************************************/
// cmdSetSite
/**************************************************************************/
void cmdDumpConfig(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;

    printConfig();
}

/************************************************************/
// Play track
// Usage: play <track number>
/************************************************************/
void cmdPlay(int argCnt, char **args)
{
    (void) argCnt;   
   
    uint8_t track = cmd.conv(args[1]);

    // enable amp
    boombox.ampEnable();       
    delay(AMP_ENABLE_DELAY); // this delay is short and just so the start of the sound doesn't get cut off as amp warms up
    
    boombox.playBusy(track);

    // disable amp before going to sleep. Short delay so sound won't get cut off too suddenly
    // with additional delay after to allow amp to shut down
    delay(500);
    boombox.ampDisable();      
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
// Pause playing
/************************************************************/
void cmdPause(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;
    
    boombox.pause();
}

/************************************************************/
// Resume playing
/************************************************************/
void cmdResume(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;
    
    boombox.resume();
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

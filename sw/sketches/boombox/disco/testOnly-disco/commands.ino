/**************************************************************************/
// Init command table
/**************************************************************************/
void cmdTableInit()
{
    cmd.add("play", cmdPlay);
    cmd.add("stop", cmdStop);
    cmd.add("vol", cmdSetVolume);
    cmd.add("pause", cmdPause);
    cmd.add("resume", cmdResume);
    cmd.add("sleep", cmdSleep);
    cmd.add("setname", cmdSetName);
    cmd.add("setid", cmdSetId);
    cmd.add("setmode", cmdSetMode);
    cmd.add("setmaxsounds", cmdSetMaxSounds);
    cmd.add("setshuffle", cmdSetShuffle);
    cmd.add("setinterval", cmdSetInterval);
    cmd.add("setdelay", cmdSetDelay);
    cmd.add("setoffdelay", cmdSetOffDelay);
    cmd.add("config", cmdDumpConfig);
    cmd.add("dumplist", cmdDumpPlaylist);
    cmd.add("ledchase", cmdLedChase);
    cmd.add("ledflash", cmdLedFlash);
    cmd.add("help", cmdHelp);
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
    Serial.println(F("ledchase      - Display LED chase effect. Mainly for testing LEDs. Usage: 'ledchase'"));
    Serial.println(F("ledflash      - Display LED flash effect. Mainly for testing LEDs. Usage: 'ledflash'"));
}

/************************************************************/

/************************************************************/
void cmdLedChase(int arg_cnt, char **args)
{    
    (void) arg_cnt;
    (void) args;

    colorWipe0(strip0.Color(127, 127, 127), 50); // white
    colorWipe1(strip1.Color(127, 127, 127), 50); // white
    delay(100);
    strip0.clear();
    strip1.clear();
    strip0.show(); 
    strip1.show();     
}

/************************************************************/

/************************************************************/
void cmdLedFlash(int arg_cnt, char **args)
{    
    (void) arg_cnt;
    (void) args;

    for (int i=0; i<10; i++)
    {
        strip0.fill(strip0.Color(255, 255, 255));
        strip1.fill(strip1.Color(255, 255, 255));
        strip0.show();
        strip1.show();
        delay(50);
        strip0.clear();
        strip1.clear();
        strip0.show();
        strip1.show();
        delay(50);
    }      
}

/************************************************************/
//
/************************************************************/
void cmdDumpPlaylist(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;

    boombox.dumpPlaylist();
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
    Serial.println("Incorrect number of arguments.");
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
    Serial.println("Incorrect number of arguments.");
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
    Serial.println("Incorrect number of arguments.");
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
void cmdSetMaxSounds(int argCnt, char **args)
{
  if (argCnt != 2)
  {
    Serial.println("Incorrect number of arguments.");
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
    Serial.println("Incorrect number of arguments.");
    return;
  }

  bool shuffleEnb = cmd.conv(args[1]);
  EEPROM.get(EEPROM_META_LOC, meta);
  meta.shuffleEnable = shuffleEnb;
  EEPROM.put(EEPROM_META_LOC, meta);

  if (meta.shuffleEnable == 0)
  {
    boombox.shuffleEnable(0);
    boombox.initPlaylist();
  }
  else
  {
    boombox.shuffleEnable(1);
    boombox.shufflePlaylist();
  }
}

/************************************************************/
//
/************************************************************/
void cmdSetInterval(int argCnt, char **args)
{
    if (argCnt != 2)
    {
        Serial.println("Incorrect number of arguments.");
        return;
    }
    
    int16_t interval = cmd.conv(args[1]);
    EEPROM.get(EEPROM_META_LOC, meta);
    meta.devInterval = interval;
    EEPROM.put(EEPROM_META_LOC, meta);
}

/**************************************************************************/
// cmdSetSite
/**************************************************************************/
void cmdDumpConfig(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;

    Serial.print(F("Device Name: \t")); Serial.println(meta.devName); 
    Serial.print(F("Device ID:   \t")); Serial.println(meta.devID);
    Serial.print(F("Shuffle:     \t")); Serial.println(meta.shuffleEnable); 
    Serial.print(F("Device Mode: \t")); Serial.println(meta.devMode ? "TRAILCAM" : "STANDALONE");
    Serial.print(F("Max Sounds:  \t")); Serial.println(meta.maxSounds);
    Serial.print(F("Delay Time:  \t")); Serial.println(meta.delayTime);
    Serial.print(F("Off Delay:   \t")); Serial.println(meta.offDelayTime);
    Serial.print(F("Interval:    \t")); Serial.println(meta.devInterval);
}

/************************************************************/
// Play track
// Usage: play <track number>
/************************************************************/
void cmdPlay(int argCnt, char **args)
{
    (void) argCnt;   
    
    uint8_t track = cmd.conv(args[1]);
    boombox.play(track);
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
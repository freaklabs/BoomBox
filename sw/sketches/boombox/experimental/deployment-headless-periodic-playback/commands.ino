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
    cmd.add("setmaxsounds", cmdSetMaxSounds);
    cmd.add("setshuffle", cmdSetShuffle);
    cmd.add("setdelay", cmdSetDelay);
    cmd.add("setoffdelay", cmdSetOffDelay);
    cmd.add("setstart", cmdSetStart);
    cmd.add("setstop", cmdSetStop);
    cmd.add("config", cmdDumpConfig);
    cmd.add("normal", cmdSetNormal);
    cmd.add("help", cmdHelp);    
    cmd.add("setinterval", cmdSetInterval);    
    cmd.add("check", cmdCheckValid);
}

/************************************************************/

/************************************************************/
void cmdHelp(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;

    Serial.println(F("play          - (single playlist) Play sound. Usage: 'play <sound number>'")); 
    Serial.println(F("play          - (multi playlist) Play sound. Usage: 'play <folder number> <sound number>'"));       
    Serial.println(F("stop          - Stop a sound from playing. Usage: 'stop'")); 
    Serial.println(F("vol           - Set volume. Usage: 'vol <num from 1-30>'"));
    Serial.println(F("pause         - Pause sound playing. Usage: 'pause'"));
    Serial.println(F("resume        - Resume sound playing. Usage: 'resume'"));
    Serial.println(F("sleep         - Go into sleep mode. Need to reset to exit sleep mode. Usage: 'sleep'"));
    Serial.println(F("setname       - Set name. Usage: 'setname <name>'"));
    Serial.println(F("setmode       - Set test mode. Usage: 'setmode <0=normal, 1=test>'"));    
    Serial.println(F("setmaxsounds  - Set max number of sounds for playlist. Usage: 'setmaxsounds <list> <num>'"));
    Serial.println(F("setshuffle    - Set shuffle mode for list. Usage: 'setshuffle <list> <0=sequential, 1=shuffle>'"));
    Serial.println(F("setdelay      - Set delay. This is delay from trigger to playback. Usage: 'setdelay <delay in seconds>'"));
    Serial.println(F("setoffdelay   - Set offdelay. This is blackout period after playback & before next trigger is allowed. Usage: 'setoffdelay <delay in seconds>'"));    
    Serial.println(F("setinterval   - Go into normal (deployment) mode and exit command line mode. Usage: 'normal'"));
    Serial.println(F("config        - Display metadata configuration data. Usage: 'config'"));
    Serial.println(F("normal        - Go into normal (deployment) mode and exit command line mode. Usage: 'normal'"));

}

/********************************************************************/
// 
/********************************************************************/
void cmdCheckValid(int argCnt, char **args)
{
    if (argCnt != 2)
    {
        Serial.println(F("Incorrect number of arguments."));
        return;   
    }
    uint8_t currTime = cmd.conv(args[1]);  
    bool allowed = checkPlaybackAllowed(currTime);   
    printf("Allowed: %s.\n", allowed?"TRUE":"FALSE"); 
    printf_P(PSTR("Playback is currently %s.\n"), allowed ? "allowed" : "not allowed");  
}

/********************************************************************/
// 
/********************************************************************/
void cmdSetStart(int argCnt, char **args)
{
    if (argCnt != 2)
    {
        Serial.println(F("Incorrect number of arguments."));
        return;   
    }
    uint8_t startTime = cmd.conv(args[1]);
    
    EEPROM.get(EEPROM_META_LOC, meta);
    meta.startTime = startTime;   
    EEPROM.put(EEPROM_META_LOC, meta);    
    printf_P(PSTR("Playback will start at %d:00.\n"), startTime);  
}

/********************************************************************/
// 
/********************************************************************/
void cmdSetStop(int argCnt, char **args)
{
    if (argCnt != 2)
    {
        Serial.println(F("Incorrect number of arguments."));
        return;   
    }
    uint8_t stopTime = cmd.conv(args[1]);
    
    EEPROM.get(EEPROM_META_LOC, meta);
    meta.stopTime = stopTime;   
    EEPROM.put(EEPROM_META_LOC, meta);    
    printf_P(PSTR("Playback will stop at %d:00.\n"), stopTime);      
}

/********************************************************************/
// 
/********************************************************************/
void cmdSetInterval(int argCnt, char **args)
{
    if (argCnt != 2)
    {
        Serial.println(F("Incorrect number of arguments."));
        return;
    }
    
    uint8_t intv = cmd.conv(args[1]);

    if (intv > 0)
    {
        // subtract 1 from interval since we count from 0
        EEPROM.get(EEPROM_META_LOC, meta);
        meta.interval = intv-1;   
        EEPROM.put(EEPROM_META_LOC, meta);    
        printf_P(PSTR("Playback will occur every %d mins.\n"), intv);
        interval = intv;        
    }    
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
    printf_P(PSTR("MaxSounds for set to %d.\n"), maxSounds);
    printf_P(PSTR("Reset for changes to take effect.\n"));
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
        Serial.println(F("Incorrect number of arguments."));
        return;
    }
    
    bool shuffleEnb = cmd.conv(args[1]);

    EEPROM.get(EEPROM_META_LOC, meta);
    meta.shuffleEnb = shuffleEnb;    
    EEPROM.put(EEPROM_META_LOC, meta);    
    printf_P(PSTR("Shuffle is %s.\n"), shuffleEnb?"Enabled":"Disabled");
}

/**************************************************************************/
// cmdSetSite
/**************************************************************************/
void cmdDumpConfig(int argCnt, char **args)
{
    (void) argCnt;
    (void) args;

    printf_P(PSTR("Device Name: \t%s\n"), meta.devName);
    printf_P(PSTR("Device Mode: \t%s\n"), meta.devMode ? "TRAILCAM" : "STANDALONE");
    printf_P(PSTR("Delay Time:  \t%d\n"), meta.delayTime);
    printf_P(PSTR("Off Delay:   \t%d\n"), meta.offDelayTime);
    printf_P(PSTR("Max Sounds: \t%d\n"), meta.maxSounds);
    printf_P(PSTR("Interval: \t%d\n"), meta.interval+1); // interval starts from 0 so add 1 to actual interval printout
    printf_P(PSTR("Shuffle: \t%s\n"), meta.shuffleEnb ? "TRUE" : "FALSE");
    printf_P(PSTR("Start Time:   \t%d:00\n"), meta.startTime);
    printf_P(PSTR("Stop Time:   \t%d:00\n"), meta.stopTime);    
}

/************************************************************/
// Play track
// Usage: play <folder number> <track number>
/************************************************************/
void cmdPlay(int argCnt, char **args)
{
    bool multiPlaylist = false;
    uint8_t track, folder;

    track = cmd.conv(args[1]);
    

    // enable amp
    boombox.ampEnable();       
    delay(AMP_ENABLE_DELAY); // this delay is short and just so the start of the sound doesn't get cut off as amp warms up
    digitalWrite(boombox.pinMute, HIGH);  
    
    boombox.playBusy(track);
    
    // disable amp before going to sleep. Short delay so sound won't get cut off too suddenly
    // with additional delay after to allow amp to shut down
    digitalWrite(boombox.pinMute, LOW);
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

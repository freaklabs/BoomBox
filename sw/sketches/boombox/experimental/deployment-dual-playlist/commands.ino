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
    cmd.add("config", cmdDumpConfig);
    cmd.add("normal", cmdSetNormal);
    cmd.add("help", cmdHelp);    
    cmd.add("setstart", cmdSetStart);
    cmd.add("setend", cmdSetEnd);
    cmd.add("setinterval", cmdSetinterval);
    cmd.add("dumplist", cmdDumpPlaylist);
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
    Serial.println(F("setplaytime   - Set time in minutes past the hour to perform playback. Usage: 'setplaytime <mins>'"));
    Serial.println(F("config        - Display metadata configuration data. Usage: 'config'"));
    Serial.println(F("normal        - Go into normal (deployment) mode and exit command line mode. Usage: 'normal'"));
}

/********************************************************************/
// 
/********************************************************************/
void cmdDumpPlaylist(int argCnt, char **args)
{
    if (argCnt != 2)
    {
        Serial.println(F("Incorrect number of arguments."));
        return;
    }     

    uint8_t listNum = cmd.conv(args[1]);

    if (listNum == 1)
    {
        printf("Playlist 1:\n");
        for (int i=0; i<meta.list1.maxSounds; i++)
        {
            printf("Index: %d, %d\n", i, playlist1[i]);
        }
    }
    else
    {
        printf("Playlist 2:\n");
        for (int i=0; i<meta.list2.maxSounds; i++)
        {
            printf("Index: %d, %d\n", i, playlist2[i]);
        }        
    }
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
    meta.list1Start = startTime;   
    EEPROM.put(EEPROM_META_LOC, meta);    
    printf_P(PSTR("Playback for list 1 will start at %d:00.\n"), startTime);
}

/********************************************************************/
// 
/********************************************************************/
void cmdSetEnd(int argCnt, char **args)
{
    if (argCnt != 2)
    {
        Serial.println(F("Incorrect number of arguments."));
        return;
    }    

    uint8_t endTime = cmd.conv(args[1]);
    EEPROM.get(EEPROM_META_LOC, meta);
    meta.list1End = endTime;   
    EEPROM.put(EEPROM_META_LOC, meta);    
    printf_P(PSTR("Playback for list 1 will end at %d:00.\n"), endTime);
}

/********************************************************************/
// 
/********************************************************************/
void cmdSetinterval(int argCnt, char **args)
{
    if (argCnt != 3)
    {
        Serial.println(F("Incorrect number of arguments."));
        return;
    }
    
    uint8_t list = cmd.conv(args[1]);
    uint8_t interval = cmd.conv(args[2]);

    EEPROM.get(EEPROM_META_LOC, meta);
    if (list == 1)
    {
        meta.list1.interval = interval;  
    }
    else if (list == 2)
    {
        meta.list2.interval = interval; 
    }
    else
    {
        Serial.println(F("Invalid playlist number"));
    }     
    EEPROM.put(EEPROM_META_LOC, meta);    
    printf_P(PSTR("Playback will occur every %d mins.\n"), interval);
}

/************************************************************/
//
/************************************************************/
void cmdSetMaxSounds(int argCnt, char **args)
{
    if (argCnt != 3)
    {
        Serial.println(F("Incorrect number of arguments."));
        return;
    }
    
    uint8_t list = cmd.conv(args[1]);
    uint16_t maxSounds = cmd.conv(args[2]);
    EEPROM.get(EEPROM_META_LOC, meta);
    if (list == 1)
    {
        meta.list1.maxSounds = maxSounds;  
    }
    else if (list == 2)
    {
        meta.list2.maxSounds = maxSounds; 
    }
    else
    {
        Serial.println(F("Invalid playlist number"));
    }
    EEPROM.put(EEPROM_META_LOC, meta);
    printf_P(PSTR("MaxSounds for playlist %d set to %d.\n"), list, maxSounds);
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
    if (argCnt != 3)
    {
        Serial.println(F("Incorrect number of arguments."));
        return;
    }
    
    uint8_t list = cmd.conv(args[1]);
    bool shuffleEnb = cmd.conv(args[2]);

    EEPROM.get(EEPROM_META_LOC, meta);
    if (list == 1)
    {
        meta.list1.shuffleEnb= shuffleEnb; 
    }
    else if (list == 2)
    {
        meta.list2.shuffleEnb = shuffleEnb; 
    }
    else
    {
        Serial.println(F("Invalid playlist number"));
    }     
    EEPROM.put(EEPROM_META_LOC, meta);    
    printf_P(PSTR("Shuffle %s for playlist %d.\n"), shuffleEnb?"Enabled":"Disabled", list);
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
    printf_P(PSTR("Start time Playlist 1: \t\t%d:00\n"), meta.list1Start);
    printf_P(PSTR("End time Playlist 1: \t\t%d:00\n"), meta.list1End);
    printf_P(PSTR("Max Sounds Playlist 1: \t\t%d\n"), meta.list1.maxSounds);
    printf_P(PSTR("Max Sounds Playlist 2: \t\t%d\n"), meta.list2.maxSounds);
    printf_P(PSTR("Playback interval Playlist 1: \t%d\n"), meta.list1.interval);
    printf_P(PSTR("Playback interval Playlist 2: \t%d\n"), meta.list2.interval);
    printf_P(PSTR("Shuffle Playlist 1: \t\t%s\n"), meta.list1.shuffleEnb ? "TRUE" : "FALSE");
    printf_P(PSTR("Shuffle Playlist 2: \t\t%s\n"), meta.list2.shuffleEnb ? "TRUE" : "FALSE");
}

/************************************************************/
// Play track
// Usage: play <folder number> <track number>
/************************************************************/
void cmdPlay(int argCnt, char **args)
{
    bool multiPlaylist = false;
    uint8_t track, folder;
    
    if (argCnt == 2)
    {
        multiPlaylist = false;
        track = cmd.conv(args[1]);
    }
    else if (argCnt == 3)
    {
        multiPlaylist = true;
        folder = cmd.conv(args[1]);
        track = cmd.conv(args[2]);   

        if (folder == 2)
        {
            track += meta.list1.maxSounds;
        }
    }
    else
    {
        Serial.println(F("Wrong number of arguments"));
    }

    // enable amp
    boombox.ampEnable();
    delay(AMP_ENABLE_DELAY); // this delay is short and just so the start of the sound doesn't get cut off as amp warms up
    digitalWrite(boombox.pinMute, HIGH);

    if (multiPlaylist)
    {
        boombox.playBusy(track);   
    }
    else
    {
        boombox.playBusy(track);
    }
    
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

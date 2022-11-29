//**************************************************************************/
// commands
//**************************************************************************
void cmdTableInit()
{
    cmd.add("setid", cmdSetId);
    cmd.add("setinterval", cmdSetInterval);
    cmd.add("dump", cmdDumpMeta);
    cmd.add("normal", cmdNormal);
    cmd.add("play", cmdPlay);
    cmd.add("stop", cmdStop);
    cmd.add("vol", cmdSetVolume);
    cmd.add("pause", cmdPause);
    cmd.add("resume", cmdResume);
    cmd.add("sleep", cmdSleep);     
}

/**************************************************************************/
// 
/**************************************************************************/
void cmdSetId(int argc, char **args)
{
    (void) argc;
    
    uint8_t id;
    
    getMeta();    
    id = cmd.conv(args[1]);
    meta.boomboxId = id;
    printf("Device ID set to: %d.\n", id);
    putMeta();    
}

/**************************************************************************/
// 
/**************************************************************************/
void cmdSetInterval(int argc, char **args)
{
    (void) argc;
    
    uint16_t interval;
    
    getMeta();    
    interval = cmd.conv(args[1]);
    meta.playbackInterval = interval;
    printf("Interval set to: %d.\n", interval);
    putMeta();    
}

/**************************************************************************/
// 
/**************************************************************************/
void cmdDumpMeta(int argc, char **args)
{
    (void) argc;
    (void) args;
    
    dumpMeta(); 
}

/**************************************************************************/
// 
/**************************************************************************/
void cmdNormal(int argc, char **args)
{
    (void) argc;
    (void) args;
    
    normalMode = true; 
}

/************************************************************/
// Play track
// Usage: play <track number>
/************************************************************/
void cmdPlay(int argc, char **args)
{
    (void) argc;
    
    uint8_t track = cmd.conv(args[1]);
    if (track > MAX_SOUNDS)
    {
        track = MAX_SOUNDS-1;
    }
    bbb.play(track);
}

/************************************************************/
// Set volume
// Usage: vol <volume level>
// volume level is between 0 and 30
/************************************************************/
void cmdSetVolume(int argc, char **args)
{
    (void) argc;    
    
    uint8_t vol = cmd.conv(args[1]);
    if (vol > 30)
    {
        vol = 30;
    }
    bbb.setVol(vol);
}

/************************************************************/
// Pause playing
/************************************************************/
void cmdPause(int argc, char **args)
{
    (void) argc;
    (void) args;
    
    bbb.pause();
}

/************************************************************/
// Resume playing
/************************************************************/
void cmdResume(int argc, char **args)
{
    (void) argc;
    (void) args;
    
    bbb.resume();
}

/************************************************************/
// Stop playing
/************************************************************/
void cmdStop(int argc, char **args)
{
    (void) argc;
    (void) args;
    
    bbb.stop();
}

/************************************************************/
// Go into hibernation mode
/************************************************************/
void cmdSleep(int argc, char **args)
{  
    (void) argc;
    (void) args;
    
    bbb.sleep();
    //LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
    
    // need to wake up if you sleep. 
    // can only wake up from external interrupt, 
    // ie: button push or motion event
    bbb.wake();
}

/**************************************************************************/
// commands
/**************************************************************************/
void cmdTableInit()
{
    cmd.add("setid", cmdSetId);
    cmd.add("setinterval", cmdSetInterval);
    cmd.add("dump", cmdDumpMeta);
    cmd.add("normal", cmdNormal);
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

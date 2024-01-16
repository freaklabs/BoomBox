#include <cmdArduino.h>
#include <EEPROM.h>
#include <limits.h>

#if (BOOMBOXBASE == 1)
    #warning "BOOMBOX_BASE"
    #include <boomboxBase.h>
    #define boombox bbb
#elif (BOOMBOX == 1)
    #warning "BOOMBOX"
    #include <boombox.h>
    #include <Rtc_Pcf8563.h>
    #define boombox bb
    Rtc_Pcf8563 rtc; 
#else
    #warning "BOOMBOX"
    #include <boombox.h>
    #include <Rtc_Pcf8563.h>
    #define boombox bb
    Rtc_Pcf8563 rtc; 
#endif

#define SKETCH_VERSION "1.21"
#define EEPROM_META_LOC 0
#define MAX_FIELD_SIZE 50
#define AMP_ENABLE_DELAY 1000
#define STRUCT_VERSION 10
#define CMD_MODE_TIME_LIMIT 30
#define START_WAIT_TIME 5000
#define ONE_MINUTE 60000

SoftwareSerial ss(9, 8);

typedef struct
{
    uint8_t maxSounds;
    uint8_t interval;
    bool shuffleEnb;
} playlist_t;

typedef struct
{
    char structVer;
    char devName[MAX_FIELD_SIZE];
    uint8_t devID;
    uint8_t devMode;    
    uint16_t delayTime;
    uint16_t offDelayTime;
    uint8_t list1Start;
    uint8_t list1End;
    playlist_t list1;
    playlist_t list2;
} meta_t;
meta_t meta;

char bufTime[MAX_FIELD_SIZE];
ts_t currentTime;
uint8_t activePlaylist = 1;
uint8_t *playlist1, *playlist2;
uint8_t interval;
bool normalMode = true;
uint8_t timeCount = 0;
uint32_t now;
uint32_t cmdModeTimeCnt;
uint32_t cmdModeTimeLimit = CMD_MODE_TIME_LIMIT * ONE_MINUTE;

// this is only used for printf
FILE uartout = {0,0,0,0,0,0,0,0};

/************************************************************/
// setup
/************************************************************/
void setup() 
{   
    // fill in the UART file descriptor with pointer to writer.
    fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartout ;       

    // initialize command line
    cmd.begin(57600, &Serial);

    cmdTableInit();

    // get metadata
    EEPROM.get(EEPROM_META_LOC, meta);
    if ((meta.devID == 0xFF) || (meta.structVer != STRUCT_VERSION))
    {
        // EEPROM uninitialized. initialize metadata
        Serial.println(F("Data uninitialized or wrong version. Installing default configuration settings."));
        Serial.println(F("Reset when finished."));
        memset(meta.devName, 0, sizeof(meta.devName));
        memcpy(meta.devName, "TEST", strlen("TEST"));
        meta.structVer = STRUCT_VERSION;
        meta.devID = 0;
        meta.devMode = 0;    
        meta.delayTime = 0;
        meta.offDelayTime= 0;
        meta.list1Start = 6;
        meta.list1End = 21;
        meta.list1.maxSounds = 5;
        meta.list2.maxSounds = 5;
        meta.list1.interval = 60;
        meta.list2.interval = 60;
        meta.list1.shuffleEnb = false;
        meta.list2.shuffleEnb = false;
        EEPROM.put(EEPROM_META_LOC, meta);
        
        while (1)
        {
            cmd.poll();
        }
    }

#if (BOOMBOX == 1) 
    boombox.begin(&ss, &rtc);               
#else
    boombox.begin(&ss);
#endif

    // set device mode
    if (meta.devMode == 0)
    {
       // in standalone mode. initialize button for triggering system
       Serial.println(F("In STANDALONE mode, initializing button..."));
       boombox.buttonInit(); 
    }
    else
    {
        Serial.println(F("Device not in STANDALONE mode. Please use STANDALONE for headless functionality."));
    }

    // disable amplifier to save power
    boombox.ampDisable();       // disable amplifier 
    
    // display banner for version and diagnostic info
    boombox.dispBanner();
    Serial.print(F("Boombox Deployment Headless Dual Playlist version: "));
    Serial.println(F(SKETCH_VERSION));
    Serial.println(F("Designed by FreakLabs"));
    printf("Current time is %s.\n", boombox.rtcPrintTimeAndDate());  
    Serial.println(F("-------------------------------------------"));  
    
    // set shuffle seed based on current number of seconds
    currentTime = boombox.rtcGetTime();
    randomSeed(currentTime.sec);

    // initialize playlists
    Serial.println(F("Initializing playlist 1..."));
    playlist1 = (uint8_t *)malloc(meta.list1.maxSounds);
    boombox.initPlaylist(playlist1, meta.list1.maxSounds, meta.list1.shuffleEnb);

    Serial.println(F("Initializing playlist 2..."));
    playlist2 = (uint8_t *)malloc(meta.list2.maxSounds);
    boombox.initPlaylist(playlist2, meta.list2.maxSounds, meta.list2.shuffleEnb);

    // set active playlist and parameters
    if ((currentTime.hour >= meta.list1Start) && (currentTime.hour < meta.list1End))
    {
        Serial.println(F("Active Playlist is Playlist 1"));
        activePlaylist = 1;
        boombox.setActivePlaylist(playlist1);
        boombox.setMaxSounds(meta.list1.maxSounds);
        boombox.setShuffle(meta.list1.shuffleEnb);
        interval = meta.list1.interval;
    }
    else
    {
        Serial.println(F("Active Playlist is Playlist 2"));
        activePlaylist = 2;
        boombox.setActivePlaylist(playlist2);
        boombox.setMaxSounds(meta.list2.maxSounds);
        boombox.setShuffle(meta.list2.shuffleEnb);
        interval = meta.list2.interval;
    }

    // clear interrupt flag if it is set
    boombox.clearAuxFlag(); 
    boombox.rtcClearIntp();

    // enable the watchdog timer for 8 seconds
    wdt_enable(WDTO_8S); 

    // set up timer
    boombox.rtcEnableTimer();

    // select if we want to go into developer mode or deployment mode
    selectMode();             
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
    uint8_t nextSound;

    wdt_reset();

    if (normalMode)
    {
        // minute interrupt received
        if (boombox.rtcIntpRcvd() || boombox.isAuxEvent())
        {            
            // get current time so we can parse it
            currentTime = boombox.rtcGetTime();
            printf_P(PSTR("Interrupt received. %02d:%02d.\n"), currentTime.hour, currentTime.min);
    
            // if within boundaries of playlist 1, play playlist 1
            if ((currentTime.hour >= meta.list1Start) && (currentTime.hour < meta.list1End))
            {
                // if it's not set, then change over playlists
                if (activePlaylist != 1)
                {
                    activePlaylist = 1;
                    boombox.setActivePlaylist(playlist1);
                    boombox.setMaxSounds(meta.list1.maxSounds);
                    boombox.setShuffle(meta.list1.shuffleEnb);
                    if (meta.list1.shuffleEnb)
                    {
                        boombox.shufflePlaylist(playlist1, meta.list1.maxSounds);
                    }
                    boombox.setIndex(0);
                    interval = meta.list1.interval;
                }
            }
            else
            {
                if (activePlaylist != 2)
                {
                    activePlaylist = 2;
                    boombox.setActivePlaylist(playlist2);
                    boombox.setMaxSounds(meta.list2.maxSounds);
                    boombox.setShuffle(meta.list2.shuffleEnb);
                    if (meta.list2.shuffleEnb)
                    {
                        boombox.shufflePlaylist(playlist2, meta.list2.maxSounds);
                    }
                    boombox.setIndex(0);
                    interval = meta.list2.interval;
                }
            }
    
            // decrement interval counter
            if (interval > 0)
            {
                interval--;
                printf_P(PSTR("Interval: %d.\n"), interval);
            }
            else
            {
                // reload interval
                if (activePlaylist == 1)
                {
                    interval = meta.list1.interval;
                }
                else
                {
                    interval = meta.list2.interval;
                }
    
                // play sound
                Serial.println(F("playback!!!"));
                
                // delay for delayTime milliseconds after trigger has happened. 
                // This delays playing the sound immediately after trigger 
                now = millis();
                while (elapsedTime(now) < ((uint32_t)meta.delayTime * 1000))
                {
                    wdt_reset();
                }
                  
                // retrieve next sound index
                nextSound = boombox.getNextSound();  
                if (activePlaylist == 2)
                {
                    // sounds are partitioned by folder
                    nextSound += meta.list1.maxSounds;
                }
        
                // enable amp
                boombox.ampEnable();
                delay(AMP_ENABLE_DELAY); // this delay is short and just so the start of the sound doesn't get cut off as amp warms up
        
                // play sound based on randomized playlist
                printf_P(PSTR("Playing sound index %d from playlist %d.\n"), nextSound, activePlaylist);
                boombox.playBusy(nextSound);    
    
                // disable amp before going to sleep
                boombox.ampDisable();     
                        
                // delay for offDelayTime milliseconds. This is the time after playback finishes but we do not allow another sound
                // to be triggered.
                now = millis();
                while (elapsedTime(now) < ((uint32_t)meta.offDelayTime * 1000))
                {
                    wdt_reset();
                }                                      
            }
            // restart timer
            rtc.resetTimer();
            
            // clear interrupt flag
            boombox.rtcClearIntp();        
    
            // clear interrupt flag
            boombox.clearAuxFlag();    
            Serial.flush();                       
        }    
        
        // go to sleep here
        boombox.sleep();
    
        // an interrupt occurred. wake up!
        boombox.wake();
    }
    else
    {
        // if we're in test mode, allow the command line interface
        cmd.poll();  

        // automatic timeout the command line interface after the time limit
        if (elapsedTime(cmdModeTimeCnt) > cmdModeTimeLimit)
        {
            selectMode();
        }        
    }
}

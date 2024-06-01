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

#define SKETCH_VERSION "TESTv1.23"
#define EEPROM_META_LOC 0
#define MAX_FIELD_SIZE 50
#define AMP_ENABLE_DELAY 1000
#define STRUCT_VERSION 16
#define CMD_MODE_TIME_LIMIT 30
#define START_WAIT_TIME 5000
#define ONE_MINUTE 60000

SoftwareSerial ss(9, 8);

typedef struct
{
    char structVer;
    char devName[MAX_FIELD_SIZE];
    uint8_t devID;
    uint8_t devMode;    
    uint16_t delayTime;
    uint8_t maxSounds;
    bool shuffleEnb;
    uint16_t offDelayTime;
    uint16_t interval;
    uint8_t startTime;
    uint8_t stopTime;
} meta_t;
meta_t meta;

char bufTime[MAX_FIELD_SIZE];
uint8_t *playlist;
ts_t currentTime;
uint8_t interval;
bool normalMode = true;
uint8_t timeCount = 0;
uint32_t now;
uint32_t cmdModeTimeCnt;
uint32_t cmdModeTimeLimit = CMD_MODE_TIME_LIMIT * ONE_MINUTE;
extern char *__brkval;

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
        meta.maxSounds = 5;
        meta.shuffleEnb = false;
        meta.offDelayTime= 0;
        meta.interval = 255;

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

    boombox.ampDisable();       // disable amplifier 
    digitalWrite(boombox.pinMute, LOW);

    // set device mode
    if (meta.devMode == 0)
    {
       // in standalone mode. initialize button for triggering system
       Serial.println(F("In STANDALONE mode, initializing button..."));
       boombox.buttonInit(); 
    }
    else
    {
        Serial.println(F("TRAILCAM mode set. Please connect trailcam to Boombox..."));
    }

    // display banner for version and diagnostic info
    boombox.dispBanner();
    Serial.println(F("Boombox Deployment Headless Periodic Playback"));
    Serial.print(F("Version: "));
    Serial.println(F(SKETCH_VERSION));
    Serial.println(F("Designed by FreakLabs"));
    printf("Current time is %s.\n", boombox.rtcPrintTimeAndDate());  
    Serial.println(F("-------------------------------------------"));  

    // setup boombox configuration
    // set max sounds, set the playlist to be active, then initialize playlist
    boombox.setMaxSounds(meta.maxSounds);
    if (meta.maxSounds > 0)
    {
        if ((playlist = (uint8_t *)malloc(meta.maxSounds)) == NULL)
        {
            Serial.println(F("No memoryto initialize playlist!!!"));
            cmd.poll();
            while(1); // stay here forever!
        }        
    }
    else
    {
        printf_P(PSTR("Max Sounds currently set to 0. Please set to whole number.\n"));
        cmd.poll();
        while(1);
    }
    boombox.setActivePlaylist(playlist);
    boombox.initPlaylist(playlist, meta.maxSounds, meta.shuffleEnb);
    
    // set shuffle seed based on current number of seconds
    currentTime = boombox.rtcGetTime();
    randomSeed(currentTime.sec);

    // set the playlist shuffle functionality based on metadata settings
    // default is sequential ordering
    if (meta.shuffleEnb == 1)
    {
        ts_t currentTime = boombox.rtcGetTime();
        randomSeed(currentTime.sec);
        boombox.setShuffle(true);
    } 

    // set parameters
    interval = meta.interval;

    // clear trigger interrupt flag if it is set
    boombox.clearAuxFlag();

    // set up real time clock timer
    boombox.rtcClearIntp();
    boombox.rtcEnableTimer();

    // enable the watchdog timer for 8 seconds
    wdt_enable(WDTO_8S); 

    // print out free RAM
    int ram = freeMemory();
    printf("Free RAM available: %d\n", ram);  
      
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
        if (boombox.rtcIntpRcvd())
        {            
            // get current time so we can parse it
            currentTime = boombox.rtcGetTime();
            //printf_P(PSTR("Interrupt received. %02d:%02d.\n"), currentTime.hour, currentTime.min);
    
            // decrement interval counter
            if (interval > 0)
            {
                interval--;
                printf_P(PSTR("Interval: %d.\n"), interval);
            }
            else
            {
                // reload interval
                interval = meta.interval;

                // check if playback is allowed in this interval
                if (checkPlaybackAllowed(currentTime.hour))
                {
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
            
                    // enable amp
                    boombox.ampEnable();
                    delay(AMP_ENABLE_DELAY); // this delay is short and just so the start of the sound doesn't get cut off as amp warms up
                    digitalWrite(boombox.pinMute, HIGH);
            
                    // play sound based on randomized playlist
                    printf_P(PSTR("Playing sound index %d.\n"), nextSound);
                    boombox.playBusy(nextSound);    
        
                    // disable amp before going to sleep
                    digitalWrite(boombox.pinMute, LOW); 
                    delay(500);
                    boombox.ampDisable();     
                            
                    // delay for offDelayTime milliseconds. This is the time after playback finishes but we do not allow another sound
                    // to be triggered.
                    now = millis();
                    while (elapsedTime(now) < ((uint32_t)meta.offDelayTime * 1000))
                    {
                        wdt_reset();
                    }                         
                }  
                else
                {
                    Serial.println(F("Playback not allowed at this time."));          
                }
            }
                                    
            // clear interrupt flag
            boombox.rtcClearIntp(); 
            
            // restart timer
            rtc.resetTimer();
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

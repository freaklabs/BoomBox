/*
Deployment - Hourly Playback
This is an application specific sketch based on the original deployment code. 
This application plays back a sound at the same time (in minutes) every hour

Instructions for use:
1) Define the number of minutes after the hour to play back the sound.
Command: 'setplaytime <minutes>'
Example: 'setplaytime 20'
Will play one sound at 20 minutes past each hour

2) To test this code, leave system connected to a computer with the USB dongle 
inserted into Boombox. Run this code and keep serial monitor open over the course
of 1 hour or more. It will print every minute that passes and also print the
minute that playback will occur.
Ex:
(set to play back 21 minutes past every hour)
interrupt received. min = 15.
interrupt received. min = 16.
interrupt received. min = 17.
interrupt received. min = 18.
interrupt received. min = 19.
interrupt received. min = 20.
interrupt received. min = 21.
playback
Index: 1, Val: 2
interrupt received. min = 22.
interrupt received. min = 23.
interrupt received. min = 24.
interrupt received. min = 25.
interrupt received. min = 26.
interrupt received. min = 27.

*/

#include <cmdArduino.h>
#include <EEPROM.h>
#include <limits.h>
#include <avr/wdt.h>

#if (BOOMBOXBASE == 1)
    #warning "BOOMBOX_BASE"
    #include <boomboxBase.h>
    #define boombox bbb
#elif (BOOMBOX == 1)
    #warning "BOOMBOX"
    #include <boombox.h>
    #include <Rtc_Pcf8563.h>
    #define boombox bb
    #define TIMER_MINUTE 1
    #define MIN_PER_DAY 1440
    Rtc_Pcf8563 rtc; 
#else
    #warning "BOOMBOX"
    #include <boombox.h>
    #include <Rtc_Pcf8563.h>
    #define boombox bb
    Rtc_Pcf8563 rtc; 
#endif

#define SKETCH_VERSION "1.16"
#define TESTONLY 0

#define EEPROM_META_LOC 0
#define MAX_FIELD_SIZE 50
#define AMP_ENABLE_DELAY 500
#define CMD_MODE_TIME_LIMIT 30
#define START_WAIT_TIME 5000
#define ONE_MINUTE 60000
#define NUM_PLAYLISTS 2
#define MAX_SOUNDS 50

SoftwareSerial ss(9, 8);

typedef struct
{
    char devName[MAX_FIELD_SIZE];
    uint8_t devID;
    uint8_t maxSounds;
    uint8_t shuffleEnable;
    uint8_t devMode;    
    uint16_t delayTime;
    uint16_t offDelayTime;
    uint8_t playbackTime;
} meta_t;
meta_t meta;

// this is only used for printf
FILE uartout = {0,0,0,0,0,0,0,0};

char bufTime[MAX_FIELD_SIZE];
bool normalMode = true;
uint8_t timeCount = 0;
uint32_t now;
uint32_t cmdModeTimeCnt;
uint32_t cmdModeTimeLimit = CMD_MODE_TIME_LIMIT * ONE_MINUTE;

// playlist management
uint8_t currentPlaylist = 0;
uint8_t *playlist[NUM_PLAYLISTS];
uint8_t index = 0;

/************************************************************/
// setup
/************************************************************/
void setup() 
{
    // fill in the UART file descriptor with pointer to writer.
    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartout ;       
    
    // initialize command line
    cmd.begin(57600);

    // get metadata
    EEPROM.get(EEPROM_META_LOC, meta);
    if (meta.devID == 0xFF)
    {
        // EEPROM uninitialized. initialize metadata
        Serial.println(F("EEPROM uninitialized. Installing default configuration settings."));
        Serial.println(F("Reset when finished."));
        memset(meta.devName, 0, sizeof(meta.devName));
        memcpy(meta.devName, "TEST", strlen("TEST"));
        meta.devID = 0;
        meta.shuffleEnable = 0;
        meta.devMode = 0;    
        meta.delayTime = 0;
        meta.offDelayTime= 0;
        meta.playbackTime = 0;
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

#if (TESTONLY == 1)
    #warning "THIS MODE SHOULD NOT BE USED FOR DEPLOYMENT!"
    Serial.println(F("THIS MODE SHOULD NOT BE USED FOR DEPLOYMENT!"));
    
    // this is ony for development testing. Do not use in deployment mode
    if (meta.devMode == 0)
    {
       // in standalone mode. initialize button for triggering system
       Serial.println(F("Initializing button."));
       boombox.buttonInit(); 
    }
#endif        

    boombox.ampDisable();       // disable amplifier  

    // display banner for version and diagnostic info
    boombox.dispBanner();
    Serial.print(F("Boombox Deployment Hourly Playback Sketch version: "));
    Serial.println(F(SKETCH_VERSION));
    Serial.println(F("Designed by FreakLabs"));
    printf("Current time is %s.\n", rtcPrintTimeAndDate());  
    Serial.println(F("-------------------------------------------"));

#if (BOOMBOX == 1)
    printf_P(PSTR("Current time is %s.\n"), boombox.rtcPrintTimeAndDate());  
#else
    printf_P(PSTR("Current time is unavailable.\n"), boombox.rtcPrintTimeAndDate()); 
#endif
    
    // set maximum sounds based on metadata   
    boombox.setMaxSounds(meta.maxSounds);      
    cmdTableInit();  
    
    // set the playlist shuffle functionality based on metadata settings
    // default is sequential ordering
    if (meta.shuffleEnable == 1)
    {
        boombox.shuffleSeed();
        boombox.shuffleEnable(true);
    } 

    // create the initial playlist
    boombox.initPlaylist();  

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
        if (boombox.rtcIntpRcvd())
        {            
            ts_t time = boombox.rtcGetTime();
            printf("interrupt received. min = %d.\n", time.min);
            
            if (time.min == meta.playbackTime)
            {
                Serial.println("playback");
                
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
        
                // play sound based on randomized playlist
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

            rtc.resetTimer();
            
            // minute counter interrupt
            boombox.rtcClearIntp(); 
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

/*
Deployment - Time-based Playlists
This is an application specific sketch based on the original deployment code. 
This application allows for a maximum of 2 playlists which can be played based on time of day.

Instructions for use:
1) Define the number of sounds for playlist 0. This assumes that the sounds on the SD card 
follow the standard naming convention, ie: 001.mp3, 001-leopard.mp3, leopard-001.mp3, etc
Command: 'setnumsounds 0 <number of sounds>'

2) Define the number of sounds for playlist 1. The starting index for playlist 1 sounds should be immediately after
the last sound in playlist 0. For example, if you have 40 sounds in playlist 0, then the starting index for the first sound
for playlist 1 should be 41. This assumes that the sounds on the SD card 
follow the standard naming convention, ie: 041.mp3, 041-leopard.mp3, leopard-041.mp3, etc.
Command: 'setnumsounds 1 <number of sounds>'

3) Set the start time of playlist 0. This is based on a daily schedule so just set the time in hours and minutes. Please note this 
is based on a 24-hour time.
Command: 'setplaylisttime 0 <hour> <min>' 
Example: setplaylisttime 0 5 00
Sets the start time of playlist 0 to 5 am every day.

4) Set the start time of playlist 1. This is based on a daily schedule so just set the time in hours and minutes. Please note this 
is based on a 24-hour time.
Command: 'setplaylisttime 1 <hour> <min>' 
Example: setplaylisttime 1 18 00
Sets the start time of playlist 1 to 6 pm every day.

5) Setting the shuffle enable config parameter to 1 will shuffle both playlists each time they go through one full cycle. 

6) To test this code is working, you can go into the command line mode (type 'c' immediately after reset). Then use the testactive command
with whatever time you want to display which playlist will be active.
Command: 'testactive <hour> <min>'
Example: 'testactive 3 15'
This will inform you which playlist is active at 3:15 am based on your settings.

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

#define SKETCH_VERSION "1.17"
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
    uint8_t shuffleEnable;
    uint8_t devMode;    
    uint16_t devInterval;
    uint16_t delayTime;
    uint16_t offDelayTime;
    ts_t playListTime[NUM_PLAYLISTS];
    uint8_t numSounds[NUM_PLAYLISTS];
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

    // limit the max sounds just so we don't crash the system on initialization
    for (int i=0; i<NUM_PLAYLISTS; i++)
    {
        if (meta.numSounds[i] > MAX_SOUNDS)
        {
            meta.numSounds[i] = MAX_SOUNDS;
            EEPROM.put(EEPROM_META_LOC, meta);
        }
    }
        
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
        meta.devInterval = 255;
        meta.delayTime = 0;
        meta.offDelayTime= 0;
        meta.playListTime[0].hour = 0;
        meta.playListTime[0].min = 0;
        meta.playListTime[1].hour = 0;
        meta.playListTime[1].min = 0;
        meta.numSounds[0] = 5;
        meta.numSounds[0] = 5;
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
    Serial.print(F("Boombox Deployment Timebased Playlist Sketch version: "));
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
    cmdTableInit();  
    
    // set the playlist shuffle functionality based on metadata settings
    // default is sequential ordering
    if (meta.shuffleEnable == 1)
    {
        boombox.shuffleSeed();
    } 

    // create the initial playlist
    initPlaylists();  

    // enable the watchdog timer for 8 seconds
    wdt_enable(WDTO_8S);

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
        // check if auxiliary (trailcam) event has triggered
        if (boombox.isAuxEvent() == true)
        {         
            uint8_t listNum;
                           
            // button has been pushed 
            Serial.println(F("Trailcam event."));

            // check which playlist we will use. 
            uint8_t hr = rtc.getHour();
            uint8_t min = rtc.getMinute();
            listNum = playlistChooser(hr, min);            
            printf_P(PSTR("Using playlist %d.\n"), listNum);        
                
            // delay for delayTime milliseconds after trigger has happened. 
            // This delays playing the sound immediately after trigger 
            now = millis();
            while (elapsedTime(now) < ((uint32_t)meta.delayTime * 1000))
            {
                wdt_reset();
            }
              
            // retrieve next sound index
            nextSound = getNextSound(listNum);  
    
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
            uint32_t totalDelay = (uint32_t)meta.offDelayTime *  1000;   
            while (elapsedTime(now) < totalDelay)
            {
                wdt_reset();
            }            
            Serial.println("*** Done ***");

            // clear interrupt flag after sample is played
            boombox.clearAuxFlag();
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

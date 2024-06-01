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

#define TESTONLY 0
#define SKETCH_VERSION "1.23"

#define EEPROM_META_LOC 0
#define MAX_FIELD_SIZE 30
#define AMP_ENABLE_DELAY 1000
#define CMD_MODE_TIME_LIMIT 30
#define START_WAIT_TIME 5000
#define ONE_MINUTE 60000
#define STRUCT_VERSION 2

SoftwareSerial ss(9, 8);

typedef struct
{
    char structVer;
    char devName[MAX_FIELD_SIZE];
    uint8_t devID;
    uint8_t maxSounds;
    uint8_t shuffleEnable;
    uint8_t devMode;    
    int16_t devInterval;
    uint16_t delayTime;
    uint16_t offDelayTime;
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
uint8_t *playlist;

/************************************************************/
// setup
/************************************************************/
void setup() 
{
    // fill in the UART file descriptor with pointer to writer.
    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartout ;       
    
    // initialize command line
    cmd.begin(57600, &Serial);
    cmdTableInit();

    // get metadata
    EEPROM.get(EEPROM_META_LOC, meta);
    if ((meta.devID == 0xFF) || (meta.structVer != STRUCT_VERSION))
    {
        // EEPROM uninitialized. initialize metadata
        Serial.println(F("EEPROM uninitialized. Installing default configuration settings."));
        Serial.println(F("Reset when finished."));
        memset(meta.devName, 0, sizeof(meta.devName));
        memcpy(meta.devName, "TEST", strlen("TEST"));
        meta.structVer = STRUCT_VERSION;
        meta.devID = 0;
        meta.maxSounds = 5;
        meta.shuffleEnable = 0;
        meta.devMode = 1;    
        meta.devInterval = 255;
        meta.delayTime = 0;
        meta.offDelayTime= 0;
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
    
    // this is ony for development testing. Do not use in deployment mode
    if (meta.devMode == 0)
    {
       // in standalone mode. initialize button for triggering system
       Serial.println(F("Initializing button."));
       boombox.buttonInit(); 
    }
#endif        

    boombox.ampDisable();       // disable amplifier  
    digitalWrite(boombox.pinMute, LOW); // mute system

    // display banner for version and diagnostic info
    boombox.dispBanner();
    Serial.print(F("Boombox Deployment Sketch version: "));
    Serial.println(F(SKETCH_VERSION));
    Serial.println(F("Designed by FreakLabs"));
    printf("Current time is %s.\n", rtcPrintTimeAndDate());  
    Serial.println(F("-------------------------------------------"));
    
    // setup boombox configuration
    // set max sounds, set the playlist to be active, then initialize playlist
    boombox.setMaxSounds(meta.maxSounds);
    if ((playlist = (uint8_t *)malloc(meta.maxSounds)) == NULL)
    {
        Serial.println(F("No memoryto initialize playlist!!!"));
        while(1); // stay here forever!
    }
    boombox.setActivePlaylist(playlist);
    boombox.initPlaylist(playlist, meta.maxSounds, meta.shuffleEnable);

#if (BOOMBOX == 1) 
    // set the playlist shuffle functionality based on metadata settings
    // default is sequential ordering
    if (meta.shuffleEnable == 1)
    {
        ts_t currentTime = boombox.rtcGetTime();
        randomSeed(currentTime.sec);
        boombox.setShuffle(true);
    } 
#else
    if (meta.shuffleEnable == 1)
    {
        boombox.shuffleSeed();
    }
#endif // BOOMBOX
    // clear interrupt flag if it is set
    boombox.clearAuxFlag(); 
   
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
            // button has been pushed 
            Serial.println(F("Trailcam event."));
    
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
            boombox.playBusy(nextSound);    

            // disable amp before going to sleep. Short delay so sound won't get cut off too suddenly
            // with additional delay after to allow amp to shut down
            digitalWrite(boombox.pinMute, LOW); 

            // disable amp before going to sleep. Short delay so sound won't get cut off too suddenly
            // with additional delay after to allow amp to shut down
            delay(500);
            boombox.ampDisable();  
            delay(1000);   
                    
            // delay for offDelayTime milliseconds. This is the time after playback finishes but we do not allow another sound
            // to be triggered.
            now = millis();
            while (elapsedTime(now) < ((uint32_t)meta.offDelayTime * 1000))
            {
                wdt_reset();
            }            

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

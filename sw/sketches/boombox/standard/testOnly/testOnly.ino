#include <cmdArduino.h>
#include <EEPROM.h>

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
#elif defined(ARDUINO_ARCH_MEGAAVR)
    #warning "BOOMBOXMEGA"
    #include <boomboxMega.h>
    #include <Rtc_Pcf8563.h>
    #define boombox bbm
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
#define STRUCT_VERSION 1

#ifndef ARDUINO_ARCH_MEGAAVR   
    SoftwareSerial ss(9, 8);
#endif

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

char bufTime[MAX_FIELD_SIZE];
uint8_t *playlist;

// this is only used for printf
FILE uartout = {0,0,0,0,0,0,0,0};

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
        meta.devMode = 0;    
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
#elif defined(ARDUINO_ARCH_MEGAAVR)
    boombox.begin(&Serial1, &rtc);              
#else
    boombox.begin(&ss);
#endif

    boombox.ampDisable();       // disable amplifier 
    digitalWrite(boombox.pinMute, LOW);

    if (meta.devMode == 0)
    {
       // in standalone mode. initialize button for triggering system
       Serial.println(F("STANDALONE mode set. Initializing button..."));  
       #if defined(ARDUINO_ARCH_MEGAAVR)
            pinMode(boombox.pinButton, INPUT_PULLUP);
            pinMode(boombox.pinTrig, INPUT_PULLUP);   
        #else
            pinMode(boombox.pinButton, INPUT_PULLUP);
        #endif        
    }
    else
    {
        Serial.println(F("TRAILCAM mode set. Please connect trailcam to Boombox..."));
    }
        
    // display banner for version and diagnostic info
    boombox.dispBanner();
    Serial.print(F("Boombox TestOnly Sketch version: "));
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
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
    uint8_t nextSound;

    wdt_reset();
    cmd.poll();

#if defined(ARDUINO_ARCH_MEGAAVR)
    if (boombox.buttonIntpRcvd())
    {
        delay(100);
        Serial.println("Button interrupt received.");
        boombox.buttonClearIntp();
    }
#endif

    if (boombox.isAuxEvent() == true)
    {    
        // button has been pushed 
        Serial.println(F("Trigger event."));
        printf("AuxFlag: %d\n", boombox.isAuxEvent());

        // retrieve next sound index
        nextSound = boombox.getNextSound();

        // enable amp
        boombox.ampEnable();       
        delay(AMP_ENABLE_DELAY); // this delay is short and just so the start of the sound doesn't get cut off as amp warms up
        digitalWrite(boombox.pinMute, HIGH);

        // play next sound
        boombox.playBusy(nextSound); 

        // disable amp before going to sleep. Short delay so sound won't get cut off too suddenly
        // with additional delay after to allow amp to shut down
        digitalWrite(boombox.pinMute, LOW); 
        delay(500);
        boombox.ampDisable();           

        // clear interrupt flag
        boombox.clearAuxFlag(); 
        printf("AuxFlag: %d\n", boombox.isAuxEvent());                    
    }      
}

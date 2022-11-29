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
#else
    #warning "BOOMBOX"
    #include <boombox.h>
    #include <Rtc_Pcf8563.h>
    #define boombox bb
    Rtc_Pcf8563 rtc; 
#endif

#define EEPROM_META_LOC 0
#define MAX_FIELD_SIZE 50

SoftwareSerial ss(9, 8);

typedef struct
{
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
    if (meta.devID == 0xFF)
    {
        // EEPROM uninitialized. initialize metadata
        Serial.println(F("EEPROM uninitialized. Please set EEPROM."));
        Serial.println(F("Reset when finished."));
        while (1)
        {
            cmd.poll();
        }
    }

#if (BOOMBOX_BASE == 1)            
    boombox.begin(&ss);
#else
    boombox.begin(&ss, &rtc);
#endif

    if (meta.devMode == 0)
    {
       // in standalone mode. initialize button for triggering system
       Serial.println(F("Initializing button."));
       boombox.buttonInit(); 
    }
    boombox.setMaxSounds(meta.maxSounds);
    
    boombox.dispBanner();
    Serial.println(F("Boombox Standalone Sketch"));

    // set the playlist shuffle functionality based on metadata settings
    // default is sequential ordering
    boombox.shuffleEnable(meta.shuffleEnable);

    // create the initial playlist
    boombox.initPlaylist();        
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
    uint8_t nextSound;

    wdt_reset();
    cmd.poll();

    if (boombox.isAuxEvent() == true)
    {    
        // button has been pushed 
        Serial.println(F("Trigger event."));

        // retrieve next sound index
        nextSound = boombox.getNextSound();

        // play next sound
        boombox.play(nextSound); 

        // clear interrupt flag
        boombox.clearAuxFlag();                     
    }      
}
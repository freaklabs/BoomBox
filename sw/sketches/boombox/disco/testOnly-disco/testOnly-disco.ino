#include <cmdArduino.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

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
#define LED_COUNT 60
#define LED_PIN0 4
#define LED_PIN1 10

SoftwareSerial ss(9, 8);
Adafruit_NeoPixel strip0(LED_COUNT, LED_PIN0, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip1(LED_COUNT, LED_PIN1, NEO_GRB + NEO_KHZ800);

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
    
    // enable power to LEDs
    boombox.auxEnable();

    // LED initialization
    strip0.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip0.show();            // Turn OFF all pixels ASAP
    strip0.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)    

    // LED handling
    strip1.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip1.show();            // Turn OFF all pixels ASAP
    strip1.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)    
    
    // set up rest of boombox
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

        // add a delay for busy pin to go low
        delay(100);
        
        // dispay LEDs
        while(boombox.isBusy())
        {
            strip0.fill(strip0.Color(255, 255, 255));
            strip1.fill(strip1.Color(255, 255, 255));
            strip0.show();
            strip1.show();
            delay(50);
            strip0.clear();
            strip1.clear();
            strip0.show();
            strip1.show();
            delay(50);         
        }        

        // clear interrupt flag
        boombox.clearAuxFlag();                     
    }      
}

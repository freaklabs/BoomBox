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

#define SKETCH_VERSION "1.18"
#define EEPROM_META_LOC 0
#define MAX_FIELD_SIZE 30
#define AMP_ENABLE_DELAY 1000
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

char bufTime[MAX_FIELD_SIZE];

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
        Serial.println(F("EEPROM uninitialized. Installing default configuration settings."));
        Serial.println(F("Reset when finished."));
        memset(meta.devName, 0, sizeof(meta.devName));
        memcpy(meta.devName, "TEST", strlen("TEST"));
        meta.devID = 0;
        meta.maxSounds = 5;
        meta.shuffleEnable = 0;
        meta.devMode = 0;    
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

    if (meta.devMode == 0)
    {
       // in standalone mode. initialize button for triggering system
       Serial.println(F("Initializing button."));
       boombox.buttonInit(); 
    }
    boombox.setMaxSounds(meta.maxSounds);
    
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
    
    // display banner for version and diagnostic info
    boombox.dispBanner();
    Serial.print(F("Boombox Disco-TestOnly Sketch version: "));
    Serial.println(F(SKETCH_VERSION));
    Serial.println(F("Designed by FreakLabs"));
    printf(PSTR("Current time is %s.\n"), boombox.rtcPrintTimeAndDate());  
    Serial.println(F("-------------------------------------------"));

    // set the playlist shuffle functionality based on metadata settings
    // default is sequential ordering
    if (meta.shuffleEnable == 1)
    {
        boombox.shuffleSeed();
        boombox.shuffleEnable(true);
    } 

    // create the initial playlist
    boombox.initPlaylist();     

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

    if (boombox.isAuxEvent() == true)
    {    
        // button has been pushed 
        Serial.println(F("Trigger event."));

        // retrieve next sound index
        nextSound = boombox.getNextSound();

        // enable amp
        boombox.ampEnable();       
        delay(AMP_ENABLE_DELAY); // this delay is short and just so the start of the sound doesn't get cut off as amp warms up

        // play next sound
        boombox.play(nextSound); 

        // add a delay for busy pin to go low
        delay(100);
        
        // dispay LEDs
        while(boombox.isBusy())
        {
            wdt_reset();
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

        // disable amp before going to sleep. Short delay so sound won't get cut off too suddenly
        // with additional delay after to allow amp to shut down
        delay(500);
        boombox.ampDisable();              

        // clear interrupt flag
        boombox.clearAuxFlag();                     
    }      
}

#include <cmdArduino.h>
#include <LowPower.h>
#include <boombox.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#include <limits.h>

#define TESTONLY 1
#define MAX_SOUNDS 5

#if (TESTONLY == 1)
    #define TICKSPERMINUTE 0
    #define DBG_PRINT(...)   Serial.print(__VA_ARGS__)
    #define DBG_PRINTLN(...) Serial.println(__VA_ARGS__)
    #define DBG_PRINTF(...)  printf(__VA_ARGS__)    
#else
    #define TICKSPERMINUTE 15
    #define DBG_PRINT(...)
    #define DBG_PRINTLN(...)
    #define DBG_PRINTF(...)     
#endif

#define METADATA_EEPROM_LOC 0
#define ONE_MINUTE 60000
#define START_WAIT_TIME 3000
#define CMD_MODE_TIME_LIMIT 10
#define AMP_ENABLE_DELAY 500

typedef struct
{
    uint16_t boomboxId;
    uint16_t playbackInterval;
} boomboxMeta_t;
boomboxMeta_t meta;

// basic configuration
static FILE uartout = {0,0,0,0,0,0,0,0};
SoftwareSerial ss(9, 8);
Rtc_Pcf8563 rtc; 

// boombox specific variables
int index = 0;
uint32_t delayTime = 0;      
uint32_t durationTime = 10000;   
uint32_t offDelayTime = 0;

// interval counter
volatile bool flagWdt = false;
int16_t minuteCtr = 0;
uint8_t intervalMinutes = 0;

// command line test mode
bool normalMode = true;
uint8_t timeCount = 0;
uint32_t cmdModeTimeCnt;
uint32_t cmdModeTimeLimit = CMD_MODE_TIME_LIMIT * ONE_MINUTE;


/**************************************************************************/
// Setup
/**************************************************************************/
void setup() 
{
    // to set up the printf command. it will make life a lot easier
    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartout ;

    configWdt();    

    // clear and retrieve meta data structure
    memset(&meta, 0, sizeof(boomboxMeta_t));
    getMeta();
    
    cmd.begin(57600);
    bb.begin(&ss, &rtc);
    
    // display setup banner
    // delay a bit before sleeping so it can print out banner
    bb.dispBanner();            
    DBG_PRINTLN(F("Standalone Playback Sketch v0.5"));

    cmdTableInit();

    // select development mode
    selectMode();    
}

/**************************************************************************/
// Loop
/**************************************************************************/
void loop() 
{  
    if (flagWdt)
    {
        wdt_reset();
        flagWdt = false;

        // check if we need to increment the number of minutes
        if (minuteCtr >= TICKSPERMINUTE)
        {
            minuteCtr = 0;
            intervalMinutes++;     
            
            // check if we're in normal mode. If so, operate as if we're in deployment
            if (normalMode)
            {           
                DBG_PRINTLN(F("ONE MINUTE"));   
                
                // if we've hit our playback interval, reset the interval and play a sound
                if (intervalMinutes >= meta.playbackInterval)
                {
                    intervalMinutes = 0;
                    
                    // play a sound now
                    playSound();
                }     
            }
        }
        else
        {
            // increment minute counter
            minuteCtr++; 
        } 
    }

    if (normalMode)
    {
        // in normal mode, our idle time will be spent in sleep mode
        Serial.flush();
        sysSleep();
    }
    else
    {
        // in command line mode, we'll idle by waiting for a new command to come in
        cmd.poll(); 
        
        // if we're not in normal mode, then we're in command line mode
        // if we've hit our command mode time limit, then query if we should stay here
        // otherwise, we'll go back to normal mode
        if (elapsedTime(cmdModeTimeCnt) > cmdModeTimeLimit)
        {
            selectMode();
        }     
    }    
}

/**************************************************************************/
// watchdog isr
/**************************************************************************/
ISR(WDT_vect) 
{
    flagWdt = true;
}

/**************************************************************************/
// sysSleep
/**************************************************************************/
void sysSleep()
{
    bb.ampDisable();
    bb.sleep(); // sleep here
}

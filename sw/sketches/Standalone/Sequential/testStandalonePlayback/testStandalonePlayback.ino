#include <cmdArduino.h>
#include <LowPower.h>
//#include <boombox.h>
#include <avr/wdt.h>
#include <EEPROM.h>

#define MAX_SOUNDS 9
#define TICKSPERMINUTE 15
#define METADATA_EEPROM_LOC 0
#define ONE_MINUTE 60000
#define START_WAIT_TIME 3000
#define CMD_MODE_TIME_LIMIT 5

typedef struct
{
    uint16_t boomboxId;
    uint16_t playbackInterval;
} boomboxMeta_t;
boomboxMeta_t meta;

// basic configuration
static FILE uartout = {0};
int index = 0;
uint32_t offDelayTime = 0;
uint8_t pin5vEnb = A1;

// interval counter
volatile bool flagWdt = false;
uint8_t minuteCtr = 0;
uint8_t intervalMinutes = 0;

// command line test mode
bool normalMode = true;
uint8_t timeCount = 0;
uint32_t cmdModeTimeCnt;
uint32_t cmdModeTimeLimit = CMD_MODE_TIME_LIMIT * ONE_MINUTE;


/**************************************************************************/
// 
/**************************************************************************/
void setup() 
{
    // to set up the printf command. it will make life a lot easier
    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartout ;

    // clear and retrieve meta data structure
    memset(&meta, 0, sizeof(boomboxMeta_t));
    getMeta();
    
    pinMode(pin5vEnb, OUTPUT);
    digitalWrite(pin5vEnb, LOW); 
    

    cmd.begin(57600);
    printf("testStandalonePlayback\n");

    cmdTableInit();
        
    //bb.init();
    configWdt();

    // select development mode
    selectMode();    
}


/**************************************************************************/
// 
/**************************************************************************/
void loop() 
{
    if (flagWdt)
    {
        wdt_reset();
        flagWdt = false;

        // check if we need to increment the number of minutes in interval
        if (minuteCtr >= TICKSPERMINUTE)
        {
            minuteCtr = 0;
            intervalMinutes++;     
            Serial.println("ONE MINUTE");   

            if (normalMode)
            {
                if (intervalMinutes >= meta.playbackInterval)
                {
                    intervalMinutes = 0;
                    Serial.println("PLAY A SOUND");  
                }     
            }
            else
            {
                if (intervalMinutes >= CMD_MODE_TIME_LIMIT)
                {
                    intervalMinutes =0;
                    selectMode();   
                }
            }
        }
        else
        {
            minuteCtr++; 
        } 
    }

    if (normalMode)
    {
        Serial.flush();
        sysSleep();
    }
    else
    {
        cmd.poll();    
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
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
}

#include <SdFat.h>
#include <cmdArduino.h>
#include <boomboxWireless.h>
#include <avr/wdt.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <LowPower.h>
#include <avr/wdt.h>
#include <Rtc_Pcf8563.h>
#include <EEPROM.h>

/****************************************************************************
 * Defines
 ****************************************************************************/
// define what we log to SD card
#define LOG_DATA 1
#define LOG_MESSAGES 1
#define SKETCH_VERSION "0.5"

#define ANALOGREFERENCE 3.3
#define ANALOGLEVELS 1024
#define META_FIELDSIZE_MAX 50
#define METADATA_EEPROM_LOC 0
#define RH95_NUM_RETRIES 5
#define RH95_CAD_TIMEOUT 10000
#define RH95_FREQ 925.0
#define RH95_TX_POWER 20
#define MAX_PAYLOAD 140
#define TIMESTAMP_SIZE 50
#define DATAFILE "DATAFILE.TXT"
#define LOGFILE "LOGFILE.TXT"
#define TRANSMIT_INTERVAL   1000
#define ONE_MINUTE 60000
#define CMD_MODE_TIME_LIMIT 5
#define START_WAIT_TIME 2000
#define MAX_SOUNDS 10

// packet structure
#define CMD_PLAY 0xA
#define CMD_STOP 0xB
#define CMD_STRING 0xC

/****************************************************************************
 * Structs
 ****************************************************************************/
typedef struct
{
    uint8_t devAddr;
    uint8_t gwAddr;
    uint16_t txInterval;
    char devID[META_FIELDSIZE_MAX];
    char devSite[META_FIELDSIZE_MAX];
} boombox_meta_t;
 boombox_meta_t meta;

typedef struct
{
    uint8_t sec;         /* seconds */
    uint8_t min;         /* minutes */
    uint8_t hour;        /* hours */
    uint8_t mday;        /* day of the month */
    uint8_t mon;         /* month */
    int16_t year;        /* year */
    uint8_t wday;        /* day of the week */
    uint8_t yday;        /* day in the year */
    uint8_t isdst;       /* daylight saving time */
    uint8_t year_s;      /* year in short notation*/
} ts_t;

/****************************************************************************
 * Variables
 ****************************************************************************/
uint8_t pinRadioSelN = 14;  
uint8_t pinRadioIntp = 2;
uint8_t pinRadioReset = 19; // not actual reset pin
uint8_t pinSdSelN = 15; 
uint8_t pinTrigger = 3;

uint8_t intpRtcNum = 2;
volatile bool flagRtcIntp = false;

/****************************************************************************
 * Objects
 ****************************************************************************/
// real time clock
Rtc_Pcf8563 rtc;

// lora 
RH_RF95 driver(pinRadioSelN, pinRadioIntp);
RHReliableDatagram lora(driver);

// sd card
SdFat sd;
File myFile;
FILE uartout = {0}; 

uint8_t index = 0;
uint32_t offDelayTime = 0;
bool sdAvailable = false;
uint8_t seq = 0;

bool normalMode = true;
uint8_t timeCount = 0;
uint32_t cmdModeTimeCnt;
uint32_t cmdModeTimeLimit = CMD_MODE_TIME_LIMIT * ONE_MINUTE;
uint8_t buf[MAX_PAYLOAD];
char bufTime[TIMESTAMP_SIZE];
char tmp[MAX_PAYLOAD];

/************************************************************/
// SETUP
/************************************************************/
void setup() 
{
    ts_t currentTime; 
    
    wdt_disable();

    pinMode(pinRadioSelN, OUTPUT);
    digitalWrite(pinRadioSelN, HIGH);

    pinMode(pinSdSelN, OUTPUT);
    digitalWrite(pinSdSelN, HIGH);

    pinMode(pinTrigger, INPUT_PULLUP);
        
    // for printf
    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartout ;
    
    // get metadata
    getMeta();

    bb.init();
            
    cmd.begin(57600);


    // init SD card
    if (sd.begin(pinSdSelN) != true)
    {
        Serial.println(F("MicroSD card not found."));        
    }
    else
    {
        sdAvailable = true;
        Serial.println(F("MicroSD card found and FAT file system initialized."));
    }

    // init LoRa module
    if (!lora.init())
    {
        Serial.println("LoRa init failed");
    }
    else
    {
        Serial.println(F("LoRa wireless radio found and initialized."));
    }
    lora.setThisAddress(meta.devAddr);
    lora.setRetries(RH95_NUM_RETRIES);
    driver.setFrequency(RH95_FREQ);
    driver.setTxPower(RH95_TX_POWER);
    driver.setModeRx();    

    currentTime = getTime();
    if (currentTime.year < 2020)
    {
        Serial.println(F("Real time clock not found or not set."));
    }
    else
    {
        Serial.println(F("Real time clock found and initialized."));
    }
    Serial.print(F("Current time is: "));
    Serial.println(printTimeAndDate());

    // set up RTC interrupt
    // assuming pin 4 for pin change interrupt
//    attachInterrupt(intpRtcNum, isrRtc, FALLING);    

    // set minute timer
//    rtc.setTimer(1, TMR_1MIN, false);        

    // enable watchdog timer
    wdt_enable(WDTO_8S);  

    // set up the file system callback so that the file can be date and timestamped
    myFile.dateTimeCallback(sdDateTime);         

    // display banner
    printf_P(PSTR("------------------------------------------\n"));
    showBanner();
    printf_P(PSTR("Sketch version %s\n"), SKETCH_VERSION);
    printf_P(PSTR("------------------------------------------\n"));
    logEvent("Reset occurred\n");

    // init command table
    cmdTableInit();
    
    // select if we want to go into developer mode or deployment mode
    selectMode();      
}

/************************************************************/
// LOOP
/************************************************************/
void loop() 
{
    wdt_reset();           

    // handle audio triggers (if connected to trigger source)
    if (bb.isAuxEvent() == true)
    {    
        // button has been pushed 
        Serial.println("Trigger event.");
        
        // play sound. 
        if (index < MAX_SOUNDS)
        {        
                index++;
                Serial.print("Index: ");
                Serial.println(index);
            }
            else
            {
                index = 1;
                Serial.print("Index: ");
                Serial.println(index);
        }
        bb.play(index);
        
        delay(offDelayTime);
        bb.clearAuxFlag();
    }    

    if (normalMode)
    {
        // handle realtime clock interrupt
        if (flagRtcIntp)
        {
            // clear interrupt and reset timer
            flagRtcIntp = false;
            rtc.resetTimer(); 

            // check number of minutes that passed. if equal to our tx interval
            // prepare to send data to gateway
            if (timeCount >= (meta.txInterval-1))
            {
                timeCount = 0;
            }
            else
            {
                timeCount++;
            }              
        }

        // go to sleep
//        sysSleep();
    }
    else
    {
        // we're in developer mode. listen for commands on the command line
        cmd.poll();
        
        // implement timeout in command line mode to confirm we want to stay in that mode. 
        // we don't want to accidentally stay in developer mode in a deployment so check 
        // every X minutes to make sure we want to stay in this mode
        if (elapsedTime(cmdModeTimeCnt) > cmdModeTimeLimit)
        {
            selectMode();
        }  
    }

    // handle lora radio receive interrupt if any data has come in
    if (lora.available())
    {
        uint8_t len = sizeof(buf);
        uint8_t from, to, id, flags;
        int16_t rssi;

        // get data from radio memory & transmit acknowledge to sender
        memset(buf, 0, sizeof(buf));
        if (lora.recvfromAck(buf, &len, &from, &to, &id, &flags))
        {
            rssi = driver.lastRssi();
            printf("Rcv'd from %d: %s, seq: %3d, rssi: %3d\n", from, (char *)buf, id, rssi);

            // put lora in rx mode
            driver.setModeRx();

            // if we're in normal mode, then handle the received data
            if (normalMode)
            {
                // handle received data here
                //sysReceive(from, id, rssi, buf, len);                   
            }
        }
    }     
}

/**************************************************************************/
/*! sysReceive
 *  Handle data arrived from LoRa interface
*/
/**************************************************************************/
void sysReceive(uint8_t from, uint8_t seq, int16_t rssi, uint8_t *rcvData, uint8_t len) 
{
    uint8_t cmdVal;
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "%s,%d,%d,%d,%s", printTimeAndDate(), from, rssi, seq, (char *)rcvData);
    printf("Received from %d w/RSSI %d: ", from, rssi);    
    logData(tmp);

    cmdVal = rcvData[0];
    switch (cmdVal)
    {        
        case CMD_PLAY:
            uint8_t track = rcvData[1];
            if (track > MAX_SOUNDS)
            {
                track = MAX_SOUNDS-1;
            }
            bb.play(track);        
        break;

        case CMD_STOP:
            bb.stop();
        break;

        case CMD_VOLUME:
            uint8_t vol = rcvData[1];
            bb.setVol(vol);
        break;        

        case CMD_STRING:
            sprintf(tmp, "%s\n", (char *)rcvData);
            logEvent(tmp);
        break;        

        default:
        break;
    }    
}

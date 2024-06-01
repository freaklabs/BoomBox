#ifdef ARDUINO_ARCH_MEGAAVR

#include "boomboxMega.h"

BoomboxMega bbm;

static volatile bool rtcFlag;
static volatile bool buttonFlag;
static volatile bool auxFlag;

/************************************************************/
// Constructor
/************************************************************/
BoomboxMega::BoomboxMega()
{
	pinPIR         	= PIN_PD0;
    pinButton      	= PIN_PD2;
    pinBusy        	= PIN_PC2;
    pinTrig         = PIN_PD3;  
    
    pinAmpShutdn   	= PIN_PF0;
    pinBoostEnb    	= PIN_PD4;
    pinMp3Enb      	= PIN_PD5;
    pin5vEnb       	= PIN_PD7;
    pinAuxLed[0]    = PIN_PF4;
    pinAuxLed[1]    = PIN_PF5;
    pinAuxEnb 		= PIN_PD6;
 	pinAuxData0 	= PIN_PF2;
    pinAuxData1 	= PIN_PF3; 
    pinMute         = PIN_PF1;    

    _vol           	= 26;
    _delayVal      	= 0;

    intNumPIR       = PIN_PD0; 
    intNumRtc       = PIN_PD1;
    intNumButton    = PIN_PD2;
    intNumTrig       = PIN_PD3;   
}

/************************************************************/
// begin
/************************************************************/
void BoomboxMega::begin(HardwareSerial *ser, Rtc_Pcf8563 *rtc)
{    
    pinMode(SDA, INPUT_PULLUP);
    pinMode(SCL, INPUT_PULLUP);   
    pinMode(pinButton, INPUT_PULLUP); 

    pinMode(pinAmpShutdn, OUTPUT);
    pinMode(pinMute, OUTPUT);
    pinMode(pinBoostEnb, OUTPUT);
    pinMode(pinMp3Enb, OUTPUT);
    pinMode(pin5vEnb, OUTPUT);
    pinMode(pinAuxEnb, OUTPUT);
    pinMode(pinAuxData0, OUTPUT);
    pinMode(pinAuxData1, OUTPUT);

    for (int i=0; i<NUM_LEDS; i++)
    {
        pinMode(pinAuxLed[i], OUTPUT);
        digitalWrite(pinAuxLed[i], LOW);
    }

    digitalWrite(pin5vEnb, HIGH);
    delay(300);
    digitalWrite(pinBoostEnb, HIGH);

    digitalWrite(pinAmpShutdn, LOW);
    digitalWrite(pinMute, HIGH);
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pinAuxEnb, LOW);
    digitalWrite(pinAuxData0, LOW);
    digitalWrite(pinAuxData1, LOW);

    attachInterrupt(intNumRtc, BoomboxMega::irqRtc, FALLING);
    //attachInterrupt(intNumTrig, BoomboxMega::irqAux, RISING);
    attachInterrupt(intNumButton, BoomboxMega::irqButton, FALLING);
    //attachInterrupt(intNumPIR, BoomboxMega::irqPIR, RISING);

    rtcFlag = false;
    buttonFlag = false;
    auxFlag = false;

    _ser = ser;
    _ser->begin(9600);

    _rtc = rtc;
    _rtc->clearTimer(); 
    delay(100);
    setVol(_vol);
    wdt_enable(WDT_PERIOD_8KCLK_gc);   
}

/*----------------------------------------------------------*/
// Methods for controlling the MP3 player
/*----------------------------------------------------------*/

/*----------------------------------------------------------*/
// enable the auxiliary port power
void BoomboxMega::auxEnable()
{
    digitalWrite(pinAuxEnb, HIGH);
}

/*----------------------------------------------------------*/
// disable the auxiliary port power
void BoomboxMega::auxDisable()
{
    digitalWrite(pinAuxEnb, LOW);
}

/*----------------------------------------------------------*/
void BoomboxMega::dispBanner()
{
    Serial.println(F("-------------------------------------------"));
    Serial.print(F("Boombox Board version: "));
    Serial.println(F(BOARD_VERSION));
    Serial.print(F("Boombox Library version: "));
    Serial.println(F(LIB_VERSION));
}

/*----------------------------------------------------------*/
void BoomboxMega::sleep()
{
    
    // shut down software serial
//    ss->end();
//    DDRB &= 0xFE;

    // shut down everything else
    digitalWrite(pinMp3Enb, LOW);
    digitalWrite(pinAuxEnb, LOW);
    wdt_disable();

    // turn off 5V supply and allow voltage to stabilize
    delay(500);
    digitalWrite(pinBoostEnb, LOW);
    delay(300);
    digitalWrite(pin5vEnb, LOW);

    Serial.println(F("SLEEP NOT IMPLEMENTED"));
//    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
}

/*----------------------------------------------------------*/
void BoomboxMega::wake()
{
    wdt_enable(WDT_PERIOD_8KCLK_gc); 

    // turn on 5V supply and allow voltage to stabilize
    // power sequence to avoid spike
    digitalWrite(pin5vEnb, HIGH);
    delay(300);
    digitalWrite(pinBoostEnb, HIGH);    
    delay(1000);

    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pinAuxEnb, HIGH);
//    DDRB |= 0x01;
//    ss->begin(9600);

    // need a delay here to start up the mp3 player
    delay(100);
}

/*----------------------------------------------------------*/
void BoomboxMega::rtcSetTime(int yr, int mon, int day, int hour, int min, int sec)
{
    _rtc->initClock();
    _rtc->setDateTime(day, 0, mon, 0, yr, hour, min, sec);       
}

/*----------------------------------------------------------*/
ts_t BoomboxMega::rtcGetTime()
{
    ts_t time;
    memset(&time, 0, sizeof(time));

    _rtc->getDateTime();
    time.year = 2000 + _rtc->getYear();
    time.mon = _rtc->getMonth();
    time.mday = _rtc->getDay();
    time.hour = _rtc->getHour();
    time.min = _rtc->getMinute();
    time.sec = _rtc->getSecond();
    return time;    
}

/*----------------------------------------------------------*/
char *BoomboxMega::rtcPrintTime()
{
  ts_t time = rtcGetTime();
  memset(bufTime, 0, sizeof(bufTime));
  sprintf(bufTime, "%02d:%02d:%02d", time.hour, time.min, time.sec);
  return bufTime;
}

/*----------------------------------------------------------*/
char *BoomboxMega::rtcPrintDate()
{
  ts_t time = rtcGetTime();
  memset(bufTime, 0, sizeof(bufTime));
  sprintf(bufTime, "%04d/%02d/%02d", time.year, time.mon, time.mday);
  return bufTime;
}

/*----------------------------------------------------------*/
char *BoomboxMega::rtcPrintTimeAndDate()
{
    ts_t time = rtcGetTime();
    memset(bufTime, 0, sizeof(bufTime));
    sprintf(bufTime, "%04d/%02d/%02d %02d:%02d:%02d", time.year, time.mon, time.mday, time.hour, time.min, time.sec);
    return bufTime;
}

/*----------------------------------------------------------*/
void BoomboxMega::rtcSetTimer(uint8_t minutes)
{
    if (testMode)
    {
        _rtc->setTimer(minutes, TMR_1Hz, false);
    }
    else
    {
        _rtc->setTimer(minutes, TMR_1MIN, false);
    }
}

/*----------------------------------------------------------*/
void BoomboxMega::rtcEnableTimer()
{    
    rtcSetTimer(1);
    _rtc->enableTimer();
}

/*----------------------------------------------------------*/
void BoomboxMega::rtcDisableTimer()
{
    _rtc->clearTimer();
}

/*----------------------------------------------------------*/
void BoomboxMega::irqRtc()
{
    rtcFlag = true;
}

/*----------------------------------------------------------*/
bool BoomboxMega::rtcIntpRcvd()
{
    return rtcFlag;
}

/*----------------------------------------------------------*/
void BoomboxMega::rtcClearIntp()
{
    rtcFlag = false;
}

/************************************************************/
// Handle button interrupt
/************************************************************/

/*----------------------------------------------------------*/
void BoomboxMega::irqButton()
{
    buttonFlag = true;
}

/*----------------------------------------------------------*/
bool BoomboxMega::buttonIntpRcvd()
{
    return buttonFlag;
}

/*----------------------------------------------------------*/
void BoomboxMega::buttonClearIntp()
{
    buttonFlag = false;
}

/*----------------------------------------------------------*/
bool BoomboxMega::buttonIrqHandler()
{
    bool trig = false;
    if (buttonFlag)
    {
        delay(DEBOUNCE_TIME);
        buttonFlag = false;
        trig = true;  
    }
    return trig;
}

/************************************************************/
// Random number seed generator for shuffle 
/************************************************************/
void BoomboxMega::shuffleSeed()
{
    ts_t time = rtcGetTime();
    randomSeed(time.min * time.sec);
}

/************************************************************/
//
/************************************************************/
void BoomboxMega::irqAux(void)
{
   auxFlag = true;
}

/************************************************************/
//
/************************************************************/
bool BoomboxMega::isAuxEvent()
{
    return auxFlag;
}

/************************************************************/
//
/************************************************************/
void BoomboxMega::clearAuxFlag()
{
    // add delay to debounce events
    delay(100);
    auxFlag = false;
}

/************************************************************/
// sendCmd
/************************************************************/
void BoomboxMega::_sendCmd(uint8_t *buf, uint8_t len)
{
    cli();
    for (int i=0; i<len; i++)
    {
        _ser->write( buf[i] );
    }
    sei();
}

#endif //ARDUINO_ARCH_MEGAAVR
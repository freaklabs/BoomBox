#include "boombox.h"

Boombox bb;

static volatile bool pirFlag;
static volatile bool rtcFlag;
static volatile bool auxFlag;

/************************************************************/
// Constructor
/************************************************************/
Boombox::Boombox()
{
	pinPIR         	= 16;
    pinButton      	= 3;
    pinBusy        	= 11;
    
    pinAmpShutdn   	= 5;
    pinBoostEnb    	= 7;
    pinMp3Enb      	= 6;
    pin5vEnb       	= 15;
    pinAuxLed       = 13;
    pinAuxEnb 		= 17;
 	pinAuxData0 	= 4;
    pinAuxData1 	= 10; 
    pinRandSeed     = A0;    

    _vol           	= 26;
    _delayVal      	= 0;

    intNumRtc 		= 0;
    intNumAux 		= 1;    
}

/************************************************************/
// begin
/************************************************************/
void Boombox::begin(SoftwareSerial *sser, Rtc_Pcf8563 *rtc)
{    
    pinMode(pinPIR, INPUT);
    pinMode(pinButton, INPUT);
    pinMode(pinBusy, INPUT);
    pinMode(SDA, INPUT_PULLUP);
    pinMode(SCL, INPUT_PULLUP);    

    pinMode(pinAmpShutdn, OUTPUT);
    pinMode(pinBoostEnb, OUTPUT);
    pinMode(pinMp3Enb, OUTPUT);
    pinMode(pin5vEnb, OUTPUT);
    pinMode(pinAuxEnb, OUTPUT);
    pinMode(pinAuxData0, OUTPUT);
    pinMode(pinAuxData1, OUTPUT);
    pinMode(pinAuxLed, OUTPUT);

    digitalWrite(pin5vEnb, HIGH);
    delay(300);
    digitalWrite(pinBoostEnb, HIGH);

    digitalWrite(pinAmpShutdn, LOW);
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pinAuxEnb, LOW);
    digitalWrite(pinAuxData0, LOW);
    digitalWrite(pinAuxData1, LOW);
    digitalWrite(pinAuxLed, LOW);

    attachInterrupt(intNumRtc, Boombox::irqRtc, FALLING);
    attachInterrupt(intNumAux, Boombox::irqAux, RISING);

    auxFlag = false;
    rtcFlag = false;

    ss = sser;
    ss->begin(9600);

    _rtc = rtc;
    _rtc->clearTimer(); 
    delay(100);
    setVol(_vol);
}

/*----------------------------------------------------------*/
// Methods for controlling the MP3 player
/*----------------------------------------------------------*/

/*----------------------------------------------------------*/
// enable the auxiliary port power
void Boombox::auxEnable()
{
    digitalWrite(pinAuxEnb, HIGH);
}

/*----------------------------------------------------------*/
// disable the auxiliary port power
void Boombox::auxDisable()
{
    digitalWrite(pinAuxEnb, LOW);
}

/*----------------------------------------------------------*/
void Boombox::dispBanner()
{
    Serial.println(F("-------------------------------------------"));
    Serial.print(F("Boombox Board version: "));
    Serial.println(F(BOARD_VERSION));
    Serial.print(F("Boombox Library version: "));
    Serial.println(F(LIB_VERSION));
}

/*----------------------------------------------------------*/
void Boombox::sleep()
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

    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
}

/*----------------------------------------------------------*/
void Boombox::wake()
{
    wdt_enable(WDTO_8S);

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
void Boombox::rtcSetTime(int yr, int mon, int day, int hour, int min, int sec)
{
    _rtc->initClock();
    _rtc->setDateTime(day, 0, mon, 0, yr, hour, min, sec);       
}

/*----------------------------------------------------------*/
ts_t Boombox::rtcGetTime()
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
char *Boombox::rtcPrintTime()
{
  ts_t time = rtcGetTime();
  memset(bufTime, 0, sizeof(bufTime));
  sprintf(bufTime, "%02d:%02d:%02d", time.hour, time.min, time.sec);
  return bufTime;
}

/*----------------------------------------------------------*/
char *Boombox::rtcPrintDate()
{
  ts_t time = rtcGetTime();
  memset(bufTime, 0, sizeof(bufTime));
  sprintf(bufTime, "%04d/%02d/%02d", time.year, time.mon, time.mday);
  return bufTime;
}

/*----------------------------------------------------------*/
char *Boombox::rtcPrintTimeAndDate()
{
    ts_t time = rtcGetTime();
    memset(bufTime, 0, sizeof(bufTime));
    sprintf(bufTime, "%04d/%02d/%02d %02d:%02d:%02d", time.year, time.mon, time.mday, time.hour, time.min, time.sec);
    return bufTime;
}

/*----------------------------------------------------------*/
void Boombox::rtcSetTimer(uint8_t minutes)
{
    _rtc->setTimer(minutes, TMR_1MIN, false);
//    _rtc->setTimer(minutes, TMR_1Hz, false);
}

/*----------------------------------------------------------*/
void Boombox::rtcEnableTimer()
{    
    rtcSetTimer(1);
    _rtc->enableTimer();
}

/*----------------------------------------------------------*/
void Boombox::rtcDisableTimer()
{
    _rtc->clearTimer();
}

/*----------------------------------------------------------*/
void Boombox::irqRtc()
{
    rtcFlag = true;
}

/*----------------------------------------------------------*/
bool Boombox::rtcIntpRcvd()
{
    return rtcFlag;
}

/*----------------------------------------------------------*/
void Boombox::rtcClearIntp()
{
    rtcFlag = false;
}

/*----------------------------------------------------------*/
bool Boombox::rtcHandleIntp()
{
    bool trig = false;
    if (rtcFlag)
    {
        _minuteCnt++;
        DBG_PRINTF("Minute count: %02d\n", _minuteCnt);
        if (_minuteCnt >= _intv)
        {
            trig = true;
            _minuteCnt = 0;
            DBG_PRINTF("Interval timeout\n");
        }

        _rtc->resetTimer();
        rtcFlag = false;   
    }
    return trig;
}

/*----------------------------------------------------------*/
void Boombox::rtcSetTrigger(uint16_t triggerIntv)
{
    _intv = triggerIntv;
}
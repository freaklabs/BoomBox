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

    _vol           	= 26;
    _delayVal      	= 0;

    intNumRtc 		= 0;
    intNumAux 		= 1;    

    pinMode(pinPIR, INPUT);
    pinMode(pinButton, INPUT);
    pinMode(pinBusy, INPUT);

    pinMode(pinAmpShutdn, OUTPUT);
    pinMode(pinBoostEnb, OUTPUT);
    pinMode(pinMp3Enb, OUTPUT);
    pinMode(pin5vEnb, OUTPUT);
    pinMode(pinAuxEnb, OUTPUT);
    pinMode(pinAuxData0, OUTPUT);
    pinMode(pinAuxData1, OUTPUT);
    pinMode(pinAuxLed, OUTPUT);

    digitalWrite(pinAmpShutdn, HIGH);
    digitalWrite(pinBoostEnb, HIGH);
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pin5vEnb, LOW);
    digitalWrite(pinAuxEnb, LOW);
    digitalWrite(pinAuxData0, LOW);
    digitalWrite(pinAuxData1, LOW);
    digitalWrite(pinAuxLed, LOW);

    attachInterrupt(intNumRtc, Boombox::irqRtc, FALLING);
    attachInterrupt(intNumAux, Boombox::irqAux, RISING);
}

/************************************************************/
// begin
/************************************************************/
void Boombox::begin(SoftwareSerial *sser, Rtc_Pcf8563 *rtc)
{
    auxFlag = false;
    rtcFlag = false;

    ss = sser;
    ss->begin(9600);
    setVol(_vol);

    _rtc = rtc;
    _rtc->clearTimer(); 
}

/*----------------------------------------------------------*/
// Methods for controlling the MP3 player
/*----------------------------------------------------------*/

// initialize the pushbutton

/*----------------------------------------------------------*/
void Boombox::dispBanner()
{
    Serial.println(F("-------------------------------------------"));
    Serial.print(F("Boombox "));
    Serial.println(F(BOARD_VERSION));
    Serial.println(F("Designed by FreakLabs"));
    Serial.print(F("Last modified: "));
    Serial.println(F(__DATE__));
    Serial.println(F("-------------------------------------------"));
}

/*----------------------------------------------------------*/
void Boombox::sleep()
{
    // shut down everything else

    digitalWrite(pinMp3Enb, LOW);
    digitalWrite(pinBoostEnb, LOW);
    digitalWrite(pin5vEnb, LOW);
    digitalWrite(pinAuxEnb, LOW);
    wdt_disable();

    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
}

/*----------------------------------------------------------*/
void Boombox::wake()
{
    wdt_enable(WDTO_8S);
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pinBoostEnb, HIGH);
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pinAuxEnb, HIGH);

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
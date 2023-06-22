#include "boomboxBase.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <LowPower.h>
#include <SoftwareSerial.h>
#include <Rtc_Pcf8563.h>

#undef BOARD_VERSION
#define BOARD_VERSION "v1.0+"
#define LIB_VERSION "v1.18"

// allows printing or not printing based on the DEBUG VAR
#define DEBUG 1
#if (DEBUG == 1)
  #define DBG_PRINT(...)   _dbg->print(__VA_ARGS__)
  #define DBG_PRINTLN(...) _dbg->println(__VA_ARGS__)
  #define DBG_PRINTF(...)  printf(__VA_ARGS__)
#else
  #define DBG_PRINT(...)
  #define DBG_PRINTLN(...)
  #define DBG_PRINTF(...) 
#endif

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


class Boombox : public BoomboxBase
{
public:
    uint8_t pinAuxEnb = 17;
    uint8_t pinAuxData0 = 4;
    uint8_t pinAuxData1 = 10;
    uint8_t pinMute = A0;
    uint8_t intNumRtc = 0;
    uint8_t intNumAux = 1;

    uint16_t _intv;
    uint16_t _minuteCnt;

    Rtc_Pcf8563 *_rtc;
    char bufTime[50];

	Boombox();
	void begin(SoftwareSerial *sser, Rtc_Pcf8563 *rtc);
    void dispBanner();
    void sleep();
    void wake();

    static void irqRtc();

   	// rtc
    void rtcSetTime(int yr, int mon, int day, int hour, int min, int sec);
    ts_t rtcGetTime();
    char *rtcPrintTime();
    char *rtcPrintDate();
    char *rtcPrintTimeAndDate();
    void rtcSetTimer(uint8_t minutes = 1);
    void rtcEnableTimer();
    void rtcDisableTimer();
    bool rtcHandleIntp();
    boolean rtcIntpRcvd();
    void rtcClearIntp();
    void rtcSetTrigger(uint16_t interval);

    // aux
    void auxEnable();
    void auxDisable();
};

extern Boombox bb;
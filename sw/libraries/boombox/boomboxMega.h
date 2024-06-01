#include "boomboxBase.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <Rtc_Pcf8563.h>
#include <SoftwareSerial.h>

#undef BOARD_VERSION
#define BOARD_VERSION "v2024"
#define LIB_VERSION "MEGA-TEST"
#define NUM_LEDS 2
#define DEBOUNCE_TIME 100

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


class BoomboxMega : public BoomboxBase
{
public:
    uint8_t pinTrig;
    uint8_t pinAuxLed[NUM_LEDS];
    uint8_t pinAuxEnb;
    uint8_t pinAuxData0;
    uint8_t pinAuxData1;
    uint8_t intNumRtc;
    uint8_t intNumTrig;
    uint8_t intNumPIR;
    uint8_t intNumButton;

    uint16_t _intv;
    uint16_t _minuteCnt;
    bool testMode = false;
    HardwareSerial *_ser;

    Rtc_Pcf8563 *_rtc;
    char bufTime[50];

	BoomboxMega();
	void begin(HardwareSerial *ser, Rtc_Pcf8563 *rtc);
    void dispBanner();
    void sleep();
    void wake();
    void shuffleSeed();

   	// rtc
    static void irqRtc();
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

    static void irqButton();
    void buttonClearIntp();
    bool buttonIntpRcvd();
    bool buttonIrqHandler();

    // aux
    static void irqAux(void);
    void auxEnable();
    void auxDisable();
    bool isAuxEvent();
    void clearAuxFlag();

    void _sendCmd(uint8_t *buf, uint8_t len);

};

extern BoomboxMega bbm;

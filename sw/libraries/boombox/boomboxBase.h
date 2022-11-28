#pragma once
#include "Arduino.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <LowPower.h>
#include <SoftwareSerial.h>

#define BOARD_VERSION "v0.6"

class BoomboxBase
{
public:
    uint8_t pinPIR;
    uint8_t pinButton;
    uint8_t pinBusy;

    uint8_t pinAmpShutdn;
    uint8_t pinBoostEnb;
    uint8_t pinMp3Enb;
    uint8_t pin5vEnb;
    uint8_t pinCurrEnb;
    uint8_t pinRangeEnb;
    uint8_t pinPIREnb;
    uint8_t intPIR ;
    uint8_t intAux;

    uint8_t _vol = 26;
    uint32_t _delayVal = 0;    
    SoftwareSerial *ss;

public:
    BoomboxBase();

    void begin(SoftwareSerial *sser);    
    void dispBanner();


    void playNext();
    void playPrev();
    void play(uint8_t file);
    void playBusy(uint8_t file);
    void stop();
    void pause();
    void resume();
    void setVol(int8_t vol);

    void buttonInit();

    bool isBusy();
    bool isPIREvent();
    bool isAuxEvent();

    void sleep();
    void wake();

    void clearPIRFlag();
    void clearAuxFlag();

    static void irqPIR();
    static void irqAux();   

    void ampEnable();
    void ampDisable();
    void reg5vEnable();
    void reg5vDisable();

private:
    void _sendCmd(uint8_t *, uint8_t);

};

extern BoomboxBase bbb;

#pragma once
#include "Arduino.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <LowPower.h>
#include <SoftwareSerial.h>

class Boombox
{
public:
    uint8_t pinPIR = 2;
    uint8_t pinButton = 3;
    uint8_t pinAmpShutdn = 5;
    uint8_t pinBusy = 11;
    uint8_t pinBoostEnb = 4;
    uint8_t pinMp3Enb = 6;
    uint8_t pinCurrEnb = 7;
    uint8_t pinRangeEnb = 16;
    uint8_t pinPIREnb = 17;
    uint8_t pin5vEnb = 15;
    uint8_t _vol = 26;
    uint32_t _delayVal = 0;

    uint8_t intPIR = 0;
    uint8_t intAux = 1;

public:
    Boombox();

    void init();    
    void dispBanner();


    void playNext();
    void playPrev();
    void play(uint8_t file);
    void playBusy(uint8_t file);
    void stop();
    void pause();
    void resume();
    void setVol(uint8_t vol);

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

extern Boombox bb;

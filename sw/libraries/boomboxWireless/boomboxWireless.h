#pragma once
#include "Arduino.h"
#include "SoftwareSerial.h"
#include <LowPower.h>

#define ADC_REF_VOLTAGE 3.3     
#define ADC_LEVELS 1024        
#define ADC_SCALE_FACTOR 2   

class Boombox
{
public:
    uint8_t pinAmpShutdn = 29;
    uint8_t pinBusy = 23;
    uint8_t pinMp3Enb = 28;
    uint8_t pin5VEnb = 22;    
    uint8_t intAux = 1;
    uint8_t pinVbat = 30;
    uint8_t pinVsol = 31;

public:
    Boombox();
    void init();

    void playNext();
    void playPrev();
    void play(uint8_t file);
    void playBusy(uint8_t file);
    void setVol(uint8_t vol);
    void stop();
    void pause();
    void resume();
    void dispBanner();
    bool isBusy();
    bool isAuxEvent();
    void clearAuxFlag();
    static void irqAux();
    void sleep();
    void wake();
    float getVbat();
    float getVsol();

private:
    int _vol = 28;
    uint32_t _delayVal = 0;
    
    void _sendCmd(uint8_t *, uint8_t);
};

extern Boombox bb;

/************************************************************
Boombox Main
Uses trailcam to trigger sound effects
*************************************************************/
#include "boombox.h"
#include "chibi.h"
#include <LowPower.h>

// customize MAX_SOUNDS based on number of samples in MP3 lib
#define MAX_SOUNDS 10   

// delay for delayTime milliseconds after trigger occurs
uint32_t delayTime = 0;        

// play sound and then wait for durationTime milliseconds
uint32_t durationTime = 10000;   

// delay for offDelay milliseconds after sound finishes playing
uint32_t offDelayTime = 1000;

int index = 0;

/************************************************************/
// setup
/************************************************************/
void setup() 
{
    Serial.begin(57600);
        
    // initialize system
    bb.init();
    
    // display setup banner
    bb.dispBanner();

    delay(500);
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
    // check if auxiliary (trailcam) event has triggered
    if (bb.isAuxEvent() == true)
    {                        
        // button has been pushed 
        Serial.println("Trailcam event.");

        // delay for delayTime milliseconds after trigger has happened. 
        // This delays playing the sound immediately after trigger 
        delay(delayTime);
        
        if (index < MAX_SOUNDS)
        {
            index++;        
        }
        else
        {
            index = 1;
        }
        Serial.print("Playing index: ");
        Serial.println(index);

        // play music here
        bb.play(index);

        // delay for durationTime milliseconds. should be adjusted to 
        // longest sample that will be played
        delay(durationTime); 
        
        // delay for offDelayTime milliseconds. This is the time after durationTime expires but we do not allow another sound
        // to be triggered.
        delay(offDelayTime);

        // clear interrupt flag after sample is played
        bb.clearAuxFlag();
    }
    
    // go to sleep here
    bb.sleep();

    // an interrupt occurred. wake up!
    bb.wake();
}

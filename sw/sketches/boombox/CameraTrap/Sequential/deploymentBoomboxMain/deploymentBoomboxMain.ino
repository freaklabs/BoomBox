/************************************************************
Boombox Main
Uses trailcam to trigger sound effects
*************************************************************/
#include <boombox.h>

SoftwareSerial ss(9, 8);
Rtc_Pcf8563 rtc; 

// customize MAX_SOUNDS based on number of samples in MP3 lib
#define MAX_SOUNDS 9   
#define AMP_ENABLE_DELAY 500

// delay for delayTime milliseconds after trigger occurs
uint32_t delayTime = 5000;        

// play sound and then wait for durationTime milliseconds
uint32_t durationTime = 10000;   

// delay for offDelay milliseconds after sound finishes playing
uint32_t offDelayTime = 0;

int index = 0;

/************************************************************/
// setup
/************************************************************/
void setup() 
{
    Serial.begin(57600);
        
    // initialize system
    bb.begin(&ss, &rtc);
    bb.ampDisable();
    
    // display setup banner
    // delay a bit before sleeping so it can print out banner
    bb.dispBanner();
    delay(100);
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

        // enable amp
        bb.ampEnable();
        delay(AMP_ENABLE_DELAY);

        // play music here
        bb.play(index);

        // delay for durationTime milliseconds. should be adjusted to 
        // longest sample that will be played
        delay(durationTime); 
        
        // disable amp before going to sleep
        bb.ampDisable();     
                
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

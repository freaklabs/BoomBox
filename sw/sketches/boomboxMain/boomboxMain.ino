/************************************************************
Boombox Example 1
This comprises a basic implementation where the pushbutton is 
used to trigger samples from the MP3 library.
*************************************************************/
#include "boombox.h"
#include "chibi.h"

// customize MAX_SOUNDS based on number of samples in MP3 lib
#define MAX_SOUNDS 20   

int index = 0;

/************************************************************/
// setup
/************************************************************/
void setup() 
{
    // initialize system
    bb.init();

    // enable watchdog timer
    bb.watchdogEnb();
    
    // display setup banner
    bb.dispBanner();
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
    // reset the watchdog timer so it doesn't reset system
    bb.watchdogKick();

    // check if auxiliary (trailcam) event has triggered
    if (bb.isAuxEvent() == true)
    {
        bb.clearAuxFlag();
    
        // button has been pushed 
        Serial.println("Trailcam event.");
        
        if (index < MAX_SOUNDS)
        {
            index++;        
        }
        else
        {
            index = 1;
        }
        bb.play(index);
    }
}


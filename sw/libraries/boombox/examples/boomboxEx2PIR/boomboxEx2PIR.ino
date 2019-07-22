/************************************************************
Boombox Example 2
This comprises a basic implementation where PIR motion sensor
will trigger a sound that is played
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
  bb.init();

  // display setup banner
  bb.dispBanner();
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
  if (bb.isPIREvent() == true)
  {
    bb.clearPIRFlag();
    
    // external PIR motion sensor has been triggered    
    Serial.println("PIR motion sensor event."); 

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


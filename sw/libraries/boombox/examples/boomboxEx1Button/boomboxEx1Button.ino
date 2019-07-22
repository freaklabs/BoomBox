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
  bb.init();

  // initialize the pushbutton by adding pullup
  bb.buttonInit(); 

  // display setup banner
  bb.dispBanner();
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
  if (bb.isAuxEvent() == true)
  {
    bb.clearAuxFlag();
    
    // button has been pushed 
    Serial.println("Button event.");

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


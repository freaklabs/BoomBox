/************************************************************
Boombox Main
Uses trailcam to trigger sound effects
*************************************************************/
#include <boomboxTest.h>

// customize MAX_SOUNDS based on number of samples in MP3 lib
#define MAX_SOUNDS 20   
#define AMP_ENABLE_DELAY 500

// delay for offDelay milliseconds after sound finishes playing
uint32_t delayTime = 0;
uint32_t offDelayTime = 0;

int index = 0;
int pinMp3Busy = 11;

// basic configuration
static FILE uartout = {0,0,0,0,0,0,0,0};

/************************************************************/
// setup
/************************************************************/
void setup() 
{
    // to set up the printf command. it will make life a lot easier
    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartout ;

    Serial.begin(57600);
            
    bb.begin();
    pinMode(bb.pinButton, INPUT_PULLUP);
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
    bool playStatus;
    
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

        // add a delay for busy pin to go low
        delay(100);
        
        playStatus = digitalRead(bb.pinBusy);
        while(!playStatus)
        {
            playStatus = digitalRead(bb.pinBusy);           
        }
        
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

/**************************************************************************/
// This is to implement the printf function from within arduino
/**************************************************************************/
static int uart_putchar (char c, FILE *stream)
{
    (void) stream;
    
    Serial.write(c);
    return 0;
}

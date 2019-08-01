/************************************************************
Boombox Main
Uses trailcam to trigger sound effects
*************************************************************/
#include "boombox.h"
#include "chibi.h"

// customize MAX_SOUNDS based on number of samples in MP3 lib
#define MAX_SOUNDS 23   
#define DELAY_TIME 30000 // delay for 30 seconds
#define DELAY_WAKE 300

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
        Serial.print("Playing index: ");
        Serial.println(index);

        // play music here
        bb.playBusy(index);

        // delay for DELAY_TIME milliseconds. should be adjusted to 
        // longest sample that will be played
        delay(DELAY_TIME); 

        // clear interrupt flag after sample is played
        bb.clearAuxFlag();
    }

    // disable watchdog before sleeping
    bb.watchdogDis();

    // go to sleep here
    bb.sleep();

    // an interrupt occurred. wake up!
    bb.wake();

    // re-enable watchdog timer
    bb.watchdogEnb();

    // print wake message and add some delay
    // delay needed to print message and also 
    // wait for mp3 decoder IC to boot
    Serial.println("Waking");
    delay(DELAY_WAKE);
}

/************************************************************/
// command line functions
/************************************************************/

/*-----------------------------------------------------------*/
// 
/*-----------------------------------------------------------*/
void cmdPlay(int arg_cnt, char **args)
{
  uint8_t file = chibiCmdStr2Num(args[1], 10);
  bb.play(file);
}

/*-----------------------------------------------------------*/
// 
/*-----------------------------------------------------------*/
void cmdSetVolume(int arg_cnt, char **args)
{
  uint8_t vol = chibiCmdStr2Num(args[1], 10);
  bb.setVol(vol);
}

/*-----------------------------------------------------------*/
// 
/*-----------------------------------------------------------*/
void cmdPause(int arg_cnt, char **args)
{
  bb.pause();
}

/*-----------------------------------------------------------*/
// 
/*-----------------------------------------------------------*/
void cmdResume(int arg_cnt, char **args)
{
  bb.resume();
}

/*-----------------------------------------------------------*/
// 
/*-----------------------------------------------------------*/
void cmdStop(int arg_cnt, char **args)
{
  bb.stop();
}

/*-----------------------------------------------------------*/
// 
/*-----------------------------------------------------------*/
void cmdShutdown(int arg_cnt, char **args)
{
    // shut down everything else
    digitalWrite(bb.pinBoostEnb, LOW);
    digitalWrite(bb.pinMp3Enb, LOW);
    digitalWrite(bb.pinCurrEnb, LOW);
    digitalWrite(bb.pinRangeEnb, LOW);
    digitalWrite(bb.pinPIREnb, LOW);
    digitalWrite(bb.pinAmpShutdn, LOW);

    delay(500);

    digitalWrite(bb.pinBoostEnb, HIGH);
    digitalWrite(bb.pinMp3Enb, HIGH);
    digitalWrite(bb.pinCurrEnb, HIGH);
    digitalWrite(bb.pinRangeEnb, HIGH);
    digitalWrite(bb.pinPIREnb, HIGH);
    digitalWrite(bb.pinAmpShutdn, HIGH);

    Serial.println("Playing");
    delay(500);
    bb.play(1);
}


/*-----------------------------------------------------------*/
// 
/*-----------------------------------------------------------*/
void cmdSleep(int arg_cnt, char **args)
{
  bb.sleep();

  // need to wake up if you sleep. 
  // can only wake up from external interrupt, 
  // ie: button push or motion event
  bb.wake();
}

/*-----------------------------------------------------------*/
// 
/*-----------------------------------------------------------*/
void cmdSetDelay(int arg_cnt, char **args)
{
    uint8_t delayVal = chibiCmdStr2Num(args[1], 10);
    bb.delaySet(delayVal);
    Serial.print("Delay has been set to: ");
    Serial.println(bb.delayGet());
}



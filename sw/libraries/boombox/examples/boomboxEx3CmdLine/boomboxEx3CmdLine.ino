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
    
    chibiCmdInit(57600);

    // add commands here
    chibiCmdAdd("play", cmdPlay);
    chibiCmdAdd("stop", cmdStop);
    chibiCmdAdd("vol", cmdSetVolume);
    chibiCmdAdd("pause", cmdPause);
    chibiCmdAdd("resume", cmdResume);
    chibiCmdAdd("sleep", cmdSleep);
    chibiCmdAdd("setdelay", cmdSetDelay);
    
    // display setup banner
    bb.dispBanner();
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
    chibiCmdPoll();
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


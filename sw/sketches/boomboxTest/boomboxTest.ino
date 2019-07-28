#include "chibi.h"
#include "boombox.h"

#define MAX_SOUNDS 20

int index = 0;

/************************************************************/
// setup
/************************************************************/
void setup() 
{   
  bb.init();
  chibiCmdInit(57600);

  bb.dispBanner();
  chibiCmdAdd("play", cmdPlay);
  chibiCmdAdd("stop", cmdStop);
  chibiCmdAdd("vol", cmdSetVolume);
  chibiCmdAdd("pause", cmdPause);
  chibiCmdAdd("resume", cmdResume);
  chibiCmdAdd("sleep", cmdSleep);
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
//  chibiCmdPoll();

  if (bb.isPIREvent() == true)
  {
    bb.clearPIRFlag();
    
    // external PIR motion sensor has been triggered    
    Serial.println("PIR motion sensor event."); 
  }

  if (bb.isAuxEvent() == true)
  {
    bb.clearAuxFlag();
    
    // button has been pushed 
    Serial.println("Button event.");

    // play sound. 
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

/************************************************************/
// command line functions
/************************************************************/
void cmdPlay(int arg_cnt, char **args)
{
  uint8_t file = chibiCmdStr2Num(args[1], 10);
  bb.play(file);
}

void cmdSetVolume(int arg_cnt, char **args)
{
  uint8_t vol = chibiCmdStr2Num(args[1], 10);
  bb.setVol(vol);
}

void cmdPause(int arg_cnt, char **args)
{
  bb.pause();
}

void cmdResume(int arg_cnt, char **args)
{
  bb.resume();
}

void cmdStop(int arg_cnt, char **args)
{
  bb.stop();
}

void cmdSleep(int arg_cnt, char **args)
{
  bb.sleep();

  // need to wake up if you sleep. 
  // can only wake up from external interrupt, 
  // ie: button push or motion event
  bb.wake();
}

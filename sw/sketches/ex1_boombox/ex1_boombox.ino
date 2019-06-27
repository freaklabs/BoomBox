#include "chibi.h"
#include "boombox.h"

/************************************************************/
// setup
/************************************************************/
void setup() 
{
  bb.init();
  chibiCmdInit(57600);

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
  chibiCmdPoll();

  if (bb.isPIREvent() == true)
  {
    bb.clearPIRFlag();
    
    // external PIR motion sensor has been triggered    
    Serial.println("PIR motion sensor event.");
  }

  if (bb.isAuxEvent() == true)
  {
    bb.clearAuxFlag();
    
    // external PIR motion sensor has been triggered    
    Serial.println("PIR motion sensor event.");
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

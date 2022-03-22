#include <cmdArduino.h>
#include "boombox.h"

#define MAX_SOUNDS 10

int index = 0;

uint32_t offDelayTime = 0;

/************************************************************/
// setup
/************************************************************/
void setup() 
{   
  bb.init();

  cmd.begin(57600);

  bb.dispBanner();
  Serial.println("Boombox Trailcam Sketch");
  
  cmd.add("play", cmdPlay);
  cmd.add("stop", cmdStop);
  cmd.add("vol", cmdSetVolume);
  cmd.add("pause", cmdPause);
  cmd.add("resume", cmdResume);
  cmd.add("sleep", cmdSleep); 
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
  cmd.poll();
/*
  if (bb.isPIREvent() == true)
  {
    bb.clearPIRFlag();
    
    // external PIR motion sensor has been triggered    
    Serial.println("PIR motion sensor event."); 
  }
*/
  if (bb.isAuxEvent() == true)
  {
    // button has been pushed 
    Serial.println("Trigger event.");

    // play sound. 
    if (index < MAX_SOUNDS)
    {        
      index++;
      Serial.print("Index: ");
      Serial.println(index);
    }
    else
    {
      index = 1;
      Serial.print("Index: ");
      Serial.println(index);
    }
    bb.play(index);

    delay(offDelayTime);
    bb.clearAuxFlag();
  }
}

/************************************************************/
// Play track
// Usage: play <track number>
/************************************************************/
void cmdPlay(int arg_cnt, char **args)
{
  uint8_t track = cmd.conv(args[1]);
  if (track > MAX_SOUNDS)
  {
    track = MAX_SOUNDS-1;
  }
  bb.play(track);
}

/************************************************************/
// Set volume
// Usage: vol <volume level>
// volume level is between 0 and 30
/************************************************************/
void cmdSetVolume(int arg_cnt, char **args)
{
  uint8_t vol = cmd.conv(args[1]);
  if (vol > 30)
  {
    vol = 30;
  }
  bb.setVol(vol);
}

/************************************************************/
// Pause playing
/************************************************************/
void cmdPause(int arg_cnt, char **args)
{
  bb.pause();
}

/************************************************************/
// Resume playing
/************************************************************/
void cmdResume(int arg_cnt, char **args)
{
  bb.resume();
}

/************************************************************/
// Stop playing
/************************************************************/
void cmdStop(int arg_cnt, char **args)
{
  bb.stop();
}

/************************************************************/
// Go into hibernation mode
/************************************************************/
void cmdSleep(int arg_cnt, char **args)
{
    bb.ampDisable();
    bb.sleep();
    
    // need to wake up if you sleep. 
    // can only wake up from external interrupt, 
    // ie: button push or motion event
    bb.wake();
    bb.ampEnable();
    delay(500);
}

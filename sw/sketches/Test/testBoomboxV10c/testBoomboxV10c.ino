#include <cmdArduino.h>
#include <boomboxTest.h>

#define MAX_SOUNDS 5

int index = 0;
int pinAuxLed = 13;

uint32_t offDelayTime = 0;
volatile bool flagTimer = 0;

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
            
    bb.begin();
    pinMode(bb.pinButton, INPUT_PULLUP);
    pinMode(pinAuxLed, OUTPUT);

    detachInterrupt(bb.intNumAux);
    attachInterrupt(bb.intNumAux, bb.irqAux, FALLING);
    
    cmd.begin(57600, &Serial);
    
    bb.dispBanner();
    Serial.println("Boombox Standlone Sketch");  
    
    cmd.add("play", cmdPlay);
    cmd.add("stop", cmdStop);
    cmd.add("vol", cmdSetVolume);
    cmd.add("pause", cmdPause);
    cmd.add("resume", cmdResume);
    cmd.add("sleep", cmdSleep); 
    cmd.add("settime", cmdSetDateTime);
    cmd.add("gettime", cmdGetDateTime);    
    cmd.add("enbtimer", cmdEnableTimer);
    cmd.add("setinterval", cmdSetInterval);
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
  bool playStatus;
  
  cmd.poll();
  
  if (bb.rtcHandleIntp())
  {
      printf("RTC Interrupt triggered - do something.\n");
  }

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

    // add a delay for busy pin to go low
    delay(100);
    playStatus = digitalRead(bb.pinBusy);
    while(!playStatus)
    {
        digitalWrite(pinAuxLed, HIGH);
        playStatus = digitalRead(bb.pinBusy);           
    }
    digitalWrite(pinAuxLed, LOW);
    
    delay(offDelayTime);
    bb.clearAuxFlag();
  }  
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void cmdSetInterval(int arg_cnt, char **args)
{
    (void) arg_cnt;

    int interval = cmd.conv(args[1]);
    bb.rtcSetTrigger(interval);      
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void cmdEnableTimer(int arg_cnt, char **args)
{
    (void) arg_cnt;
    
    int enb = cmd.conv(args[1]);
    if (enb)
    { 
        bb.rtcEnableTimer();
        printf("enabling timer\n");
    }
    else
    {
        bb.rtcDisableTimer();
        printf("disabling timer\n");
    }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void cmdSetDateTime(int arg_cnt, char **args)
{
    (void) arg_cnt;
    
    uint8_t day, mon, year, hr, min, sec;

    year = strtol(args[1], NULL, 10);
    mon = strtol(args[2], NULL, 10);
    day = strtol(args[3], NULL, 10);
    hr = strtol(args[4], NULL, 10);
    min = strtol(args[5], NULL, 10);
    sec = strtol(args[6], NULL, 10);

    bb.rtcSetTime(year, mon, day, hr, min, sec);
    
    printf("It is: %s\n", bb.rtcPrintTimeAndDate());
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void cmdGetDateTime(int arg_cnt, char **args)
{
    (void) arg_cnt;
    (void) args;
    
    printf("It is now: %s\n", bb.rtcPrintTimeAndDate());
}

/************************************************************/
// Play track
// Usage: play <track number>
/************************************************************/
void cmdPlay(int arg_cnt, char **args)
{
    (void) arg_cnt;
    
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
    (void) arg_cnt;
    
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
    (void) arg_cnt;
    (void) args;
    
    bb.pause();
}

/************************************************************/
// Resume playing
/************************************************************/
void cmdResume(int arg_cnt, char **args)
{
    (void) arg_cnt;
    (void) args;
    
    bb.resume();
}

/************************************************************/
// Stop playing
/************************************************************/
void cmdStop(int arg_cnt, char **args)
{
    (void) arg_cnt;
    (void) args;
    
    bb.stop();
}


/************************************************************/
// Go into hibernation mode
/************************************************************/
void cmdSleep(int arg_cnt, char **args)
{
    (void) arg_cnt;
    (void) args;

    bb.ampDisable();
    bb.sleep();
    
    // need to wake up if you sleep. 
    // can only wake up from external interrupt, 
    // ie: button push or motion event
    bb.wake();
    bb.ampEnable();
    delay(500);
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

#include <cmdArduino.h>
#include <boomboxTest.h>
#include <Adafruit_NeoPixel.h>

#define MAX_SOUNDS 5
#define LED_COUNT 60
#define LED_PIN0 4
#define LED_PIN1 10

int index = 0;

uint32_t offDelayTime = 0;
volatile bool flagTimer = 0;
uint8_t pinLedEnb = 17;

// basic configuration
static FILE uartout = {0,0,0,0,0,0,0,0};

Adafruit_NeoPixel strip0(LED_COUNT, LED_PIN0, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip1(LED_COUNT, LED_PIN1, NEO_GRB + NEO_KHZ800);

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
    
    pinMode(pinLedEnb, OUTPUT);
    digitalWrite(pinLedEnb, HIGH);
    
    cmd.begin(57600);
    
    bb.dispBanner();
    Serial.println("Boombox Standlone Sketch"); 

    // LED handling
    strip0.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip0.show();            // Turn OFF all pixels ASAP
    strip0.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)    

    // LED handling
    strip1.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip1.show();            // Turn OFF all pixels ASAP
    strip1.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)         
    
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
    cmd.add("ledchase", cmdLedChase);
    cmd.add("ledflash", cmdLedFlash);
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
        strip0.fill(strip0.Color(255, 255, 255));
        strip1.fill(strip1.Color(255, 255, 255));
        strip0.show();
        strip1.show();
        delay(50);
        strip0.clear();
        strip1.clear();
        strip0.show();
        strip1.show();
        delay(50);
        playStatus = digitalRead(bb.pinBusy);           
    }
    
//    delay(offDelayTime);
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

/************************************************************/
void cmdLedChase(int arg_cnt, char **args)
{    
    (void) arg_cnt;
    (void) args;

    colorWipe0(strip0.Color(127, 127, 127), 50); // white
    colorWipe1(strip1.Color(127, 127, 127), 50); // white
    delay(100);
    strip0.clear();
    strip1.clear();
    strip0.show(); 
    strip1.show();     
}

/************************************************************/

/************************************************************/
void cmdLedFlash(int arg_cnt, char **args)
{    
    (void) arg_cnt;
    (void) args;

    for (int i=0; i<10; i++)
    {
        strip0.fill(strip0.Color(255, 255, 255));
        strip1.fill(strip1.Color(255, 255, 255));
        strip0.show();
        strip1.show();
        delay(50);
        strip0.clear();
        strip1.clear();
        strip0.show();
        strip1.show();
        delay(50);
    }      
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

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe0(uint32_t color, int wait) {
  for(int i=0; i<strip0.numPixels(); i++) { // For each pixel in strip...
    strip0.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip0.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe1(uint32_t color, int wait) {
  for(int i=0; i<strip1.numPixels(); i++) { // For each pixel in strip...
    strip1.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip1.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
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

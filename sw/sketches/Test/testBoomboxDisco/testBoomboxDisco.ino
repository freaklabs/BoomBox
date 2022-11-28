#include <cmdArduino.h>
#include <Adafruit_NeoPixel.h>
#include "boombox.h"

#define MAX_SOUNDS 5
#define LED_PIN    2
#define LED_COUNT 36

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int index = 0;

uint32_t offDelayTime = 0;

/************************************************************/
// setup
/************************************************************/
void setup() 
{   
    bb.init();
    pinMode(bb.pinButton, INPUT_PULLUP);
    
    cmd.begin(57600);
    
    bb.dispBanner();
    Serial.println("Boombox Standlone Sketch");
    
    // LED handling
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();            // Turn OFF all pixels ASAP
    strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)  
    
    cmd.add("play", cmdPlay);
    cmd.add("stop", cmdStop);
    cmd.add("vol", cmdSetVolume);
    cmd.add("pause", cmdPause);
    cmd.add("resume", cmdResume);
    cmd.add("sleep", cmdSleep); 
    cmd.add("ledshow", cmdLedShow);
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

    ledShow();
    
//    delay(offDelayTime);
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
void cmdLedShow(int arg_cnt, char **args)
{    
    ledShow();
}

/************************************************************/
// Go into hibernation mode
/************************************************************/
void cmdSleep(int arg_cnt, char **args)
{
  //pinMode(LED_PIN, INPUT);
  bb.ampDisable();
  bb.sleep();

  // need to wake up if you sleep. 
  // can only wake up from external interrupt, 
  // ie: button push or motion event
  bb.wake();
  bb.ampEnable();
  delay(500);
}

void ledShow()
{
//    colorWipe(strip.Color(255,   0,   0), 50); // Red
//    colorWipe(strip.Color(  0, 255,   0), 50); // Green
//    colorWipe(strip.Color(  0,   0, 255), 50); // Blue
     
    // Do a theater marquee effect in various colors...
    theaterChase(strip.Color(127, 127, 127), 50); // White, half brightness
    theaterChase(strip.Color(127,   0,   0), 50); // Red, half brightness
    theaterChase(strip.Color(  0,   0, 127), 50); // Blue, half brightness
    theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant
    
//    rainbow(10);             // Flowing rainbow cycle along the whole strip       
    
    for(int i=0; i<strip.numPixels(); i++) 
    {
       strip.setPixelColor(i, 0); 
       strip.show();
    }  
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}


// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

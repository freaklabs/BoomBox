#include <cmdArduino.h>
#include <boombox.h>

#define MAX_SOUNDS 3
#define RANDOMIZING_PIN 5

SoftwareSerial ss(9, 8);
Rtc_Pcf8563 rtc; 

int index = 0;
int intAux = 1;
uint16_t playList[MAX_SOUNDS];
char buf[100];

uint32_t offDelayTime = 0;

/************************************************************/
// setup
/************************************************************/
void setup() 
{   
    uint16_t randSeed;
    
    bb.begin(&ss, &rtc);
    bb.buttonInit();
    
    cmd.begin(57600, &Serial);
    
    bb.dispBanner();
    Serial.println("Boombox Standlone Sketch");
    
    cmd.add("play", cmdPlay);
    cmd.add("stop", cmdStop);
    cmd.add("vol", cmdSetVolume);
    cmd.add("pause", cmdPause);
    cmd.add("resume", cmdResume);
    cmd.add("sleep", cmdSleep);

    // shuffle the random number generator so it won't always
    // give the same random sequence
    randSeed = analogRead(RANDOMIZING_PIN);
    randomSeed(randSeed);
    shufflePlaylist();     
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
    cmd.poll();

    if (bb.isAuxEvent() == true)
    {
        // button has been pushed 
        Serial.println("Trigger event.");
        
        // if we've reached the end of the playlist, 
        // shuffle playlist and then restart               
        if (index == MAX_SOUNDS)
        {
            index = 0;
            shufflePlaylist();
        }
        
        // play sound based on randomized playlist
        bb.play(playList[index]);
        
        // print out index and value   
        sprintf(buf, "Index: %d, Val: %d.\n", index, playList[index]); 
        Serial.print(buf);
        
        // increment index
        index++; 

        delay(offDelayTime);
        bb.clearAuxFlag();
    }
}

/************************************************************/
// Helper function to create a randomized, non-repeeating playlist
// this should be called once all sounds in playlist have been
// exhausted
/************************************************************/
void shufflePlaylist()
{
    uint16_t i, nvalues = MAX_SOUNDS;

    // create sequential playlist with indices that start from 1
    for(i = 0; i<nvalues; i++)
    {
        playList[i] = i+1;
    }

    // shuffle playlist
    for(i = 0; i < nvalues-1; i++) 
    {
        uint16_t c = random(nvalues-i);

        /* swap */
        uint16_t t = playList[i]; 
        playList[i] = playList[i+c]; 
        playList[i+c] = t;    
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

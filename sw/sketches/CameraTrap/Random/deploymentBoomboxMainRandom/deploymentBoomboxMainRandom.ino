/************************************************************
Boombox Main
Uses trailcam to trigger sound effects
*************************************************************/
#include "boombox.h"

// customize MAX_SOUNDS based on number of samples in MP3 lib
#define MAX_SOUNDS 9   
#define RANDOMIZING_PIN 5
#define AMP_ENABLE_DELAY 500

// delay for delayTime milliseconds after trigger occurs
uint32_t delayTime = 5000;        

// play sound and then wait for durationTime milliseconds
uint32_t durationTime = 10000;   

// delay for offDelay milliseconds after sound finishes playing
uint32_t offDelayTime = 0;

int index = 0;
uint16_t playList[MAX_SOUNDS];
char buf[100];

/************************************************************/
// setup
/************************************************************/
void setup() 
{
    uint16_t randSeed;
    
    Serial.begin(57600);
        
    // initialize system
    bb.init();
    bb.ampDisable();
    
    // display setup banner
    // delay a bit before sleeping so it can print out banner    
    bb.dispBanner();
    delay(100);

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
    // check if auxiliary (trailcam) event has triggered
    if (bb.isAuxEvent() == true)
    {                        
        // button has been pushed 
        Serial.println("Trailcam event.");

        // delay for delayTime milliseconds after trigger has happened. 
        // This delays playing the sound immediately after trigger 
        delay(delayTime);
        
        // if we've reached the end of the playlist, 
        // shuffle playlist and then restart               
        if (index == MAX_SOUNDS)
        {
            index = 0;
            shufflePlaylist();
        }
        
        // print out index and value
        sprintf(buf, "Index: %d, Val: %d.\n", index, playList[index]); 
        Serial.print(buf);

        // enable amp
        bb.ampEnable();
        delay(AMP_ENABLE_DELAY); 

        // play sound based on randomized playlist
        bb.play(playList[index]);

        // increment index
        index++; 

        // delay for durationTime milliseconds. should be adjusted to 
        // longest sample that will be played
        delay(durationTime); 
        
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

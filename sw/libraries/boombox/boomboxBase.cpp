#include "BoomboxBase.h"
//SoftwareSerial ss(9, 8);

BoomboxBase bbb;

static volatile bool pirFlag;
static volatile bool auxFlag;

/*----------------------------------------------------------*/
// Initializing the MP3 player
/*----------------------------------------------------------*/

/************************************************************/
// Constructor
/************************************************************/
BoomboxBase::BoomboxBase()
{
    pinPIR          = 2;
    pinButton       = 3;
    pinAmpShutdn    = 5;
    pinBusy         = 11;
    pinBoostEnb     = 4;
    pinMp3Enb       = 6;
    pinCurrEnb      = 7;
    pinRangeEnb     = 16;
    pinPIREnb       = 17;
    pin5vEnb        = 15;
    pinAuxLed       = 13;
    pinRandSeed     = A0;
    pinMute         = 13;
    
    intPIR          = 0;
    intAux          = 1;     

     _vol           = 26;
     _delayVal      = 0;
     _index         = 0;

    pinMode(pinBusy, INPUT);
    pinMode(pinButton, INPUT);

    pinMode(pinAmpShutdn, OUTPUT);
    pinMode(pinBoostEnb, OUTPUT);
    pinMode(pinMp3Enb, OUTPUT);
    pinMode(pin5vEnb, OUTPUT);
    pinMode(pinAuxLed, OUTPUT);
    pinMode(pinMute, OUTPUT);

    digitalWrite(pinBoostEnb, LOW);
    digitalWrite(pinAmpShutdn, HIGH);
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pin5vEnb, LOW);
    digitalWrite(pinAuxLed, LOW);
    digitalWrite(pinMute, LOW);

    attachInterrupt(intAux, BoomboxBase::irqAux, RISING);
}
/************************************************************/
// init
/************************************************************/
void BoomboxBase::begin(SoftwareSerial *sser)
{
    auxFlag = false;
    pirFlag = false;

    ss = sser;
    ss->begin(9600);

#ifdef ARDUINO_ARCH_MEGAAVR   
    wdt_enable(WDT_PERIOD_8KCLK_gc); 
#else
    wdt_enable(WDTO_8S);
#endif
    
    delay(100);
    setVol(_vol);
}

/*----------------------------------------------------------*/
// Methods for controlling the MP3 player
/*----------------------------------------------------------*/

/************************************************************/
// initialize the pushbutton
/************************************************************/
void BoomboxBase::buttonInit()
{
    detachInterrupt(intAux);
    pinMode(pinButton, INPUT_PULLUP);
    attachInterrupt(intAux, BoomboxBase::irqAux, FALLING);
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::dispBanner()
{
    Serial.println(F("-------------------------------------------"));
    Serial.print(F("Boombox "));
    Serial.println(BOARD_VERSION);
    Serial.println(F("Designed by FreakLabs and Meredith Palmer"));
    Serial.print(F("Last modified: "));
    Serial.println(__DATE__);
    Serial.println(F("-------------------------------------------"));
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::play(uint8_t file)
{
    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x03, 0X00, 0x00, file , 0xEF };
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::playBusy(uint8_t file)
{
    bool done = false;

    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x03, 0x00, 0x00, file, 0xEF };
    _sendCmd(buf, sizeof(buf));
    
//    dumpHex(buf); 

    digitalWrite(pinAuxLed, HIGH);
    delay(100); // wait for busy pin to go high

    done = digitalRead(pinBusy);
    while (!done)
    {
        done = digitalRead(pinBusy);
        wdt_reset();
    }
    digitalWrite(pinAuxLed, LOW);
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::playBusyFolder(uint8_t folder, uint8_t file)
{
    bool done = false;

    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x0F, 0x00, folder, file, 0xEF };
    _sendCmd(buf, sizeof(buf));

//    dumpHex(buf); 

    digitalWrite(pinAuxLed, HIGH);
    delay(100); // wait for busy pin to go high

    done = digitalRead(pinBusy);
    while (!done)
    {
        done = digitalRead(pinBusy);
        wdt_reset();
    }
    digitalWrite(pinAuxLed, LOW);    
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::dumpHex(uint8_t *data)
{
    for (int i=0; i<8; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

/************************************************************/
//
/************************************************************/
bool BoomboxBase::isBusy()
{
    return digitalRead(pinBusy) == LOW;
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::setVol(int8_t vol)
{
    _vol = constrain(vol, 0, 30);
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::stop()
{
    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x16, 0x00, 0x00, 0x00, 0XEF };
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::pause()
{
    uint8_t buf[8] = {0x7E, 0xFF, 0x06, 0x0E, 0x00, 0x00, 0x00, 0xEF};
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::resume()
{
    uint8_t buf[8] = {0x7E, 0xFF, 0x06, 0x0D, 0x00, 0x00, 0x00, 0xEF};
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::irqPIR(void)
{
    pirFlag = true;
}

/************************************************************/
//
/************************************************************/
bool BoomboxBase::isPIREvent()
{
    return pirFlag;
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::clearPIRFlag()
{
    // add delay to debounce events
    delay(200);
    pirFlag = 0;
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::irqAux(void)
{
   auxFlag = true;
}

/************************************************************/
//
/************************************************************/
bool BoomboxBase::isAuxEvent()
{
    return auxFlag;
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::clearAuxFlag()
{
    // add delay to debounce events
    delay(100);
    auxFlag = false;
}

/*----------------------------------------------------------*/
// Power management
/*----------------------------------------------------------*/

/************************************************************/
//
/************************************************************/
void BoomboxBase::sleep()
{
    // shut down software serial
//    ss->end();

    // shut down everything else
    digitalWrite(pinMp3Enb, LOW);
    delay(500);
    digitalWrite(pinBoostEnb, LOW);
    delay(300);
    digitalWrite(pin5vEnb, LOW);
    wdt_disable();

#ifndef ARDUINO_ARCH_MEGAAVR   
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
#endif
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::wake()
{
#ifdef ARDUINO_ARCH_MEGAAVR   
    wdt_enable(WDT_PERIOD_8KCLK_gc); 
#else
    wdt_enable(WDTO_8S);
#endif
    
    // turn on 5V supply and allow voltage to stabilize
    digitalWrite(pin5vEnb, HIGH);
    delay(300);
    digitalWrite(pinBoostEnb, HIGH);    
    delay(1000);

    digitalWrite(pinMp3Enb, HIGH);
//    ss->begin(9600);    

    // need a delay here to start up the mp3 player
    delay(100);
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::ampEnable()
{
    digitalWrite(pinAmpShutdn, HIGH);    
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::ampDisable()
{
    digitalWrite(pinAmpShutdn, LOW);    
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::reg5vEnable()
{
    digitalWrite(pin5vEnb, HIGH);    
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::reg5vDisable()
{
    digitalWrite(pin5vEnb, LOW);    
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::muteEnable()
{
    digitalWrite(pinMute, LOW);
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::muteDisable()
{
    digitalWrite(pinMute, HIGH);
}

/************************************************************/
// Helper function to initialize the playlist
/************************************************************/
// set max sounds
void BoomboxBase::setMaxSounds(uint8_t maxSounds)
{
    if (maxSounds > MAX_FILES)
    {
        Serial.println(F("ERROR: maxSounds exceeds max allowable sounds. "));
        Serial.println(F("Max Sounds set to 0."));
        _maxSounds = 0;
    }
    else
    {
        _maxSounds = maxSounds;
    }
}

// set shuffle enable
void BoomboxBase::setShuffle(bool enb)
{
    _shuffleEnable = enb;
}

// set index
void BoomboxBase::setIndex(uint8_t index)
{
    _index = index;
}

// get index 
uint8_t BoomboxBase::getIndex()
{
    return _index;
}

// set active playlist
void BoomboxBase::setActivePlaylist(uint8_t *playlist)
{
    _playlist = playlist;
}

// set active playlist
uint8_t *BoomboxBase::getActivePlaylist()
{
    return _playlist;
}

// initialize playlist
void BoomboxBase::initPlaylist(uint8_t *playlist, uint8_t maxSounds, bool shuffleEnb)
{        
    if (!playlist)
    {
        Serial.println(F("ERROR: playlist init failed"));
        Serial.flush();
        return;
    }

    if (maxSounds == 0)
    {
        Serial.println(F("ERROR: maxSounds is equal to 0"));
        return;
    }    

    // create sequential playlist with indices that start from 1
    for (uint8_t i = 0; i<maxSounds; i++)
    {
        playlist[i] = i+1;
        //printf("%d: %d\n", i, playlist[i]);
    }

    if (shuffleEnb)
    {
        shufflePlaylist(playlist, maxSounds);      
    }
}

/************************************************************/
// Random number seed generator for shuffle 
/************************************************************/
void BoomboxBase::shuffleSeed()
{
    uint16_t val = 0;

    for (uint8_t i=0; i<3; i++)
    {
        val += (analogRead(pinRandSeed) & 0x7) << (3*i);
        delay(10);
    }
    randomSeed(val);
}

/************************************************************/
// Helper function to create a randomized, non-repeeating playlist
// this should be called once all sounds in playlist have been
// exhausted
/************************************************************/
void BoomboxBase::shufflePlaylist(uint8_t *playlist, uint8_t maxSounds)
{
    uint16_t i, nvalues = maxSounds;

    if (!playlist)
    {
        Serial.println(F("ERROR: playlist is uninitialized"));
        Serial.flush();
        return;
    }

    // create sequential playlist with indices that start from 1
    for (i = 0; i<nvalues; i++)
    {
        playlist[i] = i+1;
    }

    //dumpPlaylist(playlist, maxSounds);

    // shuffle playlist
    for(i = 0; i < nvalues-1; i++) 
    {
        uint16_t c = random(nvalues-i);

        /* swap */
        uint16_t t = playlist[i]; 
        playlist[i] = playlist[i+c]; 
        playlist[i+c] = t;    
    }

    //dumpPlaylist(playlist, maxSounds);
}

/************************************************************/
// Helper function to create a randomized, non-repeeating playlist
// this should be called once all sounds in playlist have been
// exhausted
/************************************************************/
void BoomboxBase::dumpPlaylist(uint8_t *playlist, uint8_t maxSounds)
{
    for (int i=0; i<maxSounds; i++)
    {
        Serial.print(playlist[i]);
        Serial.print(", ");
    }
    Serial.println();
}

/************************************************************/
// 
/************************************************************/
uint8_t BoomboxBase::getNextSound()
{
    uint8_t retVal;

    // if we've reached the end of the playlist, 
    // shuffle playlist and then restart               
    if (_index >= _maxSounds)
    {
        _index = 0;

        if (_shuffleEnable)
        {
            shufflePlaylist(_playlist, _maxSounds);
        }
    }

    retVal = _playlist[_index];

    // print out index and value   
    Serial.print(F("Index: ")); Serial.print(_index);
    Serial.print(F(", Val: ")); Serial.println(_playlist[_index]);

    _index++; 
    
    return retVal;
}

/*----------------------------------------------------------*/
// Helper functions
/*----------------------------------------------------------*/

/************************************************************/
// sendCmd
/************************************************************/
void BoomboxBase::_sendCmd(uint8_t *buf, uint8_t len)
{
    cli();
    for (int i=0; i<len; i++)
    {
        ss->write( buf[i] );
    }
    sei();
}
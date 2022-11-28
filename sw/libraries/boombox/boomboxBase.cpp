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
     pinPIR         = 2;
     pinButton      = 3;
     pinAmpShutdn   = 5;
     pinBusy        = 11;
     pinBoostEnb    = 4;
     pinMp3Enb      = 6;
     pinCurrEnb     = 7;
     pinRangeEnb    = 16;
     pinPIREnb      = 17;
     pin5vEnb       = 15;
     _vol           = 26;
     _delayVal      = 0;

    intPIR          = 0;
    intAux          = 1;     

    pinMode(pinBusy, INPUT);
    pinMode(pinButton, INPUT);

    pinMode(pinAmpShutdn, OUTPUT);
    pinMode(pinBoostEnb, OUTPUT);
    pinMode(pinMp3Enb, OUTPUT);
    pinMode(pin5vEnb, OUTPUT);

    digitalWrite(pinBoostEnb, HIGH);
    digitalWrite(pinAmpShutdn, HIGH);
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pin5vEnb, LOW);

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
    Serial.println("-------------------------------------------");
    Serial.print("Boombox ");
    Serial.println(BOARD_VERSION);
    Serial.println("Designed by FreakLabs and Meredith Palmer");
    Serial.print("Last modified: ");
    Serial.println(__DATE__);
    Serial.println("-------------------------------------------");
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
    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x03, 0X00, 0x00, file , 0xEF };
    _sendCmd(buf, sizeof(buf));
    while (isBusy())
    {

    }
    delay(250);
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
void BoomboxBase::playNext()
{
    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0xEF };
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::playPrev()
{
    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0xEF };
    _sendCmd(buf, sizeof(buf));
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
    // shut down everything else

    digitalWrite(pinMp3Enb, LOW);
    digitalWrite(pinBoostEnb, LOW);
    digitalWrite(pin5vEnb, LOW);

    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
}

/************************************************************/
//
/************************************************************/
void BoomboxBase::wake()
{
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pinBoostEnb, HIGH);
    digitalWrite(pinMp3Enb, HIGH);

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